/****************************************/
/*					*/
/*	plot_graph takes a pointer to	*/
/* an array of points, a character, and	*/
/* alternate character to plot in, and	*/
/* a screen area, and uses grid to make	*/
/* an ascII graph.			*/
/*					*/
/****************************************/
#include <stdio.h>

void 
plot_graph(matrix, x, y, width, height, 
		scale_x, scale_y, winstart, winstop,
			points, numpoints, ch, altch,segment_flag) 
char **matrix;
int x, y, width, height, numpoints, winstart, winstop,segment_flag;
double *points;
double scale_x, scale_y;
char ch, altch;

    {
    int index,seg,pts;
    double seg_len,z;

    if(segment_flag==1)
	{
	seg_len = (scale_x / (double)width);
	if(seg_len<1) seg_len = 1.;
	for(seg = 0;seg < width; seg++)
	    {
	    z=0.;
	    pts=0;
	    for(index=(int)(seg*seg_len); index <(int)(seg_len*(seg+1.)) ; index++)
		if((index<numpoints) && (points[index] != -999.))
		   {
		   pts++;
		   z+=points[index];
		   }
	    if(pts>0)
	       z/=pts;
	    if(z != 0.)
	       grid(matrix, x, y, width, height,
			(double)width, scale_y,
			winstart, winstop,
			seg, z,
			ch, altch);
	    }
	}
    else
	{
	for(index=0;index<numpoints;index++)
	    grid(matrix,x,y,width,height,
		    scale_x,scale_y,
		    winstart,winstop,
		    index,points[index],
		    ch,altch);
	}
    }
