/* Modified by Klaus Gebhardt, 1997 */

#include "xpplim.h"
#include "shoot.h"


#define Set_ivar(a,b) variables[(a)]=(b)



extern BC_STRUCT my_bc[MAXODE];
extern int FIX_VAR,NMarkov,PrimeStart;
extern int *my_ode[];

extern double variables[];

int extra(/*double *y,double t,int nod,int neq */);
int my_rhs(/* double t,double *y,double *ydot,int neq */);
double evaluate(/* int *ar */);

MAIN__()
{
}
main(argc,argv)
     char **argv;
     int argc;
{
  do_main(argc,argv);
}

 extra(y__y, t,nod,neq)
 double *y__y,t;
 int nod,neq;
 {
  int i;
  if(nod>=neq)return;
  Set_ivar(0,t);
  for(i=0;i<nod;i++)
  Set_ivar(i+1,y__y[i]);
  for(i=nod+FIX_VAR;i<nod+FIX_VAR+NMarkov;i++)Set_ivar(i+1,y__y[i-FIX_VAR]);
  for(i=nod;i<nod+FIX_VAR;i++)
  Set_ivar(i+1,evaluate(my_ode[i]));
  
  for(i=nod+NMarkov;i<neq;i++)
  y__y[i]=evaluate(my_ode[i+FIX_VAR-NMarkov]);
}

set_fix_rhs(t,y,neq)
     int neq;
     double t,*y;
{
  int i;
  Set_ivar(0,t);
  for(i=0;i<neq;i++)
    Set_ivar(i+1,y[i]);
  for(i=neq;i<neq+FIX_VAR;i++)
    Set_ivar(i+1,evaluate(my_ode[i]));
}


 my_rhs( t,y,ydot,neq)
double t,*y,*ydot;
int neq;
 {
  int i;
  Set_ivar(0,t);
  for(i=0;i<neq;i++)
  Set_ivar(i+1,y[i]);
  for(i=neq;i<neq+FIX_VAR;i++)
  Set_ivar(i+1,evaluate(my_ode[i]));
 for(i=0;i<neq;i++)

  {
    ydot[i]=evaluate(my_ode[i]);
  }

 }





