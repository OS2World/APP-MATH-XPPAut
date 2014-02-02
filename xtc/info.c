#include <math.h>
#include <stdio.h>
#include <X11/Xlib.h>
#define MAX_LEN_SBOX 25
#define MAXPLOTS 30
#define NO2D 0
#define XPP 1
#define TPP 2
#define UVSX 3
#define UVST 4 
#define MAXCRHS 20
#define MAXCFIX 20
#define MAXSRHS 20
#define MAXSFIX 20
#define MAXCPAR 20
#define MAXCAUX 10
#define MAXSAUX 10
#define MAXUPAR 100
#define MAXALLRHS 80
#define WRITEM 1
#define READEM 0
extern char ucvar_names[MAXCRHS][10], this_file[100],
ucpar_names[MAXCPAR][10],
 ucaux_names[MAXCAUX][10],
 uaux_names[MAXSAUX][10],
 uvar_names[MAXSRHS][10],
 upar_names[MAXUPAR][10];

extern Window main_win,menu_pop;
extern Display *display;
extern int DCURY,BufSize,BufIndex;
extern float *DataBuffer;
extern int Method,FastFlag,Nout,MaxIter,MaxJac,CURRENT_GRID;
extern int NUPAR,NSCALAR,NCTM,NUCPAR;
extern double DomainSize,DeltaT,TFinal,Transient,TStart,TEnd;
extern double Tolerance,Epsilon,DtMin,DtMax,WEIGHT_0;
extern double CURRENT_H, *MyData;
extern char plotvarstring[20];
extern int PlotVar;
extern double *last_ic;
double atof();
extern char *EqnList[MAXALLRHS];
extern int NEqnList;
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
FILE *my_plot_file;
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

extern TWO_D_GRAPHS my_2d;
extern int RandSeed;

/*  The files contain the following info:
	NSCALAR
	NCTM
	NUPAR
	NUCPAR
-----------------------------------------------------------------------------
	METHOD
	DT
	TOUT
	TRANSIENT
	TSTART
	EPSILON
	TOL
	DTMIN
	DTMAX
	MAXITER
	JACUSE
	NOUT
	WEIGHT0
	GRID
--------------------------------------------------------------------------
	ZMIN
	ZMAX
	ALPHA
	ZFRACTION
	NCONTOURS
	RENDERTYPE
	VARIABLE
---------------------------------------------------------------------------
	HMIN
	HMAX
	VMIN
	VMAX
	PLOT_TYPE
	NPLOTS
        Hvar0  <---  Up to NPLOTS
    	VVar0
	FixH0
	FixV0
	Color0
	Style0
   	...
------------------------------------------------------------------------------
	SCALPAR0
	...
	CTMPAR0
	CTMPARTYPE0
	...
	SCALVAR0
	...
	CTMVAR0
	...
	
       last_ic <<<--- lots of numbers
*/

 open_file(fp,fil,ok,w,rorw)
 FILE **fp;
  char *fil;
  int *ok;
  int rorw;
  Window w;
{
 char ans;
 *ok=0;
 *fp=fopen(fil,"r");
 if(rorw==READEM){
		if(*fp==NULL){err_msg("Cannot open file to read");
		return;}
		else {
		*ok=1;
		return;
	}
 }
	if(*fp!=NULL){
		fclose(*fp); 
		ans=(char)two_choice("Yes","No",
		"File Exists! Overwrite?","yn",0,0,w);
		if(ans!='y')return;
		}	 

			*fp=fopen(fil,"w");
			if(*fp==NULL){
					err_msg("Cannot open file to write");
				      *ok=0;
				     }
		         else *ok=1;
			 return;
		    
  }
open_ps_file()
{
 int ok;
 char filename[256];
 filename[0]=0;
 new_string("Postscript File:",filename);
 if(strlen(filename)==0)return(0);
 open_file(&my_plot_file,filename,&ok,main_win,WRITEM);
 return(ok);
}

write_1d_file(v)
     double *v;
{
  FILE *fp;
  char filename[256];
  int i,j,ok;
  int npts=CURRENT_GRID;
  filename[0]=0;
  new_string("File to write: ",filename);
  if(strlen(filename)==0)return(0);
  open_file(&fp,filename,&ok,main_win,WRITEM);
  if(ok==1){
    fprintf(fp,"#%d\n",npts);
    for(i=0;i<=CURRENT_GRID;i++)
      fprintf(fp,"%lg\n",v[i]);
    fclose(fp);
  }
  return(ok);
}

write_2d_file(v)
     double *v;
{
  FILE *fp;
  char filename[256];
  int i,j,ok;
  int npts=CURRENT_GRID,ng1=CURRENT_GRID+1;
  filename[0]=0;
  new_string("File to write: ",filename);
  if(strlen(filename)==0)return(0);
  open_file(&fp,filename,&ok,main_win,WRITEM);
  if(ok==1){
    fprintf(fp,"#%d\n",npts);
    for(i=0;i<=CURRENT_GRID;i++){
      for(j=0;j<=CURRENT_GRID;j++){
	fprintf(fp,"%lg\n",v[i+j*ng1]);
      }
      fprintf(fp,"\n");
    }
    fclose(fp);
  }
    return(ok);
}

write_file()
{
 FILE *fp;
 int ok;
 char filename[256];
 filename[0]=0;
 new_string("File to write: ",filename);
 if(strlen(filename)==0)return(0);
 open_file(&fp,filename,&ok,main_win,WRITEM);
 if(ok==1)write_em(fp);
 fclose(fp);
 return(ok);
}

write_data(flag) /* 1=3d 2=2d */
     int flag;
{
 FILE *fp;
 int ok;
 char filename[256];
 filename[0]=0;
 new_string("File to save: ",filename);
 if(strlen(filename)==0)return;
 open_file(&fp,filename,&ok,main_win,WRITEM);
 if(ok!=1)return;
 if(flag==1)save_3d(fp);
 if(flag==2)save_2d(fp);
 fclose(fp);
}


read_file()
{
 FILE *fp;
 int ok;
 char filename[256];
 filename[0]=0;
 new_string("File to read: ",filename);
 if(strlen(filename)==0)return;
 open_file(&fp,filename,&ok,main_win,READEM);
 if(ok==1)read_em(fp);
 fclose(fp);
}

write_em(fp)
FILE *fp;
{
 char bob[256];
 int i,type;
 double z;
/* Basic parameters  */
 file_int(fp,&NSCALAR,WRITEM);
 file_int(fp,&NCTM,WRITEM);
 file_int(fp,&NUPAR,WRITEM);
 file_int(fp,&NUCPAR,WRITEM);
/* Numerical stuff   */
 file_int(fp,&Method,WRITEM);
 file_flt(fp,&DeltaT,WRITEM);
 file_flt(fp,&TFinal,WRITEM);
 file_flt(fp,&Transient,WRITEM);
 file_flt(fp,&TStart,WRITEM);
 file_flt(fp,&Tolerance,WRITEM);
 file_flt(fp,&Epsilon,WRITEM);
 file_flt(fp,&DtMin,WRITEM);
 file_flt(fp,&DtMax,WRITEM);
 file_flt(fp,&WEIGHT_0,WRITEM);
 file_int(fp,&MaxIter,WRITEM);
 file_int(fp,&MaxJac,WRITEM);
 file_int(fp,&Nout,WRITEM);
 file_int(fp,&CURRENT_GRID,WRITEM);
/* Threed D Parameters    */
 file_flt(fp,&my_3d.ZMin,WRITEM);
 file_flt(fp,&my_3d.ZMax,WRITEM);
  file_flt(fp,&my_3d.Alpha,WRITEM);
 file_flt(fp,&my_3d.ZFraction,WRITEM);
  file_int(fp,&my_3d.NContours,WRITEM);
  file_int(fp,&my_3d.RenderType,WRITEM);
  file_int(fp,&PlotVar,WRITEM);
/* Two-d Parameters    */
 file_int(fp,&my_2d.p2d.axis,WRITEM);
 file_int(fp,&my_2d.nplots,WRITEM);
 file_flt(fp,&my_2d.p2d.xlo,WRITEM);
 file_flt(fp,&my_2d.p2d.xhi,WRITEM);
 file_flt(fp,&my_2d.p2d.ylo,WRITEM);
 file_flt(fp,&my_2d.p2d.yhi,WRITEM);
 for(i=0;i<my_2d.nplots;i++){
  file_int(fp,&my_2d.var1[i],WRITEM);
  file_int(fp,&my_2d.var2[i],WRITEM);
  file_int(fp,&my_2d.ix[i],WRITEM);
  file_int(fp,&my_2d.it[i],WRITEM);
  file_int(fp,&my_2d.ls[i],WRITEM);
 }
 file_int(fp,&my_2d.tlo,WRITEM);
 file_int(fp,&my_2d.thi,WRITEM);
/* Initial Data   */
 for(i=0;i<NSCALAR;i++)
 file_flt(fp,&last_ic[i],WRITEM);
 for(i=0;i<NCTM;i++) {
 get_ctm_string(ucvar_names[i],bob,&type);
 file_string(fp,bob,WRITEM);
 file_int(fp,&type,WRITEM);
 }
 for(i=0;i<NUPAR;i++){
 get_val(upar_names[i],&z);
 file_flt(fp,&z,WRITEM);
 }
 for(i=0;i<NUCPAR;i++){
	get_ctm_string(ucpar_names[i],bob,&type);
	file_string(fp,bob,WRITEM);
	file_int(fp,&type,WRITEM);
 }
}
print_em()
{
  FILE *fp;
  char bob[256];
  int div,rem,j;  
  int i,type,ok;
  char filename[256];
  double z;
  filename[0]=0;
  new_string("File to write: ",filename);
  if(strlen(filename)==0)fp=stdout;
  else{
    open_file(&fp,filename,&ok,main_win,WRITEM);
    if(ok!=1)return;
  }
  fprintf(fp,"\n%s\n\n",this_file);
  fprintf(fp,"Dynamics and auxiliary functions...\n\n");
  for(i=0;i<NEqnList;i++)fprintf(fp,"%s\n",EqnList[i]);
  fprintf(fp,"\n\nInitial data...\n\n");
  for(i=0;i<NSCALAR;i++)fprintf(fp,"%s=%.16g\n",uvar_names[i],last_ic[i]);
  for(i=0;i<NCTM;i++) {
    get_ctm_string(ucvar_names[i],bob,&type);
    fprintf(fp,"%s(x)=%s\n",ucvar_names[i],bob);
  }
  fprintf(fp,"\n\nParameters...\n\n");
  div=NUPAR/4;
  rem=NUPAR%4;
  for(j=0;j<div;j++){
    for(i=0;i<4;i++)
      {
	get_val(upar_names[i+4*j],&z);
       fprintf(fp,"%s=%.16g   ",upar_names[i+4*j],z);
     }
    fprintf(fp,"\n");
  }
  for(i=0;i<rem;i++){
    get_val(upar_names[i+4*div],&z);
    fprintf(fp,"%s=%.16g   ",upar_names[i+4*div],z);
  }
fprintf(fp,"\n");
 for(i=0;i<NUCPAR;i++){
	get_ctm_string(ucpar_names[i],bob,&type);
	fprintf(fp,"%s(x)=%s\n",ucpar_names[i],bob);
      }
/* Numerical stuff   */
fprintf(fp,"\n\nNumerical stuff...\n\n");
 fprintf(fp,"Method=%d DeltaT=%g TFinal=%g Transient=%g TStart=%g\n",
	 Method,DeltaT,TFinal,Transient,TStart);
  fprintf(fp,"Tol=%g Epsilon=%g DtMin=%g DtMax=%g\n",
	  Tolerance,Epsilon,DtMin,DtMax);
  fprintf(fp,"Weight0=%g MaxIter=%d MaxJac=%d Nout=%d Grid=%d\n",
	  WEIGHT_0,MaxIter,MaxJac,Nout,CURRENT_GRID);
  fprintf(fp,"\n\n");
  if(strlen(filename)>0)fclose(fp);
}


read_em(fp)
FILE *fp;
{
 char bob[256];
 int i,type;
 int ns,nc,np,ncp;
 int old_grid;
 double z;
/* Basic parameters  */
 file_int(fp,&ns,0);
 file_int(fp,&nc,0);
 file_int(fp,&np,0);
 file_int(fp,&ncp,0);
if((ns!=NSCALAR)||(nc!=NCTM)||(np!=NUPAR)||(ncp!=NUCPAR)){
	err_msg("Not Compatible with this file !");
	return;
}
/* Numerical stuff   */
 file_int(fp,&Method,0);
 file_flt(fp,&DeltaT,0);
 file_flt(fp,&TFinal,0);
 file_flt(fp,&Transient,0);
 file_flt(fp,&TStart,0);
 file_flt(fp,&Tolerance,0);
 file_flt(fp,&Epsilon,0);
 file_flt(fp,&DtMin,0);
 file_flt(fp,&DtMax,0);
 file_flt(fp,&WEIGHT_0,0);
 file_int(fp,&MaxIter,0);
 file_int(fp,&MaxJac,0);
 file_int(fp,&Nout,0);
 file_int(fp,&old_grid,0);

/* Threed D Parameters    */
 file_flt(fp,&my_3d.ZMin,0);
 file_flt(fp,&my_3d.ZMax,0);
  file_flt(fp,&my_3d.Alpha,0);
 file_flt(fp,&my_3d.ZFraction,0);
  file_int(fp,&my_3d.NContours,0);
  file_int(fp,&my_3d.RenderType,0);
  file_int(fp,&PlotVar,0);
  check_3d_win();

/* Two-d Parameters    */
 file_int(fp,&my_2d.p2d.axis,0);
 file_int(fp,&my_2d.nplots,0);
 file_flt(fp,&my_2d.p2d.xlo,0);
 file_flt(fp,&my_2d.p2d.xhi,0);
 file_flt(fp,&my_2d.p2d.ylo,0);
 file_flt(fp,&my_2d.p2d.yhi,0);
 check_2d_win(&my_2d.p2d);
 for(i=0;i<my_2d.nplots;i++){
  file_int(fp,&my_2d.var1[i],0);
  file_int(fp,&my_2d.var2[i],0);
  file_int(fp,&my_2d.ix[i],0);
  file_int(fp,&my_2d.it[i],0);
  file_int(fp,&my_2d.ls[i],0);
 }
 file_int(fp,&my_2d.tlo,0);
 file_int(fp,&my_2d.thi,0);
/* Initial Data   */
 if(RandSeed>=0)
   srand48(RandSeed);
 for(i=0;i<NSCALAR;i++)
 file_flt(fp,&last_ic[i],0);
 for(i=0;i<NCTM;i++) {
 file_string(fp,bob,0);
 file_int(fp,&type,0);
 set_ctm_string(ucvar_names[i],bob,type);
 }
 for(i=0;i<NUPAR;i++){
 file_flt(fp,&z,0);
 set_val(upar_names[i],z);
 }
 for(i=0;i<NUCPAR;i++){
	file_string(fp,bob,0);
	file_int(fp,&type,0);
	set_ctm_string(ucpar_names[i],bob,type);
 }
 realloc_all(old_grid);
}

file_int(fp,i,f)
int *i,f;
FILE *fp;
{
 char bob[81];
 if(f==READEM){
   fgets(bob,80,fp);
   *i=atoi(bob);
 }
 else
 fprintf(fp,"%d\n",*i);
}

file_flt(fp,z,f)
int f;
FILE *fp;
double *z;
{
char bob[81];
 if(f==READEM){
   fgets(bob,80,fp);
   *z=atof(bob);
 }
 else
 fprintf(fp,"%.16g\n",*z); 
}


file_string(fp,s,f)
FILE *fp;
char *s;
int f;
{
 int i;
 if(f==READEM){
   fgets(s,512,fp);
   i=0;
   while(i<strlen(s)){
     if(s[i]=='\n')s[i]=0;
     i++;
   }
 }
 else 
   fprintf(fp,"%s\n",s);
}






