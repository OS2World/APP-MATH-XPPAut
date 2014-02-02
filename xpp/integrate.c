/* Modified by Klaus Gebhardt, 1997 */

/*    this is the main integrator routine  
      for phase-plane  
      It takes the steps looks at the interrupts, plots and stores the data
      It also loads the delay stuff if required      
 
*/


/* 
New stuff for 9/96 -- cvode added 
 cvode(command,y,t,n,tout,kflag,atol,rtol) 
 command =0 continue, 1 is start 2 finish
 kflag is error < 0 is bad -- call cvode_err_msg(kflag)
 call end_cv() to end it normally 
 on return y is new stuff, t is new time kflag is error if any
 if kflag < 0 thats bad
  
*/ 

#include <stdio.h>
#include <X11/Xlib.h>
#include <math.h>
#include "xpplim.h"
#include "struct.h"
#include "phsplan.h"

#define ESCAPE 27
#define FIRSTCOLOR 30

#define MAX_LEN_SBOX 25
#define PARAM 1
#define IC 2
#define BMAXCOL 20

#define NAR_IC 10
#define VOLTERRA 6
#define BACKEUL 7
#define RKQS 8
#define STIFF 9
#define GEAR 5
#define CVODE 10
extern double ShootIC[4][MAXODE];
extern int ShootICFlag;
extern int ShootIndex;
typedef struct {
  int index0;
  char formula[256];
  int n;
} ARRAY_IC;

ARRAY_IC ar_ic[10];

int SuppressBounds=0;
typedef struct {
		Window base,upper;
		Window find,up,down,pgup,pgdn,home,end,left,right;
		Window first,last,restore,write,get;
		Window load,repl,unrepl,table,addcol,delcol;
                Window main;
                Window label[BMAXCOL];
                Window time;
		Window hint;
		char hinttxt[256];
		int dataflag;
		int col0,row0,ncol,nrow;
		int maxrow,maxcol;
                float **data;
		int istart,iend;
                } BROWSER;

extern BROWSER my_browser;

extern char *info_message,*ic_hint[],*sing_hint[];
extern int Xup,batch_range;
extern char batchout[256];
double atof();
extern int NMarkov,STOCH_FLAG;
extern int color_total,SCALEY,DCURY,PSFlag,PointRadius;
int DelayErr;
extern Window main_win,draw_win,info_pop;
extern Display *display;
double get_ivar();
double  MyData[MAXODE],MyTime;
int MyStart;
extern int DelayFlag,DCURY,NKernel;
int RANGE_FLAG; 
extern int PAR_FOL,SHOOT;
extern char upar_names[200][11];
extern double last_ic[MAXODE];
double LastTime;

extern double DELAY;
extern int R_COL;
extern int colorline[11];
extern (*rhs)();

int PSLineStyle;
 struct {
         char item[30];
	 int steps,shoot,col,movie;
	 double plow,phigh;
       } eq_range;

 struct {
         char item[30];
	 int steps,reset,oldic,index,cycle,type,movie;
	 double plow,phigh;
       } range;

int (*solver)();

init_ar_ic()
{
  int i;
  for(i=0;i<NAR_IC;i++){
    ar_ic[i].index0=-1;
    ar_ic[i].formula[0]=0;
    ar_ic[i].n=0;
  }
}
    
init_range()
{
 init_ar_ic();
 eq_range.col=-1;
 eq_range.shoot=0;
 eq_range.steps=10;
 eq_range.plow=0.0;
 eq_range.phigh=1.0;
 eq_range.movie=0;
 sprintf(eq_range.item,upar_names[0]);
 range.type=0;
 range.index=0;
 range.steps=20;
 range.plow=0.0;
 range.phigh=1.0;
 range.reset=1;
 range.oldic=1;
 range.cycle=0;
 range.movie=0;
 sprintf(range.item,uvar_names[0]);
 init_shoot_range(upar_names[0]); 
}

set_up_eq_range()
{
static char *n[]={"Range over","Steps","Start","End",
		     "Shoot (Y/N)",
		     "Stability col","Movie (Y/N)"};
 char values[7][MAX_LEN_SBOX];
 int status,i;
 static  char *yn[]={"N","Y"};
 sprintf(values[0],"%s",eq_range.item);
 sprintf(values[1],"%d",eq_range.steps);
 sprintf(values[2],"%g",eq_range.plow);
 sprintf(values[3],"%g",eq_range.phigh);
 sprintf(values[4],"%s",yn[eq_range.shoot]);
 sprintf(values[5],"%d",eq_range.col);
 sprintf(values[6],"%s",yn[eq_range.movie]);

 
 status=do_string_box(7,7,1,"Range Equilibria",n,values,45);
 if(status!=0){
   strcpy(eq_range.item,values[0]);
   i=find_user_name(PARAM,eq_range.item);
   if(i<0){
        err_msg("No such parameter");
       return(0);
     }
   
   eq_range.steps=atoi(values[1]);
   if(eq_range.steps<=0)eq_range.steps=10;
   eq_range.plow=atof(values[2]);
   eq_range.phigh=atof(values[3]);
   if(values[4][0]=='Y'||values[4][0]=='y')eq_range.shoot=1;
   else eq_range.shoot=0;
   if(values[6][0]=='Y'||values[6][0]=='y')eq_range.movie=1;
   else eq_range.movie=0;
   eq_range.col=atoi(values[5]);
   if(eq_range.col<=1||eq_range.col>(NEQ+1))eq_range.col=-1;
 
 return(1);
 }
 return(0);
}


cont_integ()
{
  double tetemp;
  double *x;
  double dif;
  if(INFLAG==0||FFT!=0||HIST!=0)return;
  tetemp=TEND;
  wipe_rep();
  data_back();
  if(new_float("Continue until:",&tetemp)==-1)return;
  x=&MyData[0];
  tetemp=fabs(tetemp);
  if(fabs(MyTime)>=tetemp)return;
  dif=tetemp-fabs(MyTime);
  /* TEND=tetemp; */
  MyStart=1;  /*  I know it is wasteful to restart, but lets be safe.... */
  integrate(&MyTime,x,dif,DELTA_T,1,NJMP,&MyStart);
  ping();
  refresh_browser(storind);
}
  

range_item()
{
 int i;
 char bob[256];
 i=find_user_name(PARAM,range.item);
 if(i>-1){
   range.type=PARAM;
   range.index=i;
 }
 else {
   i=find_user_name(IC,range.item);
   if(i<=-1){
     sprintf(bob," %s is not a parameter or variable !",range.item);
     err_msg(bob);
     return(0);
   }
   range.type=IC;
   range.index=i;
 }
 return 1;
}

set_up_range()
{
 static char *n[]={"Range over","Steps","Start","End",
		     "Reset storage (Y/N)",
		     "Use old ic's (Y/N)","Cycle color (Y/N)","Movie(Y/N)"};
 char values[8][MAX_LEN_SBOX];
 int status,i;
 static  char *yn[]={"N","Y"};
 if(!Xup){
   return(range_item());
 }
 sprintf(values[0],"%s",range.item);
 sprintf(values[1],"%d",range.steps);
 sprintf(values[2],"%g",range.plow);
 sprintf(values[3],"%g",range.phigh);
 sprintf(values[4],"%s",yn[range.reset]);
 sprintf(values[5],"%s",yn[range.oldic]);
 sprintf(values[6],"%s",yn[range.cycle]);
 sprintf(values[7],"%s",yn[range.movie]);
 
 status=do_string_box(8,8,1,"Range Integrate",n,values,45);
 if(status!=0){
   strcpy(range.item,values[0]);
   /* i=find_user_name(PARAM,range.item);
   if(i>-1){
     range.type=PARAM;
     range.index=i;
   }
   else {
     i=find_user_name(IC,range.item);
     if(i<=-1){
       err_msg("No such name!");
       return(0);
     }
     range.type=IC;
     range.index=i;
   }
   */
   if(range_item()==0)return 0;
   range.steps=atoi(values[1]);
   if(range.steps<=0)range.steps=10;
   range.plow=atof(values[2]);
   range.phigh=atof(values[3]);
   if(values[4][0]=='Y'||values[4][0]=='y')range.reset=1;
   else range.reset=0;
   if(values[5][0]=='Y'||values[5][0]=='y')range.oldic=1;
   else range.oldic=0;
    if(values[6][0]=='Y'||values[6][0]=='y')range.cycle=1;
   else range.cycle=0;
    if(values[7][0]=='Y'||values[7][0]=='y')range.movie=1;
   else range.movie=0;
  /* printf("%s %d %d %d (%d %d) %f %f ",
	  range.item, range.steps,
	  range.reset,range.oldic,range.type,range.index,
	  range.plow,range.phigh);*/
 RANGE_FLAG=1;
 return(1);
 }
 return(0);
}

do_eq_range(x)
double *x;
{
 double parlo,parhi,dpar,temp;
 int npar,stabcol,i,j,ierr;
 char bob[50];
 float stabinfo;
 if(set_up_eq_range()==0)return;

 wipe_rep();
 data_back();
 parlo=eq_range.plow;
 parhi=eq_range.phigh;
 
 npar=eq_range.steps;
 dpar=(parhi-parlo)/(double)npar;
 stabcol=eq_range.col;

 storind=0;
 DelayErr=0;
 ENDSING=0;
 PAR_FOL=1;
 PAUSER=0;
 SHOOT=eq_range.shoot;
 reset_browser();
 if(eq_range.movie)reset_film();
 for(i=0;i<=npar;i++)
   {
      if(eq_range.movie)clr_scrn();
      temp=parlo+dpar*(double)i;
      set_val(eq_range.item,temp);
      PAR_FOL=1;
      sprintf(bob,"%s=%.16g",eq_range.item,temp);
      bottom_msg(2,bob);
      if(DelayFlag)
	do_delay_sing(x,NEWT_ERR,EVEC_ERR,BOUND,EVEC_ITER,
		      NODE,&ierr,&stabinfo);
      else do_sing(x,NEWT_ERR,EVEC_ERR,BOUND,EVEC_ITER,
		   NODE,&ierr,&stabinfo);
      if(eq_range.movie)
	if(film_clip()==0)err_msg("Out of film");

      storage[0][storind]=temp;
      for(j=0;j<NODE;j++)storage[j+1][storind]=(float)x[j];
      for(j=NODE;j<NODE+NMarkov;j++)storage[j+1][storind]=0.0;
      if(stabcol>0)storage[stabcol-1][storind]=stabinfo;

      storind++;
      if(ENDSING==1)break;
    }
    refresh_browser(storind);
 PAR_FOL=0;
}
		                    

swap_color(col,rorw)
int *col,rorw;
{
 if(rorw)MyGraph->color[0]=*col;
 else *col=MyGraph->color[0];
}

set_cycle(flag,icol)
int flag,*icol;
{
 if(flag==0)return;
 MyGraph->color[0]=*icol+1;
 *icol=*icol+1;
  if(*icol==10)*icol=0;
} 





do_range(x)
double *x;
{
 char esc;
 char bob[50];
 int ivar=0,res=0,oldic=0;
 int nit=20,i,itype,cycle=0,icol=0;
 int color=MyGraph->color[0];
 double t,dpar,plow=0.0,phigh=1.0,p;
 double temp;
 int ierr=0;
  if(set_up_range()==0)return(-1);
 MyStart=1;
 itype=range.type;
 ivar=range.index;
 res=range.reset;
 oldic=range.oldic;
 nit=range.steps;
 plow=range.plow;
 phigh=range.phigh;
 cycle=range.cycle;
 dpar=(phigh-plow)/(double)nit;
 get_ic(2,x);
 storind=0;
 PAUSER=0;
 if(range.type==PARAM)get_val(range.item,&temp);
 if(range.movie)reset_film();
 for(i=0;i<=nit;i++)
  {
   if(range.movie)clr_scrn();
   if(cycle)MyGraph->color[0]=icol+1;
   icol++;
   if(icol==10)icol=0;
   p=plow+dpar*(double)i;
   if(oldic==1){
    get_ic(1,x);
    /* if(reset_memory()==0)break; */
     if(DelayFlag){
   /* restart initial data */
   if(do_init_delay(DELAY)==0)break;
  }
  }
   t=T0;
   MyStart=1;
   POIEXT=0;
   if(res==1||STOCH_FLAG)storind=0;
   if(itype==IC)x[ivar]=p;
   else {
     set_val(range.item,p);
     re_evaluate_kernels();
   }
if(Xup){   sprintf(bob,"%s=%.16g  i=%d",range.item,p,i);
   bottom_msg(2,bob);
	 }
     if(integrate(&t,x,TEND,DELTA_T,1,NJMP,&MyStart)==1){
       ierr=-1;
       break;
     }
/*   printf("storind = %d \n",storind); */
   if(STOCH_FLAG)
     append_stoch(i,storind);
   if(range.movie)
     if(film_clip()==0){err_msg("Out of film");break;}
 }
 if(oldic==1)get_ic(1,x);
 else get_ic(0,x);
if(range.type==PARAM)set_val(range.item,temp);
 MyGraph->color[0]=color;
 INFLAG=1;
  refresh_browser(storind);
 auto_freeze_it();
 ping();
 if(STOCH_FLAG)
   do_stats(nit,ierr);
 return(ierr);
  
 
}


find_equilibrium()
{
 static char *n[]={"(R)ange","(G)o","(M)ouse"};
 static char key[]="rgm";
 char ch;
 int i,ierr;
 float xm,ym;
 int im,jm;
 int iv,jv;
 float stabinfo;
 double *x,oldtrans;
 Window temp=main_win;

 x=&MyData[0];
 if(FFT||HIST||NKernel>0)return;
 ch=(char)pop_up_list(&temp,"Equilibria",n,key,3,12,1,10,6*DCURY+8,
		      sing_hint,info_pop,info_message);
 if(ch==27)return;
 STORFLAG=0;
 POIMAP=0;
 oldtrans=TRANS;
 TRANS=0.0;
 
 switch(ch){
 case 'r': 
 do_eq_range(x);
 
   return;
  case 'm':
    /*  Get mouse values  */
        iv=MyGraph->xv[0]-1;
        jv=MyGraph->yv[0]-1;
    if(iv<0||iv>=NODE||jv<0||jv>=NODE||MyGraph->grtype>=5||jv==iv){
      err_msg("Not in useable 2D plane...");
      return;
    }
	     
   
	 /* get mouse click x,y  */
         get_ic(1,x);
	 message_box(&temp,0,SCALEY-5*DCURY,"Click on guess");
	 if(get_mouse_xy(&im,&jm,draw_win)){
	   scale_to_real(im,jm,&xm,&ym);
	   x[iv]=(double)xm;
	   x[jv]=(double)ym;
	 }
	XDestroyWindow(display,temp);

	 break;
       
 case 'g':
 default:
        get_ic(2,x);
        break;
 }

 if(DelayFlag)
   do_delay_sing(x,NEWT_ERR,EVEC_ERR,BOUND,EVEC_ITER,NODE,&ierr,&stabinfo);
 else
    do_sing(x,NEWT_ERR,EVEC_ERR,BOUND,EVEC_ITER,NODE,&ierr,&stabinfo);
 TRANS=oldtrans;
 
}
 
batch_integrate()
{
 FILE *fp;
 double *x;
 int oldstart=MyStart,i;
  MyStart=1;
  x=&MyData[0];
  RANGE_FLAG=0;
  DelayErr=0;
   MyTime=T0;
  
  STORFLAG=1;
  POIEXT=0;
  storind=0;
  reset_browser();
 if(batch_range==1){
  if(do_range(x)!=0)
    printf(" Errors occured in range integration \n"); 
 }
 else {
   get_ic(2,x);
    if(DelayFlag){
      /* restart initial data */
      if(do_init_delay(DELAY)==0)return;
    }
 
  if(fabs(MyTime)>=TRANS&&STORFLAG==1&&POIMAP==0)
    {
      storage[0][0]=(float)MyTime;
      extra(x,MyTime,NODE,NEQ);
      for(i=0;i<NEQ;i++)storage[1+i][0]=(float)x[i];
      storind=1;
    }

  if(integrate(&MyTime,x,TEND,DELTA_T,1,NJMP,&MyStart)!=0)
    printf(" Integration not completed -- will write anyway...\n");

   INFLAG=1;
  refresh_browser(storind);
 }
  fp=fopen(batchout,"w");
  if(fp==NULL){
    printf(" Unable to open %s to write \n",batchout);
    return;
  }
  write_browser_data(fp,&my_browser);
  fclose(fp);
 printf(" Run complete ... \n");
}

do_init_data()
{
  static char *n[]={"(R)ange","(L)ast","(O)ld","(G)o","(M)ouse","(S)hift","(N)ew",
		    "s(H)oot","(F)ile","form(U)la","m(I)ce"};
  static char key[]="rlogmsnhfui";
  char ch;
  char sr[20];
  int i,si;
  double *x;
  FILE *fp;
  char icfile[100];
  float xm,ym;
  int im,jm,oldstart,iv,jv,badmouse;
  Window temp=main_win;
  oldstart=MyStart;
  MyStart=1;
  x=&MyData[0];
  RANGE_FLAG=0;
  DelayErr=0;
  if(FFT||HIST)return;
  ch= (char)pop_up_list(&temp,"Integrate",n,key,11,11,3,10,3*DCURY+8,
			ic_hint,info_pop,info_message);
  if(ch==27)return;
  data_back();
  wipe_rep();
  MyTime=T0;
  
  STORFLAG=1;
  POIEXT=0;
  storind=0;
  reset_browser();
  switch(ch){
    
  case 'r': /* do range   */
    do_range(x);
    return;
  case 's':
  case 'l':
    if(INFLAG==0){
      ping();
      err_msg("No prior solution");
      return;
    }
    get_ic(0,x);
    if(ch=='s'){
      T0=LastTime;
      MyTime=T0;
    }
    if(METHOD==VOLTERRA&&oldstart==0){
      ch=(char)two_choice("No","Yes","Reset integrals?","ny",0,0,main_win);
      if(ch=='n')MyStart=oldstart;
    }
    break;
  case 'o':
    get_ic(1,x);
    if(DelayFlag){
      /* restart initial data */
      if(do_init_delay(DELAY)==0)return;
    }
    break;
  case 'm':
  case 'i':
        iv=MyGraph->xv[0]-1;
        jv=MyGraph->yv[0]-1;
    if(iv<0||iv>=NODE||jv<0||jv>=NODE||MyGraph->grtype>=5||jv==iv){
      err_msg("Not in useable 2D plane...");
      return;
    }


    /*  Get mouse values  */
    if(ch=='m'){
        get_ic(1,x);
	message_box(&temp,0,SCALEY-5*DCURY,"Click on initial data");
	if(get_mouse_xy(&im,&jm,draw_win)){
	  scale_to_real(im,jm,&xm,&ym);
	  im=MyGraph->xv[0]-1;
	  jm=MyGraph->yv[0]-1;
	  x[iv]=(double)xm;
	  x[jv]=(double)ym;
	  last_ic[im]=x[im];
	  last_ic[jm]=x[jm];
	  XDestroyWindow(display,temp);
  
	  if(DelayFlag){
	    /* restart initial data */
	    if(do_init_delay(DELAY)==0)return;
	  }
	}
	else {
	   XDestroyWindow(display,temp);
	   return;
	}
    }
    else {
      SuppressBounds=1;
        get_ic(1,x);
	message_box(&temp,0,SCALEY-5*DCURY,"Click on initial data -- ESC to quit");
	while(1){
	  badmouse=get_mouse_xy(&im,&jm,draw_win);
	  if(badmouse==0)break;
	  scale_to_real(im,jm,&xm,&ym);
	  im=MyGraph->xv[0]-1;
	  jm=MyGraph->yv[0]-1;
	  x[iv]=(double)xm;
	  x[jv]=(double)ym;
	  last_ic[im]=x[im];
	  last_ic[jm]=x[jm];
	  if(DelayFlag){
	    /* restart initial data */
	    if(do_init_delay(DELAY)==0)break;
	  }
	  usual_integrate_stuff(x);
	}
	XDestroyWindow(display,temp);
	SuppressBounds=0;
	return;
    }
    break;
  case 'n':
    man_ic(); 
    get_ic(2,x);
    break; 
  case 'u':
    if(form_ic()==0)return;
    get_ic(2,x);
    break;
  case 'h':
    if(ShootICFlag==0){
      err_msg("No shooting data available");
      break;
    }
    sprintf(sr,"Which? (1-%d)",ShootIndex);
    si=1;
    new_int(sr,&si);
    si--;
    if(si<ShootIndex&&si>=0){
      for(i=0;i<NODE;i++)
	last_ic[i]=ShootIC[si][i];
      get_ic(2,x);
    }
    else
      err_msg("Out of range");
    break;
  case 'f':
    icfile[0]=0;
    if(new_string("Filename: ",icfile)==0)return;
    if((fp=fopen(icfile,"r"))==NULL){
      err_msg(" Cant open IC file");
      return;
    }
    for(i=0;i<NODE;i++)
      fscanf(fp,"%lg",&last_ic[i]);
    fclose(fp);
    get_ic(2,x);
    break;
      
    
  case 'g':
  default:
    get_ic(2,x);
    if(DelayFlag){
      /* restart initial data */
      if(do_init_delay(DELAY)==0)return;
    }
    break;
  }
  /*  if(fabs(MyTime)>=TRANS&&STORFLAG==1&&POIMAP==0)
    {
      storage[0][0]=(float)MyTime;
      extra(x,MyTime,NODE,NEQ);
      for(i=0;i<NEQ;i++)storage[1+i][0]=(float)x[i];
      storind=1;
    }
  integrate(&MyTime,x,TEND,DELTA_T,1,NJMP,&MyStart);
  ping();
  INFLAG=1;
  refresh_browser(storind);
 auto_freeze_it();
  redraw_ics();
  */
usual_integrate_stuff(x);
}	

usual_integrate_stuff(x)
     double *x;
{
  int i;
if(fabs(MyTime)>=TRANS&&STORFLAG==1&&POIMAP==0)
    {
      storage[0][0]=(float)MyTime;
      extra(x,MyTime,NODE,NEQ);
      for(i=0;i<NEQ;i++)storage[1+i][0]=(float)x[i];
      storind=1;
    }
  integrate(&MyTime,x,TEND,DELTA_T,1,NJMP,&MyStart);
  ping();
  INFLAG=1;
  refresh_browser(storind);
 auto_freeze_it();
  redraw_ics();
}
/*  form_ic  --  u_i(0) = F(i)  where  "i" is represented by "t" 
*/

set_array_ic()
{
 char junk[20];
 int i,index0,myar=-1;
 int i1,in;
 double z;
 int flag;
 junk[0]=0;
 if(new_string("First element ",junk)==0)return 0;
 find_variable(junk,&i);
 if(i<=-1)
   return 0;
 index0=i;
 for(i=0;i<NAR_IC;i++){
   if(index0==ar_ic[i].index0){
     myar=i;
     break;
   }
 }
 if(myar<0){
   for(i=0;i<NAR_IC;i++){
     if(ar_ic[i].index0==-1){
       myar=i;
       break;
     }
   }
 }
 if(myar<0)myar=0;

 /* Now we have an element in the array index */
 ar_ic[myar].index0=index0;
 new_int("Number elements:",&ar_ic[myar].n);
 new_string("u=F(t-i0):",ar_ic[myar].formula);
 i1=index0-1;
 in=i1+ar_ic[myar].n;
 /* printf("i1=%d in=%d \n",i1,in); */
 if(i1>NODE||in>NODE)return 0; /* out of bounds */
 for(i=i1;i<in;i++){
   set_val("t",(double)(i-i1));
   flag=do_calc(ar_ic[myar].formula,&z);
   if(flag==-1){
     err_msg("Bad formula");
     return 1;
   }
   last_ic[i]=z;
 }
 return 1;
}
form_ic()
{
  int ans;
  while(1){
    ans=set_array_ic();
    if(ans==0)break;
  }
  return 1;
}
/*
form_ic()
{
  char formula[256];
  double z;
  int flag,i;
  formula[0]=0;
  if(new_string("Formula: ",formula)==0)return(0);
  for(i=0;i<NODE;i++){
    set_val("t",(double)i);
    flag=do_calc(formula,&z);
    if(flag==-1){
 
      return(0);
    }
    last_ic[i]=z;
  }
  return(1);
}
*/     
get_ic(it,x)
int it;
double *x;
{
  int i;
  switch(it){
  case 0:
    for(i=0;i<NODE+NMarkov;i++)last_ic[i]=x[i];
    break;
  case 1:
  case 2:
    for(i=0;i<NODE+NMarkov;i++)x[i]=last_ic[i];
    break;
   }
}


ode_int(y,t,istart,ishow)
double *y,*t;
int *istart,ishow;
{
 double error[MAXODE];
 int kflag,i;
 int nit,nout=NJMP;
 double tend=TEND;
 double dt=DELTA_T,tout;
  if(METHOD==0){
 nit=tend;
 dt=dt/fabs(dt);
 }
 else nit=tend/fabs(dt);

 if(ishow==1){
 integrate(t,y,tend,dt,1,nout,istart);

 return(1);
}
 
 if(METHOD!=GEAR&&METHOD!=RKQS&&METHOD!=STIFF&&METHOD!=CVODE){
   kflag=solver(y,t,dt,nit,NODE,istart,WORK);
   if(kflag<0){
     ping();
     if(RANGE_FLAG)return(0);
     switch(kflag)
	    {
	     case -1: err_msg(" Singular Jacobian "); break;
	     case -2: err_msg("Too many iterates");break;
	      }
           
            return(0);
   }
 }
 else
   {
           tout=*t+tend*dt/fabs(dt);
           switch(METHOD){
	   case GEAR:
	     if(*istart==1)*istart=0;
	     gear(NODE,t,tout,y,HMIN,HMAX,TOLER,2,error,
		  &kflag,istart,WORK,IWORK);
	     if(kflag<0)
	       {
		 ping();
		 if(RANGE_FLAG)return(0);
		 switch(kflag)
		   {
		   case -1: err_msg("kflag=-1: minimum step too big"); break;
		   case -2: err_msg("kflag=-2: required order too big");break;
		   case -3: err_msg("kflag=-3: minimum step too big");break;
		   case -4: err_msg("kflag=-4: tolerance too small");break;
		   }
		 
		 return(0);
	       }
	     break;
#ifdef CVODE_YES
	   case CVODE:
	     cvode(istart,y,t,NODE,tout,&kflag,&TOLER,&ATOLER);
	     if(kflag<0){
	       cvode_err_msg(kflag);
	       return(0);
	     }
             end_cv();
	     break;
#endif
	   case RKQS:
	   case STIFF:
	     adaptive(y,NODE,t,tout,TOLER,&dt,
		      HMIN,WORK,&kflag,NEWT_ERR,METHOD,istart);
	     if(kflag){
	       ping();
	       if(RANGE_FLAG)return(0);
	       switch(kflag){
	       case 2: err_msg("Step size too small"); break;
	       case 3: err_msg("Too many steps"); break;	 
	       case -1: err_msg("singular jacobian encountered"); break;
	       case 1: err_msg("stepsize is close to 0"); break;
	       case 4: 	err_msg("exceeded MAXTRY in stiff"); break;
	       }
	       return(0);
	     }
	     break;
	   }
	 }
  return(1);
}



integrate(t,x,tend,dt, count, nout,start)
double *t, *x, tend,dt;
int count,nout, *start;
{
 float oldxpl[MAXPERPLOT],oldypl[MAXPERPLOT],oldzpl[MAXPERPLOT];
 float xpl[MAXPERPLOT],ypl[MAXPERPLOT],zpl[MAXPERPLOT];
 float xv[MAXODE+1],xvold[MAXODE+1];
 double error[MAXODE];
 double xprime[MAXODE],oldxprime[MAXODE],hguess=dt;
 int kflag;
 int *IXPLT,*IYPLT,*IZPLT;
 int NPlots,ip;
 int rval=0;
 double oldx[MAXODE],oldt,dint,sect,sect1,tout,tzero=*t;
 float tscal=tend,tstart=*t,tv;
 float dp_time,xp_time;
 char esc;
 char error_message[50];
 int ieqn,koff,i,pflag=0;
 int icount=0;
 int nit;
 int cwidth;
 if(Xup) cwidth=get_command_width();
 LastTime=*t;
 
  NPlots=MyGraph->nvars;

  
 IXPLT=MyGraph->xv;
 IYPLT=MyGraph->yv;
 IZPLT=MyGraph->zv;

 if((METHOD==GEAR)&&(*start==1))*start=0;
 if(METHOD==0){
 nit=tend;
 dt=dt/fabs(dt);
 }
 else nit=tend/fabs(dt);
 nit=(nit+nout-1)/nout;
 if(nit==0)return(rval);
 
 extra(x,*t,NODE,NEQ); /* Note this takes care of initializing Markov variables */
 xv[0]=(float)*t;
 for(ieqn=1;ieqn<=NEQ;ieqn++)xv[ieqn]=(float)x[ieqn-1];
 for(ip=0;ip<NPlots;ip++){
 oldxpl[ip]=xv[IXPLT[ip]];
 oldypl[ip]=xv[IYPLT[ip]];
 oldzpl[ip]=xv[IZPLT[ip]];
 }

 if(POIMAP)
 {
 oldt=*t;
 for(ieqn=0;ieqn<NEQ;ieqn++)oldx[ieqn]=x[ieqn];
 }
 if(dt<0.0)tscal=-tend;
 if(tscal==0.0)tscal=1.0;
 while(1)
 {

           switch(METHOD){
	   case GEAR:
	     {
	       tout=tzero+dt*(icount+1);
	       if(fabs(dt)<fabs(HMIN)){
		 LastTime=*t;
		 return(1);
	       }
	       gear(NODE,t,tout,x,HMIN,HMAX,TOLER,2,error,&kflag,start,WORK,IWORK);
	       stor_delay(x);
	       if(DelayErr){
		 DelayErr=0;
		 LastTime=*t;
		 return(1);
		
	       }
	       if(kflag<0)
		 {
		   ping();
		   if(RANGE_FLAG){
		     LastTime=*t;
		     return(1);
		   }
		   switch(kflag)
		     {
		     case -1: err_msg("kflag=-1: minimum step too big"); break;
		     case -2: err_msg("kflag=-2: required order too big");break;
		     case -3: err_msg("kflag=-3: minimum step too big");break;
		     case -4: err_msg("kflag=-4: tolerance too small");break;
		     }
		   
		   LastTime=*t;
		   return(1);
		 }
	     }
	     break;
#ifdef CVODE_YES
	   case CVODE:
	      tout=tzero+dt*(icount+1);
	     if(fabs(dt)<fabs(HMIN)){
	       LastTime=*t;
               end_cv();
	       return(1);
	     }
	     cvode(start,x,t,NODE,tout,&kflag,&TOLER,&ATOLER);
	     stor_delay(x);
	       if(DelayErr){
		 DelayErr=0;
		 LastTime=*t;
		 return(1);
		
	       }
	     if(kflag<0){
	       ping();
	       if(RANGE_FLAG){
		 LastTime=*t;
		 return(1);
	       }
	       cvode_err_msg(kflag);
	       LastTime=*t;
	       return(1);
	     }

	     break;
#endif
	   case RKQS:
	   case STIFF:
	     tout=tzero+dt*(icount+1);
	     if(fabs(dt)<fabs(HMIN)){
	       LastTime=*t;
	       return(1);
	     }
	     adaptive(x,NODE,t,tout,TOLER,&hguess,
		      HMIN,WORK,&kflag,NEWT_ERR,METHOD,start);
	     stor_delay(x);
	       if(DelayErr){
		 DelayErr=0;
		 LastTime=*t;
		 return(1);
		
	       }
	     if(kflag){
	       ping();
	       if(RANGE_FLAG){
		 LastTime=*t;
		 return(1);
	       }
	       switch(kflag){
	       case 2: err_msg("Step size too small"); break;
	       case 3: err_msg("Too many steps"); break;	 
	       case -1: err_msg("singular jacobian encountered"); break;
	       case 1: err_msg("stepsize is close to 0"); break;
	       case 4: 	err_msg("exceeded MAXTRY in stiff"); break;
	       }
	       LastTime=*t;
	       return(1);
	     }

	     break;
           default: {
	     kflag=solver(x,t,dt,nout,NODE,start,WORK);
	      
	     if(kflag<0)
	       {
		 ping();
		 if(RANGE_FLAG)break;
		 switch(kflag)
		   {
		   case -1: err_msg("Singular Jacobian "); break;
		   case -2: err_msg("Too many iterates ");break;
		   }
           
		 LastTime=*t;
		 return(1);
	       }
	   }
	   
	   }
             
	   extra(x,*t,NODE,NEQ);
	/*  DO MARKOV CHAIN HERE    
        update_markov(x,*t,fabs(dt)*nout);  */


 
           
          if (TORUS == 1) {
	for (ieqn = 0; ieqn < NEQ; ieqn++) {
		if (itor[ieqn] == 1) {
			if (x[ieqn] > TOR_PERIOD) {
				x[ieqn] -= TOR_PERIOD;
				for (ip = 0; ip < NPlots; ip++) {
					if (ieqn == IYPLT[ip]-1)
					oldypl[ip] -= (float) TOR_PERIOD;
					if (ieqn == IXPLT[ip]-1)
					oldxpl[ip] -= (float) TOR_PERIOD;
				if ((ieqn == IZPLT[ip]-1) && (PLOT_3D == 1))
					oldzpl[ip] -= (float) TOR_PERIOD;
				}
			}
			if (x[ieqn] < 0) {
				x[ieqn] += TOR_PERIOD;
				for (ip = 0; ip < NPlots; ip++) {
					if (ieqn == IYPLT[ip]-1)
					oldypl[ip] += (float) TOR_PERIOD;
					if (ieqn == IXPLT[ip]-1)
					oldxpl[ip] += (float) TOR_PERIOD;
				if ((ieqn == IZPLT[ip]-1) &&
				    (MyGraph->ThreeDFlag == 1))
					oldzpl[ip] += (float) TOR_PERIOD;
				}
			}
		      }
	      }
      }
	   xvold[0]=xv[0];
           for(ieqn=1;ieqn<(NEQ+1);ieqn++)
           {
	    xvold[ieqn]=xv[ieqn];
	    xv[ieqn]=(float)x[ieqn-1];
            if(fabs(x[ieqn-1])>BOUND)
            {
	     if(RANGE_FLAG||SuppressBounds)break;
             sprintf(error_message," %s out of bounds at t = %f ",
             uvar_names[ieqn-1],*t);
	     err_msg(error_message);
             rval=1;

             break;
            }
           }
	                
        /*   This is where the progresser goes   */
	if(Xup){   plot_command(nit,icount,cwidth);
	   esc=my_abort();
	
       
           {
            
             if(esc==ESCAPE) break;
	     if(esc=='/'){rval=1;ENDSING=1;break;}
	    
           }
	}             

           if(DelayErr){rval=1;ENDSING=1;DelayErr=0;break;}
         /*  printf(" NEQ=%d ieqn = %d \n",NEQ,ieqn); */
           if(ieqn<(NEQ+1))break;
           tv=(float)*t;
	   xv[0]=tv;
 if((POIMAP==2)&&!(POIVAR==0))
 {
  pflag=0;
  if((oldx[POIVAR-1]<x[POIVAR-1])&&!(POIEXT<0))POIEXT=1;
  if((oldx[POIVAR-1]>x[POIVAR-1])&&!(POIEXT>0))POIEXT=-1;
  if(  ( !(oldx[POIVAR-1]<x[POIVAR-1]) && (POIEXT>0) )||
       ( !(oldx[POIVAR-1]>x[POIVAR-1]) && (POIEXT<0) )
    )
  {
     if(POISGN*POIEXT>=0)
      {
	/*  We will interpolate to get a good local extremum   */
	
	rhs(*t,x,xprime,NEQ);
	rhs(oldt,oldx,oldxprime,NEQ);
	dint=xprime[POIVAR-1]/(xprime[POIVAR-1]-oldxprime[POIVAR-1]);
	tv=(1-dint)**t+dint*oldt;
	xv[0]=tv;
	for(i=1;i<=NEQ;i++)xv[i]=dint*oldx[i-1]+(1-dint)*x[i-1];
	pflag=1;
        
      }
      POIEXT=-POIEXT;
   }
  goto poi;
 }



 if(POIMAP==1)
 {
    if(POIVAR==0)

     {
     sect1=fmod(fabs(oldt),fabs(POIPLN));
     sect=fmod(fabs(*t),fabs(POIPLN));
     if(sect<sect1)
     {
     dint=sect/(POIPLN+sect-sect1);
     i=(int)(fabs(*t)/fabs(POIPLN));
     tv=(float)POIPLN*i;
     xv[0]=tv;
     for(i=1;i<=NEQ;i++)xv[i]=(float)(dint*oldx[i-1]+(1-dint)*x[i-1]);
     pflag=1;
     }
     else pflag=0;
    }

    else

    {
     if(!(POISGN<0))
     {
     if((oldx[POIVAR-1]<POIPLN)&&!(x[POIVAR-1]<POIPLN))
     {
      dint=(x[POIVAR-1]-POIPLN)/(x[POIVAR-1]-oldx[POIVAR-1]);
      tv=(1-dint)**t+dint*oldt;
      xv[0]=tv;
      for(i=1;i<=NEQ;i++)xv[i]=dint*oldx[i-1]+(1-dint)*x[i-1];
      pflag=1;
      goto poi;

     }
     else pflag=0;
     }
    if(!(POISGN>0))
     {
       if((oldx[POIVAR-1]>POIPLN)&&!(x[POIVAR-1]>POIPLN))
       {
        dint=(x[POIVAR-1]-POIPLN)/(x[POIVAR-1]-oldx[POIVAR-1]);
        tv=(1-dint)**t+dint*oldt;
        xv[0]=tv;
        for(i=1;i<=NEQ;i++)xv[i]=dint*oldx[i-1]+(1-dint)*x[i-1];
        pflag=1;
       }
       else pflag=0;
     }
    }
poi:    for(i=0;i<NEQ;i++)oldx[i]=x[i];
    oldt=*t;
    if(pflag==0)goto out;
  }

	 
/*	   Plotting and storing data      */

	  for(ip=0;ip<NPlots;ip++){
	   xpl[ip]=xv[IXPLT[ip]];
	   ypl[ip]=xv[IYPLT[ip]];
	   zpl[ip]=xv[IZPLT[ip]];
	  }
          if(!(fabs(*t)<TRANS)&&Xup)
	  {
	      if(MyGraph->ColorFlag)
		comp_color(xv,xvold,NODE,(float)fabs(dt*NJMP));
   
	      do_plot(oldxpl,oldypl,oldzpl,xpl,ypl,zpl);
	     /* XFlush(display); */ 
	  }
	   for(ip=0;ip<NPlots;ip++){
	   oldxpl[ip]=xpl[ip];
           oldypl[ip]=ypl[ip];
	   oldzpl[ip]=zpl[ip];
           }

	   if((STORFLAG==1)&&(count!=0)&&(storind<MAXSTOR)&&!(fabs(*t)<TRANS))
	   {
           for(ieqn=0;ieqn<=NEQ;ieqn++)
		 storage[ieqn][storind]=xv[ieqn];
	    storind++;
	    if(!(storind<MAXSTOR))
            if(stor_full()==0)break;
	    if((pflag==1)&&(SOS==1))break;
	 }


out:
           icount++;
           if(icount>=nit&&count!=0)break;

 }
       LastTime=*t;
#ifdef CVODE_YES
       if(METHOD==CVODE)
	 end_cv();
#endif
       return(rval);
  }



    do_plot(oldxpl,oldypl, oldzpl,xpl,  ypl, zpl)
   float *oldxpl, *oldypl, *oldzpl,*xpl,  *ypl,*zpl;
{
	int ip,np=MyGraph->nvars;
        
        for(ip=0;ip<np;ip++){
           if(MyGraph->ColorFlag==0){

	     set_linestyle(MyGraph->color[ip]);
	   }
/*	   if(MyGraph->line[ip]<0)
	     continue;  */
           if(MyGraph->line[ip]<=0)
           {
	    PointRadius=-MyGraph->line[ip];
	   if(MyGraph->ThreeDFlag==0) point_abs(xpl[ip],ypl[ip]);
	   else point_3d(xpl[ip],ypl[ip],zpl[ip]);
           }
           else
	   {
	    if(MyGraph->ThreeDFlag==0){
            
	      line_abs(oldxpl[ip],oldypl[ip],xpl[ip],ypl[ip]);
	    }
            else line_3d(oldxpl[ip],oldypl[ip],oldzpl[ip],
		         xpl[ip],ypl[ip],zpl[ip]);
	   }
       }
}



/*
 old restore is in restore.c

*/

restore(i1,i2)
     int i1,i2;
{
  int ip,np=MyGraph->nvars;
  int ZSHFT,YSHFT,XSHFT;
  int i,j,kxoff,kyoff,kzoff;
  int iiXPLT,iiYPLT,iiZPLT;
  float oldxpl,oldypl,oldzpl,xpl,ypl,zpl;
  float v1[MAXODE+1],v2[MAXODE+1];
  XSHFT=MyGraph->xshft;
  YSHFT=MyGraph->yshft;
  ZSHFT=MyGraph->zshft;
  if(i1<ZSHFT)i1=ZSHFT;
  if(i1<YSHFT)i1=YSHFT;
  if(i1<XSHFT)i1=XSHFT;
  if(storind<2)return;
   for(ip=0;ip<np;ip++){
     kxoff=i1-XSHFT;
     kzoff=i1-ZSHFT;
     kyoff=i1-YSHFT;

    iiXPLT=MyGraph->xv[ip];
    iiYPLT=MyGraph->yv[ip];
    iiZPLT=MyGraph->zv[ip];
    set_linestyle(MyGraph->color[ip]);
    oldxpl=my_browser.data[iiXPLT][kxoff];
    oldypl=my_browser.data[iiYPLT][kyoff];
    oldzpl=my_browser.data[iiZPLT][kzoff];
    for(i=i1;i<i2;i++){
      {
	xpl=my_browser.data[iiXPLT][kxoff];
	ypl=my_browser.data[iiYPLT][kyoff];
	zpl=my_browser.data[iiZPLT][kzoff];
      }
      
      if(TORUS==1)
      {
	if (fabs(oldxpl-xpl)>(float)(.5*TOR_PERIOD))oldxpl=xpl;
	if (fabs(oldypl-ypl)>(float)(.5*TOR_PERIOD))oldypl=ypl;
	if (fabs(oldzpl-zpl)>(float)(.5*TOR_PERIOD))oldzpl=zpl;
      }
      if(MyGraph->ColorFlag!=0&&i>i1){
	  for(j=0;j<=NEQ;j++){
	    v1[j]=my_browser.data[j][i];
	    v2[j]=my_browser.data[j][i-1];
	  }
	  comp_color(v1,v2,NODE,
		     (float)fabs(my_browser.data[0][i]-my_browser.data[0][i+1]));
	}     /* ignored by postscript */
     /* if(MyGraph->line[ip]<0)
	goto noplot; */
      if(MyGraph->line[ip]<=0){
	PointRadius=-MyGraph->line[ip];
	if(MyGraph->ThreeDFlag==0)point_abs(xpl,ypl);
	else point_3d(xpl,ypl,zpl);
      }
      else {
	if(MyGraph->ThreeDFlag==0)
	  line_abs(oldxpl,oldypl,xpl,ypl);
	else
	  line_3d(oldxpl,oldypl,oldzpl,xpl,ypl,zpl);
      }
    noplot:
      oldxpl=xpl;
      oldypl=ypl;
      oldzpl=zpl;
      kxoff++;
      kyoff++;
      kzoff++;
    }
  }
}


/*  Sets the color according to the velocity or z-value */
comp_color( v1,v2,n, dt)
 float *v1, *v2;
int n;
float dt;
{
 int i,cur_color;
 float sum;
 float min_scale=(float)(MyGraph->min_scale);
 float color_scale=(float)(MyGraph->color_scale);
 if(MyGraph->ColorFlag==2){
   sum=v1[MyGraph->ColorValue];
   /* printf(" %d:sum=%g \n",MyGraph->zv[0],sum); */
 }
 else
   {
     for(i=0,sum=0.0;i<n;i++)sum+=(float)fabs((double)(v1[i+1]-v2[i+1]));
     sum=sum/(dt);
   }
 cur_color=(int)((sum-min_scale)*(float)color_total/color_scale);
/*  printf("min=%f max=%f col = %d val = %f \n",min_scale,color_scale,
	cur_color,sum); */  
 if(cur_color<0)cur_color=0;
 if(cur_color>color_total)cur_color=color_total-1;
  cur_color+=FIRSTCOLOR;
 set_color(cur_color);
}


 shoot(x,xg,evec,sgn)
double *x,*xg,*evec;
int sgn;
{
 int i;
 double t=0.0;
 for(i=0;i<NODE;i++)
 x[i]=xg[i]+sgn*evec[i]*DELTA_T*.1;
i=1;
 integrate(&t,x,TEND,DELTA_T,0,NJMP,&i);
 ping();
}


stop_integration()
{
 /*  set some global error here... */
 DelayErr=1;
}




stor_full()
{

 char ch;
 if(!Xup){
   printf(" Storage full -- increase maxstor \n");
   return(0);
 }
 if(FOREVER)goto ov;
 ping();
 ch=(char)two_choice("YES","NO","Storage full: Overwrite?",
		     "yn",0,0,main_win);
 if(ch=='y')
 {
ov:
  storind=0;
  return(1);
 }
  return(0);
}
