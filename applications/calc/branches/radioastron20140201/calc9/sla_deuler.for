      SUBROUTINE sla_DEULER (ORDER, PHI, THETA, PSI, RMAT)
*+
*     - - - - - - -
*      D E U L E R
*     - - - - - - -
*
*  Form a rotation matrix from the Euler angles - three successive
*  rotations about specified Cartesian axes (double precision)
*
*  Given:
*    ORDER  c*(*)    specifies about which axes the rotations occur
*    PHI    dp       1st rotation (radians)
*    THETA  dp       2nd rotation (   "   )
*    PSI    dp       3rd rotation (   "   )
*
*  Returned:
*    RMAT   dp(3,3)  rotation matrix
*
*  A rotation is positive when the reference frame rotates
*  anticlockwise as seen looking towards the origin from the
*  positive region of the specified axis.
*
*  The characters of ORDER define which axes the three successive
*  rotations are about.  A typical value is 'ZXZ', indicating that
*  RMAT is to become the direction cosine matrix corresponding to
*  rotations of the reference frame through PHI radians about the
*  old Z-axis, followed by THETA radians about the resulting X-axis,
*  then PSI radians about the resulting Z-axis.
*
*  The axis names can be any of the following, in any order or
*  combination:  X, Y, Z, uppercase or lowercase, 1, 2, 3.  Normal
*  axis labelling/numbering conventions apply;  the xyz (=123)
*  triad is right-handed.  Thus, the 'ZXZ' example given above
*  could be written 'zxz' or '313' (or even 'ZxZ' or '3xZ').  ORDER
*  is terminated by length or by the first unrecognised character.
*
*  Fewer than three rotations are acceptable, in which case the later
*  angle arguments are ignored.  Zero rotations produces a unit RMAT.
*
*  P.T.Wallace   Starlink   November 1988
*-

      IMPLICIT NONE

      CHARACTER*(*) ORDER
      DOUBLE PRECISION PHI,THETA,PSI,RMAT(3,3)

      INTEGER J,I,L,N,K
      DOUBLE PRECISION RESULT(3,3),ROTN(3,3),ANGLE,S,C,W,WM(3,3)
      CHARACTER AXIS



*  Initialise result matrix
      DO J=1,3
         DO I=1,3
            IF (I.NE.J) THEN
               RESULT(I,J) = 0D0
            ELSE
               RESULT(I,J) = 1D0
            END IF
         END DO
      END DO

*  Establish length of axis string
      L = LEN(ORDER)

*  Look at each character of axis string until finished
      DO N=1,3
         IF (N.LE.L) THEN

*        Initialise rotation matrix for the current rotation
            DO J=1,3
               DO I=1,3
                  IF (I.NE.J) THEN
                     ROTN(I,J) = 0D0
                  ELSE
                     ROTN(I,J) = 1D0
                  END IF
               END DO
            END DO

*        Pick up the appropriate Euler angle and take sine & cosine
            IF (N.EQ.1) THEN
               ANGLE = PHI
            ELSE IF (N.EQ.2) THEN
               ANGLE = THETA
            ELSE
               ANGLE = PSI
            END IF
            S = SIN(ANGLE)
            C = COS(ANGLE)

*        Identify the axis
            AXIS = ORDER(N:N)
            IF (AXIS.EQ.'X'.OR.
     :          AXIS.EQ.'x'.OR.
     :          AXIS.EQ.'1') THEN

*           Matrix for x-rotation
               ROTN(2,2) = C
               ROTN(2,3) = S
               ROTN(3,2) = -S
               ROTN(3,3) = C

            ELSE IF (AXIS.EQ.'Y'.OR.
     :               AXIS.EQ.'y'.OR.
     :               AXIS.EQ.'2') THEN

*           Matrix for y-rotation
               ROTN(1,1) = C
               ROTN(1,3) = -S
               ROTN(3,1) = S
               ROTN(3,3) = C

            ELSE IF (AXIS.EQ.'Z'.OR.
     :               AXIS.EQ.'z'.OR.
     :               AXIS.EQ.'3') THEN

*           Matrix for z-rotation
               ROTN(1,1) = C
               ROTN(1,2) = S
               ROTN(2,1) = -S
               ROTN(2,2) = C

            ELSE

*           Unrecognised character - fake end of string
               L = 0

            END IF

*        Apply the current rotation (matrix ROTN x matrix RESULT)
            DO I=1,3
               DO J=1,3
                  W = 0D0
                  DO K=1,3
                     W = W+ROTN(I,K)*RESULT(K,J)
                  END DO
                  WM(I,J) = W
               END DO
            END DO
            DO J=1,3
               DO I=1,3
                  RESULT(I,J) = WM(I,J)
               END DO
            END DO

         END IF

      END DO

*  Copy the result
      DO J=1,3
         DO I=1,3
            RMAT(I,J) = RESULT(I,J)
         END DO
      END DO

      END
