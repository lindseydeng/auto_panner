/* Copyright (c) 2009 Richard Dobson

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
// commented by Ming-Lun Lee

#ifndef __BREAKPOINTS_H_INCLUDED
#define __BREAKPOINTS_H_INCLUDED

#include <stdio.h>

typedef struct breakpoint {
		double time;
		double value;
} BREAKPOINT;

/* Returning the maximum breakpoint */
BREAKPOINT	maxpoint(const BREAKPOINT * points, unsigned long npoints);

/* Returning the minimum breakpoint */
BREAKPOINT	minpoint(const BREAKPOINT * points, unsigned long npoints);

/* Checking if all the breakpoints are within the specified range */
int			inrange(const BREAKPOINT * points, double minval, double maxval, unsigned long npoints);

/* Finding the value at a specified time using linear interpolation 
   betwen two neighboring breakpoints */
double		val_at_brktime(const BREAKPOINT * points, unsigned long npoints, double time);

/* Getting new breakpoints from a breakpoint text file */
BREAKPOINT * get_breakpoints(FILE * fp, unsigned long * psize); 

/* BRKSTREAM is a struct used to save and handle a stream of breakpoints. */
typedef struct breakpoint_stream {
	BREAKPOINT *	points;
	BREAKPOINT		leftpoint,rightpoint;
	unsigned long	npoints;
	double			curpos;
	double			incr;
	double			width;
	double			height;
	unsigned long   ileft,iright;
	int				more_points;
} BRKSTREAM;

/* Used to initialize a new stream of breakpoints */
/* srate cannot be 0; size pointer is optional - can be NULL */
BRKSTREAM *	bps_newstream(FILE * fp,unsigned long srate, unsigned long * size);  

/* Used to free memory used to save breakpoints */
void		bps_freepoints(BRKSTREAM * stream);

/* Using a BRKSTREAM struct to find a value at a specified time using 
   linear interpolation.
   Similar to the val_at_brktime function.
*/
double		bps_tick(BRKSTREAM * stream);		 /* NB: no error-checking, caller must ensure stream is valid */

/* Rewind stream, so we can use data from beginnign again */
void		bps_rewind(BRKSTREAM * stream); 

/* Checking if all the breakpoints are within the range */
int			bps_inrange(BRKSTREAM * stream, double minval, double maxval);

/* Returning the maximum value (*outmax), the minimum value (*outmin) 
   If both values can be found, return 0.
   If the BRKSTREAM pointer is NULL or has fewer than 2 points, return -1.
*/
int			bps_getminmax(BRKSTREAM * stream, double * outmin, double * outmax);

/* entirely arbitrary...*/
#define NPOINTS (64)  // Block size: start with a small block for the dynamic array of BREAKPOINTs.
#define LINELENGTH (80)  // Assume the max. lenghth of a string in the text file is not over this value - 1 .


#endif
