#include <stdio.h>
#include <math.h>
#include "ctm.h"

/* output variables are:
   scalar0...
   ctm0 ...
   auxscal0...
   auxctm0...
 */

extern float *DataBuffer;
extern int BufIndex;
extern CONTINUUM my_ctm[MAXCTM];
extern int CURRENT_GRID,NCTM,NSCALAR,AUX_CTM,AUX_SCAL;
extern double CURRENT_H,WEIGHT_0,DomainSize;
extern int Method,Ml,Mr,PeriodFlag;
extern double DeltaT,TFinal,Transient;
double TStart,TEnd;
extern int Nout,PlotVar;
extern char OutFile[50];
int NRHS=0;
char this_file[100];
double *last_ic,*MyData;
int NODE;
int DataHere=0,OkToSolve=0;

extern double Tolerance,DtMin,DtMax,Epsilon;
extern int MaxJac,MaxIter;

load_the_file(argc,argv)
     int argc;
     char **argv;
{
  if(argc!=2){
	printf("xtc filename\n");
	exit(0);
 }
 
 WEIGHT_0=.5;
 strcpy(this_file,argv[1]); 

 load_eqn(this_file);

  NODE=NSCALAR+NCTM*(CURRENT_GRID+1);
  last_ic=(double *)malloc(sizeof(double)*NODE);
  MyData=(double *)malloc(sizeof(double)*NODE);
  u_to_y(last_ic);
  if(get_ics(1,MyData,TStart))exit(0);
  TEnd=TStart;
   open_data();
}

realloc_all(grid)
int grid;
{
 CURRENT_GRID=grid;
 NODE=NSCALAR+NCTM*(CURRENT_GRID+1);
 CURRENT_H=DomainSize/CURRENT_GRID;
 realloc_ctm(grid);

 if(init_cvars())
   err_msg("Illegal ICs - Set to 0");
 if(init_cpars())
   err_msg("Illegal CPAR - Set to 0");
 BufIndex=0;
 DataHere=0;
 last_ic=(double *)realloc((void *)last_ic,NODE*sizeof(double));
 MyData=(double *)realloc((void *)MyData,NODE*sizeof(double));
 u_to_y(last_ic);
 get_ics(1,MyData,TStart);
 realloc_data();
}
 
reinit_all() 
{
 CURRENT_H=DomainSize/CURRENT_GRID;
 if(init_cvars())
   err_msg("Illegal ICs - Set to 0");
 if(init_cpars())
   err_msg("Illegal CPAR - Set to 0");
 BufIndex=0;
 DataHere=0;
 u_to_y(last_ic);
 get_ics(1,MyData,TStart);
}
    
get_ics(it,y,t)
     int it;
     double *y,t;
{
  int i;
  OkToSolve=0;
  switch(it){
  case 0: /* New  */
    if(init_cvars()){
      err_msg("error in setting ICs -- bad formula?");
      return(1);
    }
/*	printf("init cvars done \n"); */
    for(i=0;i<NSCALAR;i++)set_ivar(i+1,last_ic[i]);
   u_to_y(last_ic);
/*    printf("u to y done \n"); */
    for(i=0;i<NODE;i++)y[i]=last_ic[i];
    break;
  case 1: /* Old */
    for(i=0;i<NODE;i++)y[i]=last_ic[i];
    break;
  case 2: /* Last */
    if(DataHere==0){
	err_msg("No old data..");
	return(1);
    }
    for(i=0;i<NODE;i++)last_ic[i]=y[i];
    break;
  case 3: /* Shift */
    if(DataHere==0){
	err_msg("No old data..");
	return(1);
    }
    for(i=0;i<NODE;i++)last_ic[i]=y[i];
    TStart=t;
    break;
  }
  OkToSolve=1;

  return(0); /* OK */
}
    
    

integrate_em(t,y,jstart,nlen,flag)
double *t,*y;
int *jstart,nlen,flag;
{
 int i,ii,j,ng1,node,wsize;
 int kflag,*iwork,jacsize,auxsize;
 int ndynamic,npts,off=0,jump;
 int naux_scal,ntotal,is,reclen,type;
 double *work,*error,*aux;
 double v;
 double err;
 ng1=CURRENT_GRID+1;
 node=NCTM*ng1+NSCALAR;
 auxsize=(AUX_SCAL+AUX_CTM*ng1);
 jacsize=node*(Ml+Mr+1)+2*node*Ml+Ml*Ml;
 wsize=22*node+jacsize+200;
  error=(double *)malloc(node*sizeof(double));
 work=(double *)malloc(wsize*sizeof(double));
 iwork=(int *)malloc(node*sizeof(int));
 if(auxsize>0)aux=(double *)malloc(auxsize*sizeof(double));
 ndynamic=NCTM+NSCALAR;
 npts=NCTM*ng1+NSCALAR;
 naux_scal=ndynamic+AUX_SCAL;
 ntotal=naux_scal+AUX_CTM;
 type=get_offset(PlotVar,&off,&reclen);
/*  printf("off=%d rl=%d type=%d \n",off,reclen,type);  */
  if(Transient>0.0&&flag==0){
 switch(Method){
 case 0:
   kflag=euler(t,*t+Transient,DeltaT,y,work,npts);
   break;
 case 1:
 kflag=backeul(t,*t+Transient,DeltaT,y,Tolerance,Epsilon,MaxIter,
 	        MaxJac,work,npts,Ml,Mr,PeriodFlag);
   break;
 case 2:
   bandgear(npts,t,*t+Transient,y,DtMin,DtMax,Tolerance,Epsilon,2,
	    error,&kflag,jstart,work,iwork,Ml,Mr,PeriodFlag);
   break;
 case 3:
	kflag=mod_euler(t,*t+Transient,DeltaT,y,work,npts);
	break;
 }
}
 if(auxsize>0)get_aux(y,aux,*t);
 is=save_data(*t,y,aux);
 if(is!=1){
  jump=reclen*(BufIndex-1)+off;
if(type==1||type==3)one_3d_line(0,nlen+1,&DataBuffer[jump],ng1);
 }
 for(ii=0;ii<nlen;ii++) {

 switch(Method){
 case 0:
   kflag=euler(t,*t+TFinal,DeltaT,y,work,npts);
   break;
 case 1:
 kflag=backeul(t,*t+TFinal,DeltaT,y,Tolerance,Epsilon,
 	MaxIter,MaxJac,work,npts,Ml,Mr,PeriodFlag);
   break;
 case 2:
   bandgear(npts,t,*t+TFinal,y,DtMin,DtMax,Tolerance,Epsilon,2,
	    error,&kflag,jstart,work,iwork,Ml,Mr,PeriodFlag);
   break;
 case 3:
  kflag=mod_euler(t,*t+TFinal,DeltaT,y,work,npts);
	break;

 }

 if(auxsize>0)get_aux(y,aux,*t);
 
  is=save_data(*t,y,aux);
/* printf("is = %d  BL=%d \n",is,BufIndex); */
 if(is==1)break;
 
 if(my_abort()){
 	err_msg("User Break");
 	break;
 }
jump=reclen*(BufIndex-1)+off;
if(type==1||type==3)one_3d_line(ii+1,nlen+1,&DataBuffer[jump],ng1);
/* printf("Bl=%d jump=%d \n",BufIndex,jump); */
}
 TEnd=*t;
 DataHere=1;
 free(work);
 free(error);
 free(iwork);
 if(auxsize>0) free(aux);
 ping();

 }










