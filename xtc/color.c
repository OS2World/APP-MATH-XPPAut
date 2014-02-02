#include <math.h>
#include <X11/Xlib.h>
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
extern int screen,COLOR;
extern Window main_win;
 int color_min,color_max,color_scale;
extern int DCURX,DCURY,CURY_OFF,CURS_X,CURS_Y,DCURXs,DCURYs;
extern unsigned long Black,White;
extern unsigned int MyBackColor,MyForeColor,GrFore,GrBack;
#define MAX_COLORS 256
#define COL_TOTAL 100
 int rfun(),gfun(),bfun();
XColor	color[MAX_COLORS];
int	pixel[MAX_COLORS];

Pixmap MyStipple[65];
unsigned char greys[65][8];
char thresh[]={0,32,8,40,2,34,10,42,
               48,16,56,24,50,18,58,26,
   		12,44,4,36,14,46,6,38,
		60,28,52,20,62,30,54,22,
		3,35,11,43,1,33,9,41,
		51,19,59,27,49,17,57,25,
		15,47,7,39,13,45,5,37,
		63,31,55,23,61,29,53,21};

int color_mode;
 extern int COLOR;
int shades[]={15,11,14,10,8,13,9,6,3,12,2,7,5,1,4,0};
make_greys()
{
  int i;
  for(i=0;i<8;i++)greys[0][i]=0;
  for(i=0;i<64;i++)next_grey(greys[i],greys[i+1],i);
  
  for(i=0;i<65;i++)
    if(create_stipple(&MyStipple[i],i)==0){
      printf("Unable to create stipple");
      break;
    }

}
   
create_stipple(stip,type)
Pixmap *stip;
int type;
{
  if((*stip=XCreateBitmapFromData(display,
   main_win,greys[type],8,8)) ==0)
  return(0);
  return(1);
}

next_grey(xold,xnew,value)
unsigned char *xold, *xnew;
int value;
{
 int i;
 int k;
  int b,j;
 for(i=0;i<65;i++)
  if(thresh[i]==value)break;
  b=7-(i%8);
  j=i/8;
  k=1<<b;
  for(i=0;i<8;i++)
  xnew[i]=xold[i];
  xnew[j]=xnew[j]+k;
 }
   



setfillstyle(type, col )
int col,type;
{  int wid,hgt;
   Pixmap stipple;
   if(type==SOLID)XSetFillStyle(display,gc_graph,FillSolid);
   else
   {
     XSetStipple(display,gc_graph,MyStipple[type]);
    XSetFillStyle(display,gc_graph,FillOpaqueStippled);
    XSetBackground(display,gc_graph,GrBack);
    }
    if(col<0)XSetForeground(display,gc_graph,GrBack);
    else
    XSetForeground(display,gc_graph,ColorMap(col)); 
}



color16()
{ 
 
COLOR=1;
 color_min=0;
 color_max=16;
 color_scale=16;
 color_mode=0;
 make_greys();
}

color256(flag)
int flag;
{

 COLOR=2;
 color_mode=1;
 MakeColormap(flag);
 make_greys();
}




 int rsfun(y,per)
  double y;
  int per;
{  
  double x;
  x=y;
  if((y>.666666)&&(per==1))x=1.-y;

  if(x>.33333333333)return(0);
  return((int)(3.*255*sqrt((.333334-x)*(x+.33334)))); 
  /* return((int)255*sqrt(1-9*x*x)); */
}

int gsfun(y)
 double y;
{
 if(y>.666666)return(0);
 return( (int)(3.*255*sqrt((.6666667-y)*(y))));
}
 
int bsfun(y)
double y;
{
 if(y<.333334)return(0);
 return((int)(2.79*255*sqrt((1.05-y)*(y-.333333333))));
}








 int rfun(y)
  double y;
{  
  if(y<0.0||y>1.0)return(0);
  return((int)(255*sqrt(y*(2.-y))));
}


 
int bfun(y)
double y;
{
 if(y<0.0||y>1.0)return(0);
 return((int)(255*sqrt(1.0-y*y)));
}

 MakeColormap(flag)
 int flag; /* 0 for red/blue 1 for rgb 2 for periodic */
{
Colormap	cmap;

int	i;
int clo=20;
double z;

    color_min = 30;
    color_max = COL_TOTAL+color_min-1;
    cmap = DefaultColormap(display,screen);
    color_scale=COL_TOTAL;
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
    color[ORANGE].green=165;

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
	
	z=(double)(i-color_min)/(double)color_scale;
	switch(flag) {
	case 0:
	    color[i].red = rfun(z) << 8;
	    color[i].green =0;
	    color[i].blue =bfun(z)<< 8;
	    break;
	  case 1:
	    color[i].red = rsfun(z,0) << 8;
	    color[i].green =gsfun(z)<<8;
	    color[i].blue =bsfun(z)<< 8;
	    break;
	  case 2:
	    color[i].red = rsfun(z,1) << 8;
	    color[i].green =gsfun(z)<<8;
	    color[i].blue =bsfun(z)<< 8;
	    break;
	  }
     	 	color[i].flags = DoRed | DoGreen | DoBlue;
	XAllocColor(display,cmap,&color[i]);
    }
   
}

int ColorMap(i)
int i;
{   if(i<0)return(GrBack);
    if(i==0)return(GrFore);
    if(COLOR==0)return(GrFore);
    if(color_max==16){
	 i=i%16;
	return(shades[i]);
    }
    if(color_mode){
	return(color[i].pixel);
    } else {
	return(i);
    }
}










