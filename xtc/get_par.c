#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <stdio.h>

#define MAX_N_SBOX 20
#define MAX_LEN_SBOX 25
#define FORGET_ALL 0
#define DONE_ALL 2
#define FORGET_THIS 3
#define DONE_THIS 1
#define OK -2
#define CANCEL -3
#define VAL_LEN 80

#define MAXCRHS 20
#define MAXCFIX 20
#define MAXSRHS 20
#define MAXSFIX 20
#define MAXCPAR 20
#define MAXCAUX 10
#define MAXSAUX 10
#define MAXUPAR 100





#define EV_MASK (ButtonPressMask 	|\
		KeyPressMask		|\
		ExposureMask		|\
		StructureNotifyMask)	

#define BUT_MASK (ButtonPressMask 	|\
		KeyPressMask		|\
		ExposureMask		|\
		StructureNotifyMask	|\
		EnterWindowMask		|\
		LeaveWindowMask)	



	
	


extern Display *display;
extern int screen;
extern Window main_win,command_pop;
extern int DCURYs,DCURXs,CURY_OFFs,DCURX;
extern GC gc,small_gc;
extern unsigned long MyBackColor,MyForeColor;

double atof(),sqrt();

char *get_next(),*get_first();
typedef struct {
	char names[200][12],values[200][80];
	int n,nscal;
	Window w[200];
	Window cancel,ok,base;
	char title[20];
	int special[200];
}SELECTBOX;

extern double *last_ic;
Window make_window();

extern int NUPAR,NSCALAR,NCTM,NUCPAR;
char ucvar_names[MAXCRHS][10];
char ucpar_names[MAXCPAR][10];
char ucaux_names[MAXCAUX][10];
char uaux_names[MAXSAUX][10];
char uvar_names[MAXSRHS][10];
char upar_names[MAXUPAR][10];


extern int RandSeed;

change_ics()
{
 int i;
 SELECTBOX se;
 double z;
 int npar=NSCALAR+NCTM;
 int current_index=0;
 int dontsave=0;
 char value[80],name[20];
 int new_index,done=0,pos,col;
 XEvent ev;
 for(i=0;i<NSCALAR;i++){
	strcpy(se.names[i],uvar_names[i]);
	
	sprintf(se.values[i],"%.16g",last_ic[i]);
 }
 for(i=0;i<NCTM;i++){
	strcpy(se.names[i+NSCALAR],ucvar_names[i]);
	get_ctm_string(se.names[i+NSCALAR],
 	se.values[i+NSCALAR],&(se.special[i+NSCALAR]));
 }
 	se.n=npar;
	se.nscal=NSCALAR;

	make_select_box(&se,"Initial Data");
	init_param_string(0,se,name,value,&pos,&col);
	while(1){
	  XNextEvent(display,&ev);
	  new_index=-1;
	   if(ev.type==Expose||ev.type==MapNotify){
	   	expose_select(ev.xexpose.window,se);
		if(ev.xexpose.window==command_pop)
		display_command(name,value,pos,col);
		continue;
	   }
           if(ev.type==ButtonPress){
	     do_select_box(se,ev.xbutton.window,&new_index);
	     if(new_index==OK){
		save_string(current_index,value,&se,0);
		break;
              }
	    if(new_index==CANCEL){
		dontsave=1;
		break;
	    }
	     if((new_index>=0)&&(new_index!=current_index)){
	     save_string(current_index,value,&se,0);
	     current_index=new_index;
	     init_param_string(current_index,se,name,value,&pos,&col);
	     }
	}  /*  not a button on param box    */
	if(ev.type== EnterNotify){
	 XSetWindowBorderWidth(display,ev.xcrossing.window,2);
	continue;
	}
	if(ev.type==LeaveNotify){
	XSetWindowBorderWidth(display,ev.xcrossing.window,1);
	continue;}
       /*  go ahead and edit whatever   */
      if(ev.type==KeyPress)
      edit_command_string(ev,name,value,&done,&pos,&col);
	if(done==1){  /* A CR was hit   */
	            if(current_index==(npar-1)){
		    save_string(current_index,value,&se,0);
			break;
	            }
		    else {
                        save_string(current_index,value,&se,0);
			current_index++;
			init_param_string(current_index,
		    	se,name,value,&pos,&col);
			done=0;
		    }
	}
       if(done==-1){
		    init_param_string(current_index,
			se,name,value,&pos,&col);
       }
	if(done==2){
		save_string(current_index,value,&se,0);
		break;
	}
		
	}
	clr_command();

 XSelectInput(display,se.cancel,EV_MASK);
  	 XSelectInput(display,se.ok,EV_MASK);
 XDestroySubwindows(display,se.base);
 XDestroyWindow(display,se.base);

/*  permanently save the stuff   */
	if(dontsave)return(1);
            if(RandSeed>=0)
	      srand48(RandSeed);   /*  This might be annoying  */
	for(i=0;i<NSCALAR;i++){
		last_ic[i]=atof(se.values[i]);
		
	}
	for(i=0;i<NCTM;i++)
		set_ctm_string(se.names[i+NSCALAR],
		se.values[i+NSCALAR],se.special[i+NSCALAR]);
 return(0);
}



chg_par()
{
 int i;
 SELECTBOX se;
 double z;
 int npar=NUPAR+NUCPAR;
 int current_index=-1;
 int dontsave=0;
 char value[80],name[20];
 int new_index,done=0,pos,col;
 XEvent ev;
 for(i=0;i<NUPAR;i++){
	strcpy(se.names[i],upar_names[i]);
	get_val(se.names[i],&z);
	sprintf(se.values[i],"%.16g",z);
 }
 for(i=0;i<NUCPAR;i++){
	strcpy(se.names[i+NUPAR],ucpar_names[i]);
	get_ctm_string(se.names[i+NUPAR],
 	se.values[i+NUPAR],&(se.special[i+NUPAR]));
 }
 	se.n=NUCPAR+NUPAR;
	se.nscal=NUPAR;

	make_select_box(&se,"Parameters");
	init_param_string(-1,se,name,value,&pos,&col);
	while(1){
	  XNextEvent(display,&ev);
	  new_index=-1;
	   if(ev.type==Expose||ev.type==MapNotify){
	   	expose_select(ev.xexpose.window,se);
		if(ev.xexpose.window==command_pop)
		display_command(name,value,pos,col);
		continue;
	   }
           if(ev.type==ButtonPress){
	     do_select_box(se,ev.xbutton.window,&new_index);
	     if(new_index==OK){
		save_string(current_index,value,&se,1);
		break;
              }
	    if(new_index==CANCEL){
		dontsave=1;
		break;
	    }
	     if((new_index>=0)&&(new_index!=current_index)){
	     save_string(current_index,value,&se,1);
	     current_index=new_index;
	     init_param_string(current_index,se,name,value,&pos,&col);
	     }
	}  /*  not a button on param box    */
	if(ev.type== EnterNotify)
	 XSetWindowBorderWidth(display,ev.xcrossing.window,2);
	if(ev.type==LeaveNotify)
	XSetWindowBorderWidth(display,ev.xcrossing.window,1);
       /*  go ahead and edit whatever   */
      edit_command_string(ev,name,value,&done,&pos,&col);
	if(done==1){  /* A CR was hit   */
	            if(current_index==-1){  /*  Nothing yet here  */
			if(strlen(value)==0) break; /* no request... */
	            	for(i=0;i<npar;i++)if(strcasecmp(value,se.names[i])==0)
			{
			  current_index=i;
			  done=0;
			  break;
			}
	            }
		     else { /* accept the value    */
		     save_string(current_index,value,&se,1);
		     current_index=-1;
		        done=0;
		     }
			init_param_string(current_index,se,name,value,&pos,&col);
	}
       if(done==-1){
		    if(current_index==-1)break;
		    else{
			current_index=-1;
			done=0;
			init_param_string(current_index,se,name,value,&pos,&col);
		    }
       }
	if(done==2){
		save_string(current_index,value,&se,1);
		break;
	}
		
	}
	clr_command();

 XSelectInput(display,se.cancel,EV_MASK);
  	 XSelectInput(display,se.ok,EV_MASK);
 XDestroySubwindows(display,se.base);
 XDestroyWindow(display,se.base);

/*  permanently save the stuff   */
	if(dontsave)return;
/*    This allows for repeatability   */
	  srand48(abs(RandSeed));
	for(i=0;i<NUPAR;i++){
		z=atof(se.values[i]);
		set_val(se.names[i],z);
	}
	for(i=0;i<NUCPAR;i++)
		set_ctm_string(se.names[i+NUPAR],
		se.values[i+NUPAR],se.special[i+NUPAR]);

}

do_select_box(se,w,in)
SELECTBOX se;
Window w;
int *in;
{
 int i;
 *in=-1; /*  No button here */
 for(i=0;i<se.n;i++)
   if(w==se.w[i]){
	*in=i;
	return;
   }
 if(w==se.ok){*in=OK;return;}
 if(w==se.cancel){*in=CANCEL;return;}
}


draw_one_box(name,value,type,w)
Window w;
int type;
char *name,*value;
{
 char bob[100];
 
 if(type==0)sprintf(bob,"%s=%s",name,value);
 else sprintf(bob,"%s(x)=%s",name,value);
 XClearWindow(display,w);
 XDrawString(display,w,small_gc,0,CURY_OFFs,bob,strlen(bob));
}

expose_select(w,se)
SELECTBOX se;
Window w;
{
 int i;
	if(w==se.ok){
		XDrawString(display,w,small_gc,0,CURY_OFFs,"Ok",2);
		return;
	}
	if(w==se.cancel){
		XDrawString(display,w,small_gc,0,CURY_OFFs,"Cancel",6);
		return;
	}
	for(i=0;i<se.nscal;i++){
	if(w==se.w[i]){
		draw_one_box(se.names[i],se.values[i],0,w);
		return;
	}
	}
	for(i=se.nscal;i<se.n;i++){
	if(w==se.w[i]){
		draw_one_box(se.names[i],se.values[i],1,w);
		return;
	}
	}
}

make_select_box(se,ti)
SELECTBOX *se;
char *ti;
{
 int nrow,ncol;
 int x,y;
 int n=se->n;
 int i1,i2,i;
 int width,height,wid,hgt;
 double dtemp;
 Window base,w;
  XTextProperty winname;
   XSizeHints size_hints;
  dtemp=(double)n;
  dtemp=sqrt(dtemp);  /*  approximate square  */
  i1=(int)dtemp;   /* truncate the value   */
  if(i1>5)i1=5;  /* maximum columns   */
  i2=n/i1;  
  if(i2*i1<n)i2++; /*  make sure there is enough  */
  nrow=i2;
  ncol=i1;

 wid=27*DCURXs;
 hgt=DCURYs+4;
 height=(nrow+2)*(hgt+4)+2*hgt;
 width=ncol*(wid+2*DCURXs);

 base=make_window(RootWindow(display,screen),50,130,width,height,4);
 se->base=base;
 XStringListToTextProperty(&ti,1,&winname);
 size_hints.flags=PPosition|PSize|PMinSize|PMaxSize;
 size_hints.x=50;
 size_hints.y=130;
 size_hints.width=width;
 size_hints.height=height;
 size_hints.min_width=width;
 size_hints.min_height=height;
 size_hints.max_width=width;
 size_hints.max_height=height;
 XSetWMProperties(display,base,&winname,NULL,NULL,0,&size_hints,NULL,NULL);
 se->ok=make_window(base,width/2-4*DCURXs,5,2*DCURXs,hgt,4);
 se->cancel=make_window(base,width/2+2*DCURXs,5,6*DCURXs,hgt,4);
 for(i=0;i<n;i++){
	i1=i/ncol;
        i2=i%ncol;
        x=i2*(wid+DCURXs)+DCURXs;
	y=DCURYs+(hgt+4)*i1+1.5*hgt;
	se->w[i]=make_window(base,x,y,wid,hgt,1);
	XSelectInput(display,se->w[i],BUT_MASK);

	}
}
 init_param_string(in,se,name,value,pos,col)
 int *pos,*col;
 int in;
 SELECTBOX se;
 char *name,*value;
 {
	if(in<0){
	strcpy(value,"");
	strcpy(name,"Parameter:");
	}
	else{
	strcpy(value,se.values[in]);
	strcpy(name,se.names[in]);
	}
	*pos=strlen(value);
	*col=(*pos+strlen(name))*DCURX;
  clr_command();
 display_command(name,value,*pos,*col);
 }

save_string(i,val,se,flag)
int i,flag;
SELECTBOX *se;
char *val;
{
 int type=1;
 if(i<0)return;
 strcpy(se->values[i],val);
 if(i<se->nscal)type=0;
 else if(flag)chck_special(&(se->special[i]));
 draw_one_box(se->names[i],se->values[i],type,se->w[i]);
}




