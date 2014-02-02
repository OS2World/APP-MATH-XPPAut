#include <math.h>
#include <stdio.h>
#include "ctm.h"
#define EPS 1.e-4
#define MAXCRHS 20
#define MAXCFIX 20
#define MAXSRHS 20
#define MAXSFIX 20
#define MAXCAUX 10
#define MAXSAUX 10
double evaluate();
double get_ivar();
extern CONTINUUM my_ctm[MAXCTM];
extern BDRYVAL my_bval[MAXCTM];

/*  These contain the compiled versions of the RHSs   */

extern int *FixCtm[MAXCFIX],*FixScal[MAXSFIX],*RhsScal[MAXSRHS],*RhsCtm[MAXCRHS]; 
extern int *AuxCtm[MAXCAUX],*AuxScal[MAXSAUX]; 

extern int I_fixctm[MAXCFIX],I_varctm[MAXCRHS],I_auxctm[MAXCAUX],
	   I_bdryvar[MAXCRHS];
extern int FastFlag;

extern int NCTM, NSCALAR, NBVAL,CURRENT_GRID,FIX_CTM,FIX_SCAL,AUX_SCAL,AUX_CTM,
CURRENT_INDEX;

extern double SpaceValue[10],CURRENT_SPACE,CURRENT_H;

/*  This prepares all the right hand sides for the ode solver
    The total number of equations is given by:
    
    NSCALAR + NCTM*(CURRENT_GRID+1)

  The order of the right-hand sides is :
  
   scalar1                
   scalar2
     .
     .
     .
   scalarN
   ctm1 0
   ctm2 0
   ...
   ctmN 0
    .
    .
    .
   ctm1 L
   ctm2 L
   ...
   ctmN L

   Scalar storage:
   1,..,NSCALAR,NSCALAR+1,...,NSCALAR+FIX_SCAL <-- in Variable Array

  
 ***********************************************************************/
extern int NRHS;
  
u_to_y(y)
double *y;
{
 int i,j,j0,ng1=CURRENT_GRID+1;
 for(i=0;i<NSCALAR;i++)y[i]=get_ivar(i+1);
  for(j=0;j<ng1;j++){                         /* set ctm variables    */ 
    j0=j*NCTM+NSCALAR;
    for(i=0;i<NCTM;i++)y[i+j0]=my_ctm[I_varctm[i]].u[j];
  }  
}

y_to_u(y)
double *y;
{
 int i,j,j0,ng1=CURRENT_GRID+1,fixs0=NSCALAR+1;;
 
 clear_fft();
  for(i=0;i<NSCALAR;i++)set_ivar(i+1,y[i]);   /* set scalar variables */
     
  for(j=0;j<ng1;j++){                         /* set ctm variables    */ 
    j0=j*NCTM+NSCALAR;
    for(i=0;i<NCTM;i++)my_ctm[I_varctm[i]].u[j]=y[i+j0];
  }  
  
  for(i=0;i<NBVAL;i++)make_derivs(i);         /* compute derivatives */
 if(FastFlag==0){                
   if(FIX_CTM>0){
     for(i=0;i<FIX_CTM;i++){
       for(j=0;j<ng1;j++){
	 CURRENT_INDEX=j;
	 SpaceValue[0]=j*CURRENT_H;
	 my_ctm[I_fixctm[i]].u[j]=evaluate(FixCtm[i]);
       }
     }
   }
 }
 
 else {
   /* Fast way  */   
   for(i=0;i<FIX_CTM;i++)
     v_evaluate(FixCtm[i],my_ctm[I_fixctm[i]].u,ng1);
 }

  for(i=0;i<FIX_SCAL;i++)                     /*  compute fixed scalars  */
    set_ivar(i+fixs0,evaluate(FixScal[i]));

}

get_aux(y,z,t)
double *y,*z,t;
{
 int i,j,j0,ng1=CURRENT_GRID+1;
 set_ivar(0,t);
 y_to_u(y);
 for(i=0;i<AUX_SCAL;i++)z[i]=evaluate(AuxScal[i]);
 if(AUX_CTM==0)return;
 for(i=0;i<AUX_CTM;i++){
   for(j=0;j<ng1;j++){
     j0=j*AUX_CTM+AUX_SCAL;
     CURRENT_INDEX=j;
     SpaceValue[0]=CURRENT_H*j;
     z[i+j0]=evaluate(AuxCtm[i]);
   }
 }
 
}

my_rhs(t,y,ydot,neq)
     double t,*y,*ydot;
     int neq;
{
  int i,j;
  int fixs0=NSCALAR+1;
  int de0=fixs0+FIX_SCAL;
  int ng1=CURRENT_GRID+1;
  int j0,j1; 
  double *vec;
  vec=(double *)malloc(ng1*sizeof(double));
  set_ivar(0,t);                              /* set t  */
  clear_fft();
  for(i=0;i<NSCALAR;i++)set_ivar(i+1,y[i]);   /* set scalar variables */
     
  for(j=0;j<ng1;j++){                         /* set ctm variables    */ 
    j0=j*NCTM+NSCALAR;
    for(i=0;i<NCTM;i++)my_ctm[I_varctm[i]].u[j]=y[i+j0];
  }  
  
  for(i=0;i<NBVAL;i++)make_derivs(i);         /* compute derivatives */               
/*  Slow way   */
  if(FastFlag==0){  
  for(i=0;i<FIX_CTM;i++){              
    if(FIX_CTM>0){
      for(j=0;j<ng1;j++){
	CURRENT_INDEX=j;
       	SpaceValue[0]=j*CURRENT_H;
	my_ctm[I_fixctm[i]].u[j]=evaluate(FixCtm[i]);
      }
    }
  }
}
 
  else {
/* Fast way  */   
 for(i=0;i<FIX_CTM;i++)
   v_evaluate(FixCtm[i],my_ctm[I_fixctm[i]].u,ng1);
}

  for(i=0;i<FIX_SCAL;i++)                     /*  compute fixed scalars  */
    set_ivar(i+fixs0,evaluate(FixScal[i]));


 /*  Finally we can compute the right-hand sides !!   */
	   
	
  for(i=0;i<NSCALAR;i++)ydot[i]=evaluate(RhsScal[i]);  /* scalar rhs.. */

if(FastFlag==0){
/*   Slow way    */
  for(i=0;i<NCTM;i++){
    
    for(j=0;j<ng1;j++){
      CURRENT_INDEX=j;
      SpaceValue[0]=j*CURRENT_H;
      ydot[i+j*NCTM+NSCALAR]=evaluate(RhsCtm[i]);
    }
  }
}
else{

  /* Fast way  */  
  for(i=0;i<NCTM;i++){
    v_evaluate(RhsCtm[i],vec,ng1);
    for(j=0;j<ng1;j++)ydot[j*NCTM+NSCALAR+i]=vec[j];
  }
  
}
/* Now we have to fix up the fixed boundary condition problems */
for(i=0;i<NBVAL;i++)
{
 j=I_bdryvar[i];
 j0=j+NSCALAR;   /* The left-hand value   */
 j1=CURRENT_GRID*NCTM+j0;
/* printf(" j0=%d j1=%d i=%d \n",j0,j1,i); */
	do_dudt(i,t,&ydot[j0],&ydot[j1]);
}
 
NRHS++;
free(vec);
}   
  

























