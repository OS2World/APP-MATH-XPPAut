#include <math.h>
#include "ctm.h"

static double sqrarg;
#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)
 

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

double do_convolve();
double do_fft_conv();

extern CONTINUUM my_ctm[MAXCTM];
extern  int CURRENT_GRID;
extern double CURRENT_H;
typedef struct {
  int type;  /* periodic,even, or zero  */  
  int aind,ind; /* index to the weight and function */
  double *ans;
} FFT;

FFT my_fft[50];
int NFFT;


/* this is new code for xtc --  It is called as follows:
   fftconv(w,u,x,type)
   where  type = ZERO,EVEN or PERIODIC
   For EVEN/PERIODIC the grid size is 2^m
   for zero/natural  2^{m-1}
   and  u is the function to convolve with weight w
   This this is of course all done in one fell swoop so
   that it is done on the first call.  Subsequent calls
   use the result which is stored in the appropriate structure.
   
   The limitation on is is that the length of the grid must be
   a power of 2 for periodic/even and 2^n-1 for natural
   
 
   If w and u are periodic then it is straightforward.

   If the call is the ZERO type then the following is done:
   The length of u is double and padded with zeros.
   w is copied as follows (say w is length 8)
   w0 w1 w2 w3 w4 w5 w6 w7 0 w7 w6 w5 w4 w3 w2 w1
   u0 u1 u2 u3 u4 u5 u6 u7 0 0  0  0  0  0  0  0  
   \-------- orig -------/ \-------- exten------/
 
   If the call is for EVEN  then the following is set up:

   w0 w1 w2 w3 w4 w5 w6 w7 w8| w7 w6 w5 w4 w3 w2 w1
   u0 u1 u2 u3 u4 u5 u6 u7 u8| u7 u6 u5 u4 u3 u2 u1
   \------ orig ------------/ \------ added ------/
   which is the even extension of u
   
   The convolution is applied to this and then the answer is just the
   first 8 entries.     If the grid is not a correct power of two, it 
   returns the following:

   sum_j=0..NGrid u_{j} w_{k-j} *CURRENT_H * WEIGHT_0

   where the Weight is applied only to the first and last elements.
   note  w_{k-j} is interpreted to mean  w_{|k-j|} as in the 
   standard convolution.
   
*/ 







clear_fft()
{
  int i;
 
  for(i=0;i<NFFT;i++)free(my_fft[i].ans);
   NFFT=0;
}
  

double do_fft_conv(aind,ind,x,type)
     int aind,ind,type;
     double x;
{
  int n,n2,n2m1,i;
  int pow2;
  double dj=x/CURRENT_H;
  int jj=(int)dj;
  double *w,*u;
  if(type==PERIODIC||type==EVEN) 
    pow2=check_grid(CURRENT_GRID);
  else 
    pow2=check_grid(CURRENT_GRID+1);
  if(pow2==0)
    return do_convolve(my_ctm[aind].u,my_ctm[ind].u,0.0,
		       CURRENT_GRID*CURRENT_H,x,type);
  /*  OKAY  the grid is the right size */
  /*  lets look and see if this was done already */
  pow2=-1;  /*  this is the index to the FFT array */
  for(i=0;i<NFFT;i++){
    if(aind==my_fft[i].aind&&ind==my_fft[i].ind&&type==my_fft[i].type)
      {  /*   There is a hit  */  
	/*  compute the index for x --  */
	return(my_fft[i].ans[jj]*CURRENT_H);
      }
  }
  
  /* OKAY its a new one  */
  my_fft[NFFT].aind=aind;
  my_fft[NFFT].ind=ind;
  my_fft[NFFT].type=type;
  my_fft[NFFT].ans=(double *)malloc((CURRENT_GRID+1)*sizeof(double));
  
  /* now we set up the arrays according to the boundary type */
  switch(type){
  case PERIODIC:  /* easiest  */
    { 
      convolve_em(my_ctm[ind].u,my_ctm[aind].u,my_fft[NFFT].ans,CURRENT_GRID,
		  CURRENT_GRID);
      my_fft[NFFT].ans[CURRENT_GRID]=my_fft[NFFT].ans[0];
      NFFT++;
      return(my_fft[NFFT-1].ans[jj]*CURRENT_H);
    }
  case EVEN: /* this is a little trickier  */
    {
      n2=2*CURRENT_GRID;
      w=(double *)malloc(n2*sizeof(double));
      u=(double *)malloc(n2*sizeof(double));
      for(i=0;i<=CURRENT_GRID;i++){
	w[i]=my_ctm[aind].u[i];
        u[i]=my_ctm[ind].u[i];
	if(i>0){
	  w[n2-i]=w[i];
	  u[n2-i]=u[i];
	}
      }
      convolve_em(u,w,my_fft[NFFT].ans,n2,CURRENT_GRID+1);
      free(w);
      free(u);
      NFFT++;
      return(my_fft[NFFT-1].ans[jj]*CURRENT_H);
    }
  default: /* trickier still ...   */
    {
      n2=2*(CURRENT_GRID+1);
      w=(double *)malloc(n2*sizeof(double));
      u=(double *)malloc(n2*sizeof(double));
      for(i=0;i<=CURRENT_GRID;i++){
	w[i]=my_ctm[aind].u[i];
	u[i]=my_ctm[ind].u[i];
	if(i>0){
	  w[n2-i]=w[i];
	  u[n2-i]=0.0;
	}
      }

      w[CURRENT_GRID+1]=0.0;
      u[CURRENT_GRID+1]=0.0;
    /*  for(i=0;i<n2;i++)printf(" %d %d %f %f \n",jj,i,w[i],u[i]); */
      convolve_em(u,w,my_fft[NFFT].ans,n2,CURRENT_GRID+1);
      free(w);
      free(u);
      NFFT++;
      return(my_fft[NFFT-1].ans[jj]*CURRENT_H);
    }
  }
}
check_grid(n)   /* returns 1 if power of 2 else 0 */
     int n;
{
  int temp,n2=n;
  while(1) {
    temp=n2/2;
    temp=2*temp;
    if(temp!=n2)return(0);
    n2=n2/2;
    if(n2==1)return(1);
  }
}  


convolve_em(u,w,ans,n,nans)  /*  need data and w get back ans */
     int n,nans;
     double *u,*w,*ans;
{
  int i;
  int ti,ti1;
  double a;
  double *uhat,*what;
  uhat=(double *)malloc(2*n*sizeof(double));
  what=(double *)malloc(2*n*sizeof(double));
  for(i=0;i<n;i++){
    uhat[2*i]=u[i];
    uhat[2*i+1]=0.0;
    what[2*i]=w[i];
    what[2*i+1]=0.0;
  }
  four1(uhat-1,n,1);
  four1(what-1,n,1);
 
  for(i=0;i<n;i++){
    ti=2*i;
    ti1=ti+1;
    a=uhat[ti];
    uhat[ti]=a*what[ti]-uhat[ti1]*what[ti1];
    uhat[ti1]=a*what[ti1]+uhat[ti1]*what[ti];
  }
  four1(uhat-1,n,-1);
  for(i=0;i<nans;i++)
    ans[i]=uhat[2*i]/n;
  free(uhat);
  free(what);
}


    

four1(data,nn,isign)
double data[];
int nn,isign;
{
	int n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	double tempr,tempi;

	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=2*mmax;
		theta=6.28318530717959/(isign*mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}


#undef SQR
#undef SWAP


