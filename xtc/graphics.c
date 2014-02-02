#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>


/* these are the graphics primitives 
   they include two-d plotting in a clipped window
   and involve calls to integer based primitives
   This is for X
   
   Fillstyle -1 is solid
   color 0 is ForeGround
   color -1 is BackGround

   draw_line(x1,y1,x2,y2,w)  
   draw_point(x1,y1,w)
   draw_rectangle(x1,y1,width,height,w)
   fill_rectangle(x1,y1,width,height,w)
   fill_polygon(points,npoints,w)
   draw_lines(points,npoints,w)
   clear_window(w);
*/


extern Display *display;
extern GC gc_graph;
extern int DCURXs,DCURYs;
extern GC small_gc;
extern Window graph_2d;

int D_FLAG,D_WID,D_HGT,DX_0,DY_0;

int N_Xtics=5,N_Ytics=5;

typedef struct {
 Window w; 
  double xlo,xhi,ylo,yhi;
  int axis;
  int x0,y0,Width,Height;
}TWO_D_WIN;



draw_line(x1,y1,x2,y2,w) 
     int x1,y1,x2,y2;
     Window w;
{
  XDrawLine(display,w,gc_graph,x1,y1,x2,y2);
}

draw_point(x1,y1,w)
     int x1,y1;
     Window w;
{
  XDrawPoint(display,w,gc_graph,x1,y1);
}

draw_rectangle(x1,y1,width,height,w)
     int x1,y1,width,height;
     Window w;
{
  XDrawRectangle(display,w,gc_graph,x1,y1,width,height);
}

fill_rectangle(x1,y1,width,height,w)
     int x1,y1,width,height;
     Window w;
{
  XFillRectangle(display,w,gc_graph,x1,y1,width,height);
}
 
fill_polygon(points,npoints,w)
     XPoint *points;
     int npoints;
     Window w;
{
  XFillPolygon(display,w,gc_graph,points,npoints,Nonconvex,CoordModeOrigin);
}
 
draw_lines(points,npoints,w)
     XPoint *points;
     int npoints;
     Window w;
{
  XDrawLines(display,w,gc_graph,points,npoints,CoordModeOrigin);
}

clear_window(w)
     Window w;
{
  XClearWindow(display,w);
}

get_draw_area(wind)
Window wind;
{
 int x,y,w,h,bw,de;
 Window root;
 XGetGeometry(display,wind,&root,&x,&y,&w,&h,&bw,&de);
 DX_0=10;
 DY_0=16+2*DCURYs;
 D_WID=w-20;
 D_HGT=h-50;
 if((D_HGT>0)&&(D_WID>0))D_FLAG=1;
 else D_FLAG=0; 
}

/* the following are two-d floating point graphics   */



  
scale_to_screen(x,y,i,j,p2d)
  float x,y;
  int *i,*j;
  TWO_D_WIN p2d;
  {
   float dx=p2d.Width/(p2d.xhi-p2d.xlo);
   float dy=p2d.Height/(p2d.yhi-p2d.ylo);
    *i=(int)((x-p2d.xlo)*dx)+p2d.x0;
    *j=p2d.Height-(int)((y-p2d.ylo)*dy)+p2d.y0;
  }


line(x1,y1,x2,y2,p2d,ls)
     float x1,y1,x2,y2;
     TWO_D_WIN p2d;
{
  int color=ls/16;
  int style=ls%16;
  float x1_out,y1_out,x2_out,y2_out;
  float x1pl,y1pl,x2pl,y2pl;
  float xfp1,xfp2,yfp1,yfp2;
  float tolr=.1,err;
  int xp1,yp1,xp2,yp2;

  setfillstyle(-1,color);
/*	printf("%f %f %f %f \n",x1,y1,x2,y2); */
  
  if(clip(x1,x2,y1,y2,&x1_out,&y1_out,&x2_out,&y2_out,p2d))
 {
  
   scale_to_screen(x1_out,y1_out,&xp1,&yp1,p2d);
  scale_to_screen(x2_out,y2_out,&xp2,&yp2,p2d);
  if(style)draw_point(xp1,yp1,p2d.w);
   else draw_line(xp1,yp1,xp2,yp2,p2d.w);
  
}
}

     
     
  



clip(x1,x2,y1, y2,
x1_out,y1_out,x2_out,y2_out,p2d)
float x1,y1,x2,y2,*x1_out,*y1_out,*x2_out,*y2_out;
TWO_D_WIN p2d;

/************************************************************ *
*  Clipping algorithm                                         *
*   on input,                                                 *
*           (x1,y1) and (x2,y2) are endpoints for line        *
*           (x_left,y_bottom) and (x_right,y_top) is window                     *
*   output:                                                   *
*           value is 1 for drawing, 0 for no drawing          *
*           (x1_out,y1_out),(x2_out,y2_out) are endpoints     *
*            of clipped line                                  *
***************************************************************/

{
   int istack,ix1,ix2,iy1,iy2,isum,iflag;
   float  wh,xhat,yhat,wv;
   float x_left=p2d.xlo;
   float x_right=p2d.xhi;
   float y_top=p2d.yhi;
   float y_bottom=p2d.ylo;
   istack=1;
   ix1=ix2=iy1=iy2=iflag=0;
   *y1_out=y1;
   *y2_out=y2;
   *x1_out=x1;
   *x2_out=x2;
   if(x1<x_left)ix1=-1;
   if(x1>x_right)ix1=1;
   if(x2<x_left)ix2=-1;
   if(x2>x_right)ix2=1;
   if(y2<y_bottom)iy2=-1;
   if(y2>y_top)iy2=1;
   if(y1<y_bottom)iy1=-1;
   if(y1>y_top)iy1=1;
   isum=abs(ix1)+abs(ix2)+abs(iy1)+abs(iy2);
   if(isum==0)return(1); /* both inside window so plottem' */

   if(((ix1==ix2)&&(ix1!=0))||((iy1==iy2)&&(iy1!=0)))return(0); 
   if(ix1==0) goto C2;
   wv=x_left;
   if(ix1>0) wv=x_right;
   *x1_out=wv;
   yhat=(y1-y2)*(wv-x2)/(x1-x2)+y2;
   if((yhat<=y_top)&&(yhat>=y_bottom)) {
     *y1_out=yhat;
     iflag=1;
   goto C3; }
   istack=0;
C2:
   if(iy1==0) goto C3;
   wh=y_bottom;
   if(iy1>0)wh=y_top;
   *y1_out=wh;
   xhat=(x1-x2)*(wh-y2)/(y1-y2)+x2;
   if((xhat<=x_right)&&(xhat>=x_left)) {
     *x1_out=xhat;
     iflag=1; }
   else istack=0;
C3:
   istack+=iflag;
   if((ix2==0)||(istack==0)) goto C4;
   wv=x_left;
   if(ix2>0) wv=x_right;
   *x2_out=wv;
   yhat=(y2-y1)*(wv-x1)/(x2-x1)+y1;
   if((yhat<=y_top)&&(yhat>=y_bottom)) {
   *y2_out=yhat;
   return(1);
   }
C4:
  if((iy2==0)||(istack==0))return(iflag);
  wh=y_bottom;
  if(iy2>0) wh=y_top;
  *y2_out=wh;
  xhat=(x2-x1)*(wh-y1)/(y2-y1)+x1;
  if((xhat<=x_right)&&(xhat>=x_left)) {
  *x2_out=xhat;
  return(1);
  }
  return(iflag);
}


/* specially formatted number positioning  */
/* flags:
	0 smallest
        1 biggest
        2 middle
        3 2 lines down
*/





put_number(xflag,yflag,fmt,number)
char *fmt;
float number;
int xflag,yflag;
{
 int i,j;
 int n;
 char text[30];
 sprintf(text,fmt,number);
 n=strlen(text);
/* if(PSFlag){
   put_number_ps(xflag,yflag,fmt,number);
   return;
 } */
 /*  do i first   */
 switch(xflag){
		case 0: i=DX_0+DCURXs;
			break;
		case 1: i=DX_0+D_WID-DCURXs*(n+1);
			break;
		case 2: i=DX_0+D_WID/2+DCURXs;
			break;
			}
 switch(yflag){
		case 0: j=DY_0+DCURYs;
			break;
		case 1: j=DY_0+D_HGT-DCURYs;
			break;
		case 2: j=DY_0+D_HGT/2+DCURYs;
			break;
		case 3: j=DY_0+2*DCURYs;
			break;
			}
  XDrawString(display,graph_2d,gc_graph,i,j,text,n);
}

hax_tics(n,xlo,xhi,ybase,dy,flag,p2d)
     float xlo,xhi,ybase,dy;
     int n;
     TWO_D_WIN p2d;
{
  int i;
  float dtic,x=xlo,y1,y2;
  if(n<=1)return;
  dtic=(xhi-xlo)/(float)(n-1);
  for(i=0;i<n;i++){
   
    if(flag==0)line(x+i*dtic,ybase,x+i*dtic,ybase+dy,p2d,0);
    if(flag==1)ps_line(x+i*dtic,ybase,x+i*dtic,ybase+dy);
  }
}

vax_tics(n,xbase,ylo,yhi,dx,flag,p2d)
     float ylo,yhi,xbase,dx;
     int n;
     TWO_D_WIN p2d;
{
  int i;
  float dtic,x,y1,y2;
  if(n<=1)return;
  dtic=(yhi-ylo)/(float)(n-1);
  for(i=0;i<n;i++){
    if(flag==0)line(xbase,ylo+i*dtic,xbase+dx,ylo+i*dtic,p2d,0);
    if(flag==1)ps_line(xbase,ylo+i*dtic,xbase+dx,ylo+i*dtic);
  }
}



t_axis(p2d,flag)
     TWO_D_WIN p2d;
     int flag;
{
 float x1=p2d.xlo,x2=p2d.xhi,y1=p2d.ylo,y2=p2d.yhi;
  float dy=(y2-y1)/25.0;
 float yb=.5*(y1+y2);
 float dx=(x2-x1)/25.0;
 if(flag==0){
 line(x1,y1,x1,y2,p2d,0);
 line(x1,yb,x2,yb,p2d,0);
}
 if(flag==1){
   ps_line(x1,y1,x1,y2);
 ps_line(x1,yb,x2,yb);
 }
 hax_tics(N_Xtics,x1,x2,yb-dy/2,dy,flag,p2d);
 vax_tics(N_Ytics,x1-dx/2,y1,y2,dx,flag,p2d);
  if(flag==0)title_2dx(p2d);
  if(flag==1)title_2dps(p2d);
 if(flag==0)flush_display(); 
 
}  








box(p2d,flag)
TWO_D_WIN p2d;
int flag; /* 0=screen, 1=PS */
{
  float x1=p2d.xlo,x2=p2d.xhi,y1=p2d.ylo,y2=p2d.yhi;
  float dy=(y2-y1)/25.0;
  float dx=(x2-x1)/25.0;
  if(flag==0){
 line(x1,y1,x1,y2,p2d,0);
 line(x1,y2,x2,y2,p2d,0);
 line(x2,y2,x2,y1,p2d,0);
 line(x2,y1,x1,y1,p2d,0);
}
 if(flag==1){
 ps_line(x1,y1,x1,y2);
 ps_line(x1,y2,x2,y2);
 ps_line(x2,y2,x2,y1);
 ps_line(x2,y1,x1,y1);
}
  hax_tics(N_Xtics,x1,x2,y1-dy/2,dy,flag,p2d);
 vax_tics(N_Ytics,x1-dx/2,y1,y2,dx,flag,p2d);
  if(flag==0)title_2dx(p2d);
  if(flag==1)title_2dps(p2d);
 if(flag==0)flush_display(); 
 
}  

title_2dps(p2d)
     TWO_D_WIN p2d;
{
    int lx,ly;
    float dy=(p2d.yhi-p2d.ylo)/20.0;
    char xrange[50],yrange[50];
   sprintf(xrange,"%.4g < x < %.4g",p2d.xlo,p2d.xhi);
  sprintf(yrange,"%.4g < y < %.4g",p2d.ylo,p2d.yhi);
  lx=strlen(xrange);
  ly=strlen(yrange);
   ps_text(xrange,p2d.xlo,p2d.ylo-dy,0);
    ps_text(yrange,p2d.xlo,p2d.ylo-2*dy,0);
  }
    

title_2dx(p2d)
  TWO_D_WIN p2d;
  {
    int lx,ly,i1,i2,j1,j2;
    char xrange[50],yrange[50];
   sprintf(xrange,"%.4g < x < %.4g",p2d.xlo,p2d.xhi);
  sprintf(yrange,"%.4g < y < %.4g",p2d.ylo,p2d.yhi);
  lx=strlen(xrange);
  ly=strlen(yrange);
    i1=(p2d.Width-lx)/2;
    i2=(p2d.Width-ly)/2;
    j1= 8;
    j2=12+DCURYs;
    XDrawString(display,p2d.w,gc_graph,i1,j1,xrange,lx);
    XDrawString(display,p2d.w,gc_graph,i2,j2,yrange,ly);
  }
    

/*
box(p2d)
TWO_D_WIN p2d;
{
 float x1=p2d.xlo,x2=p2d.xhi,y1=p2d.ylo,y2=p2d.yhi;
 float dy=(y2-y1)/24.0;
 float dx=(x2-x1)/24.0;
 line(x1,y1,x1,y2,p2d,0);
 line(x1,y2,x2,y2,p2d,0);
 line(x2,y2,x2,y1,p2d,0);
 line(x2,y1,x1,y1,p2d,0);
 put_number(0,0,"%.4g",x1);
 put_number(1,0,"%.4g",x2);
 put_number(0,3,"%.4g",y1);
 put_number(1,3,"%.4g",y2);


 
}

*/


/* better Axes   */

