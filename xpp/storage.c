#include <stdio.h>
#include "xpplim.h"
float **storage;
double *WORK;
int MAXSTOR,storind;
int IWORK[1000];

init_stor(nrow,ncol)
int nrow,ncol;
{
 int i;
 WORK=(double *)malloc(WORKSIZE*sizeof(double));
 if(WORK!=NULL){
 storage=(float **)malloc((MAXODE+1)*sizeof(float *));
 MAXSTOR=nrow;
 storind=0;
 if(storage!=NULL){
   i=0;
   while((storage[i]=(float *)malloc(nrow*sizeof(float)))!=NULL){
   i++;
   if(i==ncol)return;
   }
 
 }  
}
err_msg("Cannot allocate sufficient storage");
   exit(0);
}


free_storage(ncol)
int ncol;
{
  int i;
  for(i=0;i<ncol;i++)free(storage[i]);
  free(storage);
  free(WORK);
  
}

 





















