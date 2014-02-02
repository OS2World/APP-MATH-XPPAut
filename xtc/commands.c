#include <X11/Xlib.h>
#include <math.h>
#include <stdio.h>
#define MAX_LEN_SBOX 25
#define MAXPLOTS 30
#define NO2D 0
#define XPP 1
#define TPP 2
#define UVSX 3
#define UVST 4 
#define MAXMODE 200
#define WRITEM 1
#define READEM 0
#define MAXCRHS 20
#define MAXCFIX 20
#define MAXSRHS 20

extern char ucvar_names[MAXCRHS][10];
extern char uvar_names[MAXSRHS][10];
extern Window main_win,menu_pop;
extern Display *display;
int MyRGB=0;
extern int DCURY,BufSize,BufIndex;
extern float *DataBuffer;
extern int Method,FastFlag,Nout,MaxIter,MaxJac,CURRENT_GRID,RandSeed;
extern int NCTM,NSCALAR,AUX_CTM,AUX_SCAL;
extern double DomainSize,DeltaT,TFinal,Transient,TStart,TEnd;
extern double Tolerance,Epsilon,DtMin,DtMax,WEIGHT_0;
extern double BOUND;
extern double CURRENT_H, *MyData;
extern char plotvarstring[20];
extern int PlotVar;
double atof(),fabs();
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

extern THREE_D_PARAM my_3d;
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

typedef struct {
  int nmodes,grid,half_flag,maxit;
  double eps,err;
  double x[50],real[MAXMODE],imag[MAXMODE];
  TWO_D_WIN p2d;
  int exist;
} TURING;

TURING turing;
  
extern TWO_D_GRAPHS my_2d;

double i_to_t();
double i_to_x();

double i_to_t(i)
int i;
{
 return(TFinal*i+Transient+TStart);
}

bound_exceed(my_y,time,type,ind,space)
     double my_y,time;
     int type,ind,space;
{
  char bob[100];
  if(type==0){
    sprintf(bob,"%s=%f>%f at t=%f",uvar_names[ind],my_y,BOUND,time);
  }
  else
   sprintf(bob,"%s(%f)=%f>%f at t=%f",ucvar_names[ind],space*CURRENT_H,
	   my_y,BOUND,time);
  err_msg(bob);
}
double i_to_x(i)
int i;
{
 return(CURRENT_H*i);
}
x_to_i(x)
double x;
{
 int i;
 double z=(x+.5*CURRENT_H)/CURRENT_H;
 i=(int)z;
 if(i>CURRENT_GRID)i=CURRENT_GRID;
 if(i<0)i=0;
 return(i);
}
t_to_i(t)
double t;
{
 int i;
 i=(int)((t-TStart-Transient+.5*TFinal)/TFinal);
 if(i<0)i=0;
 if(i>=BufIndex)i=BufIndex-1;
 return(i);
}

get_offset(plot,off,reclen)
int *off,*reclen,plot;
{
 int ng1=CURRENT_GRID+1;
 int nd=NSCALAR+NCTM;
 int nda=nd+AUX_SCAL;
 int ntot=nda+AUX_CTM;
 
 *reclen=1+NSCALAR+AUX_SCAL+(AUX_CTM+NCTM)*ng1;
 if(plot>=0&&plot<NSCALAR){
	*off=1+plot;
	return(0);
 }
 if(plot>=NSCALAR&&plot<nd){
	*off=1+NSCALAR+(plot-NSCALAR)*ng1;
	return(1);
 }
 if(plot>=nd&&plot<nda){
	*off=1+NSCALAR+NCTM*ng1+plot-(NCTM+NSCALAR);
	return(2);
 }
 if(plot>=nda&&plot<ntot){
	*off=1+NSCALAR+NCTM*ng1+AUX_SCAL+(plot-NSCALAR-NCTM-AUX_SCAL)*ng1;
	return(3);
 }
 return(-1);
}
	

solve_them()
{
 double t=TStart;
 int jstart=0;
  BufIndex=0;
 get_ics(1,MyData,t);
 integrate_em(&t,MyData,&jstart,Nout,0);
}

 my_abort()
{
 int ch;
 XEvent event;
 while(XPending(display)>0){
 XNextEvent(display,&event);
 switch(event.type){
	case Expose: 
	  	     break;
	case ButtonPress:
          break;
        case KeyPress:
        ch=get_key_press(&event);
        if(ch==27)return(1);
        
}
 }
 
 return(0);
}

com_integrate()
{
  Window temp=main_win;
  static char *n[]={"New","Old","Last","Shift"};
  static char key[]="nols";
  char ch;
  int i;
  ch=(char)pop_up_list(&temp,"Init. Data",n,key,4,10,0,10,3*DCURY);
  switch(ch){
	case 'n':
  	change_ics();
/*	printf("Changed the ics...\n"); */
	get_ics(0,MyData,TStart);
	break;
	case 'o':
	get_ics(1,MyData,TStart);
	break;
	case 'l':
	get_ics(2,MyData,TStart);
	break;
	case 's':
	get_ics(3,MyData,TEnd);
	break;
  }

}

set_2d(i)
int i;
{
switch(my_2d.p2d.axis){
		case XPP:
		get_x_pp(i);
	
		break;
		case TPP:
		get_t_pp(i);
	
		break;
		case UVST:
		get_u_vs_t(i);
	
		break;
		case UVSX:
		get_u_vs_x(i);
	
		break;
}
}

com_2d()
{
 Window temp=main_win;
 static char *n[]={"0.None","1.XPhsPln","2.TPhsPln","3.UvsX","4.UvsT",
		     "Add","Delete","Edit","Postscript","List"};
 static char key[]="01234adepl";
 int i;
 char ch;
 char bob[20];
 ch=(char)pop_up_list(&temp,"2D graphs",n,key,10,10,my_2d.p2d.axis,10,3*DCURY+(DCURY+8));
 switch(ch){
 case '0': my_2d.p2d.axis=0;
	   my_2d.nplots=0;
	   break;
 case '1':
   get_x_pp(0);
   my_2d.nplots=1;
   my_2d.p2d.axis=XPP;
   break;
 case '2':
   get_t_pp(0);
   my_2d.nplots=1;
   my_2d.p2d.axis=TPP;
   break;
 case '3':
   get_u_vs_x(0);
   my_2d.nplots=1;
   my_2d.p2d.axis=UVSX;
   break;
 case '4':
   get_u_vs_t(0);
   my_2d.nplots=1;
   my_2d.p2d.axis=UVST;
   break;
 case 'a':
	if(my_2d.p2d.axis==0){
		err_msg("Choose a graph type first");
		break;
	}
	i=my_2d.nplots;
	if(i>=MAXPLOTS){
		err_msg("Too many 2d plots..");
		break;
	}
	set_2d(i);
	++(my_2d.nplots);
	break;
  case 'd':
	  --(my_2d.nplots);
          break;
  case 'e':
	   if(my_2d.nplots==0)break;
	   i=my_2d.nplots;
 	   sprintf(bob,"Edit(1-%d)",i);
	   i=1;
	   new_int(bob,&i);
	   if(i<=0||i>my_2d.nplots)break;
	   set_2d(i-1);
	   break;
  case 'p':
             ps_2d(); 
	        ping();
	   
		break;
  case 'l':
		make_list();
		break;

 }
}

make_list()
{
 static char *nm[]={"Low","High","Number"};
 char v[3][MAX_LEN_SBOX];
 int status,i;
 double lo=0.0,hi=1.0,dl,z;
 int n=10;
 if(my_2d.p2d.axis==0)return;
 setfloat(0,v,lo);
 setfloat(1,v,hi);
 setint(2,v,n);
  status=do_string_box(3,3,1,"List",nm,v,31);
  if(status==0)return;
 getfloat(0,v,&lo);
 getfloat(1,v,&hi);
 getint(2,v,&n);
 if(n<=0)return;
 if(n>=MAXPLOTS)n=MAXPLOTS-1;
  dl=(hi-lo)/(double)n;
  for(i=0;i<=n;i++){
	z=lo+dl*i;
	my_2d.var1[i]=my_2d.var1[0];
	my_2d.var2[i]=my_2d.var2[0];	
  	my_2d.ls[i]=my_2d.ls[0];

	switch(my_2d.p2d.axis){
	case XPP:
		my_2d.ix[i]=t_to_i(z);
		my_2d.it[i]=my_2d.ix[i];
		break;
	case TPP:
		my_2d.ix[i]=x_to_i(z);
		my_2d.it[i]=my_2d.ix[i];
		break;
	case UVSX:
		my_2d.ix[i]=t_to_i(z);
		my_2d.it[i]=my_2d.ix[i];
		break;
	case UVST:
		my_2d.ix[i]=x_to_i(z);
		my_2d.it[i]=my_2d.ix[i];
		break;
	}
  }
	my_2d.nplots=n+1;
}

com_3d()
{
 Window temp=main_win;
 static char *n[]={"0.Color","1.Surf","2.Grey","3.SurfCol","4.SurfGrey",
		     "5.Contour","6.ColCont","PostScript"};
 static char key[]="0123456p";
	

 int i;
 int oldi;
 char ch;
 ch=(char)pop_up_list(&temp,"3D Plots",n,key,8,11,my_3d.RenderType,
	              10,3*DCURY+DCURY+8);
 if(ch==27)return;
  if(ch=='p'){ ps_3d(); ping(); return;}
   
 for(i=0;i<8;i++)if(ch==key[i])my_3d.RenderType=i;


 get_3d_params();
 check_3d_win();
}

check_3d_win()
{
 if(my_3d.ZMin>=my_3d.ZMax){
	my_3d.ZMin=-fabs(my_3d.ZMin)-.1;
	my_3d.ZMax=-my_3d.ZMin;
 }
	my_3d.CMax=my_3d.ZMax;
	my_3d.CMin=my_3d.ZMin;
}
check_2d_win(p2d)
TWO_D_WIN *p2d;
{
 if(p2d->xlo>=p2d->xhi){
 	p2d->xlo=-fabs(p2d->xlo)-.1;
 	p2d->xhi=-p2d->xlo;
 }
 if(p2d->ylo>=p2d->yhi){
 	p2d->ylo=-fabs(p2d->ylo)-.1;
 	p2d->yhi=-p2d->ylo;
 }
}
 
window_2d(p2d,text)
     TWO_D_WIN *p2d;
     char *text;
     
{
 static char *n[]={"Horz. Min","Horz. Max","Vert. Min","Vert. Max"};
 char v[4][MAX_LEN_SBOX];
 int status,i;
 setfloat(0,v,p2d->xlo);
 setfloat(1,v,p2d->xhi);
 setfloat(2,v,p2d->ylo);
 setfloat(3,v,p2d->yhi);
 status=do_string_box(4,4,1,text,n,v,31);
 if(status==0)return;
  getfloat(0,v,&(p2d->xlo));
 getfloat(1,v,&(p2d->xhi));
 getfloat(2,v,&(p2d->ylo));
 getfloat(3,v,&(p2d->yhi));
 check_2d_win(p2d);
}


 
com_window()
{
 Window temp=main_win;
 static char *n[]={"Fit 3d","fIt 2d","Zoom in","zOom out","Win 2D"};
 static char key[]="fizow";
 char ch;
 ch=(char)pop_up_list(&temp,"Window",n,key,5,9,0,10,3*DCURY+7*(DCURY+8));
 switch(ch){
	case 'f': fit_3d();
 	check_3d_win();
	  redraw_3d();
	break;
	case 'i':
	  redraw_2d(1);
	  redraw_2d(0);
		break;
	case 'z':
		break;
	case 'o':
		break;
	case 'w':
		window_2d(&(my_2d.p2d),"2d Window");
	        redraw_2d(0);
		break;
 }
}

fit_3d()
{
 int off=0,type,reclen;
 int i,ng1=CURRENT_GRID+1;
 int j,joff;
 float big,little,z;
 type=get_offset(PlotVar,&off,&reclen);
 if(type==1||type==3){
 big=DataBuffer[off];
 little=big;
 for(j=0;j<BufIndex;j++){
	joff=off+j*reclen;
	for(i=0;i<ng1;i++){
	z=DataBuffer[i+joff];
	if(little>z)little=z;
	if(big<z)big=z;
	}
 }
 if(little==big){
 	if(little==0.0)little=big-.1;
	else little=big-.01*fabs(big);
 }
 my_3d.ZMin=little;
 my_3d.ZMax=big;
 }
}



com_continue()
{
 double length=0.0;
 int done,len,jstart=0;
 double t=TEnd;
 done=new_float("How long: ",&length);
 len=(int)(length/TFinal);
 integrate_em(&t,MyData,&jstart,len,1);
}
com_redraw()
{
 Window temp=main_win;
 int off;
 static char *n[]={"All","3D","2D","Turing"};
 static char key[]="a32t";
 char ch;
 ch=(char)pop_up_list(&temp,"Redraw",n,key,4,8,0,10,3*DCURY+5*(DCURY+8));
 switch(ch){
 case 'a':
   redraw_2d(0);
   redraw_3d();
   redraw_turing();
   break;
 case '3':
   redraw_3d();
   break;
 case '2':
   redraw_2d(0);
   break;
 case 't':
   redraw_turing();
   break;
 }
   
}

save_2d(fp)
     FILE *fp;
{
  int i,np,nx;
  int *var1,*var2,*it1,*it2,xlo,xhi,nplots;
  int *ix1,*ix2,tlo,thi;
  int *time,*var,*x;
  int type1,type2,in1,in2;
  float x1,y1;
switch(my_2d.p2d.axis){
 	case NO2D:
		return;
	case XPP:
		var1=my_2d.var1;
		var2=my_2d.var2;
		it1=my_2d.ix;
		it2=my_2d.it;
		xlo=0;
		xhi=CURRENT_GRID+1;
		nplots=my_2d.nplots;
		for(np=0;np<nplots;np++){
		    if(var_to_type(var1[np],&in1,&type1))return;
		    if(var_to_type(var2[np],&in2,&type2))return;
		  }
		for(i=0;i<xhi;i++){
		  for(np=0;np<nplots;np++){
		    var_to_type(var1[np],&in1,&type1);
		    var_to_type(var2[np],&in2,&type2);
		    qwik_data(in1,type1,xlo+i,it1[np],&x1);
		    qwik_data(in2,type2,xlo+i,it2[np],&y1);
		    fprintf(fp,"%f %f ",x1,y1);
		  }
		  fprintf(fp,"\n");
		}
		 fclose(fp);
		break;
	case TPP:
		var1=my_2d.var1;
		var2=my_2d.var2;
		ix1=my_2d.ix;
		ix2=my_2d.it;
		tlo=0;
		thi=BufIndex;
		nplots=my_2d.nplots;
		for(np=0;np<nplots;np++){
		    if(var_to_type(var1[np],&in1,&type1))return;
		    if(var_to_type(var2[np],&in2,&type2))return;
		  }
		for(i=0;i<thi;i++){
		  for(np=0;np<nplots;np++){
		    var_to_type(var1[np],&in1,&type1);
		    var_to_type(var2[np],&in2,&type2);
		    qwik_data(in1,type1,ix1[np],tlo+i,&x1);
		    qwik_data(in2,type2,ix2[np],tlo+i,&y1);
		    fprintf(fp,"%f %f ",x1,y1);
		  }
		  fprintf(fp,"\n");
		}
		 fclose(fp);
		break;

	case UVSX:
		var=my_2d.var1;
		time=my_2d.ix;
		xlo=0;
		xhi=CURRENT_GRID+1;
		nx=xhi-xlo;
		nplots=my_2d.nplots;
		
		for(np=0;np<nplots;np++)
		if(var_to_type(var[np],&in1,&type1))return;
		for(i=0;i<nx;i++){
		  x1=i*CURRENT_H;
		  fprintf(fp,"%f ",x1);
		  for(np=0;np<nplots;np++){
		    var_to_type(var[np],&in1,&type1);
		     qwik_data(in1,type1,xlo+i,time[np],&y1);
		    fprintf(fp,"%f ",y1);
		   }
		  fprintf(fp,"\n");
		}
		fclose(fp);
		break;
	case UVST:
		var=my_2d.var1;
		x=my_2d.ix;
		tlo=0;
		thi=BufIndex;
		nx=thi-tlo;
		nplots=my_2d.nplots;
		x1=0.0;
		for(np=0;np<nplots;np++)
		if(var_to_type(var[np],&in1,&type1))return;
		for(i=0;i<nx;i++){
		  x1=i*TFinal+tlo*TFinal+Transient;
		  fprintf(fp,"%f ",x1);
		  for(np=0;np<nplots;np++){
                     var_to_type(var[np],&in1,&type1); 
		     qwik_data(in1,type1,x[np],tlo+i,&y1);
		    fprintf(fp,"%f ",y1);
		   }
		  fprintf(fp,"\n");
		}
		fclose(fp);
		break;
	      }
 
}

save_3d(fp)
     FILE *fp;
{
 int off=0,reclen;
  int cg1=CURRENT_GRID+1;
  int type;
  int flag=0;
   int xskip=1,yskip=1;
  new_int("X Skip",&xskip);
 new_int("Y Skip", &yskip);
 new_int("Format -- gnuplot(0) xplot(1)",&flag);
 if(xskip<=0)xskip=1;
 if(yskip<=0)yskip=1;
  type=get_offset(PlotVar,&off,&reclen);
  if(type==1||type==3){
   set_tot(off,reclen);
   data_3d(DataBuffer,&my_3d,cg1,BufIndex,fp,xskip,yskip,flag); 
  }
}


redraw_3d()
{
  int off=0,reclen;
  int cg1=CURRENT_GRID+1;
  int type;
  type=get_offset(PlotVar,&off,&reclen);
  if(type==1||type==3){
   set_tot(off,reclen);
   plot_3d(DataBuffer,&my_3d,cg1,BufIndex); 
  }
}
ps_3d()
{
  int off=0,reclen;
  int cg1=CURRENT_GRID+1;
  int type;
  type=get_offset(PlotVar,&off,&reclen);
  if(type==1||type==3){
   set_tot(off,reclen);
   plot_ps_3d(DataBuffer,&my_3d,cg1,BufIndex); 
  }
}
com_erase()
{
 Window temp=main_win;
 static char *n[]={"Both","3D","2D"};
 static char key[]="b32";
 char ch;
 ch=(char)pop_up_list(&temp,"Erase",n,key,3,7,0,10,3*DCURY+7*(DCURY+8));
	if(ch=='b'||ch=='3')XClearWindow(display,my_3d.w);
	if(ch=='b'||ch=='2')XClearWindow(display,my_2d.p2d.w);

}

redraw_turing()
{
 int i;
 if(turing.exist==0)return;
 get_win_area(&(turing.p2d));
 clear_window(turing.p2d.w);
 t_axis(turing.p2d,0);
 for(i=0;i<(turing.nmodes-1);i++){
   line((float)i,(float)turing.real[i],(float)(i+1),(float)turing.real[i+1],
	turing.p2d,0);
   line((float)i,(float)turing.imag[i],(float)(i+1),(float)turing.imag[i+1],
	turing.p2d,16*21);
 }
   
 
} 



init_turing()
{
 turing.eps=1.e-5;
 turing.err=1.e-4;
 turing.grid=250;
 turing.nmodes=10;
 turing.half_flag=0;
 turing.maxit=50;
 turing.exist=0;
 turing.p2d.xlo=0;
 turing.p2d.xhi=10;
 turing.p2d.ylo=-5;
 turing.p2d.yhi=5;
}
 
save_turing()
{
FILE *fp;
int i;
 int ok;
 char filename[256];
 filename[0]=0;
 if(turing.exist==0)return;
 new_string("File to write: ",filename);
 if(strlen(filename)==0)return;
 open_file(&fp,filename,&ok,main_win,WRITEM);
 if(ok==1){
   for(i=0;i<turing.nmodes;i++)
     fprintf(fp,"%d %f %f \n",i,turing.real[i],turing.imag[i]);
 }
 fclose(fp);
}

com_turing()
{
  static char *n[]={"Go","Window","Fit","Redraw","Save","Params","sUmmarize"};
  static char key[]="gwfrspu";
  char ch;
  Window temp=main_win;
  ch=(char)pop_up_list(&temp,"Turing",n,key,7,8,0,10,3*DCURY+7*(DCURY+8));
  switch(ch){
  case 'g': 
    compute_turing(); 
     break;
  case 'w':
    window_2d(&(turing.p2d),"Turing Window");
    redraw_turing();
    break;
  case 'f':
    fit_turing();
    redraw_turing();
    break;
  case 'r':
    redraw_turing();
    break;
  case 's':
    save_turing();
    break;
  case 'p':
    get_turing_params();
    break;
  case 'u':
    summarize_tur();
    break;
  }
}

get_turing_params()
{
  static char *n[]={"Modes","Grid","HalfWave","Max Iter",
		      "Epsilon","Error"};
 
 char v[6][MAX_LEN_SBOX];
 int i,status,ip,type,off,rl;
  setint(0,v,turing.nmodes);
  setint(1,v,turing.grid);
  setfloat(5,v,turing.err);
    setint(2,v,turing.half_flag);
  setint(3,v,turing.maxit);
  setfloat(4,v,turing.eps);
  status=do_string_box(6,3,2,"Turing Params",n,v,31);
 if(status==0)return;
  getint(0,v,&turing.nmodes);
  getint(1,v,&turing.grid);
  getfloat(5,v,&turing.err);
  getint(2,v,&turing.half_flag);
  getint(3,v,&turing.maxit);
  getfloat(4,v,&turing.eps);
	
}

fit_turing()
{
  int i;
  double z1,z2,tr,ti;
  if(turing.exist==0)return;
  z1=turing.real[0];
  z2=z1;
  for(i=0;i<turing.nmodes;i++){
    if((tr=turing.real[i])>z2)z2=tr;
    if(tr<z1)z1=tr;
    if((ti=turing.imag[i])>z2)z2=ti;
    if(ti<z1)z1=ti;
  }
  turing.p2d.xlo=0;
  turing.p2d.xhi=turing.nmodes;
  turing.p2d.ylo=z1;
  turing.p2d.yhi=z2;
  check_2d_win(&(turing.p2d));
}
  
    
compute_turing()
{
 int ier,iw;
 double *work;
 double xeq[40];
 int i;
 int n=NCTM+NSCALAR;
 get_work_size(&iw,turing.nmodes);
 work=(double *)malloc(iw*sizeof(double));
 for(i=0;i<n;i++)xeq[i]=MyData[i];
 turing_stab(xeq,work,turing.nmodes,turing.grid,turing.half_flag,
	     turing.maxit,turing.err,&ier,turing.eps,
	     turing.real,turing.imag);
 if(ier==0){
   if(!turing.exist){
     make_turing_window(&(turing.p2d.w));
     turing.exist=1;
   }
   redraw_turing();
   for(i=0;i<n;i++)turing.x[i]=xeq[i];
   summarize_tur();
 }
 free(work);
}   

summarize_tur()
     
{
  int i,j;
  char *mystring[50];
  double max,tr;
  int jm;
  int n=NCTM+NSCALAR;
  if(turing.exist==0)return;
  for(i=0;i<(n+5);i++)mystring[i]=(char *)malloc(80);
  sprintf(mystring[0],"Equilibria:");
  for(i=0;i<NSCALAR;i++)sprintf(mystring[i+1],"%s=%g",uvar_names[i],turing.x[i]);
  for(i=0;i<NCTM;i++)sprintf(mystring[i+1+NSCALAR],
			     "%s=%g",ucvar_names[i],turing.x[i+NSCALAR]);
  sprintf(mystring[n+1],"  ");
  max=turing.real[0];
  jm=0;
  for(j=0;j<turing.nmodes;j++){
    if((tr=turing.real[j])>max){
      max=tr;
      jm=j;
    }
  }
  sprintf(mystring[n+2],"KMAX = %d, ev= %g + i%g",jm,turing.real[jm],
	  turing.imag[jm]);
  respond_box(main_win,0,0,"OK",mystring,n+3);
  for(i=0;i<(n+5);i++)free(mystring[i]);
}



com_file()
{
  Window temp=main_win;
  static char *n[]={"Save 3d","sAve 2d","save Turing",
		      "Write set","Read set","Info", "save Ctm","Quit"};
  static char key[]="satwricq";
  char ch;
  ch=(char)pop_up_list(&temp,"File",n,key,8,14,0,10,3*DCURY+4*(DCURY+8));
  switch(ch){
  case 's':
    write_data(1); ping();
    break;
  case 'a':
    write_data(2); ping();
    break;
  case 't':
    save_turing(); ping();
    break;
   case 'w': write_file(); ping();
    break;
  case 'r': read_file(); ping();
    break;
  case 'i': print_em();ping();
    break;
  case 'c': save_ctm(); ping();
    break;
  case 'q': 
    if(yes_no_box())bye_bye();
    break;
  }
}

get_3d_params()
{
 static char *n[]={"Zmax","Zmin","Alpha","Zfrac","Variable","NContour"};
 
 char v[6][MAX_LEN_SBOX];
 int i,status,ip,type,off,rl;
  setfloat(0,v,my_3d.ZMax);
  setfloat(1,v,my_3d.ZMin);
  setint(5,v,my_3d.NContours);
    setfloat(2,v,my_3d.Alpha);
  setfloat(3,v,my_3d.ZFraction);
  strcpy(v[4],plotvarstring);
  status=do_string_box(6,2,3,"3D Params",n,v,31);
 if(status==0)return;
  getfloat(0,v,&my_3d.ZMax);
  getfloat(1,v,&my_3d.ZMin);
  getint(5,v,&my_3d.NContours);
    getfloat(2,v,&my_3d.Alpha);
  getfloat(3,v,&my_3d.ZFraction);
	strcpy(plotvarstring,v[4]);
 get_plot_name(plotvarstring,&ip);
	type=get_offset(ip,&off,&rl);
	if(type==1||type==3)PlotVar=ip;
	
}
 
setfloat(i,s,v)
double v;
int i;
char s[][MAX_LEN_SBOX];
{
 sprintf(s[i],"%.16g",v);
} 

setint(i,s,v)
int v;
int i;
char s[][MAX_LEN_SBOX];
{
 sprintf(s[i],"%d",v);
}

getfloat(i,s,v)
double *v;
int i;
char s[][MAX_LEN_SBOX];
{
 *v=atof(s[i]);
} 

getint(i,s,v)
int *v;
int i;
char s[][MAX_LEN_SBOX];
{
 *v=atoi(s[i]);
}

com_numerics()
{
 static char *m[]={"Euler","BackEul","Gear","ModEul"};
 static char key[]="ebgm";
 static char *n[]={"Dt","Tout","Transient","TStart",
 	           "Epsilon","Tol","DtMin","DtMax",
		     "Maxiter","Jacuse","Nout","Weight0","Grid","Bound",
		      "Length","Seed"};
char v[16][MAX_LEN_SBOX];
 Window temp=main_win;
 int status,i;
 char ans;
 double old_weight=WEIGHT_0;
 double old_length=DomainSize;
 int old_grid=CURRENT_GRID;
 char ch;
 ch=(char)pop_up_list(&temp,"Method",m,key,4,8,Method,10,3*DCURY+4*(DCURY+8));
 for(i=0;i<4;i++)if(ch==key[i])Method=i;
 setfloat(0,v,DeltaT);
 setfloat(1,v,TFinal);
 setfloat(2,v,Transient);
 setfloat(3,v,TStart);
 setfloat(4,v,Epsilon);
 setfloat(5,v,Tolerance);
 setfloat(6,v,DtMin);
 setfloat(7,v,DtMax);
 setint(8,v,MaxIter);
 setint(9,v,MaxJac);
 setint(10,v,Nout);
 setfloat(11,v,WEIGHT_0);
 setint(12,v,old_grid);
 setfloat(13,v,BOUND);
 setfloat(14,v,old_length);
 setint(15,v,RandSeed);
 status=do_string_box(16,6,3,"Numerics",n,v,31);
 if(status==0)return;
  getfloat(0,v,&DeltaT);
 getfloat(1,v,&TFinal);
 getfloat(2,v,&Transient);
 getfloat(3,v,&TStart);
 getfloat(4,v,&Epsilon);
 getfloat(5,v,&Tolerance);
 getfloat(6,v,&DtMin);
 getfloat(7,v,&DtMax);
 getint(8,v,&MaxIter);
 getint(9,v,&MaxJac);
 getint(10,v,&Nout);
 getint(15,v,&RandSeed);
 getfloat(11,v,&WEIGHT_0);
 getint(12,v,&old_grid);
 getfloat(13,v,&BOUND);
 if(BOUND<=0.0)BOUND=100;
 getfloat(14,v,&old_length);
 if(old_grid<=0)old_grid=CURRENT_GRID;
 if(WEIGHT_0!=old_weight&&old_grid==CURRENT_GRID)init_cpars();
 if(old_grid!=CURRENT_GRID){
   ans=(char)two_choice("Yes","No","! REALLY CHANGE GRID?","yn",0,0,main_win);
   if(ans=='y')realloc_all(old_grid);
 }
 if(old_length>0.0){
   if(old_length!=DomainSize){
     DomainSize=old_length;
     reinit_all();
   }
 }
}
  
   
 

chck_special(special)
int *special;
{
 int i,type;
 static char *n[]={"nOne","Normal","Periodize","pErnorm","2dnorm","Read file"};
 static char key[]="onpe2r";
 Window temp=main_win;
 char ch;
   ch=(char)pop_up_list(&temp,"Special",n,key,6,11,*special,10,3*DCURY+8*(DCURY+8));
  type=0;
  for(i=0;i<6;i++)if(ch==key[i])*special=i;

}

com_parameter()
{
 chg_par();
 if(init_cpars())err_msg("Bad CTM parameter - set to 0");
}

get_x_pp(in)
     int in;
{

 static char *n[]={"Horz. Var","Vert. Var", "Horz. T", "Vert. T",
		   "Color", "Style"};
 char values[6][MAX_LEN_SBOX];
 int status,i,j;
 double z;
 set_plot_name(values[0],my_2d.var1[in]);
 set_plot_name(values[1],my_2d.var2[in]);
 setfloat(2,values,i_to_t(my_2d.ix[in]));
 setfloat(3,values,i_to_t(my_2d.it[in]));
 setint(4,values,my_2d.ls[in]/16);
 setint(5,values,my_2d.ls[in]%16);
 status=do_string_box(6,4,2,"Space PhsPln",n,values,31);
 if(status==0)return;
 get_plot_name(values[0],&(my_2d.var1[in]));
 get_plot_name(values[1],&(my_2d.var2[in]));
 getfloat(2,values,&z);
 my_2d.ix[in]=t_to_i(z);
 getfloat(3,values,&z);
 my_2d.it[in]=t_to_i(z);
 getint(4,values,&i);	
 getint(5,values,&j);
 my_2d.ls[in]=j+16*i;
}

get_t_pp(in)
     int in;
{

 static char *n[]={"Horz. Var","Vert. Var", "Horz. X", "Vert. X",
		   "Color", "Style"};
 char values[6][MAX_LEN_SBOX];
 int status,i,j;
 double z;
 set_plot_name(values[0],my_2d.var1[in]);
 set_plot_name(values[1],my_2d.var2[in]);
 setfloat(2,values,i_to_x(my_2d.ix[in]));
 setfloat(3,values,i_to_x(my_2d.it[in]));
 setint(4,values,my_2d.ls[in]/16);
 setint(5,values,my_2d.ls[in]%16);
 status=do_string_box(6,4,2,"Time PhsPln",n,values,31);
 if(status==0)return;
 get_plot_name(values[0],&(my_2d.var1[in]));
 get_plot_name(values[1],&(my_2d.var2[in]));
 getfloat(2,values,&z);
 my_2d.ix[in]=x_to_i(z);
 getfloat(3,values,&z);
 my_2d.it[in]=x_to_i(z);
 getint(4,values,&i);	
 getint(5,values,&j);
 my_2d.ls[in]=j+16*i;
}

get_u_vs_t(in)
     int in;
{

 static char *n[]={"Var", "X", "Color", "Style"};
 char values[4][MAX_LEN_SBOX];
 int status,i,j;
 double z;
 set_plot_name(values[0],my_2d.var1[in]);
 setfloat(1,values,i_to_x(my_2d.ix[in]));
  setint(2,values,my_2d.ls[in]/16);
 setint(3,values,my_2d.ls[in]%16);
 status=do_string_box(4,4,1,"U vs T",n,values,31); 
 if(status==0)return;
 get_plot_name(values[0],&(my_2d.var1[in]));
  getfloat(1,values,&z);
 my_2d.ix[in]=x_to_i(z);
 getint(2,values,&i);	
 getint(3,values,&j);
 my_2d.ls[in]=j+16*i;
}

get_u_vs_x(in)
     int in;
{

 static char *n[]={"Var", "T", "Color", "Style"};
 char values[4][MAX_LEN_SBOX];
 int status,i,j;
 double z;
 set_plot_name(values[0],my_2d.var1[in]);
 setfloat(1,values,i_to_t(my_2d.ix[in]));
  setint(2,values,my_2d.ls[in]/16);
 setint(3,values,my_2d.ls[in]%16);
 status=do_string_box(4,4,1,"U vs X",n,values,31); 
 if(status==0)return;
 get_plot_name(values[0],&(my_2d.var1[in]));
  getfloat(1,values,&z);
 my_2d.ix[in]=t_to_i(z);
 getint(2,values,&i);	
 getint(3,values,&j);
 my_2d.ls[in]=j+16*i;
} 








