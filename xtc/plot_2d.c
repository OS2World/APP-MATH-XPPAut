/* remove for X  */
#include <X11/Xlib.h>
#define NO2D 0
#define XPP 1
#define TPP 2
#define UVSX 3
#define UVST 4 

#define MAXPLOTS 30

/*   a series of plotting routines for two-d stuff
     These are specifically designed to work with output
     of xtc files...

     The routines are 
    
     pp_time(var1,var2,x1,x2,tlo,thi,nplots,w)
        this plots n-phaseplanes as time varies
	plotting var1(x1) vs var2(x2) as t goes from 
	tlo to thi
	
	
     pp_space(var1,var2,t1,t2,xlo,xhi,nplots,w)
        like above but x-plots
	
     x_plots(var,time,xlo,xhi,nplots,w)
     t_plots(var,x,tlo,thi,nplots,w)


  */

typedef struct {
  Window w; 
  double xlo,xhi,ylo,yhi;
  int axis;
  int x0,y0,Width,Height;
}TWO_D_WIN;

typedef struct {
  int var1[MAXPLOTS],var2[MAXPLOTS],ix[MAXPLOTS],it[MAXPLOTS],ls[MAXPLOTS];
  int nplots;
  int thi,tlo;
  TWO_D_WIN p2d;
}TWO_D_GRAPHS;


extern Display *display;
extern  double CURRENT_H,TFinal,Transient;
extern int BufIndex,CURRENT_GRID;
extern int DX_0,DY_0,D_WID,D_HGT;
extern Window graph_2d;
TWO_D_GRAPHS my_2d;



init_2d()
{
  int i;
  my_2d.p2d.axis=0;
  my_2d.p2d.w=graph_2d;
  my_2d.p2d.xlo=-2.0;
  my_2d.p2d.xhi=2.0;
  my_2d.p2d.ylo=-2.0;
  my_2d.p2d.yhi=2.0;
  get_2d_area();
  for(i=0;i<MAXPLOTS;i++){
    my_2d.var1[i]=0;
    my_2d.var2[i]=0;
    my_2d.ix[i]=0;
    my_2d.it[i]=0;
    my_2d.ls[i]=0;
  }
  my_2d.tlo=0;
  my_2d.thi=10;
  my_2d.nplots=0;
}
 
get_2d_area()
{
/*  get_draw_area(graph_2d); 
 my_2d.p2d.x0=DX_0;
 my_2d.p2d.y0=DY_0;
 my_2d.p2d.Width=D_WID;
 my_2d.p2d.Height=D_HGT;
*/
 get_win_area(&(my_2d.p2d));
}


get_win_area(p2d)
     TWO_D_WIN *p2d;
{
 get_draw_area(p2d->w);
 p2d->x0=DX_0;
 p2d->y0=DY_0;
 p2d->Width=D_WID;
 p2d->Height=D_HGT;
}

pp_time(var1,var2,ix1,ix2,tlo,thi,nplots,w,l,fit) /* 0 screen 1 fit 2 ps */
     int *var1,*var2,*ix1,*ix2,tlo,thi,nplots;
     TWO_D_WIN w;
     int *l,fit;
{
  int i,j,np;
  float x1,y1,x2,y2;
  float h_lo,h_hi,v_lo,v_hi;
  int in1,in2,type1,type2,nt=thi-tlo;
   if(var_to_type(var1[0],&in1,&type1))return;
    if(var_to_type(var2[0],&in2,&type2))return;
    qwik_data(in1,type1,ix1[0],tlo,&x1);
    qwik_data(in2,type2,ix2[0],tlo,&y1);
      h_lo=h_hi=x1;
    v_lo=v_hi=y1;
  for(np=0;np<nplots;np++){
    if(var_to_type(var1[np],&in1,&type1))return;
    if(var_to_type(var2[np],&in2,&type2))return;
    qwik_data(in1,type1,ix1[np],tlo,&x1);
    qwik_data(in2,type2,ix2[np],tlo,&y1);

    for(i=1;i<nt;i++){
      qwik_data(in1,type1,ix1[np],tlo+i,&x2);
      qwik_data(in2,type2,ix2[np],tlo+i,&y2);
      if(x2>h_hi)h_hi=x2;
      if(x2<h_lo)h_lo=x2;
      if(y2>v_hi)v_hi=y2;
      if(y2<v_lo)v_lo=y2;
      if(fit==0)line(x1,y1,x2,y2,w,l[np]);
      if(fit==2)ps_line(x1,y1,x2,y2);
      x1=x2;
      y1=y2;
    }
  }
  if(fit==1){
    my_2d.p2d.xlo=h_lo;
    my_2d.p2d.xhi=h_hi;
    my_2d.p2d.ylo=v_lo;
    my_2d.p2d.yhi=v_hi;
    check_2d_win(&(my_2d.p2d));
  }

}
     

pp_space(var1,var2,it1,it2,xlo,xhi,nplots,w,l,fit)
     int *var1,*var2,*it1,*it2,xlo,xhi,nplots;
     TWO_D_WIN w;
     int *l,fit;
{
  int i,j,np;
  float x1,y1,x2,y2;
  float h_lo,h_hi,v_lo,v_hi;
  int in1,in2,type1,type2,nx=xhi-xlo;
   if(var_to_type(var1[0],&in1,&type1))return;
    if(var_to_type(var2[0],&in2,&type2))return;
    qwik_data(in1,type1,xlo,it1[0],&x1);
    qwik_data(in2,type2,xlo,it2[0],&y1);
     h_lo=h_hi=x1;
    v_lo=v_hi=y1;
  
  for(np=0;np<nplots;np++){
    if(var_to_type(var1[np],&in1,&type1))return;
    if(var_to_type(var2[np],&in2,&type2))return;
    qwik_data(in1,type1,xlo,it1[np],&x1);
    qwik_data(in2,type2,xlo,it2[np],&y1);
    for(i=1;i<nx;i++){
      qwik_data(in1,type1,xlo+i,it1[np],&x2);
      qwik_data(in2,type2,xlo+i,it2[np],&y2);
           if(x2>h_hi)h_hi=x2;
      if(x2<h_lo)h_lo=x2;
      if(y2>v_hi)v_hi=y2;
      if(y2<v_lo)v_lo=y2;
      if(fit==0)line(x1,y1,x2,y2,w,l[np]);
      if(fit==2)ps_line(x1,y1,x2,y2);
      x1=x2;
      y1=y2;
    }
  }
  if(fit==1){
    my_2d.p2d.xlo=h_lo;
    my_2d.p2d.xhi=h_hi;
    my_2d.p2d.ylo=v_lo;
    my_2d.p2d.yhi=v_hi;
    check_2d_win();
  }

}
     
    
  
x_plots(var,time,xlo,xhi,nplots,w,l,fit)
     int *var,*time,xlo,xhi,nplots;
     TWO_D_WIN w;
     int *l,fit;
{
  int np,i;
  float x1,y1,x2,y2;
  float h_lo,h_hi,v_lo,v_hi;
  int in1,type1,nx=xhi-xlo;
     if(var_to_type(var[0],&in1,&type1))return;

    qwik_data(in1,type1,xlo,time[0],&y1);
    v_lo=v_hi=y1;
    h_lo=0.0;
    h_hi=CURRENT_H*CURRENT_GRID;
  for(np=0;np<nplots;np++){
    if(var_to_type(var[np],&in1,&type1))return;

    qwik_data(in1,type1,xlo,time[np],&y1);
    x1=xlo*CURRENT_H;
    for(i=1;i<nx;i++){
      qwik_data(in1,type1,xlo+i,time[np],&y2);
      x2=x1+CURRENT_H;
      if(y2<v_lo)v_lo=y2;
      if(y2>v_hi)v_hi=y2;
      if(fit==0)line(x1,y1,x2,y2,w,l[np]);
      if(fit==2)ps_line(x1,y1,x2,y2);
      x1=x2;
      y1=y2;
    }
  }
 if(fit==1){
    my_2d.p2d.xlo=h_lo;
    my_2d.p2d.xhi=h_hi;
    my_2d.p2d.ylo=v_lo;
    my_2d.p2d.yhi=v_hi;
    check_2d_win(&(my_2d.p2d));
  }

}


t_plots(var,x,tlo,thi,nplots,w,l,fit)
     int *var,*x,tlo,thi,nplots;
     TWO_D_WIN w;
     int  *l,fit;
{
  int np,i;
  int nt=thi-tlo;
  int in1,type1;
  float h_lo,h_hi,v_lo,v_hi;
  float x1,y1,x2,y2;
  h_lo=Transient;
  h_hi=Transient+thi*TFinal;
  if(var_to_type(var[0],&in1,&type1))return;
    qwik_data(in1,type1,x[0],tlo,&y1);
 v_lo=v_hi=y1;
 for(np=0;np<nplots;np++){
    if(var_to_type(var[np],&in1,&type1))return;
    qwik_data(in1,type1,x[np],tlo,&y1);
    x1=tlo*TFinal+Transient;
    for(i=1;i<nt;i++){
      qwik_data(in1,type1,x[np],tlo+i,&y2);
      x2=x1+TFinal;
      if(y2<v_lo)v_lo=y2;
      if(y2>v_hi)v_hi=y2;
      if(fit==0)line(x1,y1,x2,y2,w,l[np]);
      if(fit==2)ps_line(x1,y1,x2,y2);
      x1=x2;
      y1=y2;
    }
  }
 if(fit==1){
    my_2d.p2d.xlo=h_lo;
    my_2d.p2d.xhi=h_hi;
    my_2d.p2d.ylo=v_lo;
    my_2d.p2d.yhi=v_hi;
    check_2d_win(&(my_2d.p2d));
  }
    
}
 
ps_2d()
{
 float ar,xl,yl; 
 if(open_ps_file()!=1)return; 
/* set up so it is the same aspect ratio  */
    ar=((float)my_2d.p2d.Width)/((float)my_2d.p2d.Height);
    yl=6.0;
    xl=ar*yl;
    if(xl>10.0){
      xl=10.0;
      yl=xl/ar;
    }
    ps_begin(my_2d.p2d.xlo,my_2d.p2d.ylo,my_2d.p2d.xhi,my_2d.p2d.yhi,xl,yl);
    redraw_2d(2);
    ps_close();
}
   

redraw_2d(fit)
 int fit;
{
 if(fit==0)XClearWindow(display,my_2d.p2d.w);
 switch(my_2d.p2d.axis){
 	case NO2D:
		return;
	case XPP:
		if(fit==0)box(my_2d.p2d,0);
		if(fit==2)box(my_2d.p2d,1);
		
 	        pp_space(my_2d.var1,my_2d.var2,my_2d.ix,my_2d.it,
 	                 0,CURRENT_GRID+1,my_2d.nplots,my_2d.p2d,my_2d.ls,fit);
 	         break;
	case TPP:
		if(fit==0)box(my_2d.p2d,0);
		if(fit==2)box(my_2d.p2d,1);
		
		pp_time(my_2d.var1,my_2d.var2,my_2d.ix,my_2d.it,
 	                 0,BufIndex,my_2d.nplots,my_2d.p2d,my_2d.ls,fit);
 	         break;
	case UVSX:
		if(fit==0)t_axis(my_2d.p2d,0);
		if(fit==2)t_axis(my_2d.p2d,1);
		 x_plots(my_2d.var1,my_2d.ix,0,CURRENT_GRID+1,my_2d.nplots,
                         my_2d.p2d,my_2d.ls,fit);
		break;
	case UVST:
		if(fit==0)t_axis(my_2d.p2d,0);
		if(fit==2)t_axis(my_2d.p2d,1);
		
		t_plots(my_2d.var1,my_2d.ix,0,BufIndex,my_2d.nplots,
                         my_2d.p2d,my_2d.ls,fit);
		break;
 }
}


















