/*****************************************************
*               Turing Stability Analyzer            *
*                 
*                This is experimental and will
*
*                work only on convolutions, diffusion
*                and biharmonics
*
*                 It does not work properly on compositions of
*                 these operators
*
*                 The convolutions must be normalized and periodized
*
*                Because there is no support for weigted averages,
*                there is no way to couple a scalar to a ctm so
*                in essence, you must have NSCALAR=0
*
*               The problem is to separate out each of the linear operators
*   involved and then use these to compute a small matrix parameterized
*   by wave number.  Use a standard eigenvalue finder to get
*   the eigenvalues
*
*    Here is how it is done:
*
      Consider a right hand side of the form
                  
              F(u, w1*g1(u),w2*g2(u),...,u_xx, biharm(u))
     where wj*gj(u) is a convolution.

     1. Find homogeneous equilibrium by setting all spatial derivatives
        to zero, having biharmonic return 0, and setting all convolution
        kernels to 1. Call this v. It satisfies:
                Ff(v,g1(v),...,0,0)=0
     2. Find the homogeneous linearized equation. This is done in two 
        steps:
	a) Set all convolution weights to 1 and all u_xx,biharm=0:
           F0=F(v,0,0,...,0)
        b) Evaluate Fe=F(v+e,0,...,0)
        c) Let H = (Fe-F0)/e
     3. For each diffusing variable 
        Let U_xx = e and wj=1 and evaluate
        Fd(v,g1,...,e,0)
        Subtract this from Fb to get
        D=(Fd-Ff)/e
     4. For each variable have biharm return e and do a similar calculation
        B=(Fb-Ff)/e
     5.  Now set all weights to 1 .  Compute 
         Fj0=F(v,0,...,gj(v),...)
         Fje=F(v+e,0,...,wj*gj(v+e),...,0)
         with perturbation only at the jth kernel
         Define 
              Wj= (Fje-Fj0)/e
         This will give a matrix for the jth kernel
         subtract all these from H to get true Homogeneous kernel
      The linearized matrix is then
       H - nu^2 D + nu^4 B + sum_j Wj*whatj(nu)

   where  nu=2*Pi*k/L and L is the domain size   

    whatj is the Fourier transform of wj

***********************************************************/
/*
  The order of the equations is:
  1...Scalar,1..Ctm
*/
#include <math.h>
#include "ctm.h"  
#define MAXCPAR 20
#define HOMOG -1
#define ZIP -2
#define MAXCRHS 20
#define MAXCFIX 20
#define MAXSRHS 20
#define MAXSFIX 20
#define MAXCAUX 10
#define MAXSAUX 10

#define T_NORMAL 0
#define T_FIXED 1
#define T_BIHARM 2

#define MAX(a,b) ((a)>(b)?(a):(b))
extern int TuringWeights[MAXCPAR];
extern int N_TWts,TuringHot,TuringFlag;
int TuringColumn;
extern double T_Epsilon;


extern int *FixCtm[MAXCFIX],*FixScal[MAXSFIX],*RhsScal[MAXSRHS],*RhsCtm[MAXCRHS];
double evaluate();
extern int I_fixctm[MAXCFIX],I_varctm[MAXCRHS],I_auxctm[MAXCAUX],
	   I_bdryvar[MAXCRHS];
extern CONTINUUM my_ctm[MAXCTM];
extern BDRYVAL my_bval[MAXCTM];
extern double SpaceValue[10],CURRENT_SPACE,CURRENT_H;
extern int NCTM, NSCALAR, NBVAL,CURRENT_GRID,
  FIX_CTM,FIX_SCAL,AUX_SCAL,AUX_CTM,
  CURRENT_INDEX;




      
turing_rhs(y,ydot,in_dif,in_biharm,iwgt)
     double *y,*ydot;
     int in_dif,in_biharm,iwgt;
/*   Here, y has dimension of all variables combined as does ydot
     NCTM+NSCALAR;
     in_dif  is the  ctm whose diffusion we want
      this is negative then do nothing.
     in_biharm is the index whose biharm we want
      negative is nothing
      iwgt describes the Turing stuff.
      iwgt=-1 ==> all weights are 1
      iwgt>=0 ==> all weights are 1 and Hot=iwgt. 
*/

{
  int i,ix=-1,ixx=-1,ip=-1;
  int id=-1,iw,j; 
  int col=TuringColumn;     
  double save_wgt[MAXCPAR][2];
 
  if(in_dif>=0)id=I_varctm[in_dif];
  TuringHot=-1;
  TuringFlag=T_FIXED;
  if(in_biharm>=0){
    TuringHot=I_varctm[in_biharm]; /* set up biharm  */
    TuringFlag=T_BIHARM;
    
    }
  set_ivar(0,0.0);
  for(i=0;i<NSCALAR;i++)set_ivar(i+1,y[i]);
  for(i=0;i<NCTM;i++)my_ctm[I_varctm[i]].u[col]=y[i+NSCALAR];

  for(i=0;i<NBVAL;i++){
    ip=my_bval[i].parent;
    ix=my_bval[i].ix;
    ixx=my_bval[i].ixx;
     if(ixx>=0&&ip>=0&&ix>=0){
      my_ctm[ix].u[col]=0.0;
      my_ctm[ixx].u[col]=0.0;
     }
     if(ip==id&&ixx>=0){
       my_ctm[ixx].u[col]=T_Epsilon;
        }     /* Take care of all the derivatives   */
  }
  /* now set up the weights   */
    
    for(i=0;i<N_TWts;i++){
      iw=TuringWeights[i];
      save_wgt[i][0]=my_ctm[iw].u[0];
      save_wgt[i][1]=my_ctm[iw].u[1];
      my_ctm[iw].u[0]=1.0;
       my_ctm[iw].u[1]=1.0;
      if(iwgt==i)TuringHot=iw;
    }

/* Now at longlast, we can evaluate the right-hand sides   */
  CURRENT_INDEX=col;
  SpaceValue[0]=0.0;
  if(FIX_CTM>0)
    for(i=0;i<FIX_CTM;i++)
      my_ctm[I_fixctm[i]].u[col]=evaluate(FixCtm[i]);   
   for(i=0;i<FIX_SCAL;i++)
    set_ivar(i+NSCALAR+1,evaluate(FixScal[i]));
  for(i=0;i<NSCALAR;i++)ydot[i]=evaluate(RhsScal[i]);  /* scalar rhs.. */
  for(i=0;i<NCTM;i++)ydot[i+NSCALAR]=evaluate(RhsCtm[i]);  

 /* Now return the weights to their old values  */
     for(i=0;i<N_TWts;i++){
      iw=TuringWeights[i];
      my_ctm[iw].u[0]=save_wgt[i][0];
      my_ctm[iw].u[1]=save_wgt[i][1];
      
    /*  printf(">> i=%d iw=%d iwgt=%d old=%f new=%f \n",i,iw,iwgt,
	     save_wgt[i],my_ctm[iw].u[0]); */
   
    }
  /*  if(iwgt!=-1)printf("iwgt=%d y=(%f,%f) yp=(%f,%f)\n",iwgt,
			 y[0],y[1],ydot[0],ydot[1]); */
    TuringFlag=T_NORMAL;
  }



make_dif_mat(x,yval,ytemp,m) /* yval is F(xbar,w=1,dif=0,bih=0) , x=bar) */
     double *x,*yval,*ytemp,*m;
{
  int i,j;
  int neq=NSCALAR+NCTM;
  turing_rhs(x,yval,-1,-1,-1);
  for(i=0;i<(neq*neq);i++)m[i]=0.0; /* zero it out  */
  if(NBVAL<=0)return; /* No diffusion */
  for(i=0;i<neq;i++){
    turing_rhs(x,ytemp,i-NSCALAR,-1,-1); /* a little trick  */
    for(j=0;j<neq;j++){
      m[j*neq+i]=(ytemp[j]-yval[j])/T_Epsilon;
    }
  }
}

make_bih_mat(x,yval,ytemp,m) /* yval is F(xbar,w=1,dif=0,bih=0) , x=bar) */
     double *x,*yval,*ytemp,*m;
{
  int i,j;
  int neq=NSCALAR+NCTM;
  turing_rhs(x,yval,-1,-1,-1);
  for(i=0;i<(neq*neq);i++)m[i]=0.0; /* zero it out  */
  
  for(i=0;i<neq;i++){
    turing_rhs(x,ytemp,-1,i-NSCALAR,-1); /* a little trick  */
    for(j=0;j<neq;j++){
      m[j*neq+i]=(ytemp[j]-yval[j])/T_Epsilon;
    }
  }
}

make_jac_mat(x,y,yp,xp,eps,dermat,iwgt)
double *x,*y,*yp,*xp,eps,*dermat;
int iwgt;  /*  set iwgt=-1 for normal Jac, -2 for no weights */
{
 int i,j,k,n=NSCALAR+NCTM;
 double r;
 turing_rhs(x,y,-1,-1,iwgt); 
  for(i=0;i<n;i++)
  {
    for(k=0;k<n;k++) xp[k]=x[k];
    r=eps*MAX(eps,fabs(x[i]));
    xp[i]=xp[i]+r;
    turing_rhs(xp,yp,-1,-1,iwgt);
    for(j=0;j<n;j++)
    {
    dermat[j*n+i]=(yp[j]-y[j])/r;
    }

  }
}



make_conv_mat(x,y,yp,xp,dermat,iwgt)
double *x,*y,*yp,*xp,*dermat;
int iwgt;  /*  set iwgt=-1 for normal Jac, -2 for no weights */
{
 int i,j,k,n=NSCALAR+NCTM;
 double r,eps=T_Epsilon;
 
  for(i=0;i<n;i++)
  {
    TuringColumn=0;
    turing_rhs(x,y,-1,-1,-1); /* Load unperturbed into column 0 */
    TuringColumn=1;  /* All perturbed into column 1   */
    for(k=0;k<n;k++) xp[k]=x[k];
    r=eps*MAX(eps,fabs(x[i]));
    xp[i]=xp[i]+r;
    turing_rhs(xp,yp,-1,-1,-1);
    TuringColumn=0;
    turing_rhs(x,yp,-1,-1,iwgt);
    for(j=0;j<n;j++)
    {
    dermat[j*n+i]=(yp[j]-y[j])/r;
    }

  }

}




compute_turing_arrays(x,h,d,b,w,work)
     double *x,*h,*d,*b,*work;
     double *w[MAXCPAR];
{
 int i,j;
 int n=NSCALAR+NCTM;
 double *y,*yp,*xp;
 y=work;
 yp=work+n;
 xp=work+2*n;
 TuringColumn=0;
 make_jac_mat(x,y,yp,xp,T_Epsilon,h,-2); /* homogeneous part */
 make_dif_mat(x,y,yp,d); 
 make_bih_mat(x,y,yp,b);
 for(i=0;i<N_TWts;i++){
   make_conv_mat(x,xp,y,yp,w[i],i); 
   /* Fix up homogeneous part   */
   for(j=0;j<n*n;j++)h[j]=h[j]-w[i][j];
 }


}


get_work_size(iw,nmodes)
     int *iw,nmodes;
{
 int n=NSCALAR+NCTM;
 int n2=n*n;
 *iw=5*n2+n2*N_TWts+30*n+N_TWts*nmodes+200;
}




turing_stab(x,work,nmodes,grid,half_flag,maxit,err,ier,eps,
	    max_real,max_im)
     double *x,*work,err,eps;
     double *max_real,*max_im;
     int nmodes,grid,maxit,*ier, half_flag;
{
  int i,n=NSCALAR+NCTM,j,iw,k;
  int n2=n*n;
  
  int narray,nhats;
  double *uhat[MAXCPAR];
  double *w[MAXCPAR];
  double *eval,z1,z2;
  double nu,tpi=8.*atan(1.0),period=CURRENT_GRID*CURRENT_H;
  double *h,*b,*d,*save;
  double *other;
  narray=(N_TWts+4)*n2;
  nhats=N_TWts*nmodes;
  h=work;
  b=work+n2;
  d=work+2*n2;
  save=work+3*n2;
  T_Epsilon=eps;
  TuringColumn=0;
  for(i=0;i<N_TWts;i++)w[i]=work+4*n2+i*n2;
  for(i=0;i<N_TWts;i++)uhat[i]=work+narray+i*nmodes;
  eval=work+narray+nhats;
  other=work+narray+nhats+2*n;
/* try to find a homogeneous root !  */
   rooter(x,err,eps,1000.0,other,ier,maxit);
  if(*ier>0){
    err_msg("Failed to converge on root");
    return;
  }
  for(i=0;i<n;i++)printf("x[%d]=%f\n",i,x[i]);
  compute_turing_arrays(x,h,d,b,w,other);
  printf("Homogeneous :\n");
  aprnt(h,n);
  printf("Biharmonic:\n");
  aprnt(b,n);
  printf("Diffusive :\n");
  aprnt(d,n);
  for(i=0;i<N_TWts;i++){
    printf(" Weight %d: \n",i);
    aprnt(w[i],n);
  }
  printf(" Now get first  modes of the Fourier transform\n");  
  for(i=0;i<N_TWts;i++){
   /*  printf("Weight %d \n",i);  */
    iw=TuringWeights[i];
   get_w_hat(iw,grid,uhat[i],nmodes);
 /*   for(j=0;j<nmodes;j++)printf("%d %f \n",j,uhat[i][j]);  */
  }

  for(j=0;j<nmodes;j++){
    
/*  Now we create the matrices to find the eigenvalues   
    This is the best part of the trip...
     This is it..
*/
      nu=j*tpi/period;
      if(half_flag)nu=.5*nu;
      for(k=0;k<n2;k++){
	save[k]=h[k]-nu*nu*d[k]+nu*nu*nu*nu*b[k];
	for(i=0;i<N_TWts;i++)
	  save[k]=save[k]+uhat[i][j]*w[i][k];
      }
      eigen(n,save,eval,other,ier);
      if(*ier!=0){
	err_msg("Could not compute eigenvalues");
	return;
      }
      max_real[j]=eval[0];
      max_im[j]=eval[1];
      for(i=0;i<n;i++){
	z1=eval[2*i];
	z2=eval[2*i+1];
	if(z1>max_real[j]){
	  max_real[j]=z1;
	  max_im[j]=z2;
	}
      }
      printf(" max[%d] = %f + i%f \n",j,max_real[j],max_im[j]);
    }
}





rooter(x, err, eps, big,
work,ierr,maxit)

double *x, err, eps, big,*work;
int *ierr,maxit;
{
 int i,j,k,iter,ipivot[40],info;
 char ch;
 int n=NSCALAR+NCTM;
 double *xp,*yp,*y,*xg,*dermat,*dely;
 double r;
 dermat=work;
 xg=dermat+n*n;
 yp=xg+n;
 xp=yp+n;
 y=xp+n;
 dely=y+n;
 iter=0;
 *ierr=0;
 while(1)
 {

  make_jac_mat(x,y,yp,xp,eps,dermat,-1);
  /* printf("DM=%f %f %f %f \n",dermat[0],dermat[1],dermat[2],dermat[3]); */
  sgefa(dermat,n,n,ipivot,&info);
  if(info!=-1)
  {
   *ierr=1;
   return;
  }
  for(i=0;i<n;i++)dely[i]=y[i];
  sgesl(dermat,n,n,ipivot,dely,0);
  r=0.0;
  for(i=0;i<n;i++)
  {
   x[i]=x[i]-dely[i];
   r=r+fabs(dely[i]);
  }
  if(r<err)
  {
     return; /* success !! */
  }
  if((r/(double)n)>big)
  {
   *ierr=1;
   return;
   }
   iter++;
   if(iter>maxit)
   {
    *ierr=1;
    return;
   }
 }
}




aprnt(a,n)
     double *a;
     int n;
{
  int i,j;
  for(i=0;i<n;i++){
    for(j=0;j<n;j++)printf("%f ",a[i*n+j]);
    printf("\n");
  }
}










/*

test_turing(ysent)
     double *ysent;
{
 int i,j;
 int ndim=NCTM+NSCALAR;
 double y[MAXCRHS+MAXSRHS];
 double ydot[MAXCRHS+MAXSRHS];
 for(i=0;i<NSCALAR;i++)y[i]=ysent[i];
 for(i=0;i<NCTM;i++)y[i+NSCALAR]=ysent[i+NSCALAR];
 printf(" Homogeneous...\n");
 turing_rhs(y,ydot,-1,-1,-1);
 for(i=0;i<ndim;i++)printf("y[%d]=%f F[%d]=%f\n",i,y[i],i,ydot[i]);
 printf(" Diffusion of 0 \n");
turing_rhs(y,ydot,0,-1,-1);
 for(i=0;i<ndim;i++)printf("y[%d]=%f F[%d]=%f\n",i,y[i],i,ydot[i]);
 printf(" Biharm of 0 \n");
turing_rhs(y,ydot,-1,0,-1);
 for(i=0;i<ndim;i++)printf("y[%d]=%f F[%d]=%f\n",i,y[i],i,ydot[i]);
 printf(" no weights...\n");
 turing_rhs(y,ydot,-1,-1,-2);
 for(i=0;i<ndim;i++)printf("y[%d]=%f F[%d]=%f\n",i,y[i],i,ydot[i]);
 printf(" E weight\n");
turing_rhs(y,ydot,-1,-1,0);
 for(i=0;i<ndim;i++)printf("y[%d]=%f F[%d]=%f\n",i,y[i],i,ydot[i]);
printf(" I weight\n");
turing_rhs(y,ydot,-1,-1,1);
 for(i=0;i<ndim;i++)printf("y[%d]=%f F[%d]=%f\n",i,y[i],i,ydot[i]);

} 
 
*/
