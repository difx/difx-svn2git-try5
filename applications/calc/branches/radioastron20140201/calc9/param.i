C! Param.i
C
C   Parameters:
C
       Integer*4 IRECL
C   JPL_eph 1s the path name for the JPL DE/LE403 ephemeris file on your system.
C     (GSFC: length=34, JPL_eph = '/data18/mk3/src/calc9.0/JPL.DE403 ')
C   
C WEW
C      Character*34 JPL_eph
C      Parameter (JPL_eph = '/data18/mk3/src/calc9.0/JPL.DE403 ')
C      Parameter (IRECL = 7184)
       Character*12 JPL_eph
       Parameter (JPL_eph = 'JPLEPH.403  ')
       Parameter (IRECL = 8144)


C
C     The default name of the leapsecond file.
C       (USNO: '/data/erp/ut1ls.dat';
C        GSFC: '/data1/apriori_files/ut1ls.dat')
C       (DFLEAP length - USNO: 19; GSFC: 30)
C
C     Character  DFLEAP*30
C     Parameter (DFLEAP = '/data1/apriori_files/ut1ls.dat')
C WEW
C     Character  DFLEAP*33
C     Parameter (DFLEAP = '/data18/mk3/src/calc9.0/ut1ls.dat')
      Character  DFLEAP*12
      Parameter (DFLEAP = 'ut1ls.dat   ')
C
