/* Modified by Klaus Gebhardt, 1997 */

#include <stdio.h>
#include <string.h>
#include "readpde.h"

#define PRIME 39
#define MAXCRHS 20
#define MAXCFIX 20
#define MAXSRHS 20
#define MAXSFIX 20
#define MAXCPAR 20
#define MAXCAUX 10
#define MAXSAUX 10
#define MAXUPAR 100
#define MAXALLRHS 80

#define CONVAR 0  /* continuum variable */
#define CONPAR 1 /* continuum parameter */
#define CONWGT 2  /* two-d continuum parameter  */

int TuringWeights[MAXCPAR];   /* Stuff for Turing stability analysis  */

int N_TWts;

extern int NFFT;
double evaluate();
double atof();
/* Right-hand sides       */
int *FixCtm[MAXCFIX],*FixScal[MAXSFIX],*RhsScal[MAXSRHS],*RhsCtm[MAXCRHS];
int *AuxCtm[MAXCAUX],*AuxScal[MAXSAUX]; 
int VarExtend=0;
/* RHS strings      */
char *ScalarTxt[MAXSRHS];
char *CtmTxt[MAXCRHS];

/* Indices to right-hand sides    */

int I_fixctm[MAXCFIX],I_varctm[MAXCRHS],I_auxctm[MAXCAUX];
int I_bdryvar[MAXCRHS];
char *EqnList[MAXALLRHS];
int NEqnList;
int NCTM,NSCALAR,NBVAL,FIX_CTM,FIX_SCAL,NUPAR,NUCPAR,AUX_SCAL,AUX_CTM;
extern int NARRAY,MyRGB;
extern int PER_LEN,CURRENT_GRID;
extern double CURRENT_H;
int FastFlag=0;
double DomainSize;
int Method,PeriodFlag,MaxJac,MaxIter;
int Mr,Ml;
int Nout,PlotVar,BufSize,MaxDeriv;
char OutFile[50],BinFile[50];
double Transient;
double DeltaT,Tolerance,DtMin,DtMax,Epsilon;
int RandSeed;
double TFinal,TStart,BOUND;
extern char TimeName[10];
char ucvar_names[MAXCRHS][10];
char ucpar_names[MAXCPAR][10];
char ucaux_names[MAXCAUX][10];
char uaux_names[MAXSAUX][10];
char uvar_names[MAXSRHS][10];
char upar_names[MAXUPAR][10];
char plotvarstring[20];
double default_val[MAXUPAR];

extern double *last_ic;

char *get_first(/* char *string,char *src */);
char *get_next(/* char *src */);
int take_apart(/* char *bob, double *value, char *name */);


load_eqn(filename)
char *filename;
{
 FILE *fp;
 char line[257];
 int flag,ip;
 init_reader();
 fp=fopen(filename,"r");
 if(fp==NULL){
	printf("%s not found ! \n",filename);
	exit(0);
 }
	while(1){
	fgets(line,256,fp);
	if((flag=compiler(line)))break;
	}
	fclose(fp);
	realloc_ctm(CURRENT_GRID);
	if(init_cvars()){
		printf("Failed to initialize ctm vars\n");
		exit(0);
	      }
	if(init_cpars()){
		printf("Failed to initialize ctm pars\n");
		exit(0);
	      }
	Ml=MaxDeriv*NCTM-1;
	Mr=Ml;
	get_plot_name(plotvarstring,&ip);
	PlotVar=-1;
	if(ip>=NSCALAR)PlotVar=ip;
	else if(NCTM>=0)PlotVar=NSCALAR;
	printf("plot variable set to %d \n",PlotVar);

}



init_cvars()
{
int i;
for(i=0;i<NCTM;i++)
	if(init_ctm(ucvar_names[i])){
	  printf("Problem with  %s \n",ucvar_names[i]);
	  return(1);
	}
	return(0);
}

init_cpars()
{
int i;
for(i=0;i<NUCPAR;i++)
	if(init_ctm(ucpar_names[i])){
	 printf("Problem with  %s \n",ucpar_names[i]);
	 return(1);
       }
	return(0);
}


init_reader()
{
 init_rpn();
 init_turing();
 N_TWts=0;
 NCTM=0;
 NFFT=0;
 NSCALAR=0;
 NUPAR=0;
 NUCPAR=0;
 FIX_SCAL=0;
 FIX_CTM=0;
 NBVAL=0;
 CURRENT_GRID=25;
 CURRENT_H=.04;
 MaxDeriv=2;
 NEqnList=0;
 VarExtend=0;
 DomainSize=1.0; 
 DeltaT=.001;
 MyRGB=0;
 BOUND=100.0;
 RandSeed=1;
 TFinal=.01;
 TStart=0.0;
 Method=0;
 FastFlag=0;
 PeriodFlag=0;
 Transient=0.0;
 DtMin=1.e-8;
 DtMax=1.0;
 Tolerance=1.e-4;
 Epsilon=1.e-4;
 MaxIter=20;
 MaxJac=20;
 Nout=100;
 PlotVar=0;
 strcpy(OutFile,"xtc.dat");
 strcpy(BinFile,"xtc.bin");
 BufSize=250;
 strcpy(plotvarstring,"");
}


compiler(bob)
char *bob;
{
 double value;
 int narg,done,nn,iflg=0,l,flag;
 char *ptr,*my_string,*command;
 char *bv[3];
 char myline[256];
 FILE *fp;
 char name[20],formula[256];
 char temp[20];
 int icmd,in,tempcom[400],lentemp,ii,ibval;
 int ibc,iker,iset,ns_rhs=0,nc_rhs=0,iside;
  ptr=bob;
  done=1;
 command=get_first(ptr," ,=");
 get_cmd(command,&icmd);
 switch(icmd){
 case COMMENT: /* ignore and move on ...   */
   break;
 case DONE: return(1);
 case -1:
   /*  look for the '  */
   
   l=strlen(command);
   if(command[l-1]!=PRIME)
     return(1);
   find_rhs_var(command,&in,&flag);
   if(in<0||flag>1){
     printf("%s not a differential equation variable",command); /*  <--- serious error   */
     exit(0);   
   }
   my_string=get_next("=\n");

   printf("form[%d] of type %d = |%s| \n",in,flag,my_string);
 	EqnList[NEqnList]=(char *)malloc(strlen(command)+strlen(my_string)+20);
        sprintf(EqnList[NEqnList],"%s=%s",command,my_string);
        NEqnList++;
 	if(flag==0){ /* this is a scalar variable  */       

	ScalarTxt[in]=(char *)malloc(strlen(my_string)+10);
	strcpy(ScalarTxt[in],my_string);
	strcpy(formula,my_string);
	if(add_expr(formula,tempcom,&lentemp)){
		printf("failed to add expr %s\n",formula);
		exit(0);
	      }
	RhsScal[in]=(int *)malloc((lentemp+5)*sizeof(int));
	for(ii=0;ii<lentemp;ii++)RhsScal[in][ii]=tempcom[ii];
	ns_rhs++;
	prn_comp(tempcom);
    }
 	else{  /* this is ctm rhs   */
	CtmTxt[in]=(char *)malloc(strlen(my_string)+10);
	strcpy(CtmTxt[in],my_string);
	strcpy(formula,my_string);
	if(add_expr(formula,tempcom,&lentemp)){
		printf("failed to add expression %s\n",formula);
		exit(0);
	      }
	RhsCtm[in]=(int *)malloc((lentemp+5)*sizeof(int));
	for(ii=0;ii<lentemp;ii++)RhsCtm[in][ii]=tempcom[ii];
	prn_comp(tempcom);
	nc_rhs++;
 	}

   return(0);
   
 case TIME: my_string=get_next("$\n");
	strcpy(TimeName,my_string);
	add_var(my_string,0.0);

   printf("time variable is |%s| \n",my_string);
   break;
 case SPACE: my_string=get_next("$\n");
   printf("space variable is |%s| \n",my_string);
	create_space_variables(my_string);
   break;

 case PAR:
   printf("Parameters:\n");
   while((my_string=get_next(" ,\n"))!=NULL)
     {
       take_apart(my_string,&value,name);
       default_val[NUPAR]=value;  
       strcpy(upar_names[NUPAR++],name);
       printf("|%s|=%f ",name,value);
       if(add_con(name,value)){
       	printf("Failed to add constant %s=%f\n",name,value);
       	exit(0);  
       }
     }
   printf("\n");
   break; 

 case WEIGHT:  
 case CPAR:
   my_string=get_next(" ");
   get_kertype(my_string,&iker);
   printf(" type = %d \n",iker);
   my_string=get_next("=");
   printf("name = |%s| \n",my_string);
   strcpy(ucpar_names[NUCPAR],my_string);
   NUCPAR++;
   my_string=get_next("$\n");
   printf("formula=|%s|\n",my_string);
   /* add the ctm  */
   if(icmd==CPAR){
     if(add_ctm_var(ucpar_names[NUCPAR-1],my_string,CONPAR))
       {
	 printf("Failed to add CVAR %s\n",my_string);exit(0);
       }
     set_ker(ucpar_names[NUCPAR-1],iker);
     if(iker==READFILE)set_ker(ucpar_names[NUCPAR-1],5);
     if(iker==PERNORM){
       TuringWeights[N_TWts]=NARRAY-1;
       N_TWts++;
       printf("Regarding %s as convolution kernel ind=%d \n",
	      ucpar_names[NUCPAR-1],NARRAY-1);
     }
     if(iker==NORMAL){
       TuringWeights[N_TWts]=NARRAY-1;
       N_TWts++;
       printf("Regarding %s as averaged  kernel ind=%d \n",
	      ucpar_names[NUCPAR-1],NARRAY-1);
     }
     
     
   }
   else {
     if(add_ctm_var(ucpar_names[NUCPAR-1],my_string,CONWGT)){
       printf("Failed to add array %s\n",my_string);exit(0);
	
     }
     if(iker==NORMAL)set_ker(ucpar_names[NUCPAR-1],4);
     if(iker==READFILE)set_ker(ucpar_names[NUCPAR-1],5);
	
   }
	
   break; 

 case LOAD:
   my_string=get_next("\n!");
   printf("loading <%s>\n",my_string);
   if((fp=fopen(my_string,"r"))==NULL)
     {
       printf("Failed to open file %s \n",my_string);
       exit(0);
     }
   else 
     {
       while(!feof(fp)){
	 fgets(myline,256,fp);
	 if((flag=compiler(myline)))break;
       }
       fclose(fp);
     }
   break;

 case SET:
   my_string=get_next("=");
   
   get_set(my_string,&iset);
   my_string=get_next("$\n");
   printf("parameter[%d]=<%s>\n",iset,my_string);
   set_special(iset,my_string);
   break;


 case VAR:
   printf("\nVariables:\n");
   while((my_string=get_next(" ,\n"))!=NULL){
     take_apart(my_string,&value,name);
     if(add_var(name,value)){
     	printf("Failed to add variable %s=%f",name,value); exit(0);} 
     strcpy(uvar_names[NSCALAR],name);
     /*	     last_ic[IN_VARS]=value;   */
     NSCALAR++;
     printf("|%s| ",name);
   }
           printf(" \n");
           break;
 case AUX:
	my_string=get_next("=");
	strcpy(name,my_string);
	my_string=get_next("$\n");
	strcpy(formula,my_string);
	printf("|%s| = <%s> \n",name,my_string);
        EqnList[NEqnList]=(char *)malloc(strlen(name)+strlen(my_string)+10);
        sprintf(EqnList[NEqnList],"%s=%s",name,my_string);
        NEqnList++;
	/* compile de sucka */
	strcpy(uaux_names[AUX_SCAL],name);
	if(add_expr(formula,tempcom,&lentemp)){
		printf("Failed to compile expression %s\n",formula);
		exit(0);
	}
	prn_comp(tempcom);
	AuxScal[AUX_SCAL]=(int *)malloc((lentemp+5)*sizeof(int));
	for(ii=0;ii<lentemp;ii++)AuxScal[AUX_SCAL][ii]=tempcom[ii];
	AUX_SCAL++;
	break;
 	
 case CAUX:
	my_string=get_next("=");
	strcpy(name,my_string);
	my_string=get_next("$\n");
	strcpy(formula,my_string);
	printf("|%s| = <%s> \n",name,my_string);
        EqnList[NEqnList]=(char *)malloc(strlen(name)+strlen(my_string)+10);
        sprintf(EqnList[NEqnList],"%s=%s",name,my_string);
        NEqnList++;
	/* make an array and compile de sucka */
	if(add_ctm_var(name,formula,CONVAR)){
		printf("Failed to add ctm %s\n",formula);exit(0);
	}
	strcpy(ucaux_names[AUX_CTM],name);
	if(add_expr(formula,tempcom,&lentemp)){
		printf("Failed to compile..%s\n",formula);
		exit(0);
	}
	AuxCtm[AUX_CTM]=(int *)malloc((lentemp+5)*sizeof(int));
	for(ii=0;ii<lentemp;ii++)AuxCtm[AUX_CTM][ii]=tempcom[ii];
	I_auxctm[AUX_CTM]=NARRAY-1;
	prn_comp(tempcom);
	AUX_CTM++;
	break;
 	

 case FIX:
	my_string=get_next("=");
	strcpy(name,my_string);
	my_string=get_next("$\n");
	strcpy(formula,my_string);
	printf("|%s| = <%s> \n",name,my_string);
	/* add name and compile de sucka */
	if(add_var(name,0.0)){
		printf("Failed to add name %s=%f\n",name,value);
		exit(0);
	}
	if(add_expr(formula,tempcom,&lentemp)){
		printf("Failed to compile..%s\n",formula);
		exit(0);
	}
	FixScal[FIX_SCAL]=(int *)malloc((lentemp+5)*sizeof(int));
	for(ii=0;ii<lentemp;ii++)FixScal[FIX_SCAL][ii]=tempcom[ii];
	FIX_SCAL++;
	prn_comp(tempcom);
	break;
 	
	
 case CFIX: 
	my_string=get_next("=");
	strcpy(name,my_string);
 	printf("|%s| = ",my_string);
 	my_string=get_next("$\n");
 	printf("<%s>\n",my_string);
	strcpy(formula,my_string);
/* add name and compile */
	if(add_ctm_var(name,my_string,CONVAR)){
		printf("Failed to add ctm..%s\n",formula);
		exit(0);
	}
	I_fixctm[FIX_CTM]=NARRAY-1;
	if(add_expr(formula,tempcom,&lentemp)){
		printf("failed to compile...%s\n",formula);
		exit(0);
	}
	FixCtm[FIX_CTM]=(int *)malloc((lentemp+5)*sizeof(int));
	for(ii=0;ii<lentemp;ii++)FixCtm[FIX_CTM][ii]=tempcom[ii];
	FIX_CTM++;
	prn_comp(tempcom);
 	break;
	
	
 case CVAR:
 	my_string=get_next("=");
 	strcpy(ucvar_names[NCTM],my_string);
 	printf("|%s| = ",my_string);
 	my_string=get_next("$\n");
 	printf("<%s>\n",my_string);

/* add name    */
	if(add_ctm_var(ucvar_names[NCTM],my_string,CONVAR)){
		printf("failed to add ctm...%s\n",my_string);
		exit(0);
	}
        set_ker(ucvar_names[NCTM],VarExtend);
	I_varctm[NCTM]=NARRAY-1;
        if(NCTM==0&&strlen(plotvarstring)==0)
	  strcpy(plotvarstring,ucvar_names[NCTM]);
       	NCTM++;
 	break;



  

 case FUN: 
          my_string=get_next(" ");
          strcpy(name,my_string);
          my_string=get_next(" ");
          narg=atoi(my_string);
          my_string=get_next("$\n");
          strcpy(formula,my_string);
          printf("FUN %s(#%d) = |%s|\n",name,narg,formula);
          if(add_ufun(name,formula,narg)){
          	printf("Bad user function");
          	exit(0); 
          }

          break;
 case BDRY:
   my_string=get_next(" ");
   strcpy(name,my_string);
   my_string=get_next(" ");
   iside=atoi(my_string);
   my_string=get_next("{");
   get_bctype(my_string,&ibc);
   printf(" name=%s side =%d \n",name,iside);
   if(ibc==PERIODIC)PeriodFlag=1;
   if(ibc==LEAKY||ibc==DYNAMIC){
   my_string=get_next("}");
   bv[0]=my_string;
   
   my_string=get_next("{");
   my_string=get_next("}");
   bv[1]=my_string;

   my_string=get_next("{");
   my_string=get_next("}");
   bv[2]=my_string;
   printf(" %s %s %s \n",bv[0],bv[1],bv[2]);
 }
   else {
     printf(" bc is %d \n",ibc);
   }
  find_rhs_var(name,&in,&flag);
  	if(flag!=1){
  		printf("Not a continuum variable!");
		exit(0);
  	}
   
   add_deriv(name,bv,ibc,iside,&ibval);
   I_bdryvar[ibval]=in;
   printf(" %d bdry associated with %d \n",ibval,in);
   break;
 
   }

 
 if(icmd==DONE)return(1);
 return(0);

}  



prn_comp(command)
int *command;
{
 int i=0;
 return;
	while(command[i]!=999)
	printf(" %d \n",command[i++]);
}
set_plot_name(name,in)
	int in;
 	char *name;
{
 if(in<0){strcpy(name,""); return(1);}
 if(in<NSCALAR){ strcpy(name,uvar_names[in]); return(0);}
 if(in<(NSCALAR+NCTM)){strcpy(name,ucvar_names[in-NSCALAR]);return(0);}
 if(in<(NSCALAR+NCTM+AUX_SCAL)){
		strcpy(name,uaux_names[in-NSCALAR-NCTM]);
		return(0);
 }
 if(in<(NSCALAR+NCTM+AUX_SCAL+AUX_CTM)){
	strcpy(name,ucaux_names[in-NSCALAR-NCTM-AUX_SCAL]);
	return(0);
}
 strcpy(name,"");
 return(1);
} 

get_plot_name(name,in)
     char *name;
     int *in;
{
  int type=-1;
 find_rhs_var(name,in,&type);
  if(type==-1||*in<0)return;
 switch(type){
 case 0:
   return;
 case 1:
   *in=*in+NSCALAR;
   return;
 case 2:
   *in=*in+NSCALAR+NCTM;
   return;
 case 3:
   *in=*in+NSCALAR+NCTM+AUX_SCAL;
   return;
 }

}
  
find_rhs_var(name,in,type)
     char *name;
     int *in,*type;
{
 int i,len=0,l;
 *type=0;
 *in=-1;
 for(i=0;i<NSCALAR;i++){
  l=strlen(uvar_names[i]);
  if(strncasecmp(name,uvar_names[i],l)==0){
    if(l>len){
      len=l;
      *in=i;
      *type=0;
    }
  }
}
 for(i=0;i<NCTM;i++){
 l=strlen(ucvar_names[i]);
  if(strncasecmp(name,ucvar_names[i],l)==0){
    if(l>len){
      len=l;
      *in=i;
      *type=1;
    }
  }
} 
 for(i=0;i<AUX_SCAL;i++){
  l=strlen(uaux_names[i]);
  if(strncasecmp(name,uaux_names[i],l)==0){
    if(l>len){
      len=l;
      *in=i;
      *type=2;
    }
  }
}
for(i=0;i<AUX_CTM;i++){
 l=strlen(ucaux_names[i]);
  if(strncasecmp(name,ucaux_names[i],l)==0){
    if(l>len){
      len=l;
      *in=i;
      *type=3;
    }
  }
}
}


get_vocab(command,icmd,vocab,length)
     VOCAB *vocab;
     char *command;
     int *icmd,length;
{
  int i;
  *icmd=-1;
  for(i=0;i<length;i++){
    if(strncasecmp(command,(vocab+i)->name,(vocab+i)->len)==0){
      *icmd=i;
      return;
    }
  }
}
 
set_special(i,s)
int i;
char *s;
{
 int ip;
 switch(i){
 case NPERIOD:
   PER_LEN=atoi(s);
   break;
 case LENGTH:
   DomainSize=atof(s);
   CURRENT_H=DomainSize/CURRENT_GRID;
   break;
 case GRID:
   CURRENT_GRID=atoi(s);
   CURRENT_H=DomainSize/CURRENT_GRID;
   break;
 case FAST:
   FastFlag=atoi(s);
   break;
 case METHOD:
   Method=atoi(s);
   if(Method<0||Method>3)Method=0;
   break;
 case DELTA_T:
   DeltaT=atof(s);
   break;
 case TFINAL:
   TFinal=atof(s);
   break;
 case TRANS:
   Transient=atof(s);
   break;
 case NOUT:
   Nout=atoi(s);
   printf("Nout set to %d \n",Nout);
   break;
 case PLOTVAR:
   strcpy(plotvarstring,s);
   break;
 case EXTEND:
   VarExtend=atoi(s);
   break;
 case RGB:
   MyRGB=atoi(s);
   break;
 case BUFSIZE:
   BufSize=atoi(s);
   break;
 case DTMIN:
   DtMin=atof(s);
   break;
 case DTMAX:
  DtMax=atof(s);
  break;
 case TOLERANCE:
  Tolerance=atof(s);
  break;
 case EPSILON:
  Epsilon=atof(s);
  break;
 case JACUSE:
  MaxJac=atoi(s);
  break;
 case MAXITER:
  MaxIter=atoi(s);
  break;
 case TSTART:
  TStart=atof(s);
  break;
 case MAXDERIV:
   MaxDeriv=atoi(s);
   break;
 case BOUNDSET:
   BOUND=atof(s);
   if(BOUND<=0.0)BOUND=100.0;
   break;
 case SEED:
   RandSeed=atoi(s);
   break;
 }
}

get_set(command,icmd)
     int *icmd;
     char *command;
{
  get_vocab(command,icmd,my_set,NSET);
}

get_bctype(command,icmd)
     int *icmd;
     char *command;
{
  get_vocab(command,icmd,my_bcs,NBCS);
}

get_kertype(command,icmd)
     int *icmd;
     char *command;
{
  get_vocab(command,icmd,my_ker,NKER);
}


get_cmd(command,icmd)
     int *icmd;
     char *command;
{
 get_vocab(command,icmd,my_cmd,NCMD);
}
 

take_apart(bob, value, name)
char *bob,*name;
double *value;
{
 int k,i,l;
 char number[40];
 l=strlen(bob);
 k=strcspn(bob,"=");
 if(k==l)
 {
  *value=0.0;
  strcpy(name,bob);
  }
  else
  {
  strncpy(name,bob,k);
  name[k]='\0';
  for(i=k+1;i<l;i++)number[i-k-1]=bob[i];
  number[l-k-1]='\0';
  *value=atof(number);
  }
}

 




char *get_first(string,src)
char *string,*src;
{
 char *ptr;
 ptr=strtok(string,src);
 return(ptr);
}
char *get_next(src)
char *src;
{
 char *ptr;
 ptr=strtok(NULL,src);
 return(ptr);
}




