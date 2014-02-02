

#include <X11/Xlib.h>

#include <math.h>
#define COLOR_SCALE 0
#define GRAYSCALE 1
#define RGRAYSCALE 2
#define SOLID -1
#define RED	20
#define REDORANGE	21
#define ORANGE	22
#define YELLOWORANGE	23
#define YELLOW    24
#define YELLOWGREEN 25
#define GREEN      26
#define BLUEGREEN  27
#define BLUE   28
#define PURPLE 29


 extern GC gc_graph;
extern Display *display;
extern int screen;
extern Window main_win;
 int color_mode=1,color_min,color_total,COLOR,color_max;
extern int DCURX,DCURY,CURY_OFF,CURS_X,CURS_Y,DCURXs,DCURYs;
extern unsigned int Black,White;
extern unsigned int MyBackColor,MyForeColor,GrFore,GrBack;
int periodic=0,spectral;
#define MAX_COLORS 256
#define COL_TOTAL 100
 int rfun(),gfun(),bfun();
XColor	color[MAX_COLORS];
int	pixel[MAX_COLORS];


tst_color(w)
Window w;
{
 int i;
 for(i=0;i<color_total;i++){
   set_color(i+color_min);
   XDrawLine(display,w,gc_graph,0,2*i+20,50,2*i+20);
 }
}
set_color(col)
int col;
{
 if(col<0)XSetForeground(display,gc_graph,GrBack);
 if(col==0)XSetForeground(display,gc_graph,GrFore);
 else{

   if(COLOR)XSetForeground(display,gc_graph,ColorMap(col));
   else XSetForeground(display,gc_graph,GrFore);
}

}




 int rfun(y,per)
  double y;
  int per;
{  
  double x;
  x=y;
  if((y>.666666)&&(per==1))x=1.-y;

  if(x>.33333333333)return(0);
  return((int)(3.*255*sqrt((.333334-x)*(x+.33334))));
}

int gfun(y)
 double y;
{
 if(y>.666666)return(0);
 return( (int)(3.*255*sqrt((.6666667-y)*(y))));
}
 
int bfun(y)
double y;
{
 if(y<.333334)return(0);
 return((int)(2.79*255*sqrt((1.05-y)*(y-.333333333))));
}

 MakeColormap()
{
Colormap	cmap;

int	i;
int clo=20;
double z;

    color_min = 30;
    color_max = MAX_COLORS -1;
    color_total = color_max - color_min +1;
    if(color_total>COL_TOTAL)color_total=COL_TOTAL;
    color_max=color_min+color_total;
    cmap = DefaultColormap(display,screen);
    for (i = 0; i < clo; i++) {
	color[i].pixel = i;
    }
    for(i=20;i<30;i++){
    color[i].red=0;
    color[i].blue=0;
    color[i].green=0;
    	color[i].flags = DoRed | DoGreen | DoBlue;
    }
 
  
    color[RED].red=255;
    color[BLUE].blue=255;
    color[GREEN].green=255;
    color[YELLOWGREEN].red=154;
    color[YELLOWGREEN].blue=50;
    color[YELLOWGREEN].green=205;
    color[REDORANGE].red=240;
    color[REDORANGE].green=100;
    color[ORANGE].red=255;
    color[ORANGE].green=165
;
    color[YELLOWORANGE].red=255;
    color[YELLOWORANGE].green=205;
    color[YELLOW].red=255;
    color[YELLOW].green=255;
    color[BLUEGREEN].blue=255;
    color[BLUEGREEN].green=255;
    color[PURPLE].red=160;
    color[PURPLE].green=32;
    color[PURPLE].blue=240;
    for(i=20;i<30;i++)
    {
   color[i].red=color[i].red<<8;
    color[i].blue=color[i].blue<<8;
    color[i].green=color[i].green<<8;
    	color[i].flags = DoRed | DoGreen | DoBlue;
		XAllocColor(display,cmap,&color[i]);

    }
    for (i = color_min; i <= color_max; i++) {
	/*
	**define rgb values:
	*/
       switch(spectral) {
        case COLOR_SCALE:
	z=(double)(i-color_min)/(double)color_total;
	    color[i].red = rfun(z,periodic) << 8;
	    color[i].green =gfun(z) << 8;
	    color[i].blue =bfun(z)<< 8;
           		break;
	case GRAYSCALE:
	    color[i].red = (int)((i-color_min)*(255-50)/color_total +50) << 8;
	    color[i].green = (int)((i-color_min)*(255-50)/color_total +50) << 8;
	    color[i].blue = (int)((i-color_min)*(255-50)/color_total +50) << 8;
	    break;
	case RGRAYSCALE:
	    color[i].red = (int)((color_max-i)*(255-50)/color_total +50) << 8;
	    color[i].green = (int)((color_max-i)*(255-50)/color_total +50) << 8;
	    color[i].blue = (int)((color_max-i)*(255-50)/color_total +50) << 8;
	    break;
	}
	 	color[i].flags = DoRed | DoGreen | DoBlue;
	XAllocColor(display,cmap,&color[i]);
    }
   
}

int ColorMap(i)
int i;
{   if(i==-1)return(GrBack);
    if(i==0)return(GrFore);
    if(color_mode){
      if(i<0)i=0;
      if(i>=color_max)i=color_max;
	return(color[i].pixel);
    } else {
	return(i);
    }
}


