alias PREPEND 'setenv \!^ {\!:2}:{$\!^}'

####### DIFX VERSION ########################
setenv DIFX_VERSION trunk

####### ROOT PATHS ##########################
setenv DIFXROOT /usr/local/difx
setenv DIFX_PREFIX $DIFXROOT
setenv PGPLOTDIR 
setenv IPPROOT /opt/intel/ipp/5.2/ia32

####### MPI SOFTWARE AND COMPILER ###########
setenv DIFXMPIDIR=/usr
setenv MPICXX $DIFXMPIDIR/bin/mpicxx

####### LIBRARY PATHS #######################
####### Uncomment and modify if needed, #####
####### such as 64-bit OpenSuSE #############
# setenv IPP_LIBRARY_PATH $IPPROOT/ipp/lib/intel64:$IPPROOT/compiler/lib/intel64
# setenv MPI_LIBRARY_PATH $DIFXMPIDIR/lib64

####### USE GFORTRAN IN PREFERENCE TO G77? ##
####### Comment out if not desired ##########
setenv USEGFORTRAN "yes"

setenv SPICE_ROOT /cluster/spice/cspice
####### PERL VERSION/SUBVERSION #############
set perlver="5"
set perlsver="5.10.1"
PREPEND PERL5LIB         ${DIFXROOT}/share/perl/$perlver

####### PORTS FOR DIFXMESSAGE ###############
# Uncomment these to enable DIFX_MESSAGES
setenv DIFX_MESSAGE_GROUP 224.2.2.1
setenv DIFX_MESSAGE_PORT 50201
setenv DIFX_BINARY_GROUP 224.2.2.1
setenv DIFX_BINARY_PORT 50202

####### CALC SERVER NAME ######### 
setenv CALC_SERVER localhost
setenv DIFX_DELAY_SERVER localhost
if ( -f "${DIFXROOT}/bin/setupCALCservers" ) then
  source "${DIFXROOT}/bin/setupCALCservers"
endif
if ( $OSTYPE == "linux" || $OSTYPE == "linux-gnu") then
  setenv DIFX_NUMBERCALCSERVERTHREADS `grep -c ^processor /proc/cpuinfo`
  setenv DIFX_CALCIF2_NUM_CALC_THREADS ${DIFX_NUMBERCALCSERVERTHREADS}
else
  setenv DIFX_NUMBERCALCSERVERTHREADS 4
  setenv DIFX_CALCIF2_NUM_CALC_THREADS 4
endif

####### MPI RUNTIME OPTIONS #################
####### Uncomment and modify if needed, #####
####### such as Open MPI 1.8.4 ##############
# setenv DIFX_MPIRUNOPTIONS "--mca mpi_yield_when_idle 1 --mca rmaps seq"

####### No User configurable values below here

####### Operating System, use $OSTYPE

if ( $OSTYPE == "darwin" || $OSTYPE == "linux" || $OSTYPE == "linux-gnu") then
  set OS=$OSTYPE
else if ( $OSTYPE == "darwin9.0" ) then
  set OS="darwin"
else
  echo "Warning supported O/S $OSTYPE";
  exit 1
endif
setenv DIFXOS $OS

####### 32/64 BIT DEPENDENT MODIFICATIONS ###
set arch=`uname -m`
if ( $arch == "i386" || $arch == "i686" ) then #32 bit
  setenv DIFXBITS 32
else if ( $arch == "x86_64" ) then #64 bit
  setenv DIFXBITS 64
else
  echo "Unknown architecture $arch - leaving paths unaltered"
endif

####### LIBRARY/EXECUTABLE PATHS ############
PREPEND PATH             ${DIFXMPIDIR}/bin
PREPEND PATH             ${DIFXROOT}/bin
if $?IPP_LIBRARY_PATH then
    PREPEND LD_LIBRARY_PATH $IPP_LIBRARY_PATH
endif
if $?MPI_LIBRARY_PATH then
    PREPEND LD_LIBRARY_PATH $MPI_LIBRARY_PATH
fi
if ($DIFXOS == "darwin") then
  PREPEND DYLD_LIBRARY_PATH  ${DIFXROOT}/lib
  PREPEND DYLD_LIBRARY_PATH  ${PGPLOTDIR}
else
  PREPEND LD_LIBRARY_PATH  ${DIFXROOT}/lib
  PREPEND LD_LIBRARY_PATH  ${PGPLOTDIR}
  if ( $arch == "x86_64" ) then #64 bit
    PREPEND LD_LIBRARY_PATH  ${DIFXROOT}/lib64
  endif
endif
if ($?PKG_CONFIG_PATH) then
  PREPEND PKG_CONFIG_PATH  ${DIFXROOT}/lib/pkgconfig
else
  setenv PKG_CONFIG_PATH  ${DIFXROOT}/lib/pkgconfig
endif
PREPEND PYTHONPATH  $DIFXROOT/lib/python
if ( $arch == "x86_64" ) then #64 bit
  PREPEND PKG_CONFIG_PATH  ${DIFXROOT}/lib64/pkgconfig
  PREPEND PYTHONPATH  $DIFXROOT/lib64/python
endif

echo " DiFX version $DIFX_VERSION is selected"

unalias PREPEND
