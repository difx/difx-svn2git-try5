/***************************************************************************
 *   Copyright (C) 2006 by Adam Deller                                     *
 *                                                                         *
 *   This program is free for non-commercial use: see the license file     *
 *   at http://astronomy.swin.edu.au:~adeller/software/difx/ for more      *
 *   details.                                                              *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <cpgplot.h>
#include <string.h>
#include <sstream>
#include "architecture.h"
#include "configuration.h"
#include "monserver.h"
#include <iomanip>

using namespace std;

//prototypes
vector<DIFX_ProdConfig> change_config(Configuration *config, int configindex, int refant, struct monclient client);
int calcdelay(cf32 *vis, int nchan, IppsFFTSpec_R_32f* fftspec, double *delay, 
	      float *snr);

int main(int argc, const char * argv[]) {
  int i, status, nchan, atseconds, count, prod;
  int startsec, nsamples, currentconfig, refant;
  float snr;
  cf32 *vis=NULL;
  double delay;
  char polpair[3];
  struct monclient monserver;
  ostringstream ss;

  Configuration * config;
  IppsFFTSpec_R_32f* fftspec=NULL;
  vector<struct monclient> visibilities;
  vector<struct monclient>::iterator it;
  vector<DIFX_ProdConfig> prodconfig;

  refant = 0;

  if(argc < 3 || argc > 4)
  {
    cerr << "Error - invoke with difx_monitor <inputfile> <host> [# samples]" << endl;
    return EXIT_FAILURE;
  }

  //work out the config stuff
  config = new Configuration(argv[1], 0);
  if (!config->consistencyOK()) {
    cerr << "Aborting" << endl;
    exit(1);
  }
  status = config->loaduvwinfo(true);
  if (!status) {
    cerr << "Failed to load uvw - aborting\n";
    exit(1);
  }
  startsec = config->getStartSeconds();

  if(argc == 4)
    nsamples = atoi(argv[3]);

  // Connect to monitor server
  status  = monserver_connect(&monserver, (char*)argv[2],  Configuration::MONITOR_TCP_WINDOWBYTES);
  if (status) exit(1);

  currentconfig = 0;
  prodconfig = change_config(config, currentconfig, refant, monserver);
  if (prodconfig.size()==0) exit(1);

  atseconds = 0;

  count = 0;
  while (count<nsamples) {

    status = monserver_readvis(&monserver);
    if (status) break;
    cout << "Got visibility # " << monserver.timestamp << endl;

    while (!monserver_nextvis(&monserver, &prod, &vis)) {
      cout << " " << prod;
    }
    cout << endl;

    if (monserver.timestamp==-1) continue;

    atseconds = monserver.timestamp-startsec;
    if(config->getConfigIndex(atseconds) != currentconfig) {
      currentconfig = config->getConfigIndex(atseconds);
      nsamples = 0;
      prodconfig = change_config(config, currentconfig, refant, monserver);
      for (i=0; i<count; i++) {
	monserver_clear(&visibilities[i]);
      }
      count = 0;
      visibilities.resize(0);
      continue;
    }
    count++;
    visibilities.resize(count);
    monserver_dupclient(monserver,&visibilities[count-1]);
  }
  monserver_close(&monserver);


  // (Re)allocate arrays if number ofc hannels changes, including first time
  nchan = monserver.numchannels;

  //lagx = vectorAlloc_f32(nchan*2);

  int order = 0;
  while(((nchan*2) >> order) > 1) order++;

  ippsFFTInitAlloc_R_32f(&fftspec, order, IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);


  cout << "nvis  " << visibilities.size() << endl;

  for (i=0; i<(int)visibilities.size(); i++) {
    while (!monserver_nextvis(&visibilities[i], &prod, &vis)) {

      delay = calcdelay(vis, nchan, fftspec, &delay, &snr);

      cout << setw(3) << prod << ": ";

      ss << prodconfig[prod].getTelescopeName1() << "-" 
	 << prodconfig[prod].getTelescopeName2();
      
      cout << left << setw(15) << ss.str() << right;
      ss.str("");

      prodconfig[prod].getPolPair(polpair);
      cout << " " <<  polpair << " ";

      cout << prodconfig[prod].getFreq() << " MHz  ";

      delay *= 1/(2*prodconfig[prod].getBandwidth()); // Really should use sampling rate
      if (prodconfig[prod].getTelescopeIndex2()==refant) delay *= -1;


      cout <<  delay << endl;
	    

    }
    cout << "*****************" << endl;
  }
}


vector<DIFX_ProdConfig> change_config(Configuration *config, int configindex, int refant, struct monclient client) {
  int status;
  unsigned int i;
  vector<uint32_t> iproducts;
  vector<DIFX_ProdConfig> allproducts;

  allproducts = monserver_productconfig(config, configindex);

  for (i=0; i<allproducts.size(); i++) {
    if (allproducts[i].getTelescopeIndex1()==refant || allproducts[i].getTelescopeIndex2()==refant) {
      iproducts.push_back(i);
    }
  }

  if (iproducts.size()==0) {
    cout << "No baselines selected" << endl;
    allproducts.clear();
    return allproducts;
  }

  status = monserver_requestproducts(client, &iproducts[0], iproducts.size());
  // What if status bad?

  return allproducts;
}

int calcdelay(cf32 *vis, int nchan, IppsFFTSpec_R_32f* fftspec, double *delay, 
	      float *snr) {
  static Ipp32f *lags=NULL;
  Ipp32f max;
  int imax;
  
  if (lags==NULL) lags = vectorAlloc_f32(nchan*2);

  ippsFFTInv_CCSToR_32f((Ipp32f*)vis, lags, fftspec, 0); 
  ippsAbs_32f_I(lags, nchan*2);

  ippsMaxIndx_32f(lags, nchan*2, &max, &imax);

  if (imax>nchan) imax -= 2*nchan;
  
  *delay = imax;
  *snr = 5;

  return(imax);
}
