#include <stdio.h>
#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include<X11/Xutil.h>
#include "xpplim.h"
#include "browse.h"


double evaluate();
double drand48();


extern Window main_win;
extern int DCURY;
extern char uvar_names[MAXODE][12];


extern int NCON,NSYM,NCON_START,NSYM_START;

extern float **storage;
extern int storind;
int hist_len,four_len;
float *my_hist[MAXODE+1];
float *my_four[MAXODE+1];
int HIST_HERE,FOUR_HERE;
extern int NEQ,NODE,NMarkov,FIX_VAR;

extern char *no_hint[],*info_message;
extern Window info_pop;
four_back()
{
 if(FOUR_HERE){
   my_browser.data=my_four;
   my_browser.col0=1;
   refresh_browser(four_len);
 }
}

hist_back()
{
 if(HIST_HERE){
   my_browser.data=my_hist;
   my_browser.col0=1;
   refresh_browser(hist_len);
 }
}

new_four(nmodes,col)
     int nmodes,col;
{
  int i;
  int length=nmodes+1;
  if(FOUR_HERE){
   data_back();
   free(my_four[0]);
   free(my_four[1]);
   free(my_four[2]);
   FOUR_HERE=0;
 }
  four_len=length;
  my_four[0]=(float *)malloc(sizeof(float)*length);
  my_four[1]=(float *)malloc(sizeof(float)*length);
  my_four[2]=(float *)malloc(sizeof(float)*length);
  if(my_four[2]==NULL){
   free(my_four[1]);
   free(my_four[2]);
   err_msg("Cant allocate enough memory...");
   return;
 }
 FOUR_HERE=1;
 for(i=3;i<=NEQ;i++)my_four[i]=storage[i];
 for(i=0;i<length;i++)my_four[0][i]=i;
 sft(my_browser.data[col],my_four[1],my_four[2],length,storind);
 four_back();
  ping();
}

  
new_hist(nbins,zlo,zhi,col,condition)
     int nbins;
     int col;
     double zlo,zhi;
     char *condition;
     
{
  int i,j,n=2,index;
  int command[256];
  int cond=0,flag=1;
  double z;
  double dz=(zhi-zlo)/(double)nbins;
  int length=nbins+1;
  int count=0;
  
  
  if(HIST_HERE){
    data_back();
    free(my_hist[0]);
    free(my_hist[1]);
    HIST_HERE=0;
  }
  hist_len=length;
  my_hist[0]=(float *)malloc(sizeof(float)*length);
  my_hist[1]=(float *)malloc(sizeof(float)*length);
  if(my_hist[1]==NULL){
    free(my_hist[0]);
    err_msg("Cannot allocate enough...");
    return;
  }
  HIST_HERE=1;
  for(i=2;i<=NEQ;i++)my_hist[i]=storage[i];
  for(i=0;i<length;i++){
    my_hist[0][i]=(float)(zlo+dz*i);
    my_hist[1][i]=0.0;
  }
  if(strlen(condition)==0)cond=0;
  else
    {
      if(add_expr(condition,command,&i)){
	err_msg("Bad condition. Ignoring...");
	
      }
      else {
	cond=1;
      }
    }
  /* printf(" cond=%d \n condition=%s \n,node=%d\n", 
	 cond,condition,NODE);  */
  for(i=0;i<storind;i++)
    {
      flag=1;
      if(cond){
	for(j=0;j<NODE+1;j++)set_ivar(j,(double)storage[j][i]);
        for(j=0;j<NMarkov;j++)
	  set_ivar(j+NODE+1+FIX_VAR,(double)storage[j+NODE+1][i]);
	z=evaluate(command);
	if(fabs(z)>0.0)flag=1;
	else flag=0;
      }
      z=(storage[col][i]-zlo)/dz;
      index=(int)z;
      if(index>=0&&index<length&&flag==1){
	my_hist[1][index]+=1.0;
	count++;
      }
    }
  NCON=NCON_START;
  NSYM=NSYM_START;
  hist_back();
  ping();
}

do_stat()
{
 Window temp=main_win;
 static char *n[]={"(M)ean/dev","Other"};
 static char key[]="mo";
 char ch;
 ch=(char)pop_up_list(&temp,"Statistics",n,key,2,10,0,10,12*DCURY+8,
		      no_hint,info_pop,info_message);
 switch(ch){
 case 'm': 
   column_mean();
   break;
 case 'o':
  
   break;
 }
 
}
 
column_mean()
{
 int col,i;
 char bob[100];
 double sum,sum2,ss;
 double mean,sdev;
 if(storind<=1){
   err_msg("Need at least 2 data points!");
   return;
 }
 if(get_col_info(&col)==0)return;
 sum=0.0;
 sum2=0.0;
 for(i=0;i<storind;i++){
   ss=storage[col][i];
   sum+=ss;
   sum2+=(ss*ss);
 }
 mean=sum/(double)storind;
 sdev=sqrt(sum2/(double)storind-mean*mean);
 sprintf(bob,"Mean=%g Std. Dev. = %g ",mean,sdev);
 err_msg(bob);
}
get_col_info(col)
 int *col;
{
 char variable[20];
 strcpy(variable,uvar_names[0]);
 new_string("Variable ",variable);
 find_variable(variable,col);
 if(*col<0){
   err_msg("No such variable...");
   return(0);
 }
 return(1);
}

compute_fourier()
{
  int nmodes=10,col;
  if(NEQ<2){
    err_msg("Need at least three data columns");
    return;
  }
  new_int("Number of modes ",&nmodes);
  if(get_col_info(&col)==1)
    new_four(nmodes,col);
}
 
  

compute_hist()
{
  int nbins=10;
  double zlo=0.0,zhi=1.0;
  char condition[80];
  int col=-1;
  
  condition[0]=0;
  
  new_int("Number of bins ",&nbins);
  new_float("Low ",&zlo);
  new_float("Hi ",&zhi);
  if(get_col_info(&col)==0)return;
  new_string("Condition ",condition);
  new_hist(nbins,zlo,zhi,col,condition);
}
  
  

sft(data,ct,st,nmodes,grid)
int grid,nmodes;
float *data,*ct,*st;
{
 int i,j;
 double sums,sumc;
 double tpi=6.28318530717959;
 double dx,xi,x;
 dx=tpi/(grid);
 for(j=0;j<nmodes;j++){
   sums=0.0;
   sumc=0.0;
   xi=j*dx;
   for(i=0;i<grid;i++){
    x=i*xi;
     sumc+=(cos(x)*data[i]);
     sums+=(sin(x)*data[i]);
   }
   if(j==0){
   ct[j]=sumc/(float)grid;
   st[j]=sums/(float)grid;
 }
   else{
     ct[j]=2.*sumc/(float)grid;
   st[j]=2.*sums/(float)grid;
   }
 }
}














