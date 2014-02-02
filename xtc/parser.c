/* Modified by Klaus Gebhardt, 1997 */

#include <ctype.h>

#ifdef __EMX__
#include <float.h>
#endif
#include <math.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

#define T_NORMAL 0
#define T_FIXED 1
#define T_BIHARM 2

#ifndef DBL_EPSILON
#define DBL_EPSILON 2.23E-15
#endif

#define POP stack[--stack_pointer]
#define PUSH(a) stack[stack_pointer++]=(a)

#define COM(a) my_symb[toklist[(a)]%1000].com 
#define VPOP v_stack[--v_stack_ptr[index]][index]
#define VPUSH(a) v_stack[v_stack_ptr[index]++][index]=(a)
#define VSTACKDEPTH 80

double *v_stack[VSTACKDEPTH],*v_ustack[VSTACKDEPTH];
int *v_stack_ptr;
int *v_uptr;
int             ERROUT;

int TuringFlag;
int TuringHot;
extern int TuringColumn;
double T_Epsilon;

double do_fft_conv();
double do_convolve();
double do_weight();
double do_average();
double eval_ctm2();
double eval_ctm();
double eval_rpn();
double do_biharm();
double myabs();
double do_dif();


double drand48();

/* double atof(); */
/*************************
  RPN COMPILER           *
**************************/


/*****************************
*      PARSER.C              *
*
*
*     parses any algebraic expression
*     and converts to an integer array
*     to be interpreted by the rpe_val
*     function.
*
*     the main data structure is a contiguous
*     list of symbols with their priorities
*     and their symbol value
*
*     on the first pass, the expression is converted to
*     a list of integers without any checking except for
*     valid symbols and for numbers
*
*     next this list of integers is converted to an RPN expression
*     for evaluation.
*
*************************************************************/

/* INIT_RPN    */

init_rpn()
{
	int             i;
	ERROUT = 1;
	NCON = 0;
	NFUN = 0;
	NVAR = 0;
	NSYM = STDSYM;
	NARRAY=0;
	NBVAL=0;
	two_args();
	one_arg();
	add_con("PI", M_PI); /* Pi  */
	add_con("PERIODIC",0.0);
	add_con("EVEN",2.0);
	add_con("ODD",3.0);
	add_con("ZERO",1.0);
	init_continuum();
	ini_stack(CURRENT_GRID+1);
}
add_typical()
{
	add_con("C__0",0.0); /* 0.0 */
	add_con("C__1",1.0); /* 1.0 */
	add_con("C__M1",-1.0); /* -1.0 */
	add_con("C__05",.5);  /* .5  */
	add_con("C__M05",-.5); /* -.5 */
	add_con("C__2",2.0); /* 2.0 */

}
init_continuum()
{
 int i;
 strcpy(SpaceName,"x");
 for(i=0;i<MAXCTM;i++){
   my_ctm[i].special=0;
   my_ctm[i].dim=1;
   my_ctm[i].type=EVEN;
   my_ctm[i].bv=-1;
   my_bval[i].bcl=NOFLUX;
   my_bval[i].parent=-1;
   my_bval[i].ix=-1;
   my_bval[i].ixx=-1;
   my_bval[i].bcr=NOFLUX;
 }
}
   
 
 /*  FREE_UFUNS   */

free_ufuns()
{
 int i;
 for(i=0;i<NFUN;i++)
 {
  free(ufun[i]);
  free(ufun_def[i]);
 }
}



/*  ADD_CONSTANT   */


add_constant(junk)
char *junk;
{
 int len;
 char string[100];
 if(NCON>=200)
 {
  if(ERROUT)printf("too many constants !!\n");
  return(1);
 }
 convert(junk,string);
 len=strlen(string);
 if(len>9)len=9;
 strncpy(my_symb[NSYM].name,string,len);
 my_symb[NSYM].name[len]='\0';
 my_symb[NSYM].len=len;
 my_symb[NSYM].pri=10;
 my_symb[NSYM].arg=0;
 if(NCON<100) my_symb[NSYM].com=300+NCON-1;
 else my_symb[NSYM].com=900+NCON;
 NSYM++;
 return(0);
}

/* GET_TYPE   */

get_type(index)
int index;
{
return(my_symb[index].com);
}

/*   ADD_CON      */

add_con(name,value)
char *name;
double value;
{
 if(NCON>=200)
 {
  if(ERROUT)printf("too many constants !!\n");
  return(1);
 }
 constants[NCON]=value;
 NCON++;
 return(add_constant(name));
}

/*  ADD_VAR          */

add_var(junk, value)
char *junk;
double value;
{
 char string[100];
 int len;
 if(NVAR>=100)
 {
  if(ERROUT)printf("too many variables !!\n");
  return(1);
 }
 convert(junk,string);
 len=strlen(string);
 if(len>9)len=9;
 strncpy(my_symb[NSYM].name,string,len);
 my_symb[NSYM].name[len]='\0';
 my_symb[NSYM].len=len;
 my_symb[NSYM].pri=10;
 my_symb[NSYM].arg=0;
 my_symb[NSYM].com=200+NVAR;
 NSYM++;
 variables[NVAR]=value;
 NVAR++;
 return(0);
}



create_space_variables(name)   /* adds space variable and 5 dummies to list */
char *name;
{
 char string[10];
 int i;
 convert(name,string);
 strcpy(my_symb[NSYM].name,string);
 strcpy(SpaceName,string);
 my_symb[NSYM].len=strlen(string);
 my_symb[NSYM].pri=10;
 my_symb[NSYM].arg=0;
 my_symb[NSYM].com=1100;
 NSYM++;
 for(i=1;i<6;i++){
   sprintf(my_symb[NSYM].name,"%s%d",string,i);
   my_symb[NSYM].len=strlen(my_symb[NSYM].name);
   my_symb[NSYM].pri=10;
   my_symb[NSYM].arg=0;
   my_symb[NSYM].com=1100+i;
   NSYM++;
   SpaceValue[i]=0.0;
 }
}

realloc_ctm(grid)
int grid;
{
 int i,j;
 int ng2=(grid+1)*(grid+1);
 int ndoub=(grid+1)*sizeof(double);
 int ndoub2=(grid+1)*(grid+1)*sizeof(double);
	for(i=0;i<NARRAY;i++){
		if(my_ctm[i].dim==1){
		  my_ctm[i].u=(double *)realloc((void *)my_ctm[i].u,ndoub);
		  for(j=0;j<=grid;j++)my_ctm[i].u[j]=0.0;}
		else {
		  my_ctm[i].u=(double *)realloc((void *)my_ctm[i].u,ndoub2);
		  for(j=0;j<ng2;j++)my_ctm[i].u[j]=0.0;
		}
	      }
 realloc_stack(grid+1);
}
 
/* Add a derivative structure and its boundary value parts  */

add_deriv(name,sbc,bv,side,ibval)
char *name,**sbc;
int bv,side,*ibval;
{
  int in;
  int bval=NBVAL;
  char junk[50];
  int form[100];
  int len,i,j;
  if(find_ctm(name,&in))return(1);
  for(i=0;i<NBVAL;i++){
	if(my_bval[i].parent==in){
	bval=i;
	break;
	}
  }
	if(bval==NBVAL){
		my_ctm[in].bv=NBVAL;
		sprintf(junk,"%s_%s",name,SpaceName);
		my_bval[NBVAL].ix=NARRAY;
		my_bval[NBVAL].ixx=NARRAY+1;
		my_bval[NBVAL].parent=in;
		add_ctm_var(junk,"0",0);
		sprintf(junk,"%s_%s%s",name,SpaceName,SpaceName);
		add_ctm_var(junk,"0",0);
	        NBVAL++;
	}
	*ibval=bval;
	switch(side){
	case 0:
	my_bval[bval].bcl=bv;
	 if(bv==LEAKY||bv==DYNAMIC){
	 	for(j=0;j<3;j++){
	 		my_bval[bval].sbc[j]=(char *)malloc(strlen(sbc[j])+2);
	 		strcpy(my_bval[bval].sbc[j],sbc[j]);
	 		if(add_expr(sbc[j],form,&len))return(1);
	 		my_bval[bval].ibc[j]=
	 	           (int *)malloc((len+2)*sizeof(int));
	 		for(i=0;i<len;i++)my_bval[bval].ibc[j][i]=form[i];
	 	}
	 }
	break;
	case 1:
	my_bval[bval].bcr=bv;
	if(bv==LEAKY||bv==DYNAMIC){
	 	for(j=0;j<3;j++){
		  my_bval[bval].sbc[j+3]=(char *)malloc(strlen(sbc[j])+2);
		  strcpy(my_bval[bval].sbc[j+3],sbc[j]);
		  if(add_expr(sbc[j],form,&len))return(1);
		  my_bval[bval].ibc[j+3]=
		    (int *)malloc((len+2)*sizeof(int));
		  for(i=0;i<len;i++)my_bval[bval].ibc[j+3][i]=form[i];
	 	}
	 }
	break;
	}
	return(0);
}
/* add_derivs(name,sbcl,sbcr,bcl,bcr)
char *name,**sbcl,**sbcr;
int bcl,bcr;
{
 
 if(add_deriv(name,sbcl,bcl,0))return(1);
 if(add_deriv(name,sbcr,bcr,1))return(1);
  return(0);
}
 
*/ 
get_ctm_string(name,string,special)
     char *name,*string;
	int *special;
{
  int in;
  if(find_ctm(name,&in))return(1);
  strcpy(string,my_ctm[in].formula);
	*special=my_ctm[in].special;
  return(0);
}
 
set_ctm_string(name,string,special)
     char *name,*string;
	int special;
{
  int in;
  if(find_ctm(name,&in))return(1);
  strcpy(my_ctm[in].formula,string);
	my_ctm[in].special=special;
  return(0);
}
 
 


 
/*   Add an array type variable and initialize to zero   */

add_ctm_var(name,formula,type)
char *name,*formula;
int type;
{
int i=NARRAY,ndoub,j,len;
char string[20];
if(i>=MAXCTM){
	printf("Maximum continua exceeded ...\n");
	return(1);
}
        my_ctm[i].type=type;
        if((type==CONVAR)||(type==CONPAR))ndoub=CURRENT_GRID+1;
        else ndoub=(CURRENT_GRID+1)*(CURRENT_GRID+1);
        
	my_ctm[i].u=(double *)malloc(ndoub*sizeof(double));
        
	my_ctm[i].formula=(char *)malloc(80);

	my_ctm[i].length=CURRENT_GRID+1;
	
        if(type==CONWGT)my_ctm[i].dim=2;
        else my_ctm[i].dim=1;
        strcpy(my_ctm[i].formula,formula);
         convert(name,string);
 len=strlen(string);
 if(len>9)len=9;
 strncpy(my_symb[NSYM].name,string,len);
 my_symb[NSYM].name[len]='\0';
 my_symb[NSYM].len=len;
 my_symb[NSYM].pri=10;
 my_symb[NSYM].arg=0;
 my_symb[NSYM].com=2000+100*type+NARRAY;
 NSYM++;
 NARRAY++;
   for(j=0;j<ndoub;j++)my_ctm[i].u[j]=0.0;
   return(0);
}




/* ADD_EXPR   */

add_expr(expr,command, length )
char *expr;
int *command, *length;
{
 char dest[512];
 int my_token[256];
 int err,i;
 convert(expr,dest);
 err=make_toks(dest,my_token);
 if(err!=0)return(1);
 err = alg_to_rpn(my_token,command);
 if(err!=0)return(1);
   i=0;
   while(command[i]!=ENDEXP)i++;
   *length=i+1;
/*  for(i=0;i<*length;i++)printf("%d \n",command[i]); */
   return(0);
}

/* ADD_UFUN   */

add_ufun(junk,expr,narg)
char *junk, *expr;
int narg;
{
 char string[50];
 int i;
 int end;
 int len=strlen(junk);

 if(NFUN>=MAXUFUN)
 {
  if(ERROUT)printf("too many functions !!\n");
  return(1);
 }
 if((ufun[NFUN]=(int *)malloc(1024))==NULL)
 {
  if(ERROUT)printf("not enough memory!!\n");
  return(1);
 }
 if((ufun_def[NFUN]=(char *)malloc(256))==NULL)
 {
  if(ERROUT)printf("not enough memory!!\n");
  return(1);
 }

 convert(junk,string);
 if(add_expr(expr,ufun[NFUN],&end)==0)
 {
  if(len>9)len=9;
  strncpy(my_symb[NSYM].name,string,len);
  my_symb[NSYM].name[len]='\0';
  my_symb[NSYM].len=len;
  my_symb[NSYM].pri=10;
  my_symb[NSYM].arg=narg;
  my_symb[NSYM].com=400+NFUN;
  NSYM++;
  ufun[NFUN][end-1]=ENDFUN;
  ufun[NFUN][end]=narg;
  ufun[NFUN][end+1]=ENDEXP;
  strcpy(ufun_def[NFUN],expr);
  NFUN++;
  return(0);
 }
       if(ERROUT)printf(" ERROR IN FUNCTION DEFINITION");
       return(1);
}

#ifdef FULLPARS

/* symb_list          */

symb_list( type)
int type;
{
 int i;
 switch(type)
 {

 case FUNTYPE:    /*     user functions   */
 if(ERROUT)printf("\n USER DEFINED FUNCTIONS:\n");
 for(i=STDSYM;i<NSYM;i++)
  if(is_ufun(my_symb[i].com))
  printf("  %s of %d args = %s \n",my_symb[i].name,my_symb[i].arg,ufun_def[my_symb[i].com-400]);
 break;

 case VARTYPE:   /* variables        */
 printf("\n USER DEFINED VARIABLES:\n");
 for(i=STDSYM;i<NSYM;i++)
  if(is_uvar(my_symb[i].com))
    printf("         %s = %f \n",my_symb[i].name,variables[my_symb[i].com-200]);
 break;

 case CONTYPE:    /* constants      */
 printf("\n USER DEFINED CONSTANTS:\n");
 for(i=STDSYM;i<NSYM;i++)
  if(is_ucon(my_symb[i].com))
    printf("         %s = %f \n",my_symb[i].name,constants[my_symb[i].com-300]);
  break;
  }
}
#endif

/* is_ufun         */

is_ufun( x)
int x;
{
 if((x/100)==UFUN)return(1);
 else return(0);
}

is_2dar(x)
int x;
{
  if((x/100)==42)return(1);
  else return(0);
}

check_num(tok,value)
int *tok;
double value;
{
 int bob,in,m,i;
 for(i=0;i<NSYM;i++){
	
	if(strncmp(my_symb[i].name,"NUM##",5)==0){
	bob=my_symb[i].com;
	in=bob%100;
	m=bob/100;
	if(m==10)in+=100;
	if(constants[in]==value){
	*tok=i;
	return(1);
	}
	}
 }
 return(0);
}
	
	
		

/* IS_UCON        */

is_ucon( x)
int x;
{
 if(((x/100)==3)||((x/100)==10))return(1);
 else return(0);
}

/* IS_UVAR       */

is_uvar(x)
int x;
{
 if((x/100)==2)return(1);
 else return(0);
}

is_array(x)
int x;
{
 int z=x/100;
 if(z==20||z==21||z==22)return(1);
 else return(0);
}
/* FIND_NAME    */

find_name(string, index)
 char *string;
 int *index;
{
  char junk[100];
  int i,len;
  convert(string,junk);
  len=strlen(junk);
  for(i=0;i<NSYM;i++)
  {
   if(len==my_symb[i].len)
    if(strncmp(my_symb[i].name,junk,len)==0)break;
  }
   if(i<NSYM)
    *index=i;
   else *index=-1;
}

/* GET_VAL   */

get_val(name,value)
char *name;
double *value;
{
  int type,com;
  *value=0.0;
  find_name(name,&type);
  if(type<0)return(0);
  com=my_symb[type].com;
  if(is_ucon(com))
  {
   if(com<400)
   *value=constants[com-300];
   else *value=constants[com-900];
   return(1);
  }
  if(is_uvar(com))
  {
   *value=variables[com-200];
   return(1);
  }
  return(0);
}

/* SET_VAL         */

single_value(name,x,y,value,flag,twod)
     char *name;
     double x,y;
     double *value;
     int flag;
     int twod;
{
  int type,com,in,it;
  find_name(name, &type);
  if(type<0)return;
  com=my_symb[type].com;
  it=com/100;
  in=com%100;
  if(twod!=2){
    if(it!=21&&it!=20)return;
    if(flag)*value=eval_ctm(in,x);
    else setval_ctm(in,x,*value);
  }
  else{
    if(it!=22)return;
    if(flag)*value=eval_ctm2(in,x,y);
    else setval_ctm2(in,x,y,*value);
  }
}

get_ctm_arrays(name,u,ux,uxx)
     char *name;
     double **u,**ux,**uxx;
{
 int in,bv,ix,ixx;
 if(find_ctm(name,&in))return(1);
 *u=my_ctm[in].u;
 bv=my_ctm[in].bv;
 if(bv>=0){
   ix=my_bval[bv].ix;
   ixx=my_bval[bv].ixx;
   *ux=my_ctm[ix].u;
   *uxx=my_ctm[ixx].u;
 }
 return(0);
}

save_ctm()
{
  char name[20];
  int in,it,com;
  name[0]=0;
  new_string("Ctm to save: ",name);
  if(strlen(name)==0)return(0);
  find_name(name,&in);
  if(in<0){
    err_msg("Name not found");
    return(1);
  }
  com=my_symb[in].com;
  in=com%100;
  it=com/100;
  switch(it){
  case 20:
  case 21:
    return(write_1d_file(my_ctm[in].u));
  case 22:
    return(write_2d_file(my_ctm[in].u));
  default:
    err_msg("Not a continuum");
    return(1);
  }
}
init_ctm(name)
char *name;
{
 int in,com;
 int it;
 char formula[256];
 find_name(name,&in);
 if(in<0){
	printf("cannot find %d\n",name);
	return(1);
 }
	com=my_symb[in].com;
	in=com%100;
	it=com/100;
	switch(it){
	case 20:
	case 21:
	       strcpy(formula,my_ctm[in].formula);
	       return(fill_array(formula,my_ctm[in].u,my_ctm[in].special));
	case 22:
		strcpy(formula,my_ctm[in].formula);
		return(fill_2d_array(formula,my_ctm[in].u,my_ctm[in].special));
	default:
		printf("Not an array...");
		return(1);
	}
}
find_ctm(name,in)
     char *name;
     int *in;
{
 int type,com,it;
  find_name(name, &type);
  if(type<0)return(1);
  com=my_symb[type].com;
  it=com/100;
  *in=com%100;
  if(it!=21&&it!=20&&it!=22){
    printf("%s is not ctm expression ! \n",name);
    return(1);
  }
  return(0);
}  

set_ker(name,special)
char *name;
int special;
{
 int in;
 if(find_ctm(name,&in))return(1);
 my_ctm[in].special=special;
 return(0);
}

set_ctm(name,formula,special)
     char *name;
     char *formula;
     int special;
{
  int in,type;
  if(find_ctm(name,&in))return(1);
  strcpy(my_ctm[in].formula,formula);
 my_ctm[in].special=special;
 type=my_ctm[in].type;
  if(type==CONPAR||type==CONVAR)
	return(fill_array(formula,my_ctm[in].u,special));
  return(fill_2d_array(formula,my_ctm[in].u,special));  
}



set_val(name, value)
char *name;
double value;
{
  int type,com;
  find_name(name,&type);
  if(type<0)return(0);
  com=my_symb[type].com;
  if(is_ucon(com))
  {
   if(com<400)
   constants[com-300]=value;
   else constants[com-900]=value;
   return(1);
  }
  if(is_uvar(com))
  {
   variables[com-200]=value;
   return(1);
  }
  return(0);
}



set_ivar(i, value)
int i;
double value;
{
 variables[i]=value;
}

double get_ivar(i)
int i;
{       	 return(variables[i]);
}



/* This alters the tokenlist according to context    
   It converts array names to their symbolic value
   when they occur inside CONV,WEIGHT,RWEIGHT,AVERAGE,BIHARM,DIFOP,FFTOP
   functions.
   It converts SPACE variables to their symbolic value
   inside the INT function
   It converts array variables to evaluative ones when
   they are followed by a parenthesis
  
 */               

first_pass(toklist)
int *toklist;
{
 int i=0;
 int token,tok2,tok3,tok4,tok8,ty;
 /* First make a command table     */

   i=0; 
   while(1)
   {
    token=COM(i);
    if(token==ENDEXP)break;
    if(token==DIFOP){
      tok2=COM(i+2);
      if(!is_array(tok2)){
	printf("Illegal use of difop \n");
	return(1);
      }
      toklist[i+2]=toklist[i+2]+1000;
    }
    if(token==BIHARM){
      tok2=COM(i+2);
      if(!is_array(tok2)){
	printf("Illegal use of biharmonic \n");
	return(1);
      }
      toklist[i+2]=toklist[i+2]+1000;
    }
      
      if(token==WEIGHT||token==AVERAGE||token==RWEIGHT||token==CONV
	 ||token==FFTOP) {
      tok2=COM(i+2);
      tok3=COM(i+4);
      tok4=COM(i+5);
      if(!is_array(tok2)||!is_array(tok3)||tok4 != CCOMMA){
	printf("Illegal use of weights \n");
	return(1);
      }
      toklist[i+2]=toklist[i+2]+1000;
      toklist[i+4]=toklist[i+4]+1000;
    }
    if(token==INT){
      tok2=COM(i+2);
      ty=tok2/100;
      if(ty!=SPACE){
	printf("Illegal use of INT \n");
	return(1);
      }
      toklist[i+2]=toklist[i+2]+3000;
    }
    if(is_array(token)){
      if(COM(i+1)==CLPAREN)toklist[i]=toklist[i]+2000;
    }
    i++;
  }
 return(0);
}
      
get_mod_tok(tok)
int tok;
{
 int flag=tok/1000;
 int in=tok%1000;
 switch(flag){
 case 0: return(my_symb[in].com);
 case 1: return(my_symb[in].com+1000);
 case 2: return(my_symb[in].com+2000);
 case 3: return(my_symb[in].com+100);
 default: return(my_symb[in].com);
 }
}
   

alg_to_rpn(toklist,command)
int *toklist,*command;
{
  int tokstak[500],comptr=0,tokptr=0,lstptr=0,temp;
  int comtab[500];
  int ncomma=0,deltok;
  int loopstk[100];
  int lptr=0;
  int newtok,oldtok;
  int i,my_com,my_arg,jmp;
  char ch;
  int jj;
  
  if(first_pass(toklist)==1)exit(0);
  tokstak[0]=STARTTOK;
  tokptr=1;
  oldtok=STARTTOK;
  while(1)
         {
 getnew:
          newtok=toklist[lstptr++];


          
           

 next:
          if((newtok==ENDTOK)&&(oldtok==STARTTOK))break;
           if(newtok==LPAREN)
           {
             tokstak[tokptr]=LPAREN;
             tokptr++;
             oldtok=LPAREN;
             goto getnew;
            }
	  
           if(newtok==RPAREN)
           {
            switch(oldtok)
                  {
                     case LPAREN:
                                 tokptr--;
                                 oldtok=tokstak[tokptr-1];
                                 goto getnew;
                     case COMMA:
                                 tokptr--;
                                 ncomma++;
                                 oldtok=tokstak[tokptr-1];
                                 goto next;
                  }
           }
           if((newtok==COMMA)&&(oldtok==COMMA))
           {
            tokstak[tokptr]=COMMA;
            tokptr++;
            goto getnew;
           }
           if(my_symb[oldtok%1000].pri>=my_symb[newtok%1000].pri)
           {
            command[comptr]=get_mod_tok(oldtok);
	    if((my_symb[oldtok%1000].arg==2)&&(my_symb[oldtok%1000].com/100==1))
	      ncomma--;
            my_com=command[comptr];
	                comptr++;
	   if(my_com==INT){
	     loopstk[lptr]=comptr; /* add some space for jump */
	     comptr++;
	     lptr++;
	   }    
	    if(my_com==ENDINT){
              lptr--;
	      jmp=comptr-loopstk[lptr]-1;
	      command[loopstk[lptr]]=jmp;
	    }
	   if(my_com==MYIF){
         	     loopstk[lptr]=comptr; /* add some space for jump */
                     comptr++;
		     lptr++;
                }    
	   if(my_com==MYTHEN){ 
		              /* First resolve the if jump */
			lptr--;
			jmp=comptr-loopstk[lptr]-1;
			command[loopstk[lptr]]=jmp;
			   /* Then set up for the then jump */
			loopstk[lptr]=comptr;
			lptr++;
			comptr++;
			}
	   if(my_com==MYELSE){
			     lptr--;
			     jmp=comptr-loopstk[lptr]-1;
			     command[loopstk[lptr]]=jmp;
			     }


			      
           if(my_com==CONV)ncomma-= 5;
           if(my_com==FFTOP)ncomma-=3;
	    if(my_com==AVERAGE)ncomma-=3;
	    if(my_com==WEIGHT||my_com==RWEIGHT)ncomma-= 4;
	    if(my_com==INT)ncomma-= 2;
	    if(my_com==BIHARM)ncomma-=2;
	    if(my_com==DIFOP)ncomma-=3;          
    

   
            /*    CHECK FOR USER FUNCTION       */
            if(is_2dar(my_com))
	      {
		ncomma--;
	      }
            if(is_ufun(my_com))
            {
             my_arg=my_symb[oldtok%1000].arg;
                         command[comptr]=my_arg;
             comptr++;
             ncomma=ncomma+1-my_arg;
            }
           /*      USER FUNCTION OKAY          */
            tokptr--;
            oldtok=tokstak[tokptr-1];
            goto next;
          }
          tokstak[tokptr]=newtok;
          oldtok=newtok;
          tokptr++;
          goto getnew;
       }
        if(ncomma!=0){
        printf("Illegal number of arguments\n");
	return(1);
        }
        command[comptr]=my_symb[ENDTOK].com;
        return(0);
    }


/******************************
*    PARSER                   *
******************************/




 make_toks(dest,my_token)
 char *dest;
 int *my_token;
 {
 char num[40],junk[10];
 double value;
  int old_tok=STARTTOK,tok_in=0;
 int index=0,err,token,nparen=0;
  while(dest[index]!='\0')
  {
   find_tok(dest,&index,&token);
   if((token==MINUS)&&
   ((old_tok==STARTTOK)||(old_tok==COMMA)||(old_tok==LPAREN)))
  token=NEGATE;
  if(token==LPAREN)++nparen;
  if(token==RPAREN)--nparen;
  if(token==NSYM)
   {
    if(do_num(dest,num,&value,&index))return(1);
    if(check_num(&token,value)==0){
    sprintf(junk,"num##%d",NCON);
    constants[NCON]=value;
    NCON++;
    if(add_constant(junk))return(1);
    }
   }
 my_token[tok_in++]=token;
 old_tok=token;
 }

my_token[tok_in++]=ENDTOK;
if(nparen!=0)
{
 if(ERROUT)printf(" parentheses don't match\n");
 return(1);
}
return(0);
}




do_num(source,num,value,ind)
char *source,*num;
double *value;
int *ind;
{
 int j=0,i=*ind,error=0;
 int ndec=0,nexp=0,ndig=0;
 char ch,oldch;
 oldch='\0';
 *value=0.0;
 while(1)
 {
  ch=source[i];
  if(((ch=='+')||(ch=='-'))&&(oldch!='E'))break;
  if((ch=='*')||(ch=='^')||(ch=='/')||(ch==',')||(ch==')')||(ch=='\0'))break;
  if((ch=='E')||(ch=='.')||(ch=='+')||(ch=='-')||isdigit(ch))
  {
   if(isdigit(ch))ndig++;
   switch(ch)
             {
              case 'E':
                       nexp++;
                       if((nexp==2)||(ndig==0))goto err;break;
              case '.':
                       ndec++;
                       if((ndec==2)||(nexp==1))goto err;break;

             }
   num[j]=ch;
   j++;
   i++;
   oldch=ch;
  }
  else
  {
err:
    num[j]=ch;
    j++;
    error=1;
    break;
  }
  }
  num[j]='\0';
  if(error==0)*value=atof(num);
  else
  if(ERROUT)printf(" illegal number string: %s\n",num);
  *ind=i;
  return(error);
}



convert(source,dest)
char *source,*dest;
{
 char ch;
 int i=0,j=0;
 while (1)
 {
  ch=source[i];
  if(!isspace(ch))dest[j++]=ch;
  i++;
  if(ch=='\0')break;
 }
 strupr(dest);
}



find_tok(source,index,tok)
char *source;
int *index,*tok;
{
 int i=*index,maxlen=0,symlen;
 int k,j,my_tok,match;
 my_tok=NSYM;
 for(k=0;k<NSYM;k++)
 {
  symlen=my_symb[k].len;
  if(symlen<=maxlen)continue;

   match=1;
   for(j=0;j<symlen;j++)
   {
    if(source[i+j]!=my_symb[k].name[j])
     {
      match=0;
      break;
     }
   }
   if(match!=0)
    {
     my_tok=k;
     maxlen=symlen;
    }
 }
   *index=*index+maxlen;
   *tok=my_tok;
}


two_args()
{
 fun2[4]=atan2;
 fun2[5]=pow;
 fun2[6]=max;
 fun2[7]=min;
 fun2[8]=fmod;
 fun2[9]=dand;
 fun2[10]=dor;
 fun2[11]=dgt;
 fun2[12]=dlt;
 fun2[13]=deq;
 fun2[14]=dge;
 fun2[15]=dle;
 
 




}




 
 


double myabs(z)
double z;
{
 return(fabs(z));
}

one_arg()
{
 fun1[0]=sin;
 fun1[1]=cos;
 fun1[2]=tan;
 fun1[3]=asin;
 fun1[4]=acos;
 fun1[5]=atan;
 fun1[6]=sinh;
 fun1[7]=tanh;
 fun1[8]=cosh;
 fun1[9]=myabs;
 fun1[10]=exp;
 fun1[11]=log;
 fun1[12]=log10;
 fun1[13]=sqrt;
 fun1[14]=neg;
 fun1[15]=recip;
 fun1[16]=heaviside;
 fun1[17]=signum;
 fun1[18]=floor;
 fun1[19]=rndom;
}

double max(x,y)
double x,y;
{
 return(((x>y)?x:y));
}

double min(x,y)
double x,y;
{
 return(((x<y)?x:y));
}

double neg(z)
double z;
{
 return(-z);
}

double recip(z)
double z;
{
 return(1.00/z);
}

double heaviside(z)
double z;
{
 float w=1.0;
 if(z<0)w=0.0;
 return(w);
}

double rndom( z)
double z;
{
 /*return (z*((double)random()/2147483647.0)); */
 return(z*drand48());
}

double signum(z)
double z;

{
  if(z<0.0)return(-1.0);
  if(z>0.0)return(1.0);
  return(0.0);
}


/*  logical stuff  */



double dand(x,y)
double x,y;
{
 return((double)(x&&y));
}
double dor(x,y)
double x,y;
{
 return((double)(x||y));
}
double dge(x,y)
double x,y;
{
 return((double)(x>=y));
}
double dle(x,y)
double x,y;
{
 return((double)(x<=y));
}
double deq(x,y)
double x,y;
{
 return((double)(x==y));
}
double dne(x,y)
double x,y;
{
 return((double)(x!=y));
}
double dgt(x,y)
double x,y;
{
 return((double)(x>y));
}
double dlt(x,y)
double x,y;
{
 return((double)(x<y));
}


/*              end of logical stuff    */





 double evaluate(equat)
 int *equat;
 {
  uptr=0;
  stack_pointer=0;
  return(eval_rpn(equat));
 }


 double eval_rpn(equat)
 int *equat;
 {
   int i,it,in,j,*tmpeq;
  int sumcount,ii,new[500],is,iord;
  int isum,aind;
  int low,high,ind,iloop,ijmp;
  double temp,temx,temy,temz;
  double tlow,thigh;
  double sum;

  while((i=*equat++)!=ENDEXP)
  {
 
   switch(i)
   {
   case ENDFUN:
   		 i=*equat++;
		
    		 uptr-=i;
		
   		 break;

   case MYIF:
		temx=POP;
		ijmp=*equat++;
		if(temx==0.0)equat+=ijmp;
                break;
   case MYTHEN:
	       ijmp=*equat++;
	       equat+=ijmp;
		break;
   case MYELSE:
		break;
   case DIFOP:
                isum = (int) POP;
                iord = (int) POP;
                temx = POP;
                ind = (int) POP;
	        PUSH(do_dif(my_ctm[ind].u,temx,iord,isum));
		 break;   
   case BIHARM:
		 isum=(int)POP;
		 temx=POP;
		 ind=(int)POP;
		 if(TuringFlag){
		   switch(TuringFlag){
		   case T_FIXED:
		     PUSH(0.0);
		     break;
		   case T_BIHARM:
		     if(ind==TuringHot)
		     PUSH(T_Epsilon);
		     break;
		   }
		 }
		 else PUSH(do_biharm(my_ctm[ind].u,temx,isum));
		 break;
	       
   case CONV:
		 isum=(int)POP;
                 temx=POP;
		 thigh=POP;
		 tlow=POP;
		 ind=(int)POP;
		 aind=(int)POP;
		 if(TuringFlag){
		   if(TuringFlag!=T_BIHARM){
		     if(ind==TuringHot)PUSH(my_ctm[aind].u[1]);
		     if(aind==TuringHot)PUSH(my_ctm[ind].u[1]);
		     if(ind!=TuringHot&&aind!=TuringHot)
		       PUSH(my_ctm[ind].u[TuringColumn]*my_ctm[aind].u[TuringColumn]);
		   }
		   else PUSH(my_ctm[ind].u[TuringColumn]*my_ctm[aind].u[TuringColumn]);
		 }	   
		 else PUSH(do_convolve(my_ctm[aind].u,
				       my_ctm[ind].u,
				       tlow,thigh,temx,isum));
		 break;



 case FFTOP:
		 isum=(int)POP;
                 temx=POP;
		 ind=(int)POP;
		 aind=(int)POP;
		 if(TuringFlag){
		   if(TuringFlag!=T_BIHARM){
		     if(ind==TuringHot)PUSH(my_ctm[aind].u[1]);
		     if(aind==TuringHot)PUSH(my_ctm[ind].u[1]);
		     if(ind!=TuringHot&&aind!=TuringHot)
		       PUSH(my_ctm[ind].u[TuringColumn]*my_ctm[aind].u[TuringColumn]);
		   }
		   else PUSH(my_ctm[ind].u[TuringColumn]*my_ctm[aind].u[TuringColumn]);
		 }	   
		 else PUSH(do_fft_conv(aind,ind,temx,isum));
		 break;

  case WEIGHT: 
		 temx=POP;
		 thigh=POP;
		 tlow=POP;
		 ind=(int)POP;
		 aind=(int)POP;
		 PUSH(do_weight(my_ctm[aind].u,my_ctm[ind].u,
				tlow,thigh,temx,0));
		 break;

  case RWEIGHT:
	      temx=POP;
		 thigh=POP;
		 tlow=POP;
		 ind=(int)POP;
		 aind=(int)POP;
		 PUSH(do_weight(my_ctm[aind].u,my_ctm[ind].u,
				tlow,thigh,temx,1));
		 break;

  case AVERAGE:
		 thigh=POP;
		 tlow=POP;
		  ind=(int)POP;
		 aind=(int)POP;
		  if(TuringFlag){
		    if(TuringFlag!=T_BIHARM){
		      if(ind==TuringHot)PUSH(my_ctm[aind].u[1]);
		      if(aind==TuringHot)PUSH(my_ctm[ind].u[1]);
		      if(ind!=TuringHot&&aind!=TuringHot)
			 PUSH(my_ctm[ind].u[TuringColumn]*my_ctm[aind].u[TuringColumn]);
		    }
		    else PUSH(my_ctm[ind].u[TuringColumn]*my_ctm[aind].u[TuringColumn]);
		  }
                 else PUSH(do_average(my_ctm[aind].u,my_ctm[ind].u,
				 tlow,thigh));
		 break;

   case INT: 
		 
		 thigh=POP/CURRENT_H;
		 tlow=POP/CURRENT_H;
		low=(int)tlow;
		high=(int)thigh;
		 it=(int)POP;
		 ijmp=*equat++;

		 sum=0.0;
		 for(is=low;is<=high;is++){
		   tmpeq=equat;
		   SpaceValue[it]=CURRENT_H*is;
		   if(is==low||is==high)
		   sum+= eval_rpn(tmpeq)*WEIGHT_0;
		   else sum+= eval_rpn(tmpeq);
		 }
		 equat+=ijmp;
		 sum=sum*CURRENT_H;
		 PUSH(sum);
		 break;

   case ENDINT: return(POP);		   
		 
                  
		  
   
   default:
   {
   it=i/100;
   in=i%100;
   switch(it)
    {
     case 0: PUSH(fun1[in](POP));
            break;
     case 1:
	    {
	     if(in==0){temx=POP;temy=POP;PUSH(temx+temy);goto bye;}
	     if(in==2){temx=POP;temy=POP;PUSH(temx*temy);goto bye;}
	     if(in==1){temx=POP;temy=POP;PUSH(temy-temx);goto bye;}
	     if(in==3){temx=POP;if(temx==0.0)temx=DBL_EPSILON;
		      temy=POP;PUSH(temy/temx);goto bye;}
             temx=POP;
             temy=POP;
	     PUSH(fun2[in](temy,temx));break;
	    }
     case 2: PUSH(variables[in]);break;
     case 3: PUSH(constants[in]);break;
     case 8:
            PUSH(ustack[uptr-1-in]); break;
     case 10: PUSH(constants[in+100]);break;
     case 11: PUSH(SpaceValue[in]);break;
     case 12: PUSH((double)in);break;
     case 4: i=*equat++;
         
            for(j=0;j<i;j++)
            {
            ustack[uptr]=POP;
	 
	    uptr++;
            }
           
            PUSH(eval_rpn(ufun[in])); 
       break;
     case 20: PUSH(my_ctm[in].u[CURRENT_INDEX]);  /* generic value  */
       break;
     case 21: PUSH(my_ctm[in].u[CURRENT_INDEX]); /* generic value */
       break;
     case 30: 
     case 31:
     case 32:
             PUSH((double)in);   /* symbolic expression   */
       break;
     case 40:   /*  evaluative context    */
     case 41: 
       PUSH(eval_ctm(in,POP));
       break;
     case 42:
       temy=POP;
       temx=POP;
       PUSH(eval_ctm2(in,temx,temy));
       break;

       
      



     }
bye: j=0;
 }
	       }
 }
/*   printf("%d \n ",stack_pointer); */
   return(POP);

}


#ifndef __EMX__
/*  STRING STUFF  */
strupr(s)
char *s;
{
 int i=0;
 while(s[i])
 {
  if(islower(s[i]))s[i]-=32;
  i++;
  }
}


strlwr(s)
char *s;
{
 int i=0;
 while(s[i])
 {
  if(isupper(s[i]))s[i]+=32;
  i++;
  }
}
#endif


/************    Vectorized stuff      ****************/


ini_stack(n)
{
 int i;
 for(i=0;i<VSTACKDEPTH;i++){
   v_stack[i]=(double *)malloc(n*sizeof(double));
   v_ustack[i]=(double *)malloc(n*sizeof(double));
 }
 v_stack_ptr=(int *)malloc(n*sizeof(int));
 v_uptr=(int *)malloc(n*sizeof(int));
}

realloc_stack(n)
{
int i;
 for(i=0;i<VSTACKDEPTH;i++){
   v_stack[i]=(double *)realloc(v_stack[i],n*sizeof(double));
   v_ustack[i]=(double *)realloc(v_ustack[i],n*sizeof(double));
 }
 v_stack_ptr=(int *)realloc(v_stack_ptr,n*sizeof(int));
 v_uptr=(int *)realloc(v_uptr,n*sizeof(int));
}


v_evaluate(equat,vector,n)
     int *equat,n;
     double *vector;
{
 int i;
 for(i=0;i<n;i++){
   v_stack_ptr[i]=0;
   v_uptr[i]=0;
 }
 v_eval_rpn(equat,vector,n);
}
 

v_eval_rpn(equat,vector,n)
     int *equat,n;
     double *vector;
{
 int index;
 int i,it,in,j;
  
  int isum,aind,iord;
  int low,high,ind;
  double temp,temx,temy,temz;
  double tlow,thigh;
  double sum;
 double *funvec;
 funvec=(double *)malloc(n*sizeof(double));

 while((i=*equat++)!=ENDEXP)
   {
     switch(i)
       {
       case ENDFUN:
	 i=*equat++;
	 for(index=0;index<n;index++)v_uptr[index]-=i;
	 break;
       case DIFOP:
	  for(index=0;index<n;index++){
	   isum=(int)VPOP;
	   iord=(int)VPOP;
	   temx=VPOP;
	   ind=(int)VPOP;
	   VPUSH(do_dif(my_ctm[ind].u,temx,iord,isum));
	 }
	 break;

       case BIHARM:
	 for(index=0;index<n;index++){
	   isum=(int)VPOP;
	   temx=VPOP;
	   ind=(int)VPOP;
	   VPUSH(do_biharm(my_ctm[ind].u,temx,isum));
	 }
	 break;

       case CONV:
	 for(index=0;index<n;index++){
	 isum=(int)VPOP;
	 temx=VPOP;
	 thigh=VPOP;
	 tlow=VPOP;
	 ind=(int)VPOP;
	 aind=(int)VPOP;
	 VPUSH(do_convolve(my_ctm[aind].u,my_ctm[ind].u,
			   tlow,thigh,temx,isum));
       }
	 break;
	
       case FFTOP:
	 for(index=0;index<n;index++){
	 isum=(int)VPOP;
	 temx=VPOP;
	 ind=(int)VPOP;
	 aind=(int)VPOP;
	 VPUSH(do_fft_conv(aind,ind,temx,isum));
       }
	 break;
 
       case WEIGHT: 
	 for(index=0;index<n;index++){
	 temx=VPOP;
	 thigh=VPOP;
	 tlow=VPOP;
	 ind=(int)VPOP;
	 aind=(int)VPOP;
	 VPUSH(do_weight(my_ctm[aind].u,my_ctm[ind].u,
			 tlow,thigh,temx,0));
       }
	 break;

       case RWEIGHT:
	 for(index=0;index<n;index++){
	 temx=VPOP;
	 thigh=VPOP;
	 tlow=VPOP;
	 ind=(int)VPOP;
	 aind=(int)VPOP;
	 VPUSH(do_weight(my_ctm[aind].u,my_ctm[ind].u,
			 tlow,thigh,temx,1));
       }
	 break;
	 

       case AVERAGE:
	 for(index=0;index<n;index++){
	   thigh=VPOP;
	   tlow=VPOP;
	   ind=(int)VPOP;
	   aind=(int)VPOP;
	   VPUSH(do_average(my_ctm[aind].u,my_ctm[ind].u,
			    tlow,thigh));
	 }
	 break;
       
       default:
	 {
	   it=i/100;
	   in=i%100;
	   switch(it)
	     {
	 
	     case 0: for(index=0;index<n;index++)VPUSH(fun1[in](VPOP));
            break;
     case 1:
	    
	     if(in==0){
	       for(index=0;index<n;index++){
		 temx=VPOP;temy=VPOP;VPUSH(temx+temy);
	       }
	       goto bye;
	     }
	     if(in==2){
	       for(index=0;index<n;index++){
		 temx=VPOP;temy=VPOP;VPUSH(temx*temy);
	       }
	       goto bye;
	     }
	     if(in==1){
	       for(index=0;index<n;index++){
		 temx=VPOP;temy=VPOP;VPUSH(temy-temx);
	       }
	       goto bye;
	     }
	     if(in==3){
	       for(index=0;index<n;index++){
		 temx=VPOP;
		 if(temx==0.0)temx=1.e-50;
		 temy=VPOP;VPUSH(temy/temx);
	       }
	       goto bye;
	     }
	     for(index=0;index<n;index++){
	       temx=VPOP;
	       temy=VPOP;
	       VPUSH(fun2[in](temy,temx));
	     }
	     break;
	     
     case 2: 
	       for(index=0;index<n;index++)
		 VPUSH(variables[in]);
	       break;
     case 3: 
	       for(index=0;index<n;index++)
		 VPUSH(constants[in]);
	       break;
     case 8:
	       for(index=0;index<n;index++)
		 VPUSH(v_ustack[v_uptr[index]-1-in][index]);
	       break;
     case 10: 
	       for(index=0;index<n;index++)
		 VPUSH(constants[in+100]);
	       break;

     case 11: 
	       for(index=0;index<n;index++)
		 VPUSH(index*CURRENT_H);  /* push X(i) */
	       break;
     case 12: 
	       for(index=0;index<n;index++)
		 VPUSH((double)in);
	       break;
     case 4: i=*equat++;
	       for(index=0;index<n;index++){
		 
		 for(j=0;j<i;j++)
		   {
		     v_ustack[v_uptr[index]][index]=VPOP;
		     v_uptr[index]++;
		   }
	       }
	       v_eval_rpn(ufun[in],funvec,n);
	       for(index=0;index<n;index++)
		 VPUSH(funvec[index]);
	       break;
     case 20: 
	       for(index=0;index<n;index++)
		 VPUSH(my_ctm[in].u[index]);  /* generic value  */
	       break;
     case 21: 
	       for(index=0;index<n;index++)
		 VPUSH(my_ctm[in].u[index]); /* generic value */
	       break;
     case 30: 
     case 31:
     case 32:
	       for(index=0;index<n;index++)
		 VPUSH((double)in);   /* symbolic expression   */
	       break;
     case 40:   /*  evaluative context   not used very much.. */
     case 41: 
	       for(index=0;index<n;index++)
		 VPUSH(eval_ctm(in,VPOP));
	       break;
     case 42:
	       for(index=0;index<n;index++){
		 temy=VPOP;
		 temx=VPOP;
		 VPUSH(eval_ctm2(in,temx,temy));
	       }
	       break;

       
      



	     }
bye: j=0;
	 }
       }
   }
 free(funvec);
 for(index=0;index<n;index++)vector[index]=VPOP;

}









