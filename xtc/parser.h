/* Modified by Klaus Gebhardt, 1997 */

#include "ctm.h"
#define NEGATE 9
#define MINUS 4
#define LPAREN 0
#define RPAREN 1
#define COMMA  2
#define LBRACE 57
#define RBRACE 58
#define CLPAREN 900
#define CRPAREN 901
#define CCOMMA 902
#define STARTTOK 10
#define ENDTOK 11
#define UFUN   4
#define MAXUFUN 50
#define ENDEXP 999
#define ENDFUN 998

#define FUNTYPE 4
#define VARTYPE 2
#define CONTYPE 3
#define BIHARM 935
#define CONV 950
#define WEIGHT 951
#define AVERAGE 955
#define RWEIGHT 954
#define INT 952
#define ENDINT 949
#define INTL 956
#define INTR 957
#define DIFOP 948
#define FFTOP 947

#define MYIF  995
#define MYELSE 993
#define MYTHEN 994
#define MAX_SYMBS 250
#define LASTTOK 249
#define ARRAYTYPE 20
#define ARR_SYM 1000
#define ARR_FUN 2000
#define CONVAR 0  /* continuum variable */
#define CONPAR 1 /* continuum parameter */
#define CONWGT 2  /* two-d continuum parameter  */
#define MAXCTM 60 /* total number of continuous things  */


#define STDSYM 63
#define SPACE 11
#define SPACE_SYM 12



typedef struct
        {
         char name[10];
         int len;
         int com;
         int arg;
         int pri;
        } SYMBOL;

SYMBOL my_symb[MAX_SYMBS]=
{
   "(",1,900,0,1,      /*  0   */
   ")",1,901,0,2,
   ",",1,902,0,3,
   "+",1,100,0,4,
   "-",1,101,0,4,
   "*",1,102,0,6,
   "/",1,103,0,6,
   "^",1,105,0,7,
   "**",2,105,0,7,
   "~",1,14,0,6,
   "START",5,-1,0,0,  /* 10  */
   "END",3,999,0,-1,
   "ATAN2",5,104,2,10,
   "MAX",3,106,2,10,
   "MIN",3,107,2,10,
   "SIN",3,0,0,10,
   "COS",3,1,0,10,
   "TAN",3,2,0,10,
   "ASIN",4,3,0,10,
   "ACOS",4,4,0,10,
   "ATAN",4,5,0,10,  /* 20  */
   "SINH",4,6,0,10,
   "TANH",4,7,0,10,
   "COSH",4,8,0,10,
   "ABS",3,9,0,10,
   "EXP",3,10,0,10,
   "LN",2,11,0,10,
   "LOG",3,11,0,10,
   "LOG10",5,12,0,10,
   "SQRT",4,13,0,10,
   "HEAV",4,16,0,10,  /*  30 */
   "SIGN",4,17,0,10,
   "ARG1",4,800,0,10,
   "ARG2",4,801,0,10,
   "ARG3",4,802,0,10,
   "ARG4",4,803,0,10,
   "ARG5",4,804,0,10,
   "ARG6",4,805,0,10,
   "ARG7",4,806,0,10,
   "ARG8",4,807,0,10,
   "FLR",3,18,0,10,  /*  40 */
   "MOD",3,108,2,10, /*  41 */
   "DELAY",5,996,2,10,      /*  42 */   /*  Delay symbol */
   "RAN",3,19,1,10, /* 43 */
    "&",1,109,0,6,  /* logical stuff  */
   "|",1,110,0,4,
   ">",1,111,0,7,
   "<",1,112,0,7,
   "==",2,113,0,7,
   ">=",2,114,0,7,
   "<=",2,115,0,7, /*50 */
   "IF",2,995,1,10, 
   "THEN",4,994,1,10,
   "ELSE",4,993,1,10,
                      /*  Spatial things... */
   "CONV",4,950,6,10, /* conv(u,v,y0,y1,x,type) */
   "WEIGHT",6,951,5,10, /* weight(w(x,y),u(y),y0,y1,x) */
   "INT",3,952,3,10,   /* int(z,z0,z1)of(expr)   */
   "OF",2,949,0,10,
   "RWEIGHT",7,954,5,10, /* weight(w(y,x),u(y),y0,y1,x) */
   "AVERAGE",7,955,4,10, /*  weight(w(y),u(y),y0,y1) */     
   "BIHARM",6,BIHARM,3,10, /* biharm(u,x,type) */  /* 60 */
   "DIF",3,DIFOP,4,10, /*   dif(u,x,order,type)   */  /* 61  */
   "FFTCONV",7,FFTOP,4,10, /* fftconv(w,u,x,type) */ /* 62   */      };




int NSYM=STDSYM,NCON=0,NVAR=0,NFUN=0,NARRAY=0,NBVAL=0;

/*     pointers to functions    */

double (*fun1[20])(/* double */ );
double (*fun2[20])(/* double,double */ );


/*              double functions of two values             */



double dand(),dor(),dge(),dle(),deq(),dne(),dlt(),dgt();


double d_if();

double max(/* double,double */ );
double min(/* double,double */ );
double neg(/* double */ );
double recip(/* double */ );
double signum(/* double */ );
double heaviside(/* double */ );
double rndom(/* double */ );
/*****************************************************/

double evaluate(/* int* */ );
int set_var(/* int,double */ );
int add_var(/* char *,double */ );
int add_con(/* char *,double */ );
int add_expr(/* char *expr,int *command, int *length  */ );
int add_ufun(/* char *string,char *expr,int narg */ );
int get_val(/* char *name,double *value */ );
int set_val(/* char *name,double value */ );
int set_ivar(/* int i,double value */ );
double get_ivar(/* int i */ );
int symb_list(/* int type */ );

int find_name(/*  char *string, int *index */ );
double eval_rpn(/* int* */ );
int alg_to_rpn(/* int *toklist,int *command */ );
int make_toks(/* char *dest,int *my_token */ );
double pop(  );
int push(/* double */ );
int convert(/* char*,char* */ );
int do_num(/* char *,char *,double *,int * */ );
int find_tok(/* char *,int *,int * */ );

int stack_pointer,uptr;
double constants[200];
double variables[100];
int *ufun[MAXUFUN];
char *ufun_def[MAXUFUN];
double stack[200],ustack[200];

CONTINUUM my_ctm[MAXCTM];
BDRYVAL my_bval[MAXCTM];

int CURRENT_GRID=50; /* Global grid size */
int PER_LEN=10;   /* periodizing kernels  */
int CURRENT_INDEX=0;   /* main space variable  */

char SpaceName[10];  /* Name of space variable   */
char TimeName[10];  /* Name of time variable  */
double SpaceValue[10]; /* values of space variables */
double CURRENT_SPACE=0.0,CURRENT_H=.02; 
double WEIGHT_0=1;  /* riemann  */
