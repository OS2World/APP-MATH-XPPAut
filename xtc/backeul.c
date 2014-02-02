#include <stdio.h>
#include <math.h>



/*   Backward Euler for banded/periodic systems   
     takes one step of size dt using data
  initial data y.  
  iwork[0]=ml
  iwork[1]=mr
  iwork[2]=maxiter
  iwork[3]=flag=0 banded, 1 for periodic 
  work[0]=deriv_eps
  work[1]=newt_tol 
  work is a work array it must be:
  (ml+mr+1+5)*n+2  for regular
  (4m+1+5)*n + 2 + m*m for periodic bcs
  where n is the size of the system

  The program calls 
               rhs(t,y,yp,n)
	       Given, (t,y) return yp=dy/dt
	       
   The program also calls band_jac(jac,y,yold,n,ml,mr,scal,flag)
   which creates the banded/periodic Jacobian matrix 

   The function bandsolve/borsolve is also called which solves
   a banded system

*/
/* Modified Euler  */

mod_euler(time,tout,dt,y,work,n)
 double *time,dt,*work,*y,tout;
     int n;
{
 double *yval[2];
 int i;
 double t=*time,t1;
 yval[0]=work;
 yval[1]=work+n;
 while(t<tout){
   if((t+dt)>tout)dt=tout-t;
   my_rhs(t,y,yval[0],n);
  for(i=0;i<n;i++)yval[0][i]=dt*yval[0][i]+y[i];
  t1=t+dt;
  my_rhs(t1,yval[0],yval[1],n);
  for(i=0;i<n;i++)y[i]=.5*(y[i]+yval[0][i]+dt*yval[1][i]);
  t=t1;
   if(t>=tout)break;
  }
  *time=t;
 return(0);
}


euler(time,tout,dt,y,work,n)
     double *time,dt,*work,*y,tout;
     int n;
{
  int i;

  while(*time<tout){
	if((*time+dt)>tout)dt=tout-*time;
  my_rhs(*time,y,work,n);
  for(i=0;i<n;i++)y[i] += (dt*work[i]);
  *time += dt;
  
}
  return(0);
}
     
 backeul(time,tout,dt,y,Tol,Deps,maxit,maxjac,work,n,ml,mr,flag)
     double *time,dt,Deps,Tol,tout;
     double *y,*work;
     int ml,mr,flag,n,maxit,maxjac;
{
    
  int i,j,iter=0,mt=ml+mr+1,iret,njac=0,jachere;
  double *jac,*y1,*y2,*y3,*ytemp,*errvec,t=*time;
   double err,max,dt2=dt/2.0;
  y1=work;
  y2=y1+n;
  y3=y2+n;
  ytemp=y3+n;
  errvec=ytemp+n;
  jac=errvec+n;
    while(t<tout){
   if((t+dt)>tout)dt=tout-t;
    njac=0;
    jachere=0;
  my_rhs(t,y,y1,n);
  for(i=0;i<n;i++){
    y2[i]=y[i]+dt2*y1[i];
    y3[i]=y[i];
  }
   my_rhs(t+dt2,y,y1,n);
  while(1){
    err=0.0;
    for(i=0;i<n;i++){
      errvec[i]=y3[i]-y1[i]*dt2-y2[i];
      err+=fabs(errvec[i]);
    }
	err=err/n;

/* Good guess  !!     */
    if(err<Tol)break;
    if(iter>maxit)return(1);
    if(jachere==0||njac==maxjac){
      njac=0;
      get_band_jac(jac,y3,t+dt2,ytemp,y1,n,ml,mr,Deps,-dt2,flag);
      for(i=0;i<n;i++)jac[i*mt+ml] += 1.0;
          if(flag==0)iret=bandfac(jac,ml,mr,n);
      else iret=borfac(jac,ml,n-ml);
      if(iret<0)return(iret);
      jachere=1;
    }
	    if(flag==0)bandsol(jac,errvec,ml,mr,n);

    else borsol(jac,errvec,ml,n-ml);
    njac++;
      for(i=0;i<n;i++)y3[i]-=errvec[i];
      my_rhs(t+dt2,y3,y1,n);
   
    }
  for(i=0;i<n;i++)y[i]=y3[i];
  *time += dt;
    if(*time>=tout)break;
  }
  return(0);
  
}

get_band_jac(a,y,t,ypnew,ypold,n,ml,mr,eps,scal,flag)
     double *a,*y,*ypnew,*ypold,eps,scal,t;
     int n,ml,mr,flag;
{
  int i,j,k,n1=n-1,mt=ml+mr+1;
  double yhat;
  double dy;
  double dsy;
  for(i=0;i<n;i++){
    yhat=y[i];
    dy=eps*(eps+fabs(yhat));
    dsy=scal/dy;
    y[i] += dy;
    my_rhs(t,y,ypnew,n);
    for(j=-ml;j<=mr;j++){
      k=i-j;
      if(flag){
	if(k<0)k += n;
	else if(k>n1) k -= n;
      }
      else
	if(k<0||k>n1)continue;
      a[k*mt+j+ml]=dsy*(ypnew[k]-ypold[k]);
    }
    y[i]=yhat;
  } 
 
}














