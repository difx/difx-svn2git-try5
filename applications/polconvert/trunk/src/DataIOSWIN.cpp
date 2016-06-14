/* Copyright (C) 2013  Nordic Node of EU ALMA Regional Center (Onsala, Sweden)
                       Max-Planck-Institut fuer Radioastronomie (Bonn, Germany)
  
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
  
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
  
You should have received a copy of the GNU General Public License   
along with this program.  If not, see <http://www.gnu.org/licenses/>
  
*/


#include <sys/types.h>
#include <iostream> 
#include <fstream>
#include <stdlib.h>  
#include <string.h>
#include <math.h>
#include <dirent.h>
#include "DataIOSWIN.h"





DataIOSWIN::~DataIOSWIN() {

};


DataIOSWIN::DataIOSWIN(int nfiledifx, std::string* difxfiles, int NlinAnt, int *LinAnt, double *Range, int nIF, int *nChan, double **FreqVal, bool Overwrite, bool doTest, double jd0, FILE *logF) {



nfiles = nfiledifx;
int i;


//char *message;

logFile = logF;


success = true;
NLinAnt = NlinAnt;
linAnts = new int[NlinAnt];

for (i=0;i<NlinAnt;i++){
  linAnts[i] = LinAnt[i];
};

Records = new Record[RECBUFFER];
is1orig = new bool[RECBUFFER];
is2orig = new bool[RECBUFFER];

is1 = new bool[RECBUFFER];
is2 = new bool[RECBUFFER];

day0 = jd0 ;

currFreq = 0;
currVis = 0;

doRange = Range;


// READ FREQUENCIES FOR ALL IFs:
Nfreqs = nIF;
int MaxNChan = 0;
Freqs = new FreqSetup[Nfreqs];
   for(i=0;i<nIF;i++){
     if (nChan[i]>MaxNChan){MaxNChan = nChan[i];};
     Freqvals[i] = new double[nChan[i]];
     Freqs[i].Nchan = nChan[i];
     Freqs[i].BW = FreqVal[i][nChan[i]-1] - FreqVal[i][0];
     Freqs[i].Nu0 = FreqVal[i][0];
     Freqs[i].SB = 0;
     memcpy(Freqvals[i], FreqVal[i],nChan[i]*sizeof(double)); 
   };


for (i=0; i<4; i++){
  currentVis[i] = new std::complex<float>[MaxNChan+1];
  bufferVis[i] = new std::complex<float>[MaxNChan+1];
}

  isOverWrite = Overwrite ;

  openOutFiles(difxfiles);
  readHeader(doTest);

};



void DataIOSWIN::finish(){

  int auxI;
  for (auxI=0; auxI<nfiles; auxI++) {
     newdifx[auxI].close();
     if (!isOverWrite){olddifx[auxI].close();};
  };

};






void DataIOSWIN::openOutFiles(std::string* difxfiles) {

  std::string SEP = "NEW/";
  int auxI;

//  char *message;

  olddifx = new std::ifstream[nfiles];
  newdifx = new std::ofstream[nfiles];

  long begin, end;
  filesizes = new long[nfiles];

  for (auxI=0; auxI<nfiles; auxI++) {

   olddifx[auxI].open((difxfiles[auxI]).c_str(), std::ios::in | std::ios::binary);

// Get file size:
   begin = olddifx[auxI].tellg();
   olddifx[auxI].seekg(0, olddifx[auxI].end);
   end = olddifx[auxI].tellg();
   filesizes[auxI] = end - begin;
   olddifx[auxI].clear();


   if (!isOverWrite) {
     newdifx[auxI].open((SEP+difxfiles[auxI]).c_str(), std::ios::out | std::ios::binary);
     newdifx[auxI] << olddifx[auxI].rdbuf();
     newdifx[auxI].close();
     newdifx[auxI].open((SEP+difxfiles[auxI]).c_str(), std::ios::out | std::ios::binary | std::ios::in);
     olddifx[auxI].clear();
   } else {
     newdifx[auxI].open((difxfiles[auxI]).c_str(), std::ios::out | std::ios::binary | std::ios::in);
   };

 };

};



// SET IF TO CHANGE:
bool DataIOSWIN::setCurrentIF(int i){

//  char *message;

  if (i>=Nfreqs){success=false; 
      sprintf(message,"\nERROR! IF %i CANNOT BE FOUND!\n",i+1); 
      fprintf(logFile,message); std::cout<<message; fflush(logFile);

      success=false; return success;
  };
  currFreq = i;
  currVis = 0;
  memcpy(is1, is1orig, nrec*sizeof(bool));
  memcpy(is2, is2orig, nrec*sizeof(bool));
  return success;
};





void DataIOSWIN::readHeader(bool doTest) {

//  char *message;
  long loc, beg, end, polpos;
  int basel, fridx, cfidx, mjd;
  double secs, daytemp;

  char *pol = new char[2];
  char *auxC = new char[4];

  delete[] Records;
  Records = new Record[RECBUFFER];

  delete[] is1orig;
  is1orig = new bool[RECBUFFER];

  delete[] is2orig;
  is2orig = new bool[RECBUFFER];

  delete[] is1;
  is1 = new bool[RECBUFFER];

  delete[] is2;
  is2 = new bool[RECBUFFER];

  nrec = 0;
  long CURRSIZE = RECBUFFER;
  int ant1, ant2, auxI, auxJ;

//Assume binary index (i.e., DiFX version >= 2.0):
  sprintf(message,"\nThere are %i IFs.",Nfreqs);
  fprintf(logFile,message); std::cout<<message; fflush(logFile);

  sprintf(message,"\n\n Searching for visibilities with mixed (or linear) polarization.\n\n");
  fprintf(logFile,message); std::cout<<message; fflush(logFile);

  long bperp, cp;

 for (auxI=0; auxI<nfiles; auxI++) {

  loc = 8;

// Bits per percentage:
  bperp = filesizes[auxI]/100; if(bperp==0){bperp=1;};

  sprintf(message,"\n\nReading file %i of %i (size %i MB)\n",auxI+1,nfiles,filesizes[auxI]/(1024*1024));
  fprintf(logFile,message); std::cout<<message; fflush(logFile);

  cp = 0;

  while(!olddifx[auxI].eof()) {

   if(loc/bperp > cp){cp += 1; printf("\r  %i%% DONE",cp);fflush(stdout);};

   olddifx[auxI].seekg(loc,olddifx[auxI].beg);
   olddifx[auxI].read(reinterpret_cast<char*>(&basel), sizeof(int));
   olddifx[auxI].read(reinterpret_cast<char*>(&mjd), sizeof(int));
   olddifx[auxI].read(reinterpret_cast<char*>(&secs), sizeof(double));
   olddifx[auxI].read(reinterpret_cast<char*>(&cfidx), sizeof(int));
   olddifx[auxI].read(auxC, sizeof(int));
   olddifx[auxI].read(reinterpret_cast<char*>(&fridx), sizeof(int));
   polpos = olddifx[auxI].tellg();
   olddifx[auxI].read(pol, 2*sizeof(char));

////////////////
// WHAT IS THE DIFFERENCE BETWEEN CFIDX AND FRIDX !!!!!!!!
////////////////

  beg = olddifx[auxI].tellg(); 
  if (beg>0) {
  beg += endhead;
  end = beg + (Freqs[fridx].Nchan)*sizeof(cplx32f);
  loc = end + sizeof(cplx32f);



// RE-ALLOCATE MEMORY IF BUFFER IS FULL:
  if (nrec == CURRSIZE) {
    CURRSIZE += RECBUFFER; 
    Records = (Record*) realloc(Records, CURRSIZE*sizeof(Record));
    is1orig = (bool*) realloc(is1orig, CURRSIZE*sizeof(bool));
    is2orig = (bool*) realloc(is2orig, CURRSIZE*sizeof(bool));
  };

// Check if we are in the time window:
    secs /= 86400.;
    daytemp = ((double) mjd) + secs - day0;

    if(daytemp>=doRange[0] && daytemp<=doRange[1]){


// Check if a linear-feed antenna is in the baseline:
  ant1 = basel / 256;
  ant2 = basel % 256;
  is1orig[nrec] = false ; is2orig[nrec] = false;
  for (auxJ=0;auxJ<NLinAnt;auxJ++){
    if(ant1 == linAnts[auxJ]){
       is1orig[nrec] = true;
       if (pol[0] == 'R' || pol[0]=='X'){pol[0] = 'X';} else {pol[0] = 'Y';};
    };
    if(ant2 == linAnts[auxJ]){
       is2orig[nrec] = true;
       if (pol[1] == 'R' || pol[1]=='X'){pol[1] = 'X';} else {pol[1] = 'Y';};
    };
  };


// Read entry metadata:
  if (is1orig[nrec] || is2orig[nrec]) {

     Records[nrec].Baseline = basel;
     Records[nrec].fileNumber = auxI;
     Records[nrec].Time = (daytemp + day0)*86400.;  //  /86400.;
     Records[nrec].notUsed = true;
     Records[nrec].Antennas[0] = ant1;
     Records[nrec].Antennas[1] = ant2;
     Records[nrec].byteIni = beg;
     Records[nrec].byteEnd = end;
     Records[nrec].Pol[0] = pol[0];
     Records[nrec].Pol[1] = pol[1];

/////////
// Overwrite pol label entry in SWIN file:
     if(!doTest){
       if(pol[0]=='X'){pol[0]='R';} else if(pol[0]=='Y'){pol[0]='L';};
       if(pol[1]=='X'){pol[1]='R';} else if(pol[1]=='Y'){pol[1]='L';};
       newdifx[auxI].seekp(polpos, newdifx[auxI].beg);
       newdifx[auxI].write(reinterpret_cast<char*>(pol),2*sizeof(char));
     };
/////////

     Records[nrec].freqIndex = fridx;
     nrec ++;
    };
  };

  };

  };



// Rewind:
  olddifx[auxI].clear();
  olddifx[auxI].seekg(0,olddifx[auxI].beg);



 };


// day0 is JD (not MJD):
day0 += 2400000.5 ;

if (nrec==0) {sprintf(message,"\n NO VALID DATA FOUND!"); 
  fprintf(logFile,message); std::cout<<message; fflush(logFile);

success = false;};

// Allocate memory for booleans:
NLinVis = nrec/4;
is1 = new bool[nrec];
is2 = new bool[nrec];

};






bool DataIOSWIN::getNextMixedVis(double &JDTime, int &antenna, int &otherAnt, bool &conj) {


//char *message;
long rec, rec1, k;
int basel, idx, fnum;
double time;
long indices[4];
bool complete;


  if (NLinVis==0){return false;};

///////////////////
// BEWARE WHETHER currFreq IS ZERO-BASED!!!!!
///////////////////


// Find the four correlation products:

while(true){

idx = 0;
for (rec=0; rec<nrec; rec++) {
  if (Records[rec].notUsed && Records[rec].freqIndex==currFreq) {
     indices[idx] = rec;
     complete = !(is1[rec] && is2[rec]) ; 
     if (complete){
       Records[rec].notUsed = false;};
     idx ++;
     basel = Records[rec].Baseline;
     time = Records[rec].Time;
     currVis = rec;
     for (rec1=rec+1; rec1<nrec; rec1++) {
       if (Records[rec1].Baseline==basel && Records[rec1].Time==time && Records[rec1].freqIndex==currFreq) {
          indices[idx] = rec1; idx ++;
          if (complete){
            Records[rec1].notUsed = false;};
          if (idx==4) {break;};
       };
     }; break;
  };
};


// If no more mixed-pol visibilities are found, we return false:
// If not all the 4 products are found, report a warning:
if (idx==0) {return false;}
else if (idx <4) {
//sprintf(message,"\nWARNING: Missing correlation products!\n");
//fprintf(logFile,message); fflush(logFile);// std::cout<<message;
//flush(logFile);

 sprintf(message,"WARNING: Missing: Baseline: %08x - Time: %f sec: ",
    basel,time - Records[0].Time); 
 fprintf(logFile,message); fflush(logFile); // std::cout<<message;

// sprintf(message, "Products found: ");
// fprintf(logFile,message); fflush(logFile);// std::cout<<message;
// fflush(logFile);

 for (rec=0; rec<idx; rec++) {
   sprintf(message,"%c%c ",Records[indices[rec]].Pol[0],Records[indices[rec]].Pol[1]);
   fprintf(logFile,message); fflush(logFile);// std::cout<<message;
   fflush(logFile);
//  Records[indices[rec]].notUsed = false;
 };
 fprintf(logFile,"\n");
 fflush(logFile);

 if(idx==2){indices[2] = -1; indices[3] = -1; break;};

// std::cout<<"\n";
// sprintf(message,"\nTrying next visibility...\n");
// fprintf(logFile,message); fflush(logFile);// std::cout<<message;


} else {
 break;
};




};




// Classify the correlation products:
if (is1[indices[0]]){is1[indices[0]]=false; conj = true;} else {is2[indices[0]]=false; conj=false;};

int i = conj?0:1;
antenna = conj?Records[indices[0]].Antennas[0]:Records[indices[0]].Antennas[1];
otherAnt = conj?Records[indices[0]].Antennas[1]:Records[indices[0]].Antennas[0];


char p1, p2;
currEntries[currFreq][0] = -1;
currEntries[currFreq][1] = -1;
currEntries[currFreq][2] = -1;
currEntries[currFreq][3] = -1;

for (rec = 0; rec < 4; rec++) {

 if (indices[rec]>0){

  p1 = Records[indices[rec]].Pol[i];
  p2 = Records[indices[rec]].Pol[1-i];

// All pol. products must have consistent booleans (for sanity)
  is1[indices[rec]] = is1[indices[0]];
  is2[indices[rec]] = is2[indices[0]];


  if (p1=='X' && (p2=='R' || p2=='X')) {
     currEntries[currFreq][0] = indices[rec];
  }
  else if (p1=='Y' && (p2=='R' || p2=='X')) {
     currEntries[currFreq][3] = indices[rec];
  }
  else if (p1=='X' && (p2=='L' || p2=='Y')) {
     currEntries[currFreq][2] = indices[rec];
  }
  else if (p1=='Y' && (p2=='L' || p2=='Y')) {
     currEntries[currFreq][1] = indices[rec];
  };

 };

};

// Get the data and return them:
for (i=0; i<4; i++) {
  if (currEntries[currFreq][i]>0){
  rec = currEntries[currFreq][i];
  fnum = Records[rec].fileNumber;
  olddifx[fnum].seekg(Records[rec].byteIni, olddifx[fnum].beg);
  olddifx[fnum].sync();
  olddifx[fnum].read(reinterpret_cast<char*>(currentVis[i]),Records[rec].byteEnd-Records[rec].byteIni);
  } else {

    for (k=0; k<Freqs[currFreq].Nchan; k++) {
      currentVis[i][k] = 0.0;
    };
  };
};


JDTime = time;
currConj = conj ;
return true;



};










bool DataIOSWIN::setCurrentMixedVis() { 


//char *message;
long rec;
int i, fnum;

// Write:

for (i=0; i<4; i++) {
 if (currEntries[currFreq][i]>0){
  rec = currEntries[currFreq][i];
  fnum = Records[rec].fileNumber;
  newdifx[fnum].seekp(Records[rec].byteIni, newdifx[fnum].beg);
  newdifx[fnum].write(reinterpret_cast<char*>(bufferVis[i]),Records[rec].byteEnd-Records[rec].byteIni);
  newdifx[fnum].clear();
};
};

  newdifx[fnum].flush();
  olddifx[fnum].sync();

  return true;

};







void DataIOSWIN::applyMatrix(std::complex<float> *M[2][2], bool swap, bool print, FILE *plotFile) {
 
  long k, a11, a12, a21, a22, ca11, ca12, ca21, ca22 ;
  std::complex<float>  auxVis;

   if (swap){

    if (currConj) {
        a21 = 0;
        a12 = 1;
        a22 = 2;
        a11 = 3;
    } else {
        a12 = 0;
        a21 = 1;
        a11 = 2;
        a22 = 3;
    };

   } else {

       a11 = 0;
       a22 = 1;
       a12 = 2;
       a21 = 3;

   };


     ca11 = 0;
     ca22 = 1;
     ca12 = 2;
     ca21 = 3;



     for (k=0; k<Freqs[currFreq].Nchan; k++) {

       if (currConj) {

         bufferVis[ca11][k] = M[0][0][k]*currentVis[a11][k]+M[0][1][k]*currentVis[a21][k];
         bufferVis[ca12][k] = M[0][0][k]*currentVis[a12][k]+M[0][1][k]*currentVis[a22][k];
         bufferVis[ca21][k] = M[1][0][k]*currentVis[a11][k]+M[1][1][k]*currentVis[a21][k];
         bufferVis[ca22][k] = M[1][0][k]*currentVis[a12][k]+M[1][1][k]*currentVis[a22][k];

       } else {

         bufferVis[ca11][k] = std::conj(M[0][0][k])*currentVis[a11][k]+std::conj(M[1][0][k])*currentVis[a12][k];
         bufferVis[ca12][k] = std::conj(M[0][1][k])*currentVis[a11][k]+std::conj(M[1][1][k])*currentVis[a12][k];
         bufferVis[ca21][k] = std::conj(M[0][0][k])*currentVis[a21][k]+std::conj(M[1][0][k])*currentVis[a22][k];
         bufferVis[ca22][k] = std::conj(M[0][1][k])*currentVis[a21][k]+std::conj(M[1][1][k])*currentVis[a22][k];

       };



   if (print) {
 //    std::cout << currConj << "\n";
     if (currConj){
     fwrite(&Records[currVis].Time,sizeof(double),1,plotFile);
     fwrite(&currentVis[a11][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&currentVis[a12][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&currentVis[a21][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&currentVis[a22][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&bufferVis[ca11][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&bufferVis[ca12][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&bufferVis[ca21][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&bufferVis[ca22][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&M[0][0][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&M[0][1][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&M[1][0][k],sizeof(std::complex<float>),1,plotFile);
     fwrite(&M[1][1][k],sizeof(std::complex<float>),1,plotFile);
     } else {
     fwrite(&Records[currVis].Time,sizeof(double),1,plotFile);
     auxVis = std::conj(currentVis[a11][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(currentVis[a21][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(currentVis[a12][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(currentVis[a22][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(bufferVis[ca11][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(bufferVis[ca21][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(bufferVis[ca12][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(bufferVis[ca22][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(M[0][0][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(M[1][0][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(M[0][1][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     auxVis = std::conj(M[1][1][k]);
     fwrite(&auxVis,sizeof(std::complex<float>),1,plotFile);
     };

   };
 };

};




