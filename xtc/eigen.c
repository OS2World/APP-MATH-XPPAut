#include <stdio.h>
#include <math.h>
double sign();
double sdot();
double Max();
double amax();

      eigen( n,a,ev,work,ierr)
	int n,*ierr;
	double *a,*ev,*work;   
   {
       int i;
      orthes(n,1,n,a,work);
      hqr(n,1,n,a,ev,ierr);
      }


      hqr( n, low, igh,h,ev,ierr)
      int n,low,igh,*ierr;
      double *h,*ev;
      {
      int i,j,k,l,m,en,ll,mm,na,its,mp2,enm2;
      double p,q,r,s,t,w,x,y,zz,norm,machep=1.e-10;
      int notlas;
      *ierr = 0;
      norm = 0.0;
      k = 1;
      for( i = 1;i<= n;i++)
      {
	 for(j = k;j<= n;j++)
   	 norm = norm + fabs(h[i-1+(j-1)*n]);
	 k = i;
	 if ((i >= low)&&( i<=  igh))continue;
	 ev[(i-1)*2] = h[i-1+(i-1)*n];
	 ev[1+(i-1)*2] = 0.0;
      }
      en = igh;
      t = 0.0;
l60:   if (en < low) return;
      its = 0;
      na = en - 1;
      enm2 = na - 1;
l70:   for( ll = low;ll<= en;ll++)
      {
	 l = en + low - ll;
	 if (l == low) break;
	 s = fabs(h[l-2+(l-2)*n]) + fabs(h[l-1+(l-1)*n]);
	 if (s == 0.0) s = norm;
	 if (fabs(h[l-1+(l-2)*n]) <= machep * s) break;
      }
      x = h[en-1+(en-1)*n];
      if (l == en) goto l270;
      y = h[na-1+(na-1)*n];
      w = h[en-1+(na-1)*n] * h[na-1+(en-1)*n];
      if (l == na) goto l280;
      if (its == 30) goto l1000;
      if ((its != 10) && (its != 20)) goto l130;
      t = t + x;
      for(i = low;i<= en;i++)
      h[i-1+(i-1)*n] = h[i-1+(i-1)*n] - x;
      s = fabs(h[en-1+(na-1)*n]) + fabs(h[na-1+(enm2-1)*n]);
      x = 0.75 * s;
      y = x;
      w = -0.4375 * s * s;
l130:  its = its++;
      for(mm = l;mm <= enm2;mm++)
      {
	 m = enm2 + l - mm;
	 zz = h[m-1+(m-1)*n];
	 r = x - zz;
	 s = y - zz;
	 p = (r * s - w) / h[m+(m-1)*n] + h[m-1+m*n];
	 q = h[m+m*n] - zz - r - s;
	 r = h[m+1+m*n];
	 s = fabs(p) + fabs(q) + fabs(r);
	 p = p / s;
	 q = q / s;
	 r = r / s;
	 if (m == l) break;
	 if ((fabs(h[m-1+(m-2)*n])*(fabs(q)+fabs(r)))<=(machep*fabs(p)
         * (fabs(h[m-2+(m-2)*n])+ fabs(zz) + fabs(h[m+m*n])))) break;
  }
      mp2 = m + 2;
      for( i = mp2;i<= en;i++)
      {
	 h[i-1+(i-3)*n] = 0.0;
	 if (i == mp2) continue;
	 h[i-1+(i-4)*n] = 0.0;
      }
      for( k = m;k<= na;k++) /*260 */
      {
	 notlas=0;
	 if(k != na)notlas=1;
	 if (k == m) goto l170;
	 p = h[k-1+(k-2)*n];
	 q = h[k+(k-2)*n];
	 r = 0.0;
	 if (notlas) r = h[k+1+(k-2)*n];
	 x=fabs(p) + fabs(q) + fabs(r);
	 if (x == 0.0) continue;
	 p = p / x;
	 q = q / x;
	 r = r / x;
l170:	 s = sign(sqrt(p*p+q*q+r*r),p);
	 if (k != m)
	 h[k-1+(k-2)*n] = -s * x;
	 else if (l != m) h[k-1+(k-2)*n] = -h[k-1+(k-2)*n];
  	 p = p + s;
	 x = p / s;
	 y = q / s;
	 zz = r / s;
	 q = q / p;
	 r = r / p;
	 for(j = k;j<= en;j++)
	 {
	    p = h[k-1+(j-1)*n] + q * h[k+(j-1)*n];
	    if (notlas)
	    {
	     p = p + r * h[k+1+(j-1)*n];
	    h[k+1+(j-1)*n] = h[k+1+(j-1)*n] - p * zz;
	    }
  	    h[k+(j-1)*n] = h[k+(j-1)*n] - p * y;
	    h[k-1+(j-1)*n] = h[k-1+(j-1)*n] - p * x;
        }
	 j = imin(en,k+3);
	 for(i = l;i<= j ;i++)
	 {
	    p = x * h[i-1+(k-1)*n] + y * h[i-1+k*n];
	    if (notlas)
	    {
	     p = p + zz * h[i-1+(k+1)*n];
	    h[i-1+(k+1)*n] = h[i-1+(k+1)*n] - p * r;
	    }
  	    h[i-1+k*n] = h[i-1+k*n] - p * q;
	    h[i-1+(k-1)*n] = h[i-1+(k-1)*n] - p;
         }
    }
      goto l70;
l270:
      ev[(en-1)*2]=x+t;
      ev[1+(en-1)*2]=0.0;
      en = na;
      goto l60;
l280:
      p = (y - x) / 2.0;
      q = p * p + w;
      zz = sqrt(fabs(q));
      x = x + t;
      if (q < 0.0) goto l320;
      zz = p + sign(zz,p);
      ev[(na-1)*2] = x + zz;
      ev[(en-1)*2] = ev[(na-1)*2];
      if (zz != 0.0) ev[(en-1)*2] = x-w/zz;
      ev[1+(na-1)*2] = 0.0;
      ev[1+(en-1)*2] = 0.0;
      goto l330;
l320:
      ev[(na-1)*2] = x+p;
      ev[(en-1)*2] = x+p;
      ev[1+(na-1)*2] = zz;
      ev[1+(en-1)*2] = -zz;
l330:
     en = enm2;
      goto l60;

l1000:
     *ierr = en;
}
      orthes(n,low,igh,a,ort)
      int n,low,igh;
      double *a,*ort;
      {
      int i,j,m,ii,jj,la,mp,kp1;
      double f,g,h,scale;
      la = igh - 1;
      kp1 = low + 1;
      if (la < kp1) return;
      for(m = kp1;m<=la;m++) /*180*/
      {
	 h = 0.0;
	 ort[m-1] = 0.0;
	 scale = 0.0;
	 for(i = m;i<= igh;i++)	 scale = scale + fabs(a[i-1+(m-2)*n]);
	 if (scale == 0.0) continue;
	 mp = m + igh;
	 for( ii = m;ii<= igh;ii++) /*100*/
	 {
	    i = mp - ii;
	    ort[i-1] = a[i-1+(m-2)*n] / scale;
	    h = h + ort[i-1] * ort[i-1];
	 }
	 g = -sign(sqrt(h),ort[m-1]);
	 h = h - ort[m-1] * g;
	 ort[m-1] = ort[m-1] - g;
	 for(j = m;j<= n;j++) /*130 */
	 {
	    f = 0.0;
	    for( ii = m;ii<= igh;ii++)
	    {
	       i = mp - ii;
	       f = f + ort[i-1] * a[i-1+(j-1)*n];
	    }
	    f = f / h;
	    for(i = m;i<= igh;i++)
	    a[i-1+(j-1)*n] = a[i-1+(j-1)*n] - f * ort[i-1];
	}
	 for(i = 1;i<= igh;i++) /*160*/
	 {
	    f = 0.0;
	    for( jj = m;jj<= igh;jj++) /*140 */
	    {
	       j = mp - jj;
	       f = f + ort[j-1] * a[i-1+(j-1)*n];
	    }
	    f = f / h;
	    for(j = m;j<= igh;j++)
  	    a[i-1+(j-1)*n] = a[i-1+(j-1)*n] - f * ort[j-1];
         }
	 ort[m-1] = scale * ort[m-1];
	 a[m-1+(m-2)*n] = scale * g;
    }
 }

double sign( x, y)
double x,y;
{
 if(y>=0.0) return(fabs(x));
 return(-fabs(x));
}

imin( x, y)
int x,y;
{
 if(x<y)return(x);
 return(y);
}

double amax( u, v)
double u,v;
{
 if(u>v)return(u);
 return(v);
}


double Max( x, y)
double x,y;
{
 if(x>y)return(x);
 return(y);
}

double Min( x, y)
double x,y;
{
 if(x<y)return(x);
 return(y);
}

sgefa(a,lda, n,ipvt,info)
double *a;
int lda, n, *ipvt, *info;
{
 int j,k,kp1,l,nm1;
 double t;
 *info=-1;
 nm1=n-1;
 if(nm1>0)
 {
  for(k=1;k<=nm1;k++)
  {
   kp1=k+1;
   l=isamax(n-k+1,&a[(k-1)*lda+k-1],lda)+k-1;
   ipvt[k-1]=l;
   if(a[l*lda+k-1]!=0.0)
   {
    if(l!=(k-1))
    {
     t=a[l*lda+k-1];
     a[l*lda+k-1]=a[(k-1)*lda+k-1];
     a[(k-1)*lda+k-1]=t;
    }
    t=-1.0/a[(k-1)*lda+k-1];
    sscal(n-k,t,(a+k*lda+k-1),lda);
    for(j=kp1;j<=n;j++)
    {
     t=a[l*lda+j-1];
     if(l!=(k-1))
     {
      a[l*lda+j-1]=a[(k-1)*lda+j-1];
      a[(k-1)*lda+j-1]=t;
     }
     saxpy(n-k,t,(a+k*lda+k-1),lda,(a+k*lda+j-1),lda);
   }
  }
  else *info=k-1;
 }
}
 ipvt[n-1]=n-1;
 if(a[(n-1)*lda+n-1]==0.0)*info=n-1;
}

sgesl(a,lda, n,ipvt,b,job)
double *a,*b;
int lda,n,*ipvt,job;
{
 int k,kb,l,nm1;
 double t;
 nm1=n-1;
/* for(k=0;k<n;k++)printf("ipiv=%d  b=%f \n",
			ipvt[k],b[k]);*/


 if(job==0)
 {
  if(nm1>=1)
  {
   for(k=1;k<=nm1;k++)
   {
    l=ipvt[k-1];
    t=b[l];
    if(l!=(k-1))
    {
     b[l]=b[k-1];
     b[k-1]=t;
    }
    saxpy(n-k,t,(a+lda*k+k-1),lda,(b+k),1);
   }
  }
  for(kb=1;kb<=n;kb++)
  {
   k=n+1-kb;
   b[k-1]=b[k-1]/a[(k-1)*lda+k-1];
   t=-b[k-1];
   saxpy(k-1,t,(a+k-1),lda,b,1);
  }
  return;
}
  for(k=1;k<=n;k++)
  {
   t=sdot(k-1,(a+k-1),lda,b,1);
   b[k-1]=(b[k-1]-t)/a[(k-1)*lda+k-1];
  }
  if(nm1>0)
  {
   for(kb=1;kb<=nm1;kb++)
   {
    k=n-kb;
    b[k-1]=b[k-1]+sdot(n-k,(a+k*lda+k-1),lda,b+k,1);
    l=ipvt[k-1];
    if(l!=(k-1))
    {
     t=b[l];
     b[l]=b[k-1];
     b[k-1]=t;
    }
   }
   }
}

saxpy(n, sa,sx,incx,sy,incy)
int n,incx,incy;
double sa,*sx, *sy;
{
 int i,ix,iy,m;
 if(n<=0)return;
 if(sa==0.0)return;
  ix=0;
  iy=0;
  if(incx<0)ix=-n*incx;
  if(incy<0)iy=-n*incy;
  for(i=0;i<n;i++,ix+=incx,iy+=incy)
  sy[iy]=sy[iy]+sa*sx[ix];
}



isamax(n,sx,incx)
double *sx;
int incx,n;
{
 int i,ix,imax;
 double smax;
 if(n<1)return(-1);
 if(n==1)return(0);
 if(incx!=1)
 {
  ix=0;
  imax=0;
  smax=fabs(sx[0]);
  ix+=incx;
  for(i=1;i<n;i++,ix+=incx)
  {
   if(fabs(sx[ix])>smax)
   {
    imax=i;
    smax=fabs(sx[ix]);
    }
   }
   return(imax);
}
 imax=0;
 smax=fabs(sx[0]);
 for(i=1;i<n;i++)
 {
  if(fabs(sx[i])>smax)
  {
   imax=i;
   smax=fabs(sx[i]);
  }
 }
 return(imax);
}


double sdot( n,sx,incx,sy,incy)
int n,incx,incy;
double *sx, *sy;
{
int i,ix,iy,m;
double stemp=0.0;
if(n<=0)return(0.0);
 ix=0;
 iy=0;
 if(incx<0)ix=-n*incx;
 if(incy<0)iy=-n*incy;
 for(i=0;i<n;i++,ix+=incx,iy+=incy)
 stemp+=sx[ix]*sx[iy];
 return(stemp);
}

sscal( n, sa,sx,incx)
int n,incx;
double sa,*sx;
{
 int i,m,mp1,nincx;
 if(n<=0)return;
  nincx=n*incx;
  for(i=0;i<nincx;i+=incx)
  sx[i]*=sa;
}

