/************************************************************************/
/*                                                                      */
/* Standard record version control.  This routine returns the address   */
/* of a structure containing the desired record information.  This can  */
/* either be the address of the raw memory image of the disk record     */
/* that was read in, or a memory-allocated structure filled in element  */
/* by element, depending on whether or not the disk format and the      */
/* structure definitions match.  Either way, byte flipping is performed */
/* as necessary by the architecture-dependent macros cp_xxxx() defined  */
/* in bytflp.h                                                          */
/*                                                                      */
/*      Inputs:         version         Version number of disk image    */
/*                      address         Memory address of disk image    */
/*                                                                      */
/*      Output:         size		number of bytes read from input	*/
/*					address				*/
/*			Return value    Address of filled app structure */
/*									*/
/* Created 25 September 1995 by CJL					*/
/* Redesigned 17 September 1997 by CJL                                  */
/*									*/
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bytflp.h"
#include "type_302.h"
#include "mk4_dfio.h"
#include "mk4_util.h"

#define FALSE 0
#define TRUE 1

struct type_302 *
addr_302 (short version,
          void *address,
          int *size)
    {
    int i, malloced;
    struct type_302 *t302;
    struct type_302_v0 *t302_v0;
					/* Create application structure, which */
					/* might be simply the overlay structure */
    malloced = FALSE;
    if (version == T302_VERSION) t302 = (struct type_302 *)address;
    else
	{
	t302 = (struct type_302 *)malloc (sizeof (struct type_302));
	if (t302 == NULL)
	    {
	    msg ("Memory allocation failure in addr_302()", 2);
	    return (NULL);
	    }
	clear_302 (t302);
	malloced = TRUE;
	}
					/* Handle each version number */
					/* individually. */
    if (version == 0)
	{
					/* Overlay version-specific structure */
					/* noting size so we can maintain */
					/* pointer in file image */
	*size = sizeof (struct type_302_v0);
	t302_v0 = (struct type_302_v0 *)address;
					/* Copy structure elements, */
					/* with hidden byte flipping if needed */
					/* (see bytflp.h) */
	strncpy (t302->record_id, "302", 3);
	strncpy (t302->version_no, "00", 2);
	cp_short (t302->interval, t302_v0->interval);
    if (t302 != t302_v0)
	    strncpy (t302->chan_id, t302_v0->chan_id , 32);
	for (i=0; i<6; i++)
	    {
	    cp_double (t302->phase_spline[i], t302_v0->phase_spline[i]);
	    }

	return (t302);
	}
    else 
	{
	msg ("Unrecognized type 302 record version number %d", 2, version);
	if (malloced) free (t302);
	return (NULL);
	}
    }
