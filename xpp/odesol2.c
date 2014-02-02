/* Modified by Klaus Gebhardt, 1997 */

#include <math.h>
#include "xpplim.h"

int set_ivar(/* int ,double */);
int my_rhs(/* double, double *,double *,int */);
int (*rhs)(/* double t,double *y,double *ydot,int neq */);
int mod_euler(/* double *,double *,double,int,int,int *,double * */);
int rung_kut(/* double *,double *,double,int,int,int *,double * */);
int adams(/* double *,double *,double,int,int, int*,double * */);
int abmpc(/* double *,double *,double,int */);


double coefp[]={ 6.875/3.00,-7.375/3.00,4.625/3.00,-.375},
       coefc[]={ .375,2.375/3.00,-.625/3.00,0.125/3.00 };
double *y_s[4],*y_p[4],*ypred;

extern int MaxEulIter;
extern double EulTol,NEWT_ERR;
double amax();
extern int NFlags;

/*   DISCRETE    */

discrete(y,tim,dt,nt,neq,istart,work)
double *y,*tim,dt,*work;
int nt,neq,*istart;
{
  int i,j;
  double t=*tim;
  for(i=0;i<nt;i++)
  {
   set_wieners(dt,y,t);
   rhs(t,y,work,neq);
   t=t+dt;
   for(j=0;j<neq;j++)y[j]=work[j];
  }
  *tim=t;
  return(0);
}

/* Backward Euler  */

bak_euler(y,tim,dt,nt,neq,istart,work)
double *y,*tim,dt,*work;
int nt,neq,*istart;
{
 int i,j;
  double *jac,*yg,*yp,*yp2,*ytemp,*errvec;
  yp=work;
  yg=yp+neq;
  ytemp=yg+neq;
  errvec=ytemp+neq;
  yp2=errvec+neq;
  jac=yp2+neq;
  if(NFlags==0){
    for(i=0;i<nt;i++)
      {
	
	if((j=one_bak_step(y,tim,dt,neq,yg,yp,yp2,ytemp,errvec,jac,istart))!=0)
	  return(j);
	stor_delay(y);
      }
    return(0);
  }
 for(i=0;i<nt;i++)
      {
	
	if((j=one_flag_step_backeul(y,tim,dt,neq,yg,yp,yp2,
				    ytemp,errvec,jac,istart))!=0)
	  return(j);
	stor_delay(y);
      }
    return(0);
}

one_bak_step(y,t,dt,neq,yg,yp,yp2,ytemp,errvec,jac,istart)
     double *y,*t,dt,*yg,*yp,*yp2,*ytemp,*errvec,*jac;
     int neq,*istart;
{
  int i,j;
  double err=0.0,err1=0.0;
  double yold,del,h;
  int iter=0,info,ipivot[MAXODE1];
  set_wieners(dt,y,*t);
  *t=*t+dt;
  rhs(*t,y,yp2,neq);
  for(i=0;i<neq;i++)yg[i]=y[i];
  while(1)
    {
      err1=0.0;
      err=0.0;
      rhs(*t,yg,yp,neq);
      for(i=0;i<neq;i++){
	errvec[i]=yg[i]-.5*dt*(yp[i]+yp2[i])-y[i];
	err1+=fabs(errvec[i]);
	ytemp[i]=yg[i];
      }
      /* Compute Jacobian   */
      for(i=0;i<neq;i++){
	del=NEWT_ERR*amax(NEWT_ERR,fabs(yg[i]));
	yold=yg[i];
	yg[i]=yg[i]+del;
	rhs(*t,yg,ytemp,neq);
	h=.5*dt/del;
	for(j=0;j<neq;j++)
	  jac[j*neq+i]=(yp[j]-ytemp[j])*h;
	yg[i]=yold;
      }
      for(i=0;i<neq;i++)jac[i*neq+i]+=1.0;
      sgefa(jac,neq,neq,ipivot,&info);
      if(info!=-1)
	{
	 
	  return(-1);
	}
      sgesl(jac,neq,neq,ipivot,errvec,0);
      for(i=0;i<neq;i++){
	err+=fabs(errvec[i]);
	yg[i]-=errvec[i];
      }
      if(err<EulTol||err1<EulTol){
	for(i=0;i<neq;i++)y[i]=yg[i];
	return(0);
      }
      iter++;
      if(iter>MaxEulIter)return(-2);
    }
}
	
  

one_step_euler(y,dt,yp,neq,t)
     double dt,*t;
     double *y,*yp;
     int neq;
{
   
 int j;
   set_wieners(dt,y,*t);
   rhs(*t,y,yp,neq);
   *t+=dt;
   for(j=0;j<neq;j++)y[j]=y[j]+dt*yp[j];
}

one_step_rk4(y,dt,yval,neq,tim)
     double dt,*tim,*yval[3],*y;
     int neq;
{
 int i;
 double t=*tim,t1,t2;
 set_wieners(dt,y,t);
 rhs(t,y,yval[1],neq);
 for(i=0;i<neq;i++)
   {
     yval[0][i]=y[i]+dt*yval[1][i]/6.00;
     yval[2][i]=y[i]+dt*yval[1][i]*0.5;
  }
  t1=t+.5*dt;
  rhs(t1,yval[2],yval[1],neq);
  for(i=0;i<neq;i++)
    {
      yval[0][i]=yval[0][i]+dt*yval[1][i]/3.00;
      yval[2][i]=y[i]+.5*dt*yval[1][i];
    }
 rhs(t1,yval[2],yval[1],neq);
 for(i=0;i<neq;i++)
   {
     yval[0][i]=yval[0][i]+dt*yval[1][i]/3.000;
     yval[2][i]=y[i]+dt*yval[1][i];
   }
 t2=t+dt;
 rhs(t2,yval[2],yval[1],neq);
 for(i=0;i<neq;i++)y[i]=yval[0][i]+dt*yval[1][i]/6.00;
 *tim=t2;
}

one_step_heun(y,dt,yval,neq,tim)
     double dt,*tim,*yval[2],*y;
     int neq;
{
 int i;
 double t=*tim,t1;
  set_wieners(dt,y,*tim);
  rhs(t,y,yval[0],neq);
  for(i=0;i<neq;i++)yval[0][i]=dt*yval[0][i]+y[i];
  t1=t+dt;
  rhs(t1,yval[0],yval[1],neq);
  for(i=0;i<neq;i++)y[i]=.5*(y[i]+yval[0][i]+dt*yval[1][i]);
  *tim=t1;
}

/*  Euler  */


euler(y,tim,dt,nt,neq,istart,work)
double *y,*tim,dt,*work;
int nt,neq,*istart;
{
  int i,j;
  if(NFlags==0){ 
    for(i=0;i<nt;i++)
      {
	one_step_euler(y,dt,work,neq,tim);
	stor_delay(y);
      }
    return(0);
  }
  for(i=0;i<nt;i++)
      {
	one_flag_step_euler(y,dt,work,neq,tim,istart);
	stor_delay(y);
      }
    return(0);
}

/* Modified Euler  */

mod_euler(y,tim,dt,nt,neq,istart,work)
double *y,*tim,dt,*work;
int nt,neq,*istart;
{
 double *yval[2];
 int i,j;
 double t=*tim,t1;
 yval[0]=work;
 yval[1]=work+neq;
 if(NFlags==0){
   for(j=0;j<nt;j++)
     {
       one_step_heun(y,dt,yval,neq,tim);
       stor_delay(y);
     }
   return(0);
 }
 for(j=0;j<nt;j++)
     {
       one_flag_step_heun(y,dt,yval,neq,tim,istart);
       stor_delay(y);
     }
   return(0);
}

/*  Runge Kutta    */

rung_kut(y,tim,dt,nt,neq,istart,work)
double *y,*tim,dt,*work;
int nt,neq, *istart;
{
 register i,j;
 double *yval[3];
 double t=*tim,t1,t2;
 yval[0]=work;
 yval[1]=work+neq;
 yval[2]=work+neq+neq;
 if(NFlags==0){
   for(j=0;j<nt;j++)
     {
       one_step_rk4(y,dt,yval,neq,tim);
       stor_delay(y);
     }
   return(0);
 }

 for(j=0;j<nt;j++)
   {
     one_flag_step_rk4(y,dt,yval,neq,tim,istart);
       stor_delay(y);
   }
 return(0);
 
}

/*   ABM   */

adams(y,tim,dt,nstep,neq,ist,work)
double *y,*tim,dt,*work;
int nstep,neq,*ist;

{
  int istart=*ist,i,istpst,k,ik,n;
  int irk;
  double *work1;
  double x0=*tim,xst=*tim;
  work1=work;
 if(istart==1)
 {
   for(i=0;i<4;i++)
  {
   y_p[i]=work+(4+i)*neq;
   y_s[i]=work+(8+i)*neq;
  }
 ypred=work+3*neq;
 goto n20;
 }
 if(istart>1) goto n350;
 istpst=0;
 goto n400;

n20:

 x0=xst;
 rhs(x0,y,y_p[3],neq);
 for(k=1;k<4;k++)
 {
  rung_kut(y,&x0,dt,1,neq,&irk,work1);
  stor_delay(y);
  for(i=0;i<neq;i++)y_s[3-k][i]=y[i];
  rhs(x0,y,y_p[3-k],neq);
 }
 istpst=3;
 if(istpst<=nstep) goto n400;
  ik=4-nstep;
  for(i=0;i<neq;i++)y[i]=y_s[ik-1][i];
  xst=xst+nstep*dt;
  istart=ik;
  goto n1000;

n350:

  ik=istart-nstep;
  if(ik<=1)goto n370;
  for(i=0;i<neq;i++)y[i]=y_s[ik-1][i];
  xst=xst+nstep*dt;
  istart=ik;
  goto n1000;

n370:
  for(i=0;i<neq;i++)y[i]=y_s[0][i];
  if(ik==1){x0=xst+dt*nstep; goto n450; }

  istpst=istart-1;

n400:

  if(istpst==nstep) goto n450;
  for(n=istpst+1;n<nstep+1;n++) {
    set_wieners(dt,y,x0);
   abmpc(y,&x0,dt,neq);
   stor_delay(y);
 }

n450:
  istart=0;
  xst=x0;

n1000:

 *tim=*tim + nstep*dt;
 *ist=istart;
 return(0);
}

abmpc(y,t,dt,neq)
double *t,*y,dt;
int neq;
{
 double x1,x0=*t;
 int i,k;
 for(i=0;i<neq;i++)
 {
  ypred[i]=0;
  for(k=0;k<4;k++)ypred[i]=ypred[i]+coefp[k]*y_p[k][i];
  ypred[i]=y[i]+dt*ypred[i];
 }

 for(i=0;i<neq;i++)
 for(k=3;k>0;k--)y_p[k][i]=y_p[k-1][i];
 x1=x0+dt;
 rhs(x1,ypred,y_p[0],neq);

 for(i=0;i<neq;i++)
 {
  ypred[i]=0;
  for(k=0;k<4;k++)ypred[i]=ypred[i]+coefc[k]*y_p[k][i];
  y[i]=y[i]+dt*ypred[i];
 }
   *t=x1;
 rhs(x1,y,y_p[0],neq);
}















