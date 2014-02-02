 extern int CURRENT_GRID,NCTM,NSCALAR,AUX_CTM,AUX_SCAL;
 extern double CURRENT_H,TFinal,Transient;
 extern double BOUND;
 extern int BufSize;
 int TotalRecord;
 int BufIndex;
 float *DataBuffer;
 double fabs();
get_time_scale(tmin,tmax)
float *tmin,*tmax;
{
 int i;

 int reclen=1+NSCALAR+AUX_SCAL+(NCTM+AUX_CTM)*(CURRENT_GRID+1);
 if(BufIndex==0){
	*tmin=0.0;
	*tmax=1.0;
 	return; 
 }
 *tmin=DataBuffer[0];
 *tmax=DataBuffer[(BufIndex-1)*reclen];
}
	
var_to_type(iv,in,type)
     int iv,*in,*type;
{
  int ndym=NCTM+NSCALAR;
  int n2=ndym+AUX_SCAL;
  int ntot=n2+AUX_CTM;
  if(iv<NSCALAR){*in=iv;*type=0;}
  if(iv>=NSCALAR&&iv<ndym){*in=iv-NSCALAR;*type=1;}
  if(iv>=ndym&&iv<n2){*in=iv-ndym;*type=2;}
  if(iv>=n2&&iv<ntot){*in=iv-n2;*type=3;}
  if(iv<0||iv>=ntot)return(1);
  return(0);
}

qwik_data(in,type,ix,it,v)
     int ix,it,type,in;
     float *v;
{
  int ng=CURRENT_GRID+1;
  int recsize=(AUX_CTM+NCTM)*ng+AUX_SCAL+NSCALAR+1;
  int dynsize=ng*NCTM+NSCALAR+1;

 switch(type){
  case 0:
    *v=DataBuffer[it*recsize+in+1];
    return(0);
  case 1:
    *v=DataBuffer[it*recsize+NSCALAR+1+ng*in+ix];
    return(0);
  case 2:
    *v=DataBuffer[it*recsize+dynsize+in];
    return(0);
  case 3:
    *v=DataBuffer[it*recsize+dynsize+AUX_SCAL+ng*in+ix];
    return(0);
  }
}
  


get_data(iv,ix,it,v)
     int ix,it,iv;
     float *v;
{
  int in,type;

  if(it>=BufIndex){
    printf("Not stored... \n");
    return(1);
  }
  if(var_to_type(iv,&in,&type)){
    printf("No such variable ... \n");
    return(1);
  }
  qwik_data(in,type,ix,it,v);
  return(0);
 }  
  
  
     
open_data()
{
 int recsize=sizeof(float)*(1+NSCALAR+AUX_SCAL+(NCTM+AUX_CTM)*(CURRENT_GRID+1));
 BufIndex=0;
 DataBuffer=(float *)malloc(recsize*BufSize);
}


realloc_data()
{
  int recsize=sizeof(float)*(1+NSCALAR+AUX_SCAL+(NCTM+AUX_CTM)*(CURRENT_GRID+1));
 BufIndex=0;
 DataBuffer=(float *)realloc((void *)DataBuffer,recsize*BufSize);
}

save_data(t,y,aux)
     double *y,*aux,t;
{
  int len_y=NSCALAR+NCTM*(CURRENT_GRID+1);
  int len_a=AUX_SCAL+AUX_CTM*(CURRENT_GRID+1);
  int recsize=len_a+len_y+1;
  int i0,i,j,ng1=CURRENT_GRID+1;
  double my_y;
  if(BufIndex<BufSize){
    i0=BufIndex*recsize;
    DataBuffer[i0]=t;
    i0++;
    for(i=0;i<NSCALAR;i++){
      my_y=y[i];
      DataBuffer[i+i0]=(float)my_y;
      if(fabs(my_y)>BOUND){
	bound_exceed(my_y,t,0,i,i);
	return(1);
      }
    }
    i0+=NSCALAR;
    for(i=0;i<ng1;i++){
	for(j=0;j<NCTM;j++){
	  my_y=y[i*NCTM+NSCALAR+j];
	  DataBuffer[i+j*ng1+i0]=(float)my_y;
	  if(fabs(my_y)>BOUND){
	    bound_exceed(my_y,t,1,j,i);
	    return(1);
	  }
	}
    }
     if(len_a>0){
	i0+=(NCTM*ng1);
	for(i=0;i<AUX_SCAL;i++)DataBuffer[i+i0]=(float)aux[i];
	i0+=AUX_SCAL;
	for(i=0;i<ng1;i++){
	for(j=0;j<AUX_CTM;j++)DataBuffer[i+j*ng1+i0]=(float)aux[i*AUX_CTM+AUX_SCAL+j];
    }
     }
    BufIndex++;
    return(0);
  }
 err_msg("Buffer exceeded");
return(1);
  
} 
    








