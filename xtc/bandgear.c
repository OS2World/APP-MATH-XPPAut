#include <math.h>
#define MAX(a,b) ((a)>(b)? (a):(b))
#define MIN(a,b) ((a)<(b)? (a):(b))

double pertst[7][2][3]={{{2,3,1},{2,12,1}},
                        {{4.5,6,1},{12,24,1}},
			{{7.333,9.167,.5},{24,37.89,2}},
			{{10.42,12.5,.1667},{37.89,53.33,1}},
			{{13.7,15.98,.04133},{53.33,70.08,.3157}},
			{{17.15,1,.008267},{70.08,87.97,.07407}},
			{{1,1,1},{87.97,1,.0139}}};

double sgnum( x, y)
double x,y;
{
 if(y<0.0)return(-fabs(x));
 else return(fabs(x));
}

get_stor(ieul,iback,igear,mlp,mrp,ng,neq,per)
     int ng,neq,per;
     int *igear,*ieul,*iback,*mlp,*mrp;
{
  int n=ng*neq;
  int jacsize,ml=2*neq-1,mr=ml;
  *ieul=n;
  *mlp=ml;
  *mrp=mr;
  if(per)
    jacsize=n*(ml+mr+1)+ml*ml+2*ml*n;
  else jacsize=n*(ml+mr+1);
  *iback=5*n+jacsize;
  *igear=21*n+jacsize+40;
}
    

double sqr2(z)
double z;
{
return(z*z);
}

/* A version of Gear for banded/periodic systems  */

double HMin=1.e-8,HMax=1.0,Toler=1.e-4,Delta=1.e-4;

qgear(n,t,tout,y,work,ml,mr,periodic,jstart)
     int n,ml,mr,periodic,*jstart;
     double *work,*y,*t,tout;
{
 int iwork[200];
 int kflag;
	double error[200];
 bandgear(n,t,tout,y,HMin,HMax,Toler,Delta,2,error,&kflag,jstart,
	  work,iwork,ml,mr,periodic);
 return(kflag);
}

bandgear( n,t, tout,y, hmin, hmax,eps,delta,
     mf,error,kflag,jstart,work,iwork,ml,mr,periodic)
 int n,mf,*kflag,*jstart,*iwork;
 int mr,ml,periodic;
double *t, tout, *y, hmin, hmax, eps,*work,*error,delta;

{

  double deltat,hnew,hold,h,racum,told,r,d;
  double *a,pr1,pr2,pr3,r1;
  double *dermat,*save[8],*save9,*save10,*save11,*save12;
   double enq1,enq2,enq3,pepsh,e,edwn,eup,bnd;
  double *ytable[8],*ymax,*work2;
  int i,iret,maxder,j,k,iret1,nqold,nq,newq;
  int idoub,mtyp,iweval,j1,j2,l,info,job,nt;
  int jacsize=n*(ml+mr+1),jij,nm1=n-1,mt=ml+mr+1;
  if(periodic) jacsize += 2*n*ml+ml*ml;
   for(i=0;i<8;i++)
  {
  save[i]=work+i*n;
  ytable[i]=work+(8+i)*n;
  }
  save9=work+16*n;
  save10=work+17*n;
  save11=work+18*n;
  save12=work+19*n;
  ymax=work+20*n;
  dermat=work+21*n;
   a=work+21*n+jacsize; 
  
  work2=work+21*n+jacsize+10;
 
  if(*jstart!=0)
  {
  
  k=iwork[0];
  nq=iwork[1];
  nqold=iwork[2];
  idoub=iwork[3];
  maxder=6;
  mtyp=1;
  iret1=iwork[4];
  iret=iwork[5];
  newq=iwork[6];
  iweval=iwork[7];
  hold=work2[1];
  h=work2[0];
  hnew=work2[2];
  told=work2[3];
  racum=work2[4];
  enq1=work2[5];
  enq2=work2[6];
  enq3=work2[7];
  pepsh=work2[8];
  e=work2[9];
  edwn=work2[10];
  eup=work2[11];
  bnd=work2[12];

  }
  deltat=tout-*t;
   if(*jstart==0)h=sgnum(hmin,deltat);
  if(fabs(deltat)<hmin)
  {
    return(-1);
  }
  maxder=6;
  for(i=0;i<n;i++)ytable[0][i]=y[i];

L70:
  iret=1;
  *kflag=1;
  if((h>0.0)&&((*t+h)>tout))h=tout-*t;
  if((h<0.0)&&((*t+h)<tout))h=tout-*t;
  if(*jstart<=0) goto L120;

L80:

  for(i=0;i<n;i++)
   for(j=1;j<=k;j++)
     save[j-1][i]=ytable[j-1][i];

    hold=hnew;
    if(h==hold)goto L110;

L100:

    racum=h/hold;
    iret1=1;
    goto L820;

L110:

    nqold=nq;
    told= *t;
    racum=1.0;
    if(*jstart>0)goto L330;
    goto L150;

L120:

    if(*jstart==-1)goto L140;
    nq=1;
        my_rhs(*t,ytable[0],save11,n);

    for(i=0;i<n;i++)
    {
     ytable[1][i]=save11[i]*h;
     ymax[i]=1.00;
    }
   
     hnew=h;
     k=2;
     goto L80;

L140:

    if(nq==nqold)*jstart=1;
    *t=told;
    nq=nqold;
    k=nq+1;
    goto L100;

L150:

   if(nq>6)
   {
    *kflag=-2;
    goto L860;
   }
   switch(nq)
   {

   case 1:
	  a[0]=-1.00;
	  a[1]=-1.00;
	  break;
   case 2:
	  a[0]=-2.0/3.0;
	  a[1]=-1.00;
	  a[2]=-1.0/3.0;
	  break;
   case 3:
	  a[0]=-6.0/11.0;
	  a[1]=-1.00;
	  a[2]=a[0];
	  a[3]=-1.0/11.0;
	  break;
   case 4:
	  a[0]=-.48;
	  a[1]=-1.00;
	  a[2]=-.70;
	  a[3]=-.2;
	  a[4]=-.02;
	  break;
   case 5:
	  a[0]=-120.0/274.;
	  a[1]=-1.00;
	  a[2]=-225./274.;
	  a[3]=-85./274.;
	  a[4]=-15./274.;
	  a[5]=-1./274.;
	  break;
  case 6:
	  a[0]=-180./441.;
	  a[1]=-1.0;
	  a[2]=-58./63.;
	  a[3]=-5./12.;
	  a[4]=-25./252.;
	  a[5]=-3./252.;
	  a[6]=-1./1764;
	  break;
       }
 
L310:
    k=nq+1;
    idoub=k;
    mtyp=(4-mf)/2;
    enq2=.5/(double)(nq+1);
    enq3=.5/(double)(nq+2);
    enq1=.5/(double)nq;
    pepsh=eps;
    eup=sqr2(pertst[nq-1][0][1]*pepsh);
    e=sqr2(pertst[nq-1][0][0]*pepsh);
    edwn=sqr2(pertst[nq-1][0][2]*pepsh);
    if(edwn==0.0)goto L850;
    bnd=eps*enq3/(double)n;
L320:

    iweval=2;
    if(iret==2)goto L750;

L330:
    *t=*t+h;
    for(j=2;j<=k;j++)
     for(j1=j;j1<=k;j1++)
     {
      j2=k-j1+j-1;
      for(i=0;i<n;i++) ytable[j2-1][i]=ytable[j2-1][i]+ytable[j2][i];
     }
     for(i=0;i<n;i++)
     error[i]=0.0;
     for(l=0;l<3;l++)
     {
      my_rhs(*t,ytable[0],save11,n);
      if(iweval<1)
	{ 
       /*  printf("iweval=%d \n",iweval);
         for(i=0;i<n;i++)printf("up piv = %d \n",gear_pivot[i]);*/
	  goto L460;
       }
/*       JACOBIAN COMPUTED   */
/*   Compute banded/periodic jacobian   */
 for(i=0;i<n;i++)save9[i]=ytable[0][i];
    get_band_jac(dermat,save9,*t,save12,save11,n,ml,mr,delta,a[0]*h,periodic);


      for(i=0;i<n;i++)dermat[mt*i+ml] += 1.0;
/*	prtband(dermat,ml,mr,n); */




       iweval=-1;
/* *************************************************  */

      if(periodic)info=borfac(dermat,ml,n-ml);
      else info=bandfac(dermat,ml,mr,n);
   
 
      if(info==0)j1=1;
      else j1=-1;
      if(j1<0)goto L520;

L460:

      for(i=0;i<n;i++)save12[i]=ytable[1][i]-save11[i]*h;
      for(i=0;i<n;i++)save9[i]=save12[i];
      job=0;
      if(periodic)borsol(dermat,save9,ml,n-ml);
      else bandsol(dermat,save9,ml,mr,n);
  /*    for(i=0;i<n;i++)printf("save9[%d]=%g\n",i,save9[i]); */
      nt=n;
      for(i=0;i<n;i++)
      {
       ytable[0][i]=ytable[0][i]+a[0]*save9[i];
       ytable[1][i]=ytable[1][i]-save9[i];
       error[i]+=save9[i];
       if(fabs(save9[i])<=(bnd*ymax[i]))nt--;
      }
      if(nt<=0)goto L560;
   }

L520:

/*        UH Oh */
  
   *t=told;
  if((h<=(hmin*1.000001))&&((iweval-mtyp)<-1))goto L530;
  if(iweval!=0)racum*=.25;
   iweval=mf;
   iret1=2;
   goto L820;

L530:

     *kflag=-3;

L540:

    for(i=0;i<n;i++)
      for(j=1;j<=k;j++)ytable[j-1][i]=save[j-1][i];
    h=hold;
    nq=nqold;
    *jstart=nq;
    goto L860;

L560:

     d=0.0;
     for(i=0;i<n;i++)
     d+=sqr2(error[i]/ymax[i]);
     iweval=0;
     if(d>e)goto L610;
     if(k>=3)
     {
      for(j=3;j<=k;j++)
       for(i=0;i<n;i++)
	ytable[j-1][i]=ytable[j-1][i]+a[j-1]*error[i];
     }
     *kflag=1;
     hnew=h;
     if(idoub<=1)goto L620;
     idoub--;
     if(idoub<=1)
     for(i=0;i<n;i++)save10[i]=error[i];
     goto L770;

L610:

    *kflag-=2;
    if(h<=hmin*1.00001)goto L810;
    *t=told;
    if(*kflag<=-5)goto L790;

L620:

    pr2=1.2*pow(d/e,enq2);
    pr3=1.0e20;
    if((nq<maxder)&&(*kflag>-1))
    {
     d=0.0;
     for(i=0;i<n;i++)
     d+=sqr2((error[i]-save10[i])/ymax[i]);
     pr3=1.4*pow(d/eup,enq3);
    }
    pr1=1.0e20;
    if(nq>1)
    {
     d=0.0;
     for(i=0;i<n;i++)
     d+=sqr2(ytable[k-1][i]/ymax[i]);
     pr1=1.3*pow(d/edwn,enq1);
    }
    if(pr2<=pr3)goto L720;
    if(pr3<pr1)goto L730;

L670:

   r=1.0/MAX(pr1,0.0001);
   newq=nq-1;

L680:

    idoub=10;

    if((*kflag==1)&&(r<1.1))goto L770;
    if(newq<=nq) goto L700;
    for(i=0;i<n;i++)
    ytable[newq][i]=error[i]*a[k-1]/(double)k;

L700:

    k=newq+1;
    if(*kflag==1)goto L740;
    racum=racum*r;
    iret1=3;
    goto L820;

L710:

    if(newq==nq)goto L330;
    nq=newq;
    goto L150;

L720:

    if(pr2>pr1) goto L670;
    newq=nq;
    r=1.0/MAX(pr2,.0001);
    goto L680;

L730:

    r=1.0/MAX(pr3,.0001);
    newq=nq+1;
    goto L680;

L740:

    iret=2;
    h=h*r;
    hnew=h;
    if(nq==newq)goto L750;
    nq=newq;
    goto L150;

L750:

    r1=1.0;
    for(j=2;j<=k;j++)
    {
     r1=r1*r;
     for(i=0;i<n;i++)
     ytable[j-1][i]=ytable[j-1][i]*r1;
    }
    idoub=k;

L770:

    for(i=0;i<n;i++)ymax[i]=MAX(ymax[i],fabs(ytable[0][i]));
    *jstart=nq;
    if((h>0.0)&&(*t>=tout))goto L860;
    if((h<0.0)&&(*t<=tout))goto L860;
    goto L70;

L790:

    if(nq==1)goto L850;
    my_rhs(*t,ytable[0],save11,n);
    r=h/hold;
    for(i=0;i<n;i++)
    {
     ytable[0][i]=save[0][i];
     save[1][i]=hold*save11[i];
     ytable[1][i]=r*save[1][i];
    }
    nq=1;
    *kflag=1;
    goto L150;

L810:

   *kflag=-1;
   hnew=h;
   *jstart=nq;
   goto L860;

L820:

    racum=MAX(fabs(hmin/hold),racum);
    racum=MIN(racum,fabs(hmax/hold));
    r1=1.0;
    for(j=2;j<=k;j++)
    {
     r1=r1*racum;
     for(i=0;i<n;i++)
     ytable[j-1][i]=save[j-1][i]*r1;
    }
    h=hold*racum;
    for(i=0;i<n;i++)
    ytable[0][i]=save[0][i];
    idoub=k;
    if(iret1==1)goto L110;
    if(iret1==2)goto L330;
    if(iret1==3)goto L710;

L850:

   *kflag=-4;

   goto L540;

L860:

   for(i=0;i<n;i++)y[i]=ytable[0][i];
  iwork[0]=k;
  iwork[1]=nq;
  iwork[2]=nqold;
  work2[0]=h;
  work2[1]=hold;
  work2[2]=hnew;
  work2[3]=told;
  work2[4]=racum;
  work2[5]=enq1;
  work2[6]=enq2;
  work2[7]=enq3;
  work2[8]=pepsh;
  work2[9]=e;
  work2[10]=edwn;
  work2[11]=eup;
  work2[12]=bnd;
  iwork[3]=idoub;
  iwork[4]=iret1;
  iwork[5]=iret;
  iwork[6]=newq;
  iwork[7]=iweval;

  return(1);

}

