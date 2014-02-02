#include <math.h>
#include <stdio.h>
#include "xpplim.h"

#define Z(a,b) z[(a)+n*(b)]
#define DING ping()
/* this code takes the determinant of a complex valued matrix
*/

extern double variable_shift[2][MAXODE],AlphaMax,OmegaMax;
extern double delay_list[MAXDELAY];
extern int NDelay,del_stab_flag,WhichDelay,DelayGrid;

double amax();
typedef struct{
  double r,i;
}COMPLEX;

/* The
 code here replaces the do_sing code if the equation is
   a delay differential equation. 
*/

do_delay_sing(x,eps,err,big,maxit,n,ierr,stabinfo)
     double *x,eps,err,big;
     int *ierr,n,maxit;
     float *stabinfo;
{
 
 double *work,old_x[MAXODE],sign;
 double *coef,yp[MAXODE],y[MAXODE],xp[MAXODE],dx;
 int kmem=n*(2*n+5)+50,i,j,k;
 /* first we establish how many delays there are */
 del_stab_flag=0;
 for(i=0;i<n;i++)old_x[i]=x[i];
 work=(double *)malloc(kmem*sizeof(double));
 rooter(x,err,eps,big,work,ierr,maxit,n);
 if(*ierr!=0)
   {
     del_stab_flag=1;
     free(work);
     err_msg("Could not converge to root");
     for(i=0;i<n;i++)x[i]=old_x[i];
     return;
   }
 /* OKAY -- we have the root */
 NDelay=0;
 my_rhs(0.0,x,y,n); /* one more evaluation to get delays */
 for(i=0;i<n;i++){
   variable_shift[0][i]=x[i];  /* unshifted  */
   variable_shift[1][i]=x[i];
 }
 free(work);
 coef=(double *)malloc(n*n*(NDelay+1)*sizeof(double));
 
 /* now we must compute a bunch of jacobians  */
 /* first the normal one   */
 del_stab_flag=-1;
 WhichDelay=-1;
 for(i=0;i<n;i++)
   {
     for(j=0;j<n;j++)xp[j]=x[j];
     dx=eps*amax(eps,fabs(x[i]));
     xp[i]=xp[i]+dx;
     my_rhs(0.0,xp,yp,n);
     for(j=0;j<n;j++){
       coef[j*n+i]=(yp[j]-y[j])/dx;
/*       printf("a(0,%d,%d)=%g \n",i,j,coef[j*n+i]);  */
     }
   }
 for(j=0;j<n;j++)xp[j]=x[j];
 /* now the jacobians for the delays */
 for(k=0;k<NDelay;k++){
/*    printf(" found delay=%g \n",delay_list[k]);  */
   WhichDelay=k;
   for(i=0;i<n;i++){
     for(j=0;j<n;j++)
       variable_shift[1][j]=variable_shift[0][j];
     dx=eps*amax(eps,fabs(x[i]));
     variable_shift[1][i]=x[i]+dx;
     my_rhs(0.0,x,yp,n);
     variable_shift[1][i]=x[i];
     for(j=0;j<n;j++){
       coef[j*n+i+n*n*(k+1)]=(yp[j]-y[j])/dx;
/*       printf("a(%d,%d,%d)=%g \n",k+1,i,j,coef[j*n+i+n*n*(k+1)]);  */
     }
   }
 }
 
 sign=plot_args(coef,delay_list,n,NDelay,DelayGrid,AlphaMax,OmegaMax);
 *stabinfo=(float)fabs(sign);
 free(coef); 
 i=(int)sign;
 create_eq_box(abs(i),2,0,0,0,x,n);
 DING;
 del_stab_flag=1;
}





COMPLEX csum(z,w)
     COMPLEX z,w;
{
  COMPLEX sum;
  sum.r=z.r+w.r;
  sum.i=z.i+w.i;
  return sum;
}

COMPLEX cdif(z,w)
     COMPLEX z,w;
{
   COMPLEX sum;
  sum.r=z.r-w.r;
  sum.i=z.i-w.i;
  return sum;
 }

COMPLEX cmlt(z,w)
     COMPLEX z,w;
{
   COMPLEX sum;
  sum.r=z.r*w.r-z.i*w.i;
  sum.i=z.r*w.i+z.i*w.r;
  return sum;
}

COMPLEX cdiv(z,w)
     COMPLEX z,w;
{
  COMPLEX sum;
  double amp=w.r*w.r+w.i*w.i;
  sum.r=(z.r*w.r+z.i*w.i)/amp;
  sum.i=(z.i*w.r-z.r*w.i)/amp;
  return sum;
}

COMPLEX cexp(z)
     COMPLEX z;
{
  COMPLEX sum;
  double ex=exp(z.r);
  sum.r=ex*cos(z.i);
  sum.i=ex*sin(z.i);
  return sum;
}

COMPLEX rtoc(x,y)
     double x,y;
{
  COMPLEX sum;
  sum.i=y;
  sum.r=x;
  return sum;
}

cprintn(z)
     COMPLEX z;
{
  printf(" %g + i %g \n",z.r,z.i);
}

cprint(z)
     COMPLEX z;
{
printf("(%g,%g) ",z.r,z.i);
}

cprintarr(z,n,m)
     COMPLEX *z;
     int n,m;
{
  int i,j;
  for(i=0;i<m;i++){
    for(j=0;j<n;j++)
      cprint(z[i+m*j]);
    printf("\n");
  }
}
  
COMPLEX cdeterm(z,n)
     COMPLEX *z;
     int n;
{
  int i,j,k;
  COMPLEX ajj,sum,mult,temp;
  for(j=0;j<n;j++){
    ajj=Z(j,j);
    for(i=j+1;i<n;i++){
      mult=cdiv(Z(i,j),ajj);
      for(k=j+1;k<n;k++){
        Z(i,k)=cdif(Z(i,k),cmlt(mult,Z(j,k)));
      }
    }
  }
 /* now it should be diagonalized */
  sum=rtoc(1.0,0.0);
  for(j=0;j<n;j++){
    sum=cmlt(sum,Z(j,j));
  }
  return sum;
}
		 


double get_arg(delay,coef,m,n,lambda)  
     COMPLEX lambda;
     double *coef;
     double *delay;
     int m,n;
{
  int i,j,k,km;
  COMPLEX *z;
  COMPLEX temp,eld;
  double arg;
  if(m==0)return(0);  /* no delays so don't use this! */
  z=(COMPLEX *)malloc(sizeof(COMPLEX)*n*n); 
  for(j=0;j<n;j++)
    for(i=0;i<n;i++){
      if(i==j)temp=lambda;
      else temp=rtoc(0.0,0.0);
      /* cprintn(temp); */
      z[i+j*n]=cdif(temp,rtoc(coef[i+j*n],0.0)); /* initialize the array */
    }
  for(k=0;k<m;k++){
    km=(k+1)*n*n;
    temp=rtoc(-delay[k],0.0); /* convert delay to complex number */
    eld=cexp(cmlt(temp,lambda)); /* compute exp(-lambda*tau) */
    /* cprintn(eld); */
    for(j=0;j<n;j++)
      for(i=0;i<n;i++)
	z[i+j*n]=cdif(z[i+j*n],cmlt(eld,rtoc(coef[km+i+n*j],0.0)));
  }
  /*  the array is done  */
 /* cprintarr(z,n,n); */ 
  temp=cdeterm(z,n);
  /* cprint(lambda); 
  cprint(temp); */
   free(z); 
  arg=atan2(temp.i,temp.r);
  /* printf("%g %g %g \n",lambda.r,lambda.i,arg); */
  return(arg);
}   

test_sign(old,new)
     double old,new;
{
  if(old>0.0&&new<0.0){
    if(old>2.9&&new<-2.9)return 1;
    return(0); /* doesnt pass threshold */
  }
  if(old<0.0&&new>0.0){
    if(old<-2.9&&new>2.9)return -1;
    return 0;
  }
  return 0;
}

/* code for establishing delay stability
   sign=plot_args(coef,delay,n,m,npts,amax,wmax)
    coef is a real array of length  (m+1)*n^2
    each n^2 block is the jacobian with respect to the mth delay
    m total delays
    n is size of system
    npts is number of pts on each part of contour
    contour is
      i wmax -----<---------    amax+i wmax
       |                            |
       V                            ^
       |                            |
     -i wmax ----->-----------  amax-i wmax
  
     sign is the number of roots in the contour using the argument
     principle
*/  

plot_args(coef,delay,n,m,npts,amax,wmax)
     double *coef;
     int n,m,npts;
     double amax,wmax,*delay;
{
  int i;
  int sign=0;
  COMPLEX lambda;
  double x,y,arg,oldarg=0.0;
  double ds;  /* steplength */
  /* first the contour from i wmax -- -i wmax */
  ds=2*wmax/npts;
   x=0.0;
  for(i=0;i<npts;i++){
    y=wmax-i*ds;
    lambda=rtoc(x,y);
    arg=get_arg(delay,coef,m,n,lambda);
   /* printf(" %d %g \n",i,arg); */
    sign=sign+test_sign(oldarg,arg);
    oldarg=arg;
 
  }
 /* lower contour   */
  y=-wmax;
  ds=amax/npts;
  for(i=0;i<npts;i++){
    x=i*ds;
    lambda=rtoc(x,y);
    arg=get_arg(delay,coef,m,n,lambda);
/*        printf(" %d %g \n",i+npts,arg); */
       sign=sign+test_sign(oldarg,arg);
    oldarg=arg;
 
  }
/* right contour */
 x=amax;
 ds=2*wmax/npts;
  for(i=0;i<npts;i++){
    y=-wmax+i*ds;
    lambda=rtoc(x,y);
    arg=get_arg(delay,coef,m,n,lambda);
/*     printf(" %d %g \n",i+2*npts,arg); */
      sign=sign+test_sign(oldarg,arg);
    oldarg=arg;
 
  }
 
/* top contour */
  y=wmax;
  ds=amax/npts;
  for(i=0;i<npts;i++){
    x=amax-i*ds;
    lambda=rtoc(x,y);
    arg=get_arg(delay,coef,m,n,lambda);
/*         printf(" %d %g \n",i+3*npts,arg); */
    sign=sign+test_sign(oldarg,arg);
    oldarg=arg;
  
  }
  return sign;
}
 















