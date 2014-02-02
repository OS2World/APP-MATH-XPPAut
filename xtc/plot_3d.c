/* 3d stuff for plotting    */
#include <math.h>
#include <stdio.h>
#include <X11/Xlib.h>
#define COLORPLOT 0
#define SURFACE 1
#define GREYPLOT 2
#define COLORSURFACE 3
#define GREYSURFACE 4
#define CONTOUR 5
#define COLORCONTOUR 6

extern char plotvarstring[20];

extern float *DataBuffer;
/* remove these for X stuff   */


extern int MyRGB;

extern double DomainSize;

typedef struct {
  float x,y,z;
} Pt;
 


typedef struct {
  Window w ; 
  int wid_gr,hgt_gr;
  int color_scale,color_min;
  double ZMin,ZMax;
  double Alpha,ZFraction;
  double CMax,CMin;
  int NContours;
  int RenderType;
  int style;
  float Delx,Dely,Xorg,Yorg;
  int xoff,yoff,xtot;
} THREE_D_PARAM;

THREE_D_PARAM my_3d;


extern int DX_0,DY_0,D_WID,D_HGT;
extern int color_scale,color_min;
extern Window graph_3d,command_pop;
extern Display *display;
extern int DCURXs,DCURYs;
extern GC gc_graph;
 
extern int CURRENT_GRID,BufIndex;

set_tot(xoff,reclen)
int xoff,reclen;
{
 my_3d.xoff=xoff;
 my_3d.xtot=reclen;
}

init_3d()
{
 get_3d_area();
 my_3d.w=graph_3d;
 my_3d.color_scale=color_scale;
 my_3d.color_min=color_min;
 my_3d.ZMin=0.0;
 my_3d.ZMax=1.0;
 my_3d.Alpha=.9;
 my_3d.ZFraction=.2;
 my_3d.CMax=1.0;
 my_3d.CMin=0.0;
 my_3d.NContours=10;
 my_3d.RenderType=GREYPLOT;
 my_3d.style=0;
 my_3d.xoff=0;
 my_3d.yoff=0;
}

get_3d_area()
{
  get_draw_area(graph_3d); 
  my_3d.wid_gr=D_WID/* +DX_0 */;
  my_3d.hgt_gr=D_HGT/* +DY_0 */;
}

  
 
do_org(p3d,XLength,YLength)
     THREE_D_PARAM *p3d;
     int XLength,YLength;
{
   get_3d_area();
   p3d->Delx=(float)p3d->wid_gr*.9/(float)(XLength);
   p3d->Dely=(float)p3d->hgt_gr*.9/(float)(YLength);
   p3d->Xorg=.05*(float)p3d->wid_gr;
   p3d->Yorg=.05*(float)p3d->hgt_gr;
 }


do_graph_buttons(w)
     Window w;
{
  do_graph_3d_button(w,my_3d);
 
}

do_graph_3d_button(w,p3d)
     Window w;
     THREE_D_PARAM p3d;
{
  int flag=1,first=1;
  int i,j;
  XEvent ev;
  
  if(w!=p3d.w||BufIndex<=2||p3d.RenderType==SURFACE||
     p3d.RenderType==GREYSURFACE||p3d.RenderType==COLORSURFACE)
    return;
  XSelectInput(display,w,
    KeyPressMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask);
  while(flag)
    {
      XNextEvent(display,&ev);
      switch(ev.type){
      case KeyPress:
      case ButtonRelease:
	flag=0;
	break;
      case MotionNotify:
		
	write_z_values(ev.xmotion.x,ev.xmotion.y,p3d);
	break;
      }
    }
  XSelectInput(display,w,KeyPressMask|ButtonPressMask|ExposureMask);
  blank_screen(command_pop);
}

write_z_values(i,j,p3d)
     int i,j;
     THREE_D_PARAM p3d;
{
  char bob[80];
  float x,y,z=-1000;
  int cg1=CURRENT_GRID;
  int ip,jp;
  int ioff=p3d.xtot*p3d.yoff+p3d.xoff;
  int joff;
  float tmin,tmax,dy,dx;
  int ilo=(int)p3d.Xorg,jlo=(int)p3d.Yorg;
  int ihi=ilo+(int)((cg1+1)*p3d.Delx);
  int jhi=jlo+(int)(BufIndex*p3d.Dely);
  /* first check the limits   of  i  and j  --
     are they in the region  
  */

  if(i<=ilo||i>=ihi||j<=jlo||j>=jhi)return;
  get_time_scale(&tmin,&tmax);
  x=(tmax-tmin)*(j-jlo)/(jhi-jlo)+tmin;
  y=DomainSize*(i-ilo)/(ihi-ilo);
  dx=(i-ilo)/p3d.Delx;
  dy=(j-jlo)/p3d.Dely;
  ip=(int)(dx);
  jp=(int)(dy);
  if(ip>=0&&jp>=0&&ip<=CURRENT_GRID&&jp<=BufIndex){
    joff=jp*p3d.xtot+ioff;
    z=DataBuffer[ip+joff];
   
  }
 
  sprintf(bob,"%s(%g,%g)=%g",
	  plotvarstring,y,x,z);
  blank_screen(command_pop);
  ftext(0,0,bob,command_pop);
}









one_3d_line(line,YLength,TheZValues,XLength)
float *TheZValues;
int line,XLength,YLength;
{
 float scale;
 float vmin= my_3d.ZMin;
 int tval;
 int i,ystart,xstart;
 float dx,dy;
 int DX,DY;
 int joff;
 if(YLength==0)YLength=1;
 if(line==0)do_org(&my_3d,XLength,YLength);
 dx=my_3d.Delx;
 dy=my_3d.Dely;
 DX=(int)(dx+.99);
 DY=(int)(dy+.99);
 if(DY<=0)DY=1;
 if(DX<=0)DX=1;
 if(my_3d.RenderType==GREYPLOT){
  scale = 64./(my_3d.ZMax-my_3d.ZMin);
 	ystart=line*dy+my_3d.Yorg;
	for(i=0;i<XLength;i++){ 
		xstart=i*dx+my_3d.Xorg;
		tval=(int)(scale*(TheZValues[i]-vmin));
		if(tval<0)tval=0;
		if(tval>64)tval=64;
		 setfillstyle(tval,0);
		fill_rectangle(xstart,ystart,DX,DY,my_3d.w);
		}
 
 }
 else{
	scale=(float)my_3d.color_scale/(my_3d.ZMax-my_3d.ZMin);
		ystart=line*dy+my_3d.Yorg;
	for(i=0;i<XLength;i++){ 
		xstart=i*dx+my_3d.Xorg;
		tval=(int)(scale*(TheZValues[i]-vmin));
		if(tval<0)tval=0;
		if(tval>=my_3d.color_scale)tval=my_3d.color_scale-1;
		setfillstyle(-1,tval+my_3d.color_min);
		fill_rectangle(xstart,ystart,DX,DY,my_3d.w);
	}
 }
}



plot_ps_3d(TheZValues,p3d,XLength,YLength)
     float *TheZValues;
     int XLength,YLength;
     THREE_D_PARAM *p3d;
{   float ar,xl,yl;
    if(YLength==0)return;
   if(open_ps_file()!=1)return; 
   do_org(p3d,XLength,YLength);
/* set up so it is the same aspect ratio  */
    ar=((float)p3d->wid_gr)/((float)p3d->hgt_gr);
    yl=6.0;
    xl=ar*yl;
    if(xl>10.0){
      xl=10.0;
      yl=xl/ar;
    }
    ps_begin((double)0.0,(double)0.0,(double)XLength,(double)YLength,
	     xl,yl);
    switch(p3d->RenderType){
	case COLORPLOT:
           ps_color_render(TheZValues,*p3d,XLength,YLength,1);
	    ps_boxit();
 		break;
	case SURFACE: 
	   /* surf_plot(TheZValues,*p3d,XLength,YLength,0); */
	    break;
	case COLORSURFACE:
/*	    surf_plot(TheZValues,*p3d,XLength,YLength,1); */
	    break;
	case GREYSURFACE:
/*	    surf_plot(TheZValues,*p3d,XLength,YLength,2); */
	    break;
	case GREYPLOT:
		ps_color_render(TheZValues,*p3d,XLength,YLength,0);
		ps_boxit();
		break;
	case CONTOUR:
/*	    contour_plot(TheZValues,*p3d,XLength,YLength,0); 
	    c_box_it(*p3d,XLength,YLength); */
	    break;
	case COLORCONTOUR:
/*	    contour_plot(TheZValues,*p3d,XLength,YLength,1);
	    c_box_it(*p3d,XLength,YLength); */
	    break;
	}
    ps_close();
 }

data_3d(TheZValues,p3d,XLength,YLength,fp,xskip,yskip,flag)
     FILE *fp;
     float *TheZValues;
     int XLength,YLength;
     THREE_D_PARAM *p3d;
     int xskip,yskip,flag;
{
  int joff;
  int i,j;
  int xl=XLength/xskip,yl=YLength/yskip;
 int ioff=p3d->xtot*p3d->yoff+p3d->xoff;
  if(YLength==0)return;
  fprintf(fp,"# %d by %d\n",xl,yl);
  for(j=0;j<yl;j++){
    joff=j*p3d->xtot*yskip+ioff;
    for(i=0;i<xl;i++){
      if(flag=0)
	fprintf(fp,"%f\n",TheZValues[i*xskip+joff]);
      else
	fprintf(fp,"%f %f %f\n",(float)i/(float)xl,(float)j/(float)yl,
		TheZValues[i*xskip+joff]);
    }
    fprintf(fp,"\n");
  }
}

plot_3d(TheZValues,p3d,XLength,YLength)
     float *TheZValues;
     int XLength,YLength;
     THREE_D_PARAM *p3d;
{
    if(YLength==0)return;
 
   do_org(p3d,XLength,YLength);
    switch(p3d->RenderType){
	case COLORPLOT:
            color_render(TheZValues,*p3d,XLength,YLength);
            box_it(*p3d,XLength,YLength);
 		break;
	case SURFACE: 
	    surf_plot(TheZValues,*p3d,XLength,YLength,0);
	    break;
	case COLORSURFACE:
	    surf_plot(TheZValues,*p3d,XLength,YLength,1);
	    break;
	case GREYSURFACE:
	    surf_plot(TheZValues,*p3d,XLength,YLength,2);
	    break;
	case GREYPLOT:
		dither_render(TheZValues,*p3d,XLength,YLength);
		box_it(*p3d,XLength,YLength);
		break;
	case CONTOUR:
	    contour_plot(TheZValues,*p3d,XLength,YLength,0);
	    c_box_it(*p3d,XLength,YLength);
	    break;
	case COLORCONTOUR:
	    contour_plot(TheZValues,*p3d,XLength,YLength,1);
	    c_box_it(*p3d,XLength,YLength);
	    break;
	}
    title_it(*p3d,XLength,YLength);
 }


 surf_plot(TheZValues,p3d,XLength,YLength,ColorFlag)
     int ColorFlag,XLength,YLength;
     THREE_D_PARAM p3d;
     float *TheZValues;
   {
    
  /*  Lets work on the axes to make sure they work !!  */
   float xwid=.9*p3d.wid_gr,yhgt=.9*p3d.hgt_gr;
   float lz,ly,lx;
   float scale,cscale,gscale,z,zbar,zv;
   XPoint v[5];
   int i,j,index,color;
   float dx,dy,dxy,x,y;
     float x0,x1,x2,x3,x4,x5,x6,x7;
   float y0,y1,y2,y3,y4,y5,y6,y7;
   int ioff=p3d.yoff*p3d.xtot+p3d.xoff;
  clear_window(p3d.w);

   lz=p3d.ZFraction*yhgt;
   ly=(yhgt-lz)/sin(p3d.Alpha);
   lx=xwid-cos(p3d.Alpha)*ly;
   scale=lz/(p3d.ZMax-p3d.ZMin);
   cscale=(float)(p3d.color_scale)/(p3d.ZMax-p3d.ZMin);
   gscale=64./(p3d.ZMax-p3d.ZMin);
   setfillstyle(-1,0);
   x0=p3d.Xorg;
   y0=p3d.Yorg+yhgt;
   x1=x0;
   y1=y0-lz;
   x2=x0+lx;
   y2=y1;
   x3=x2;
   y3=y0;
   x4=x0+ly*cos(p3d.Alpha);
   y4=p3d.Yorg;
   x5=x4;
   y5=y4+lz;
   x6=x4+lx;
   y6=y4;
   x7=x6;
   y7=y5;
   dx=lx/(XLength-1);
   dy=(yhgt-lz)/(YLength-1);
   dxy=ly*cos(p3d.Alpha)/(YLength-1);
   for(j=0;j<(YLength-1);j++){
	for(i=0;i<(XLength-1);i++){
		zbar=0.0;
 		index=j*p3d.xtot+i+ioff;
		x=i*dx-j*dxy+x5;
		y=y5+j*dy;
		zv=TheZValues[index];
		zbar+=zv;
 		z=scale*(zv-p3d.ZMin);
		v[0].x=(int)x;
		v[0].y=(int)(y-z);

		zv=TheZValues[index+1];
		zbar+=zv;
 		z=scale*(zv-p3d.ZMin);
		v[1].x=(int)(x+dx);
		v[1].y=(int)(y-z);

		zv=TheZValues[index+1+p3d.xtot];
		zbar+=zv;
 		z=scale*(zv-p3d.ZMin);
		v[2].x=(int)(x+dx-dxy);
		v[2].y=(int)(y-z+dy);

		zv=TheZValues[index+p3d.xtot];
		zbar+=zv;
 		z=scale*(zv-p3d.ZMin);
		v[3].x=(int)(x-dxy);
		v[3].y=(int)(y-z+dy);

		v[4].x=v[0].x;
		v[4].y=v[0].y;
		if(ColorFlag==0)setfillstyle(-1,-1);

 		if(ColorFlag==1){
		zbar=(.25*zbar-p3d.ZMin)*cscale;
		color=(int)zbar;
		if(color>=p3d.color_scale)color=p3d.color_scale-1;
		if(color<0)color=0;
		setfillstyle(-1,color+p3d.color_min);
		}

		if(ColorFlag==2){
		zbar=(.25*zbar-p3d.ZMin)*gscale;
		color=(int)zbar;
		if(color>64)color=64;
		if(color<0)color=0;
		setfillstyle(color,0);
		}

		 fill_polygon(v,5,p3d.w);
		 setfillstyle(-1,0);
		 draw_lines(v,5,p3d.w);

		}

      }
   
/*  Main axes   */
   fline(x0,y0,x3,y3,p3d.w);
   fline(x0,y0,x1,y1,p3d.w);
 fline(x0,y0,x5,y5,p3d.w);   

/*  rest for box  uncomment to get it */

 if(p3d.style){
 fline(x2,y2,x3,y3,p3d.w);
 fline(x7,y7,x3,y3,p3d.w);
 fline(x1,y1,x2,y2,p3d.w);
 fline(x1,y1,x4,y4,p3d.w);  
 fline(x5,y5,x4,y4,p3d.w);
 fline(x6,y6,x4,y4,p3d.w);
 fline(x5,y5,x7,y7,p3d.w);
 fline(x6,y6,x7,y7,p3d.w);
 fline(x6,y6,x2,y2,p3d.w);
  } 
   
   
  


 }



fline(x1,y1,x2,y2,w)
     float x1,y1,x2,y2;
     Window w;
{ 
  draw_line((int)x1,(int)y1,(int)x2,(int)y2,w);
}

fpoint(x,y,w)
     float x,y;
     Window w;
{
  draw_point((int)x,(int)y,w);
}
 
 
title_it(p3d,XLength,YLength)
     THREE_D_PARAM p3d;
     int XLength,YLength;
{
 float tmin,tmax;
 char bob[30];
 get_time_scale(&tmin,&tmax);
  sprintf(bob,"t=%g",tmin);
  XDrawString(display,p3d.w,gc_graph,(int)p3d.Xorg,DCURYs,
	      bob,strlen(bob));
  sprintf(bob,"0 < X < %g",DomainSize);
   XDrawString(display,p3d.w,gc_graph,(int)p3d.Xorg+(int)(XLength*p3d.Delx/2.0)
	       ,DCURYs,
	      bob,strlen(bob));
  sprintf(bob,"t=%g",tmax);
  XDrawString(display,p3d.w,gc_graph,(int)p3d.Xorg,
              (int)(YLength*p3d.Dely)+1+(int)p3d.Yorg+DCURYs,
	      bob,strlen(bob));
}

box_it(p3d,XLength,YLength)
     THREE_D_PARAM p3d;
     int XLength,YLength;
{
  float tmin,tmax;
  char bob[30]; 
  setfillstyle(-1,0);
  draw_rectangle((int)p3d.Xorg,(int)p3d.Yorg,(int)(XLength*p3d.Delx),
			(int)(YLength*p3d.Dely)+1,p3d.w);

}

c_box_it(p3d,XLength,YLength)
     THREE_D_PARAM p3d;
     int XLength,YLength;
{
   float tmin,tmax;
  char bob[30]; 
  setfillstyle(-1,0);
  draw_rectangle((int)p3d.Xorg,(int)p3d.Yorg,(int)((XLength-1)*p3d.Delx),
			(int)((YLength-1)*p3d.Dely)+1,p3d.w);

}




dither_render(TheZValues,p3d,XLength,YLength)
     int XLength,YLength;
     float *TheZValues;
     THREE_D_PARAM p3d;
{
 float scale=64./(float)(p3d.ZMax-p3d.ZMin);
 float vmin=(float)p3d.ZMin;
 
 
 int tval;
 int i,j,ystart,xstart;
 float dx,dy;
 int DX,DY;
 int joff;
 int ioff=p3d.xtot*p3d.yoff+p3d.xoff;
 dx=p3d.Delx;
 dy=p3d.Dely;
 DX=(int)(dx+.99);
 DY=(int)(dy+.99);
  clear_window(p3d.w);
  for(j=0;j<YLength;j++){
	ystart=j*dy+p3d.Yorg;
         joff=j*p3d.xtot+ioff;
/*	  printf("3d - %d %f \n",joff,TheZValues[joff]); */
	for(i=0;i<XLength;i++){ 
		xstart=i*dx+p3d.Xorg;
		tval=(int)(scale*(TheZValues[i+joff]-vmin));
		if(tval<0)tval=0;
		if(tval>64)tval=64;
		 setfillstyle(tval,0);
		fill_rectangle(xstart,ystart,DX,DY,p3d.w);
		}
  }
}




color_render(TheZValues,p3d,XLength,YLength)
     int XLength,YLength;
     float *TheZValues;
     THREE_D_PARAM p3d;
{
 float scale=((float)p3d.color_scale)/((float)(p3d.ZMax-p3d.ZMin));
 float vmin=(float)p3d.ZMin;
 
 int tval;
 int i,j,ystart,xstart;
 int DX,DY;
 float dx=p3d.Delx,dy=p3d.Dely;
 int joff;
 int ioff=p3d.xtot*p3d.yoff+p3d.xoff;
 DX=(int)(dx+.99);
 DY=(int)(dy+.99);
  clear_window(p3d.w);
  for(j=0;j<YLength;j++){
	ystart=j*dy+p3d.Yorg;
         joff=j*p3d.xtot+ioff;
	for(i=0;i<XLength;i++){ 
		xstart=i*dx+p3d.Xorg;
		tval=(int)(scale*(TheZValues[i+joff]-vmin));
		if(tval<0)tval=0;
		if(tval>=p3d.color_scale)tval=p3d.color_scale-1;
		setfillstyle(-1,tval+p3d.color_min);
		fill_rectangle(xstart,ystart,DX,DY,p3d.w);
	
	}
  }


}

ps_color_render(TheZValues,p3d,XLength,YLength,flag)
     int XLength,YLength;
     float *TheZValues;
     THREE_D_PARAM p3d;
     int flag;
{
 float scale=1./((float)(p3d.ZMax-p3d.ZMin));
 float vmin=(float)p3d.ZMin,fill;
 int i,j;
 int joff;
 int ioff=p3d.xtot*p3d.yoff+p3d.xoff;
   for(j=0;j<YLength;j++){
              joff=j*p3d.xtot+ioff;
       	for(i=0;i<XLength;i++){ 
		fill=scale*(TheZValues[i+joff]-vmin);
	        if(fill<0.0)fill=0.0;
	        if(fill>1.0)fill=1.0;
	        if(flag==0)
		  ps_bar((float)i,(float)(YLength-j-1),1.0,1.0,1.-fill,0);
	        else ps_rgb_bar((float)i,(float)(YLength-j-1),1.0,1.0,fill,0,MyRGB);

	}
  }


}



/**************************************
 *            CONTOURS                *
 *************************************/

contour_plot(TheZValues,p3d,XLength,YLength,ColorFlag)
     int ColorFlag,XLength,YLength;
     THREE_D_PARAM p3d;
     float *TheZValues;
{
 float dc;
 int i,col;
  if(p3d.NContours<=1)dc=1.0;
  else
  dc=(float)(p3d.CMax-p3d.CMin)/(p3d.NContours-1);
 clear_window(p3d.w);

	for(i=0;i<p3d.NContours;i++){
        if(p3d.NContours<=1)col=0;
	else 
        if(ColorFlag==1)
       	col=(i*p3d.color_scale)/(p3d.NContours-1)+p3d.color_min;
	if(ColorFlag==0)col=0;
	
	do_contour(p3d.CMin+(float)i*dc,col,p3d,XLength,YLength,TheZValues);
 }
}

interpolate(p1,p2,z,x,y)
 Pt p1,p2;
 float z,*x,*y;
{
 float scale;
  if(p1.z==p2.z)return(0);
  scale=(z-p1.z)/(p2.z-p1.z);
  *x=p1.x+scale*(p2.x-p1.x);
  *y=p1.y+scale*(p2.y-p1.y);
   return(1);
 }

triangle_contour(p1,p2,p3,cval,w)
float cval;
Pt p1,p2,p3;
Window w;
{
 float x[3],y[3];
 int count=0;
 if(((cval<=p1.z)&&(cval>=p2.z))||
	((cval>=p1.z)&&(cval<=p2.z)))
	if(interpolate(p1,p2,cval,&x[count],&y[count]))count++;
 if(((cval<=p1.z)&&(cval>=p3.z))||
	((cval>=p1.z)&&(cval<=p3.z)))
	if(interpolate(p1,p3,cval,&x[count],&y[count]))count++;
 
if(((cval<=p3.z)&&(cval>=p2.z))||
	((cval>=p3.z)&&(cval<=p2.z)))
	if(interpolate(p3,p2,cval,&x[count],&y[count]))count++;
 
 if(count==2)draw_line((int)x[0],(int)y[0],(int)x[1],(int)y[1],w);
 }

do_contour(cval,col,p3d,XLength,YLength,TheZValues)
int col,XLength,YLength;
float cval;
THREE_D_PARAM p3d;
float *TheZValues;

{
  int i,j,index,k;
  Pt p[5];
  float x,y,z;
  float Dx=p3d.Delx,Dy=p3d.Dely; 
  int ioff=p3d.xtot*p3d.yoff+p3d.xoff;
  setfillstyle(-1,col);
  for(j=0;j<(YLength-1);j++){
	for(i=0;i<(XLength-1);i++){
	 x=p3d.Xorg+Dx*i;
	 y=p3d.Yorg+Dy*j;
	 index=i+j*p3d.xtot+ioff;
	 
	 p[0].x=x;
	 p[0].y=y;
	 p[0].z=TheZValues[index];
	 p[1].x=x+Dx;
	 p[1].y=y;
	 p[1].z=TheZValues[index+1];
	 p[2].x=x+Dx;
	 p[2].y=y+Dy;
	 p[2].z=TheZValues[index+1+p3d.xtot];
	 p[3].x=x;
	 p[3].y=y+Dy;
	 p[3].z=TheZValues[index+p3d.xtot];
	 p[4].x=.25*(p[0].x+p[1].x+p[2].x+p[3].x);	
	 p[4].y=.25*(p[0].y+p[1].y+p[2].y+p[3].y);
	 p[4].z=.25*(p[0].z+p[1].z+p[2].z+p[3].z); 

      
	 triangle_contour(p[0],p[1],p[4],cval,p3d.w);
	 triangle_contour(p[1],p[4],p[2],cval,p3d.w);
	 triangle_contour(p[4],p[3],p[2],cval,p3d.w);
	 triangle_contour(p[0],p[4],p[3],cval,p3d.w);
       /*
 	 triangle_contour(p[0],p[1],p[2],cval,p3d.w);
	 triangle_contour(p[0],p[3],p[2],cval,p3d.w);  */

         }
   }
}




 















