#include <stdio.h>
#define MIN(a,b) ((a)<(b))? (a):(b)
#define MAX(a,b) ((a)>(b))? (a):(b)
#define Left(a,b) left[(a)*m+(b)]
#define Bottom(a,b) bottom[(a)*n+(b)]
#define Little(a,b) little[(a)*m+(b)]
#define A(r,b) a[(r)*mt+(b)+m]



/*  bandsolve solves Ax=b where A is a banded matrix 
    with ml as the left and mr as the right bandwidth
    n is the number of rows in the matrix.  The array A
    is n*(mr+ml+1) elements numbered as:
    a_{0,-ml},...,a_{0,mr}, ... , a_{n1,-ml},...,a_{n1,mr}
    
    where a_{i,j} is the element of the matrix at row i j columns
    away from the diagonal.  
    b is the right-hand side and on return contains the answer
    a negative return value indicates a zero in the diagonal at that
    row.
      No pivoting is performed 
*/

bandfac(a,ml,mr,n)   /*   factors the matrix    */
     int ml,mr,n;
     double *a;
{
  int i,j,k;
  int n1=n-1,mt=ml+mr+1,row,rowi,m,r0,ri0;
  int mlo;
  double al;
  for(row=0;row<n;row++){
    r0=row*mt+ml;
    if((al=a[r0])==0.0)return(-1-row);
    al=1.0/al;
    m=MIN(mr,n1-row);
    for(j=1;j<=m;j++)a[r0+j]=a[r0+j]*al;
    a[r0]=al;
    for(i=1;i<=ml;i++){
      rowi=row+i;
      if(rowi>n1)break;
      ri0=rowi*mt+ml;
      al=a[ri0-i];
      if(al==0.0)continue;
      for(k=1;k<=m;k++)
	a[ri0-i+k]=a[ri0-i+k]-(al*a[r0+k]);
        a[ri0-i]=-al;
    }



  }
	return(0);
}

bandsol(a,b,ml,mr,n)  /* requires that the matrix be factored   */
     double *a,*b;
     int ml,mr,n;
{
  int i,j,k,r0;
  int mt=ml+mr+1;
  int m,n1=n-1,row;
  double sum;
  for(i=0;i<n;i++){
    r0=i*mt+ml;
    m=MAX(-ml,-i);
    for(j=m;j<0;j++)b[i] += a[r0+j]*b[i+j];
    b[i] *= a[r0];
  }
  for(row=n1-1;row>=0;row--){
    m=MIN(mr,n1-row);
    r0=row*mt+ml;
    for(k=1;k<=m;k++)
      b[row]=b[row]-a[r0+k]*b[row+k];
  }
}







/*  
borsolve(a,work,b,m,n)
 solves border banded systems of the form:
 ***0000 **
 ****000 ** <--- left
 *****00 **
 0*****0 **
 00***** **    

 ******* **  <--- little
 ******* **
    ^
    |
  bottom 

 where m=width of sidebands 
 n is size of banded submatrix
 mxm is size of little matrix
 
 storage required is n*(4m+1)+m*m
 
 The elements of a are in the following order:

 The first (2m+1)n elements are usual banded storage
 
 the next  m*n elements are left matrix 
 l00 l01 ... l1(m-1)
 l10 l11 ...
 .
 .
 .
 l(n-1)0... l(n-1)(m-1)

 the next m*n elements are 
 bottom matrix:
 b(0,0),b(0,1),...,b(0,n-1),
  ...
 b(m-1,0),b(m-1,1),...,b(m-1,n-1)

The last m*n elements are little matrix
z(0,0),z(0,1),..,z(0,m-1),
...
z(m-1,0),...,z(m-1,m-1)

The subroutine per_bor(a,work,m,n) will convert a periodic 
array to the above form:
the periodic array has the form 
a(0,-m),...,a(0,m),
a(1,-m),...,a(1,m),
   ...
a(N-1,-m),...,a(N-1,m)
where N=n+m



*/
/* This fills up the work array given the periodic array a */
per_bor(a,work,m,n)
     double *a,*work;
     int m,n;
{
  int i,j,mn=m*n,m1=m-1,n1=n-1,mt=2*m+1;
  double *left,*bottom,*little;
  left=work;
  bottom=left+mn;
  little=bottom+mn;
  
/* Fill little first   */

  for(i=0;i<m;i++)
    for(j=0;j<m;j++)
      Little(i,j)=A(n+i,j-i);

/* Fill left & bottom */ for(i=0;i<mn;i++){ /* Zero these suckers */
left[i]=0.0; bottom[i]=0.0; } for(i=0;i<m;i++) for(j=0;j<=i;j++){
Bottom(i,j)=A(n+i,m-i+j); 	Bottom(m1-i,n1-j)=A(n+m1-i,i-j-m);
Left(n-m+i,j)=A(n-m+i,m+j-i); 	Left(m1-i,m1-j)=A(m1-i,i-j-m); } }

	    
 
/* Total dimension is (n+m)x(n+m)  */

borfac(a,m,n)
     double *a;
     int m,n;
{
  int i,j,k;
  int n1=n-1,mt=2*m+1,row,rowi;
  int m1=m-1,r0,ri0,col,m0,mn=m*n;
  double al;
  double *work;
  double *left;
  double *bottom,*little;
  work=a+(n+m)*mt;
  per_bor(a,work,m,n);  /*  First set up the arrays   */
  /* Now we do factorization on the banded part   */
  left=work;
  bottom=left+mn;
  little=bottom+mn;
  for(row=0;row<n;row++){
    r0=row*mt+m;
    if((al=a[r0])==0.0)return(-1-row);
    al=1.0/al;
    m0=MIN(m,n1-row);
    for(j=1;j<=m0;j++)a[r0+j] *= al;
    a[r0]=al;
    for(j=0;j<m;j++)Left(row,j) *= al;  /* Normalize extra stuff  */
    for(i=1;i<=m;i++){
      rowi=row+i;
      if(rowi>n1)break;
      ri0=rowi*mt+m;
      al=a[ri0-i];
      if(al==0.0)continue;
      for(k=1;k<=m0;k++)
	a[ri0-i+k] -= (al*a[r0+k]);
      a[ri0-i]=-al;
      for(j=0;j<m;j++)Left(rowi,j)-=(al*Left(row,j));
    }
	
	
  }
 
  /* Now we must deal with the last m rows   */
  for(col=0;col<n;col++){
    m0=MIN(m,n1-col);
    for(i=0;i<m;i++){
      al=Bottom(i,col);
      if(al==0.0)continue;
      for(j=1;j<=m0;j++)
	Bottom(i,col+j) -= (al*A(col,j));
      for(j=0;j<m;j++)
	Little(i,j) -= (al*Left(col,j));
	Bottom(i,col)=-al;
	}
  }
	
/* Finally just do the Gauss elimination on the full little array  */
  for(i=0;i<m;i++){
    al=Little(i,i);
    if(al==0.0)return(-n-1-i);
    al=1./al;
    for(j=i;j<m;j++)Little(i,j) *= al;  /* Normalize   */
    Little(i,i)=al;
    for(j=i+1;j<m;j++){
      al=Little(j,i);
      if(al==0.0)continue;
      for(k=i+1;k<m;k++)Little(j,k) -= (al*Little(i,k));
      Little(j,i)=-al;
    }
  }
  return(0);
}


borsol(a,b,m,n) /* solves border banded problem after it is factored  */
     double *a,*b;
     int n,m;
{
       
 int i,j,k;
  int n1=n-1,mt=2*m+1,row,rowi;
  int m1=m-1,r0,ri0,col,m0,mn=m*n;
  double al;
  double *work;
  double *left;
  double *bottom,*little;
  work=a+(n+m)*mt;
  left=work;
  bottom=left+mn;
  little=bottom+mn;
/*   Do the transformation on the diagonal part  */

 for(i=0;i<n;i++){
   r0=i*mt+m;
   m0=MAX(-m,-i);
   for(j=m0;j<0;j++)b[i] += a[r0+j]*b[i+j];
   b[i] *= a[r0];
 }
 /* Now do the transformation on the Bottom part  */
 for(i=0;i<m;i++)
   for(j=0;j<n;j++)b[i+n] += (Bottom(i,j)*b[j]);
 
 
 /* Transform with the Little guy  */
 for(i=0;i<m;i++){
   for(j=0;j<i;j++)b[i+n] += (Little(i,j)*b[j+n]);
   b[i+n] *= Little(i,i);
 }

    for(i=m-2;i>=0;i--)
      for(j=i+1;j<m;j++)b[i+n] -= (Little(i,j)*b[j+n]);
  
/*  Now the big guy...   */

    for(i=n1;i>=0;i--){
      m0=MIN(m,n1-i);
      r0=i*mt+m;
      for(j=1;j<=m0;j++)b[i] -= (a[r0+j]*b[i+j]);
      for(j=0;j<m;j++)b[i] -= (Left(i,j)*b[n+j]);
    }
  return(0);
}
    
    
prtband(a,ml,mr,n) /* print a banded matrix   */
     double *a;
     int ml,mr,n;
{
  int i,j,k;
  int mt=ml+mr+1;
  for(i=0;i<n;i++){
    for(k=0;k<n;k++){
      j=k-i;
      if(j<-ml||j>mr)printf("%.3f ",0.0);
      else(printf("%.3f ",a[i*mt+ml+j]));
    } 
    printf("\n");
  }
}

prtper(a,m,n) /* print periodic array  */
     double *a;
     int m,n;
{
  int i,j,k;
  int mt=2*m+1,m0;
  double *left,*bottom,*little;
  left=a+(n+m)*mt;
  bottom=left+n*m;
  little=bottom+m*n;
  for(i=0;i<n;i++){
    for(k=0;k<n;k++){
      j=k-i;
      if(j<-m||j>m)printf("%.3f ",0.0);
      else printf("%.3f ",a[i*mt+m+j]);
    }
    for(k=0;k<m;k++)printf("%.3f ",Left(i,k));
    printf("\n");
  }
  for(i=0;i<m;i++){
    for(j=0;j<n;j++)printf("%.3f ",Bottom(i,j));
    for(j=0;j<m;j++)printf("%.3f ",Little(i,j));
    printf("\n");
  }
}
  
prvec(v,n,com)
double *v;
int n;
char *com;
{
 int i;
 printf("%s \n",com);
 for(i=0;i<n;i++)printf("%.4f \n",v[i]);
}






