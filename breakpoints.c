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
// Updated and commented by Ming-Lun Lee (Spring 2022)

/* basic breakpoint text file support */
#include <breakpoints.h>
#include <stdlib.h>

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

/* Returning the maximum breakpoint */
BREAKPOINT maxpoint(const BREAKPOINT * points, unsigned long npoints)
{
	unsigned long i;
	BREAKPOINT point;

	point.time  = points[0].time;
	point.value = points[0].value;
	if(npoints >1) {	
		for(i = 1;i < npoints; i++){
			if(point.value < points[i].value){
				point.time  = points[i].time;
				point.value = points[i].value;
				
			}
		}
	}
	return point;
}

/* Returning the minimum breakpoint */
BREAKPOINT minpoint(const BREAKPOINT * points, unsigned long npoints)
{
	unsigned long i;
	BREAKPOINT point;

	point.time  = points[0].time;
	point.value = points[0].value;
	if(npoints > 1) {	
		for(i = 1;i < npoints; i++){
			if(points[i].value < point.value){
				point.time  = points[i].time;
				point.value = points[i].value;
				
			}
		}
	}
	return point;
}

/* Checking if all the breakpoints are within the specified range */
int inrange(const BREAKPOINT * points, double minval, double maxval, unsigned long npoints)
{
   unsigned long i;
   int range_OK = 1;
 
   for(i = 0; i < npoints; i++){
	   // If a breakpoint value minval is less than minval or greater than maxval, the function returns 0.
	   if(points[i].value < minval || points[i].value > maxval)   {
			range_OK = 0;  
			break;
	   }
   }
   return range_OK;
}

/* Finding the value at a specified time using linear interpolation 
   betwen two neighboring breakpoints 
   const BREAKPOINT *points: a pointer to an array of breakpoints
   unsigned long npoints: number of breakpoings
   double time: the specified time 
   return value: the value at the specified time using linear interpolation.
*/
double val_at_brktime(const BREAKPOINT * points, unsigned long npoints, double time)
{
	unsigned long i;
	BREAKPOINT left, right;
	double frac, val, width;

	/* scan until we find a span containing our time   */
	for(i=1; i < npoints; i++){
		if(time <= points[i].time) 		// The span is found 
			break;
	}
	/* maintain final value if time beyond end of data */
	if(i == npoints){		
		return points[i-1].value; // return the value of the last point
	}
	left  = points[i-1];  // the left point of the span
	right = points[i];	  // the right point of the span
	
	/* check for instant jump - two points with same time */
	width = right.time - left.time;  // the time width of the span
	if(width == 0.0)     // It's an instant jump! The width of the span is 0.
		return right.value; // Use the value of the breakpoint on the right
	/* get value from this span using linear interpolation */
	frac = (time - left.time) / width; 
	val  = left.value + ((right.value - left.value) * frac);

	return val; // return the calculated value at the requested time.
}

/* Getting new breakpoints from a breakpoint text file.
   Input arguments:
   FILE *fp: a fp that has been initialized and points to a text file.
   FILE unsigned long *psize: a pointer to an unsigned long.
            *psize is not initiazlied. It will be updated with the 
			number of breakpoints later in this function.
   Returning: a pointer to an array of BREAKPOINTs.
*/
BREAKPOINT * get_breakpoints(FILE * fp, unsigned long * psize)
{
	int got;
	
	unsigned long npoints = 0, size = NPOINTS;
	double lasttime = 0.0;
	BREAKPOINT *points = NULL;	
	char line[LINELENGTH];

	if(fp == NULL)
		return NULL;
	points = (BREAKPOINT *) malloc(sizeof(BREAKPOINT) * size); // dynamic memory allocation
	if(points == NULL)
		return NULL;

	while(fgets(line, LINELENGTH, fp)){		// get a line of string from the breakpoint file		
		if((got = sscanf(line, "%lf%lf", &points[npoints].time, &points[npoints].value)) < 0) // parse a line of string and get a time and a value.
			continue;			  /* empty line */
		if(got == 0){
			printf("Line %lu has non-numeric data\n", npoints + 1);
			break;
		}
		if(got == 1){  // only one number is valid
			printf("Incomplete breakpoint found at point %lu\n", npoints + 1);
			break;
		}		
		if(points[npoints].time < lasttime){
			printf("error in breakpoint data at point %lu: time not increasing\n", npoints +1 );
			break;
		}
		lasttime = points[npoints].time;
		if(++npoints == size){ // The current block is full!
			BREAKPOINT * tmp;
			size += NPOINTS;  // Update the size of the new block
			tmp = (BREAKPOINT *) realloc(points, sizeof(BREAKPOINT) * size); // if the size is not large enough.
			if(tmp == NULL)	{	/* too bad! */
				/* have to release the memory, and return NULL to caller */
				npoints = 0;
				free(points);
				points = NULL;
				break;		 
			}
			points = tmp;  // update the pointer pointing to the array of BREAKPOINTs
		}			
	}
	if(npoints)							
		*psize = npoints;  // It is like returning *psize!!!
	return points;         // returning a pointer to an array of BREAKPOINTs
}


/******** breakpoint stream handling **************/

/* Used to initialize a new stream of breakpoints */
/* return a pointer to the initialized BRKSTREAM struct or NULL for error */
/* size can be NULL, if that info is not of interest */
BRKSTREAM * bps_newstream(FILE * fp, unsigned long srate, unsigned long * size)
{
	BRKSTREAM * stream;
	BREAKPOINT * points;
	unsigned long npoints;   // not initialized

	if(srate == 0){
		printf("Error creating stream - srate cannot be zero\n");
		return NULL;
	}
	stream = (BRKSTREAM *) malloc(sizeof(BRKSTREAM)); // only one BRKSTREAM
	if(stream == NULL)
		return NULL;
	/* load breakpoint file and setup stream info  */
	points = get_breakpoints(fp, &npoints); 
	if(points == NULL){
		free(stream);
		return NULL;
	}
	if(npoints < 2){ // at least two breakpoints are required
		printf("breakpoint file is too small - at least two points required\n");
		free(stream);
		return NULL;
	}
	/* init the stream object */
	stream->npoints = npoints;
	stream->points  = points;
	/*  counters */
	stream->curpos  = 0.0;
	stream->ileft   = 0;
	stream->iright  = 1;
	stream->incr    = 1.0 / srate;
	/*  first span */
	stream->leftpoint  = stream->points[stream->ileft];
	stream->rightpoint = stream->points[stream->iright];
	stream->width	   = stream->rightpoint.time - stream->leftpoint.time; 
	stream->height	   = stream->rightpoint.value - stream->leftpoint.value; 	
	stream->more_points = 1;
	if(size)
		*size = npoints;   // return *size to the function calling bps_newstream

	return stream; // returning the pointer pointing to a BRKSTREAM struct
}

/* destructor fucntion for breakpoint streams; need to call this before destroying stream itself */
void bps_freepoints(BRKSTREAM * stream)
{
	if(stream && stream->points){
		free(stream->points);	
		stream->points = NULL;
	}
}

/* Returning the maximum value (*outmax), the minimum value (*outmin) 
   If both values can be found, return 0.
   If the BRKSTREAM pointer is NULL or has fewer than 2 points, return -1.
*/
int bps_getminmax(BRKSTREAM * stream,double * outmin,double * outmax)
{
	double val,minval,maxval;
	unsigned long i;
	
	// The BRKSTREAM pointer is NULL or has fewer than 2 points
	if(stream == NULL || stream->npoints < 2)
		return -1;
	
	minval = maxval = stream->points[0].value;

	for(i = 1; i < stream->npoints; i++){
		val = stream->points[i].value;
		minval = MIN(minval,val);
		maxval = MAX(maxval,val);
	}
	*outmin = minval;   // return the minimum value
	*outmax = maxval;   // return the maximum value
	return 0; // successful!
}

/* Using a BRKSTREAM struct to find a value at a specified time using 
   linear interpolation.
   Similar to the val_at_brktime function.
*/
double bps_tick(BRKSTREAM * stream)
{	
	double thisval, frac;
	
	/* beyond end of brkdata? */
	if(stream->more_points == 0)
		return stream->rightpoint.value;
	if(stream->width == 0.0) 
		thisval = stream->rightpoint.value;
	else {
		/* get value from this span using linear interpolation */
		frac = (stream->curpos - stream->leftpoint.time) / stream->width;
		thisval = stream->leftpoint.value + ( stream->height * frac);
	}
	/* move up ready for next sample */
	stream->curpos += stream->incr;
	if(stream->curpos > stream->rightpoint.time){  /* need to go to next span? */
		stream->ileft++; stream->iright++;
		if(stream->iright < stream->npoints) {
			stream->leftpoint = stream->points[stream->ileft];
			stream->rightpoint = stream->points[stream->iright];
			stream->width	= stream->rightpoint.time - stream->leftpoint.time; 
			stream->height	= stream->rightpoint.value - stream->leftpoint.value;	
		}
		else
			stream->more_points = 0;
	}
	return thisval;
}

/* some other utility functions */

/* Rewind stream, so we can use data from beginnign again */
void bps_rewind(BRKSTREAM * stream)
{
	if(stream == NULL)
		return;			  /* a "do-nothing" error! */
	stream->ileft = 0;
	stream->iright = 1;
	stream->leftpoint.time	= stream->points[stream->ileft].time;
	stream->rightpoint.time	= stream->points[stream->iright].time;
	stream->leftpoint.value	= stream->points[stream->ileft].value;
	stream->rightpoint.value	= stream->points[stream->iright].value;
	stream->width	= stream->rightpoint.time - stream->leftpoint.time; 
	stream->height	= stream->rightpoint.value - stream->leftpoint.value;
	stream->curpos	= 0.0;	
}

/* Checking if all the breakpoints are within the range */
int bps_inrange(BRKSTREAM * stream, double minval, double maxval)
{	
	if(stream == NULL)
		return -1;
	return inrange(stream->points, minval, maxval, stream->npoints);
}
