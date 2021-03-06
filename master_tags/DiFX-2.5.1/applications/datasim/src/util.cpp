/*****************************************************************************
*    <DataSim: VLBI data simulator>                                          * 
*    Copyright (C) <2015> <Zheng Meyer-Zhao>                                 *
*                                                                            *
*    This file is part of DataSim.                                           *
                                                                             *
*    DataSim is free software: you can redistribute it and/or modify         *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    DataSim is distributed in the hope that it will be useful,              *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
*****************************************************************************/

#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <gsl/gsl_randist.h>
#include "vdifio.h"
#include "util.h"
#include "datasim.h"
#include "configuration.h"

using namespace std;

     
int initSubbands(Configuration* config, int configindex, float specRes, 
                  float minStartFreq, vector<Subband*> &subbands, Model* model,
                  float tdur, setup setupinfo)
{
  size_t length, startIdx, blksize;
  size_t vpsamps;   // number of samples in a vdif packet
  size_t vpbytes;   // number of bytes in a single-thread vdif packet
  size_t framebytes, numrecordedbands;
  size_t numdatastreams = (size_t)config->getNumDataStreams();
  float freq, bw;
  string antname;
  int mjd, seconds;
  f64* tempcoeffs;
  f64* delaycoeffs;
  double vptime;
  size_t framespersec;
  
  mjd = config->getStartMJD();
  seconds = config->getStartSeconds();
  if(setupinfo.verbose >= 1)
  {
    cout << "MJD is " << mjd << ", start seconds is " << seconds << endl;
  }

  // allocate memory for delaycoeffs
  tempcoeffs = vectorAlloc_f64(2);
  delaycoeffs = vectorAlloc_f64(2);

  for(size_t i = 0; i < numdatastreams; i++)
  {
    framebytes = (size_t)config->getFrameBytes(configindex, i);
    numrecordedbands = (size_t)config->getDNumRecordedBands(configindex, i);
    framespersec = config->getFramesPerSecond(configindex, i);
    vptime = 1.0 * 1e6 / framespersec;
    antname = config->getTelescopeName(i);
    // change the last character of the output vdif name to lower case for fourfit postprocessing
    //antname.back() = tolower(antname.back());
    antname.at(antname.size()-1) = tolower(antname.at(antname.size()-1));
    if(setupinfo.verbose >= 1)
    { 
      cout << "Antenna " << i << endl;
      cout << " framebytes is " << framebytes << endl;
      cout << " numrecordedbands is " << numrecordedbands << endl;
      cout << " antenna name is " << antname << endl;
    }

    // only consider scan 0 sourc 0
    // scanindex, offsettime in seconds, timespan in seconds, numincrements, antennaindex, scansourceindex, order, delaycoeffs
    model->calculateDelayInterpolator(0, 0, vptime*1e-6, 1, i, 0, 1, tempcoeffs);
    model->calculateDelayInterpolator(0, tempcoeffs[1]*1e-6, vptime*1e-6, 1, i, 0, 1, delaycoeffs);
    if(setupinfo.verbose >= 2)
    {
      cout << "delay in us for datastream " << i << " at offsettime 0s is " << tempcoeffs[1] << endl;
      cout << "delay in us for datastream " << i << " at offsettime " << tempcoeffs[1]*1e-6 << "s " << " is " << delaycoeffs[1] << endl;
    }

    // calculate vdif packet size in terms of bytes and number of samples
    // each sample uses 4 bits, as the sample is complex and we use 2 bits sampling
    // therefore 2 samples per byte
    vpbytes = (framebytes - VDIF_HEADER_BYTES) / numrecordedbands + VDIF_HEADER_BYTES;
    vpsamps = (framebytes - VDIF_HEADER_BYTES) / numrecordedbands * 2;
  
    for(size_t j = 0; j < numrecordedbands; j++)
    {
      freq = config->getDRecordedFreq(configindex, i, j);
      bw = config->getDRecordedBandwidth(configindex, i, j);
      if(!is_integer((freq - minStartFreq) / specRes))
      {
        cout << "StartIndex position is not an integer ... " << endl
             << "Something is wrong here ... " << endl;
        return EXIT_FAILURE;
      }
      else
        startIdx = (freq - minStartFreq) / specRes;
      
      blksize = bw / specRes; // number of samples to copy from startIdx
      length = bw * tdur * 2; // size of the array twice of tdur

      if(setupinfo.verbose >= 1)
      {
        cout << "Ant " << i << " subband " << j << ":" << endl
             << "  start index is " << startIdx << endl 
             << "  block size is " << blksize << " length is " << length << endl
             << "  each vdif packet has " << vpsamps << " samples" << endl
             << "  VDIF_HEADER_BYTES is " << VDIF_HEADER_BYTES << " vpbytes is " << vpbytes << endl
             << "  number of samples in vdif packet is " << vpsamps << " framebytes is " << framebytes << endl
             << "  recorded freq is " << freq << endl;
      }
      subbands.push_back(new Subband(startIdx, blksize, length, i, setupinfo.antSEFDs[i], j, vpbytes, vpsamps, delaycoeffs, bw, antname, mjd, seconds, freq, setupinfo.verbose));
    }
  } 

  vectorFree(tempcoeffs);
  vectorFree(delaycoeffs);
  return EXIT_SUCCESS;
}

void freeSubbands(vector<Subband*> &subbands)
{
  vector<Subband*>::iterator it;
  for(it = subbands.begin(); it != subbands.end(); it++)
  {
    (*it)->closevdif();
  }
  // free the memory allocated
  for(size_t i = 0; i < subbands.size(); i++)
  {
    delete subbands[i];
  }
  subbands.clear();
}

/*
 * Check whether the fractional part of a floating point number is 0
 */
bool is_integer(float num)
{
  return floor(num)==num;
}

/*
 * Calculate the maximum spectrum resolution that can be used by all antennas
 */
int getSpecRes(Configuration* config, int configindex, float& specRes, size_t verbose)
{
  size_t cnt = 0;
  float tempSpecRes;
  bool isGCD = true; // Greatest Common Divisor
  vector<float> startfreq, freqdiff, bandwidth;
  vector<float>::iterator it, ity;
  for(int i = 0; i < config->getNumDataStreams(); i++)
  {
    startfreq.push_back(config->getDRecordedFreq(configindex, i, 0));
    bandwidth.push_back(config->getDRecordedBandwidth(configindex, i, 0));
  }
  for(it = startfreq.begin(); it != startfreq.end(); it++)
  {
    for(ity = it + 1; ity != startfreq.end(); ity++)
    {
      freqdiff.push_back(fabs(*it - *ity));
      if(verbose >= 2) cout << "Freqdiff is " << freqdiff.back() << endl;
    }
  }

  // loop through all frequency difference and bandwidth
  // we want to find the greatest common divisor
  // that all results after division are integers
  do
  {
    cnt++; // the highest chosen specRes is 0.5MHz, therefore, cnt starts from 1
    tempSpecRes = 1.0 / pow(2, cnt);
    for(it = freqdiff.begin(); it != freqdiff.end(); it++)
      isGCD &= is_integer(*it / tempSpecRes);
    for(it = bandwidth.begin(); it != bandwidth.end(); it++)
      isGCD &= is_integer(*it / tempSpecRes);

  // if one or more results are not integers
  // divide the value of tempSpecRes by 2
  // if GCD cannot be found at 1/(2^10), stop the calculation 
  }while(!isGCD && (cnt < 10));

  if(cnt == 10)
  {
    cout << "Failed to calculate spectral resoluation ..." << endl;
    return EXIT_FAILURE;
  }
  else
  {
    specRes = tempSpecRes;
    return EXIT_SUCCESS;
  }
}

/*
 * Calculate the maximum band covereage (bandwidth * nChans) among all antennas
 */
float getMaxChanFreq(Configuration* config, int configindex, size_t verbose)
{
  float chanFreq, maxChanFreq = 0;
  for(int i = 0; i < config->getNumDataStreams(); i++)
  {
    chanFreq = config->getDRecordedBandwidth(configindex, i, 0) * config->getDNumRecordedBands(configindex, i);
    maxChanFreq = (chanFreq > maxChanFreq) ? chanFreq : maxChanFreq;
  }
  if(verbose >= 1)
  {
    cout << "Max chan Freq is " << maxChanFreq << " MHz" << endl;
  }
  return maxChanFreq;
}

/*
 * Calculate the number of samples to be generated per time block for the common signal
 */
int getNumSamps(Configuration* config, int configindex, float specRes, size_t verbose)
{
  float maxChanFreq = getMaxChanFreq(config, configindex, verbose);
  assert(is_integer(maxChanFreq / specRes));
  return (int)maxChanFreq / specRes;
}

/*
 * Get the lowest start frequency among all antennas
 */
float getMinStartFreq(Configuration* config, int configindex, size_t verbose)
{
  float startFreq;
  float minStartFreq = config->getDRecordedFreq(configindex, 0, 0);
  for(int i = 0; i < config->getNumDataStreams(); i++)
  {
    startFreq = config->getDRecordedFreq(configindex, i, 0);
    minStartFreq = (startFreq < minStartFreq) ? startFreq : minStartFreq;
  }
  if(verbose >= 1)
  {
    cout << "Min start Freq is " << minStartFreq << " MHz" << endl;
  }
  return minStartFreq;
}

/*
 * Generate complex numbers using GSL
 */
void gencplx(cf32* cpDst, size_t len, f32 stdev, gsl_rng *rng_inst, size_t verbose)
{
  int status;
  f32* real = vectorAlloc_f32(len);
  f32* imag = vectorAlloc_f32(len);

  for(size_t idx = 0; idx < len; idx++)
  {
    real[idx] = gsl_ran_gaussian_ziggurat(rng_inst, stdev);
    imag[idx] = gsl_ran_gaussian_ziggurat(rng_inst, stdev);
  }
  
  status = vectorRealToComplex_f32(real, imag, cpDst, len);
  if(status != vecNoErr)
    cerr << "Error assembling random signals to complex!!!" << status << endl;

  if(verbose >= 3)
  {
    static size_t cnt = 1;
    if(cnt == 1)
    {
      for(size_t i = 0; i < len; i++)
        cout << "Real part value: " << real[i] << ", imaginary part value: " << imag[i] << endl;
      cnt--;
    }
  }
  for(size_t idx = 0; idx < len; idx++)
  {
    assert(cpDst[idx].re == real[idx]);
    assert(cpDst[idx].im == imag[idx]);
  }

  vectorFree(real);
  vectorFree(imag);
}

/*
 * Generate signal of time duration 'tdur' for all subband of all antennas
 */
void genSignal(size_t stdur, cf32* commFreqSig, vector<Subband*>& sbVec, int numSamps, gsl_rng *rng_inst, float tdur, float sfluxdensity, size_t verbose)
{
  vector<Subband*>::iterator it;
  if(verbose >= 1)
  {
    cout << "Generate " << tdur << " us signal" << endl;
  }
  for(size_t t = 0; t < stdur; t++)
  {
    gencplx(commFreqSig, numSamps, STDEV, rng_inst, verbose);
    // loop through each subband array of each antenna
    if(verbose >= 2) cout << "At time " << t << "us:" << endl;
    for(it = sbVec.begin(); it != sbVec.end(); ++it)
    {
      /* select data from commFreqSig 
       * add station noise
       * apply Ormsby filter
       * inverse DFT/FFT
       * copy data to the subband array
       */
      if(verbose >= 2)
      {
        cout << " Fabricate data for Antenna " << (*it)->getantIdx() << "subband " << (*it)->getsbIdx() << endl;
      }
      (*it)->fabricatedata(commFreqSig, rng_inst, sfluxdensity); 
    }
  }

  // after TDUR time signal is generated for each subband array
  // set the current pointer of each array back to the beginning of the second half
  for(it = sbVec.begin(); it != sbVec.end(); ++it)
  {
    (*it)->setcptr((*it)->getlength() / 2);
    if(verbose >= 2) 
    {
      cout << "Antenna " << (*it)->getantIdx() << " subband " << (*it)->getsbIdx()
                         << " set current pointer back to " << (*it)->getlength() / 2 << endl;
    }
  }
}

/*
 * loop through each subband of each antenna
 * move data from the second half of the array to the first half
 * set the process pointer to the proper location
 */
void movedata(vector<Subband*>& sbVec, size_t verbose)
{
  if(verbose >= 2) cout << "Move data in each subband array forward" << endl;
  vector<Subband*>::iterator it;
  for(it = sbVec.begin(); it != sbVec.end(); ++it)
  {
    (*it)->movedata();
  }
}

/*
 * loop through each subband of each antenna
 * select vdif packet size of data to process
 * process the data
 * quantization
 * pack to vdif
 */
int processAndPacketize(size_t framespersec, vector<Subband*>& sbVec, Model* model, size_t verbose)
{
  vector<Subband*>::iterator it;
  for(it = sbVec.begin(); it != sbVec.end(); ++it)
  {
    if(verbose >= 2)
      cout << "Antenna " << (*it)->getantIdx() << " Subband "
           << (*it)->getsbIdx() << " process vdif packet" << endl;
    (*it)->fillprocbuffer(); 
    (*it)->processdata(); 
    (*it)->updatevalues(model);
 
    (*it)->quantize();
    (*it)->writetovdif();
    if(verbose >= 2)
      cout << "current seconds is " << ((vdif_header *)(*it)->getvdifbuf())->seconds
           << ", frame number is " << ((vdif_header *)(*it)->getvdifbuf())->frame << endl;
    //update VDIF Header for the next packet
    nextVDIFHeader((vdif_header *) (*it)->getvdifbuf(), (int) framespersec);    
  }
  return (EXIT_SUCCESS);
}

/*
 * calculate the lowest process pointer in terms of time among all subband arrays
 */
double getMinProcPtrTime(vector<Subband*>& sbVec, size_t verbose)
{
  vector<Subband*>::iterator it = sbVec.begin();
  double minprocptrtime = (*it)->getprocptr() * (1.0 / (*it)->getbandwidth());
  double temp;
  for(it = sbVec.begin(); it != sbVec.end(); ++it)
  {
    temp = (*it)->getprocptr() * (1.0 / (*it)->getbandwidth());
    if(verbose >= 2)
      cout << "Process Pointer in time for Antenna " << (*it)->getantIdx() << " Subband "
           << (*it)->getsbIdx() << " is " << temp << " us" <<endl;
    minprocptrtime = (minprocptrtime > temp) ? temp : minprocptrtime;
  }
  return minprocptrtime;
}
