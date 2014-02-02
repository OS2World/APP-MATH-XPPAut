#include <math.h>
#include <stdio.h>
#include "ctm.h"
#define TRUNC .001



#define MIN(a,b) ((a)<(b))? (a):(b)
double do_convolve();
double do_weight();
double do_average();
double eval_ctm();
double eval_ctm2();
double evaluate();
double do_biharm();
double do_dif();
double sgn();

extern CONTINUUM my_ctm[MAXCTM];
extern BDRYVAL my_bval[MAXCTM];

extern int CURRENT_GRID,PER_LEN;
int CURRENT_INDEX;   /* main space variable  */
extern int NCON_START,NSYM_START,NCON,NSYM;
extern double WEIGHT_0;

extern double SpaceValue[10];
extern double CURRENT_SPACE,CURRENT_H; 


get_w_hat(index,grid,what,nmodes)
     int grid,nmodes,index;
     double *what;
{
  double *data;
  double w0=WEIGHT_0;
  double period=CURRENT_GRID*CURRENT_H;
  double ch=CURRENT_H;
  int cg=CURRENT_GRID;
  int j;
  CURRENT_GRID=grid;
  WEIGHT_0=1;
  CURRENT_H=period/(double)grid;
  data=(double *)malloc((grid+1)*sizeof(double));
  fill_array(my_ctm[index].formula,data,my_ctm[index].special);
   sft(data,what,nmodes,grid,period);
  CURRENT_GRID=cg;
  CURRENT_H=ch;
  WEIGHT_0=w0;
  free(data);
  }

sft(data,uhat,nmodes,grid,period)  /* a "slow" Fourier Transform */
double period;
int grid,nmodes;
double *data,*uhat;
{
 int i,j;
 double sum,jx;
 double tpi=6.28318530717959;
 double dx;
 dx=period/grid;
   for(j=0;j<nmodes;j++){
   sum=0.0;
   jx=j*dx*tpi/period;
   for(i=0;i<=grid;i++)sum=sum+cos(jx*(double)i)*data[i];
   uhat[j]=sum*CURRENT_H;
   }
}




read_2d_file(filename,v)
     char *filename;
     double *v;
{
  FILE *fp;
  int i,j;
  int npts,ng1=CURRENT_GRID+1;
  fp=fopen(filename,"r");
  if(fp==NULL){
    printf("Cant open file %s for formula\n",filename);
    return(1);
  }
  fscanf(fp,"#%d",&npts);
  if(npts!=CURRENT_GRID){
    printf("%s has wrong number of pts -- %d\n",filename,npts);
    return(1);
  }
  for(i=0;i<=CURRENT_GRID;i++){
    for(j=0;j<=CURRENT_GRID;j++){
      fscanf(fp,"%lg",&v[i+j*ng1]);
    }
  }
  fclose(fp);
  return(0);
}


read_1d_file(filename,v)
     char *filename;
     double *v;
{
  FILE *fp;
  int i;
  int npts;
  fp=fopen(filename,"r");
  if(fp==NULL){
    printf("Cant open file %s for formula\n",filename);
    return(1);
  }
  fscanf(fp,"#%d",&npts);
  if(npts!=CURRENT_GRID){
    printf("%s has wrong number of pts -- %d\n",filename,npts);
    return(1);
  }
  for(i=0;i<=npts;i++)
    fscanf(fp,"%lg",&v[i]);
  fclose(fp);
  return(0);
}

fill_array(formula,array,special)
     char *formula;
     double *array;
     int special;
{
 int command[200],len;
 int i,n1=NCON,n2=NSYM;
 int j;
 
 double period=CURRENT_GRID*CURRENT_H;
 double sum=0.0;
 if(special==READFILE){
   return(read_1d_file(formula,array));
 }
 if(add_expr(formula,command,&len)){
   printf("Illegal formula...\n");
   return(1);
 }
 if(special==PERIODIZE||special==PERNORM){
   for(i=0;i<=CURRENT_GRID;i++){
     array[i]=0.0;
     for(j=-PER_LEN;j<=PER_LEN;j++){
       SpaceValue[0]=i*CURRENT_H+j*period;
       
       array[i]+= evaluate(command);
     }
     if(i==0||i==CURRENT_GRID)
    sum=sum+array[i]*WEIGHT_0;
     else sum=sum+array[i];
   }
   
 }
 else{
   for(i=0;i<=CURRENT_GRID;i++){
     SpaceValue[0]=i*CURRENT_H;
     array[i]=evaluate(command);
       if(i==0||i==CURRENT_GRID)
    sum=sum+array[i]*WEIGHT_0;
     else sum=sum+array[i];
   
   }
   
 }
 NCON=n1;
 NSYM=n2;
 sum=sum*CURRENT_H;
 if(special==NORMALIZE||special==PERNORM)
   for(i=0;i<=CURRENT_GRID;i++)array[i]=array[i]/sum;
/* printf("_________________\n");
for(i=0;i<=CURRENT_GRID;i++)printf("%f %f \n", i*CURRENT_H,array[i]);
 printf("_________________\n"); */
 return(0);
}
 
fill_2d_array(formula,array,special)
     char *formula;
     double *array;
     int special;
{
int command[200],len;
 int i,j,ng1=CURRENT_GRID+1;
int n1=NCON,n2=NSYM;
double sum=0.0;
if(special==READFILE){
  return(read_2d_file(formula,array));
}
 if(add_expr(formula,command,&len)){
   printf("Illegal formula...\n");
   return(1);
 }

 for(i=0;i<=CURRENT_GRID;i++){
   SpaceValue[0]=i*CURRENT_H;
   for(j=0;j<=CURRENT_GRID;j++){
     SpaceValue[1]=j*CURRENT_H;
     array[i+j*ng1]=evaluate(command);

   }
 }
 NCON=n1;
 NSYM=n2;
 if(special==NORM2D){
   for(i=0;i<=CURRENT_GRID;i++){
     sum=0.0;
     for(j=0;j<=CURRENT_GRID;j++){ 
       if(j==0||j==CURRENT_GRID)sum+=(array[i+j*ng1]*WEIGHT_0);
       else sum+=array[i+j*ng1];
     }
     sum=sum*CURRENT_H;
     for(j=0;j<=CURRENT_GRID;j++)array[i+j*ng1]=array[i+j*ng1]/sum;
   }
 }
 return(0);
}


double sgn(v)
double v;
{
  if(v<0.0)return(-1.0);
  return(1.0);
}


setval_ctm(i,x,v)
     int i;
     double x,v;
{
 double z=x/CURRENT_H+TRUNC*sgn(x);
 int j= (int)(z);
 if(j>-1&&j<=CURRENT_GRID){ my_ctm[i].u[j]=v;return;}
 switch(my_ctm[i].special){
 case PERIODIC:

   if(j<0)
     my_ctm[i].u[j+CURRENT_GRID]=v;
   else
     my_ctm[i].u[j-CURRENT_GRID]=v;
   break;
 case ZERO:
   break;
 case EVEN:
      if(j>CURRENT_GRID)
	my_ctm[i].u[2*CURRENT_GRID-j]=v;
      else 
	my_ctm[i].u[abs(j)]=v;
   break;
 case ODD:
   if(j>CURRENT_GRID)
     my_ctm[i].u[2*CURRENT_GRID-j]=-v;
   else 
     my_ctm[i].u[abs(j)]=-v;
   break;
 }
}
    
 
setval_ctm2(in,x,y,v)
int in;
double x,y,v;
{
 
 double dj=x/CURRENT_H+TRUNC*sgn(x),dk=y/CURRENT_H+TRUNC*sgn(y);
 int j=(int)dj,k=(int)dk;
 if(j<0||k<0||j>CURRENT_GRID||k>CURRENT_GRID)return;
 my_ctm[in].u[j+(CURRENT_GRID+1)*k]=v;
} 


double eval_ctm(i,x)
int i;
double x;
{
 double dj=(x+.5*CURRENT_H)/CURRENT_H;
 int j= (int)dj;
 if(j>-1&&j<=CURRENT_GRID)return(my_ctm[i].u[j]);
 switch(my_ctm[i].special){
 case PERIODIC:

   if(j<0)return(my_ctm[i].u[j+CURRENT_GRID]);
   return(my_ctm[i].u[j-CURRENT_GRID]);

 case ZERO:
   return(0.0);
 case EVEN:
      if(j>CURRENT_GRID)
     return(my_ctm[i].u[2*CURRENT_GRID-j]);

   else return(my_ctm[i].u[abs(j)]);

 case ODD:
   if(j>CURRENT_GRID)
     return(-my_ctm[i].u[2*CURRENT_GRID-j]);
   else return(-my_ctm[i].u[abs(j)]);

 }
 return(0.0);
}
    
 
double eval_ctm2(in,x,y)
int in;
double x,y;
{
 double dj=(x+.5*CURRENT_H)/CURRENT_H,dk=(y+.5*CURRENT_H)/CURRENT_H;
 double zz;
 int j=(int)dj,k=(int)dk;
 if(j<0||k<0||j>CURRENT_GRID||k>CURRENT_GRID)return(0.0);
 zz=my_ctm[in].u[j+(CURRENT_GRID+1)*k];

 return(zz);
} 






 double do_dif(u,x,ord,type)
     int type,ord;
     double *u,x;
{
  double z0=x/CURRENT_H+TRUNC*sgn(x);
  int i0=abs((int)z0);
  double hm=1/CURRENT_H;
  double hm2=hm*hm,hm3=hm*hm2;
  int cg2=CURRENT_GRID-2,cg1=CURRENT_GRID-1;
  int im1=i0-1,im2=i0-2,ip1=i0+1,ip2=i0+2;
  if(i0>CURRENT_GRID)return(0.0);
  if(ord==0)return(u[i0]);
  switch(ord){
  case 1:
    if(i0>0&&i0<CURRENT_GRID)return(.5*hm*(u[ip1]-u[im1]));
   
      switch(type){
      case PERIODIC:
	if(i0==0)return(.5*hm*(u[ip1]-u[cg1]));
	return(.5*hm*(u[1]-u[im1]));
      case EVEN:
	return(0.0);
      case ODD:
	if(i0==0)return(u[1]*hm);
	return(-u[cg1]*hm);
      }
   
  case 2:
    if(i0>0&&i0<CURRENT_GRID)return(hm2*(u[ip1]+u[im1]-2*u[i0]));
   
      switch(type){
      case PERIODIC:
	if(i0==0)return(hm2*(u[ip1]-2*u[0]+u[cg1]));
	return(hm2*(u[im1]+u[1]-2*u[i0]));
      case EVEN:
	if(i0==0)return(2*hm2*(u[1]-u[0]));
	return(2*hm2*(u[cg1]-u[i0]));
      case ODD:
	return(0.0);
      }
  
  case 3:
    if(i0>1&&i0<cg1)
      return((u[im1]-u[ip1]+.5*(u[ip2]-u[im2]))*hm3);
   
    switch(type){
    case PERIODIC:
      if(i0==0){
	return((u[cg1]-u[ip1]+.5*(u[ip2]-u[cg2]))*hm3);}
      if(i0==1)
	return((u[0]-u[ip1]+.5*(u[ip2]-u[cg1]))*hm3);
      if(i0==CURRENT_GRID){
	return((u[im1]-u[1]+.5*(u[2]-u[im2]))*hm3);}
      if(i0==cg1)
	return((u[im1]-u[ip1]+.5*(u[1]-u[im2]))*hm3);
      break;
    case EVEN:
      if(i0==0||i0==CURRENT_GRID)return(0.0);
      if(i0==1)
	return((u[0]-u[2]+.5*(u[3]-u[2]))*hm3);
      if(i0==cg1)
	return((u[im1]-u[CURRENT_GRID]+.5*(u[cg1]-u[im2]))*hm3);
      break;
    case ODD:
      if(i0==0)
	return(hm3*(u[2]-2*u[1]));
      if(i0==1)
	return(hm3*(u[0]-u[2]+.5*(u[3]+u[1])));
      if(i0==CURRENT_GRID)
	return((2*u[cg1]-u[cg2])*hm3);
      if(i0==cg1)
	return((u[im1]-u[ip1]-.5*(u[i0]+u[im2]))*hm3);
      break;
    }
  }
  return(0.0);
}










double do_biharm(u,x,type)
     int type;
     double *u,x;
{
   double z0=x/CURRENT_H;
  int i0=abs((int)z0);
  double hm4=1/(CURRENT_H*CURRENT_H*CURRENT_H*CURRENT_H);
  int cg2=CURRENT_GRID-2,cg1=CURRENT_GRID-1;
  int im1=i0-1,im2=i0-2,ip1=i0+1,ip2=i0+2;
  if(i0>CURRENT_GRID)return(0.0);

 if(i0>1&&i0<cg1)
    return((u[im2]+u[ip2]-4*(u[im1]+u[ip1])+6*u[i0])*hm4);
  switch(type){
  case 0:
    if(i0==0){
      return((u[cg2]+u[ip2]-4*(u[cg1]+u[ip1])+6*u[i0])*hm4);}
    if(i0==1)
      return((u[cg1]+u[ip2]-4*(u[im1]+u[ip1])+6*u[i0])*hm4);
    if(i0==CURRENT_GRID){
      return((u[im2]+u[2]-4*(u[im1]+u[1])+6*u[i0])*hm4);}
    if(i0==cg1)
      return((u[im2]+u[1]-4*(u[im1]+u[ip1])+6*u[i0])*hm4);
    break;
  case 2:
    if(i0==0)
      return((u[2]+u[ip2]-4*(u[1]+u[ip1])+6*u[i0])*hm4);
    if(i0==1)
      return((u[1]+u[ip2]-4*(u[im1]+u[ip1])+6*u[i0])*hm4);
    if(i0==CURRENT_GRID)
      return((u[im2]+u[cg2]-4*(u[im1]+u[cg1])+6*u[i0])*hm4);
    if(i0==cg1)
      return((u[im2]+u[cg1]-4*(u[im1]+u[ip1])+6*u[i0])*hm4);
    break;
  case 3:
      if(i0==0)
      return((-u[2]+u[ip2]-4*(-u[1]+u[ip1])+6*u[i0])*hm4);
    if(i0==1)
      return((-u[1]+u[ip2]-4*(u[im1]+u[ip1])+6*u[i0])*hm4);
    if(i0==CURRENT_GRID)
      return((u[im2]-u[cg2]-4*(u[im1]-u[cg1])+6*u[i0])*hm4);
    if(i0==cg1)
      return((u[im2]-u[cg1]-4*(u[im1]+u[ip1])+6*u[i0])*hm4);
    break;
  }
    return(0.0);
  
}

    
double do_average(u,v,xlo,xhi)
double *u,*v,xlo,xhi;
{
double  di1=xlo/CURRENT_H,di2=xhi/CURRENT_H;
 int i,i1=abs((int)di1),i2=abs((int)di2);
 double sum=0.0;
i1=MIN(i1,CURRENT_GRID);
i2=MIN(i2,CURRENT_GRID);
 for(i=i1;i<=i2;i++){
   if(i==i1||i==i2)
   sum+=(u[i]*v[i]*WEIGHT_0);
   else sum+= (u[i]*v[i]);
 }
 return(sum*CURRENT_H);
}

double do_weight(w,u,ylo,yhi,x,flag)
     double *w,*u,ylo,yhi,x;
     int flag;
{
 double di1=ylo/CURRENT_H,di2=yhi/CURRENT_H,dj=x/CURRENT_H;
 int i,i1=(int)di1,c1=CURRENT_GRID+1;
 int i2=(int)di2,j=(int)dj;
 int jp=j*c1;
 double sum=0.0;
 if(flag==0){
   for(i=i1;i<=i2;i++){
     if(i==i1||i==i2)sum+=(u[i]*w[j+i*c1]*WEIGHT_0);
     else sum+= (u[i]*w[j+i*c1]);
   }
 }
 else {
   for(i=i1;i<=i2;i++){
     if(i==i1||i==i2)sum+= (u[i]*w[i+jp]*WEIGHT_0);
     else  sum+=(u[i]*w[i+jp]);
   }
 }
  return(sum*CURRENT_H);
}
 
double do_convolve(v,u,xlo,xhi,x,type)
     double *v,*u,xlo,xhi,x;
     int type;
{
   double di1=xlo/CURRENT_H,di2=xhi/CURRENT_H,dj=x/CURRENT_H;
  int i,i1=(int)di1,i2=(int)di2,cg2=2*CURRENT_GRID;
  int j=(int)dj;
  double sum=0.0,sgn=1.0;
  int k;
 /*  printf("i1=%d i2=%d j=%d type=%d \n",i1,i2,j,type); */
  switch(type){
  case PERIODIC:
    for(i=i1;i<=i2;i++){
      k=j-i;
      if(k<0)
	k+=CURRENT_GRID;
      if(k>=CURRENT_GRID)
	k-=CURRENT_GRID;
      if(i==i1||i==i2)
	sum=sum+v[k]*u[((i+CURRENT_GRID)%CURRENT_GRID)]*WEIGHT_0;
      else 
	sum=sum+v[k]*u[((i+CURRENT_GRID)%CURRENT_GRID)];
    }
    break;
  case ZERO:
    for(i=i1;i<=i2;i++){
      k=abs(j-i);
      if(i<0||k>CURRENT_GRID||i>CURRENT_GRID)continue;
      if(i==i1||i==i2)
	sum=sum+v[k]*u[i]*WEIGHT_0;
      else
	sum=sum+v[k]*u[i];
    }
    break;
  case EVEN:
    for(i=i1;i<=i2;i++){
      k=abs(j-i);
      
      if(k>CURRENT_GRID)k=cg2-k;
      if(i==i1||i==i2)
	sum=sum+v[k]*u[abs(i)]*WEIGHT_0;
      else
	sum=sum+v[k]*u[abs(i)];
    }
    break;
  case ODD:
    for(i=i1;i<=i2;i++){
      k=j-i;
      sgn=1.0;
      if(k<0){k=-k;sgn=-1.0;}
      if(k>CURRENT_GRID){k=cg2-k;sgn=-1.0;}
      if(i==i1||i==i2)
	sum=sum+v[k]*u[abs(i)]*WEIGHT_0*sgn;
      else
	sum=sum+v[k]*u[abs(i)]*sgn;
    }
  
    break;
  }
  return(sum*CURRENT_H);
}


/*   Given a bval index, make the derivatives of the parent and place them
     in the appropriate continuum arrays   */
make_derivs(index)
     int index;
{

  int i;
  double a,b,c;
  int ibl=my_bval[index].bcl,ibr=my_bval[index].bcr,cg1=CURRENT_GRID-1;
  int ip=my_bval[index].parent,ix=my_bval[index].ix,ixx=my_bval[index].ixx;
  double *u,*ux,*uxx,h=CURRENT_H,hm2=1./(h*h),hm=1./h,dx2=.5*hm,
    dxic=2./h;

  if(ip==-1||ix==-1||ixx==-1){
    printf("No differentiable continua allocated \n");
    return(1);
  }
  
  u=my_ctm[ip].u;
  ux=my_ctm[ix].u;
  uxx=my_ctm[ixx].u;
/*  printf(" ibl=%d ibr=%d i=%d \n",ibl,ibr,index); */
/*  First get derivatives of the left hand side   */
  switch(ibl){
  case NOFLUX:
    ux[0]=0.0;
    break;
  case PERIODIC:
    ux[0]=dx2*(u[1]-u[cg1]);
    break;
  case ZERO:
	u[0]=0;
    ux[0]=hm*(u[1]-u[0]);
    break;
  case DYNAMIC:
    ux[0]=hm*(u[1]-u[0]);
    break;
  case LEAKY:
    a=evaluate(my_bval[index].ibc[0]);
    b=evaluate(my_bval[index].ibc[1]);
    c=evaluate(my_bval[index].ibc[2]);
/*	printf("a=%f b=%f c=%f \n",a,b,c); */
    if(a!=0.0)
      ux[0]=(c-b*u[0])/a;
    else { 
	if(b==0.0)u[0]=c;
    	else u[0]=c/b;
      ux[0]=hm*(u[1]-u[0]);
    }
    break;
  }
  uxx[0]=dxic*((u[1]-u[0])*hm-ux[0]);

/*  Now do the middle stuff     */
  
  for(i=1;i<CURRENT_GRID;i++){
    ux[i]=dx2*(u[i+1]-u[i-1]);
    uxx[i]=hm2*(u[i+1]+u[i-1]-2.*u[i]);
  }

/* Finally the right-hand side    */

switch(ibr){
  case NOFLUX:
    ux[CURRENT_GRID]=0.0;
    break;
  case PERIODIC:
    ux[CURRENT_GRID]=dx2*(u[1]-u[cg1]);
    break;
  case ZERO:
     u[CURRENT_GRID]=0.0;
    ux[CURRENT_GRID]=hm*(u[CURRENT_GRID]-u[cg1]);
    break;
  case DYNAMIC:
    ux[CURRENT_GRID]=hm*(u[CURRENT_GRID]-u[cg1]);
    break;
  case LEAKY:
    a=evaluate(my_bval[index].ibc[3]);
    b=evaluate(my_bval[index].ibc[4]);
    c=evaluate(my_bval[index].ibc[5]);
    if(a!=0.0)
      ux[CURRENT_GRID]=(c-b*u[CURRENT_GRID])/a;
    else {
	if(b==0.0)u[CURRENT_GRID]=c;
	else u[CURRENT_GRID]=c/b;
    
      ux[CURRENT_GRID]=hm*(u[CURRENT_GRID]-u[cg1]);}
    break;
  }
  uxx[CURRENT_GRID]=dxic*(ux[CURRENT_GRID]-(u[CURRENT_GRID]-u[cg1])*hm);

  return(0);
}
/* for Dynamic boundary conditions:
   b du/dt = c+a*ux 
   so
   du/dt = (c+a*du/dx)/b
*/

do_dudt(index,t,yldot,yrdot)
double t;
int index;
double *yldot,*yrdot;
{
  double a,b,c,bp,cp;
  double eps=1.e-5;
  int ibl=my_bval[index].bcl,ibr=my_bval[index].bcr,ix=my_bval[index].ix;
  if(ibl==ZERO)*yldot=0.0;
  if(ibr==ZERO)*yrdot=0.0;
/*  printf(" In dudt, index=%d \n",index); */
  if(ibl==DYNAMIC){
    CURRENT_INDEX=0;
    SpaceValue[0]=0.0;
    a=evaluate(my_bval[index].ibc[0]);
    b=evaluate(my_bval[index].ibc[1]);
    c=evaluate(my_bval[index].ibc[2]);
    if(b!=0.0) *yldot=(c+a*my_ctm[ix].u[0])/b;
/*    printf("a=%f b=%f c=%f ux=%f \n",a,b,c,my_ctm[ix].u[0]); */
  }

    
  if(ibl==LEAKY){

    a=evaluate(my_bval[index].ibc[0]);
	if(a==0.0){
		set_ivar(0,t-eps);
    b=evaluate(my_bval[index].ibc[1]);
    c=evaluate(my_bval[index].ibc[2]);
    		set_ivar(0,t+eps);
	 bp=evaluate(my_bval[index].ibc[1]);
    cp=evaluate(my_bval[index].ibc[2]);
	if(bp==0.0||b==0.0)*yldot=0.0;
	else	*yldot=(cp/bp-c/b)/(2*eps);
	set_ivar(0,t);
	}
  }
/*  printf("*yldot=%f \n",*yldot); */
if(ibr==DYNAMIC){
    CURRENT_INDEX=CURRENT_GRID;
    SpaceValue[0]=CURRENT_GRID*CURRENT_H;
    a=evaluate(my_bval[index].ibc[3]);
    b=evaluate(my_bval[index].ibc[4]);
    c=evaluate(my_bval[index].ibc[5]);
    if(b!=0.0) *yrdot=(c+a*my_ctm[ix].u[CURRENT_GRID])/b;
  }

  if(ibr==LEAKY){
     a=evaluate(my_bval[index].ibc[3]);
	if(a==0.0){
		set_ivar(0,t-eps);
    b=evaluate(my_bval[index].ibc[4]);
    c=evaluate(my_bval[index].ibc[5]);
    		set_ivar(0,t+eps);
	 bp=evaluate(my_bval[index].ibc[4]);
    cp=evaluate(my_bval[index].ibc[5]);
	if(bp==0.0||b==0.0)*yrdot=0.0;
	else	*yrdot=(cp/bp-c/b)/(2*eps);
	set_ivar(0,t);
	}
 
  }
/*  printf("*yrdot=%f \n",*yrdot); */
}





