/* Modified by Klaus Gebhardt, 1997 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <math.h>
#include "help_defs.h"
#include "graph.bitmap"
#include "struct.h"
#include "browse.h"

#define MAXLAB 50
#define MAXGROB 100

#define POINTER 0
#define ARROW 1
#define MARKER 2 /* markers start at 2  there are several of them */


#define WDMARK .001
#define HTMARK .0016

typedef struct {
  float xs,ys,xe,ye;
  double size;
  short use;
  Window w;
  int type, color;
} GROB;

extern char *info_message;

extern Atom deleteWindowAtom;
LABEL lb[MAXLAB];
GROB grob[MAXGROB];
GRAPH graph[MAXPOP];
CURVE frz[MAXFRZ];
NCLINE nclines[MAXNCLINE];
GRAPH *MyGraph;
extern int help_menu,screen;
extern int SCALEY,CURY_OFF,CURY_OFFs,DCURYs,DCURXs;

extern int storind;

extern char *half_hint[],*text_hint[],*edit_hint[],*no_hint[];
extern Display *display;
extern Window main_win,draw_win,command_pop,info_pop;
int current_pop;
extern unsigned int MyBackColor,MyForeColor,GrFore,GrBack;
extern GC gc, gc_graph,small_gc;
extern int COLOR,color_min;
extern int xor_flag,DCURX,DCURY;
int num_pops;
int text_abs();
double signum();

Window make_window();


#define MAX_INTERN_SET 50

typedef struct {
  char *name;
  char *does;
} INTERN_SET;

extern INTERN_SET intern_set[MAX_INTERN_SET];
extern int Nintern_set;

get_intern_set()
{
  char *n[MAX_INTERN_SET],key[MAX_INTERN_SET],ch;
  int i,j;
  int count=Nintern_set;
  Window temp=main_win;
  if(count==0)return;
  for(i=0;i<Nintern_set;i++){
    n[i]=(char *)malloc(256);
    key[i]='a'+i;
    sprintf(n[i],"%c: %s",key[i],intern_set[i].name);
  }
  key[count]=0;
  ch=(char)pop_up_list(&temp,"Param set",n,key,count,12,0,10,8*DCURY+8,
		       no_hint,info_pop,info_message);
   for(i=0;i<count;i++)free(n[i]);
  j=(int)(ch-'a');
  if(j<0||j>=Nintern_set){
    err_msg("Not a valid set");
    return;
  }
  get_graph();
  extract_internset(j);
  chk_delay();
  redraw_params();
  redraw_ics();
  reset_graph();
}


make_icon(icon,wid,hgt,w)
char *icon;
Window w;
int wid,hgt;
{
Pixmap icon_map; 
XWMHints wm_hints;
 icon_map=XCreateBitmapFromData(display,w,icon,wid,hgt);
wm_hints.initial_state=NormalState;
wm_hints.input=True;
wm_hints.icon_pixmap=icon_map;
 wm_hints.flags=StateHint|IconPixmapHint|InputHint;

XSetWMProperties(display,w,NULL,NULL,NULL,0,NULL,&wm_hints,NULL);

}

 title_text(string)
 char *string;
{
   gtitle_text(string,draw_win);
 }

 gtitle_text(string,win)
 Window win;
 char *string;
 {
  XTextProperty wname,iname;
  GrCol();
  if(win!=graph[0].w){
  XStringListToTextProperty(&string,1,&wname);
  XStringListToTextProperty(&string,1,&iname);
 XSetWMProperties(display,win,&wname,&iname,NULL,0,NULL,NULL,NULL);
  }
  else
  {
  int len=strlen(string);
  int x,y,w,h,bw,de;
 int xs,ys=2;
 Window root;
 XGetGeometry(display,win,&root,&x,&y,&w,&h,&bw,&de);
 xs=(w-len*DCURX)/2;
 if(xs<0)xs=0;
 Ftext(xs,ys,string,win);
 set_color(0);
 xline(0,18,w,18,win);
 }
BaseCol();
 }




 restore_off()
{
 MyGraph->Restore=0;
 /* MyGraph->Nullrestore=0; */
 }

 restore_on()
{
  MyGraph->Restore=1;
/*  MyGraph->Nullrestore=1; */

}
 
  
add_label(s,x,y,size,font)
char *s;
int x,y,size,font;
{ int i;
 float xp,yp;
 scale_to_real(x,y,&xp,&yp);
 for(i=0;i<MAXLAB;i++)
 {
  if(lb[i].use==0){
	lb[i].use=1;
	lb[i].x=xp;
	lb[i].y=yp;
	lb[i].w=draw_win;
        lb[i].font=font;
        lb[i].size=size;
	strcpy(lb[i].s,s);
        return;
  }
 }
}



draw_marker(x,y,size,type)
     float x,y,size;
     int type;
{
int pen=0;
float x1=x,y1=y,x2,y2;
int ind=0;
int offset;

int sym_dir[] = {
  /*          box              */
  0, -6, -6,1, 12,  0,1,  0, 12,1,-12,  0,
  1,  0,-12,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  
  /*          diamond             */
  0, 8, 0,1, -8,  -8,1,  8, -8,1,8,  8,
  1, -8, 8,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  /*          triangle         */
  0, -6, -6,1, 12,  0,1, -6, 12,1, -6,-12,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  
  /*          plus            */
  0, -6,  0,1, 12,  0,0, -6, -6,1,  0, 12,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  
  /*          cross            */
  0, -6,  6,1, 12, -12,0, -12, 0,1,  12, 12,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  3,  0,  0,3,  0,  0,3,  0,  0,3,  0,  0,
  
  /*          circle           */
  0,  6,  0,1, -1,  3,1, -2,  2,1, -3,  1,
  1, -3, -1,1, -2, -2,1, -1, -3,1,  1, -3,
  1,  2, -2,1,  3, -1,1,  3,  1,1,  2,  2,
  1,  1,  3,3,  0,  0,3,  0,  0,3,  0,  0,
  
};
float dx=(MyGraph->xhi-MyGraph->xlo)*WDMARK*size;
float dy=(MyGraph->yhi-MyGraph->ylo)*HTMARK*size;
while(1)
  {
    offset=48*type+3*ind;
    pen=sym_dir[offset];
    if(pen==3)break;
    x2=dx*sym_dir[offset+1]+x1;
    y2=dy*sym_dir[offset+2]+y1;
    if(pen==1)line_abs(x1,y1,x2,y2);
    x1=x2;
    y1=y2;
    ind++;
  }

}




draw_grob(i)
     int i;
{
 float xs=grob[i].xs,ys=grob[i].ys,xe=grob[i].xe,ye=grob[i].ye;
 set_linestyle(grob[i].color);
 if(grob[i].type==POINTER)
   line_abs(xs,ys,xe,ye);
 if(grob[i].type==ARROW||grob[i].type==POINTER)
   arrow_head(xs,ys,xe,ye,grob[i].size);
 if(grob[i].type>=MARKER)
   draw_marker(xs,ys,grob[i].size,grob[i].type-2);
}

arrow_head(xs,ys,xe,ye,size)
     float xs,ys,xe,ye;
     double size;
{
 float l=xe-xs,h=ye-ys;
 float ar=(MyGraph->xhi-MyGraph->xlo)/(MyGraph->yhi-MyGraph->ylo);
 float x0=xs+size*l,y0=ys+size*h;
 float tot=(float)sqrt((double)(l*l+h*h));
 
 float xp=x0+.5*size*h*ar,yp=y0-.5*size*l/ar;
 float xm=x0-.5*size*h*ar,ym=y0+.5*size*l/ar; 
 line_abs(xs,ys,xp,yp);
 line_abs(xs,ys,xm,ym);
}

destroy_grob(w)
Window w;
{
  int i;
  for(i=0;i<MAXGROB;i++){
  if((grob[i].use==1)&&(grob[i].w==w)){
	grob[i].use=0;
	grob[i].w=(Window)0;
   }
  }
}


destroy_label(w)
Window w;
{
  int i;
  for(i=0;i<MAXLAB;i++){
  if((lb[i].use==1)&&(lb[i].w==w)){
	lb[i].use=0;
	lb[i].w=(Window)0;
   }
  }
}
 
draw_label(w)
Window w;
{
 int i;
 GrCol();
 for(i=0;i<MAXLAB;i++){
	if((lb[i].use==1)&&(lb[i].w==w))
	fancy_text_abs(lb[i].x,lb[i].y,lb[i].s,lb[i].size,lb[i].font);
 }
 for(i=0;i<MAXGROB;i++){
   if((grob[i].use==1)&&(grob[i].w==w))
     draw_grob(i);
 }
 BaseCol();
}

add_grob(xs,ys,xe,ye,size,type,color)
     double size;
     float xs,ys,xe,ye;
     int type,color;
{
  int i;
  for(i=0;i<MAXGROB;i++){
    if(grob[i].use==0){
      grob[i].use=1;
      grob[i].xs=xs;
      grob[i].xe=xe;
      grob[i].ys=ys;
      grob[i].ye=ye;
      grob[i].w=draw_win;
      grob[i].size=size;
      grob[i].color=color;
      grob[i].type=type;
 /*     redraw_all(); */
      return;
    }
  }
}
    
select_marker_type(type)
     int *type;
{
  int ival=*type-MARKER;
  int i;
  char *list[]={"Box","Diamond","Triangle","Plus","X","Circle"};
  static char key[]="bdtpxc";
  Window temp=main_win;
  char ch;
  ch=(char)pop_up_list(&temp,"Markers",list,key,6,9,ival,10,4*DCURY+8,
		       no_hint,info_pop,info_message);
  if(ch==27)return(0);
  for(i=0;i<6;i++){
    if(ch==key[i])
      ival=i;
  }
  if(ival<6)*type=MARKER+ival;

  return(1);
}
man_xy( float *xe,float *ye)
{
  double x=0,y=0;
  if(new_float("x: ",&x))
    return 0;
  if(new_float("y: ",&y))
    return 0;
  *xe=x;
  *ye=y;
  return 1;
}
add_marker()
{
  double size=1;
  int i1,j1,color=0,flag;
  float xe=0.0,ye=0.0,xs,ys;
  Window temp=main_win;
  int type=MARKER;
  if(select_marker_type(&type)==0)return;
  if(new_float("Size: ",&size))return;
  if(new_int("Color: ",&color))return;
  message_box(&temp,0,SCALEY-5*DCURY,"Position");
  flag=get_mouse_xy(&i1,&j1,draw_win);
  XDestroyWindow(display,temp);
  XFlush(display);
  if(flag==0)return;
 /* if(flag==-2){
    top_store(&xs,&ys);
    add_grob(xs,ys,xe,ye,size,type,color);
    return;
  }
  */
  if(flag==-3){
    if(man_xy(&xs,&ys))
      add_grob(xs,ys,xe,ye,size,type,color);
     redraw_all(); 
    return;
  }
  
  scale_to_real(i1,j1,&xs,&ys);

  add_grob(xs,ys,xe,ye,size,type,color);
 redraw_all(); 
}

add_markers()
{
  double size=1;
  int i;
  int i1,j1,color=0,flag;
  int nm=1,nskip=1; 
  float xe=0.0,ye=0.0,xs,ys,x,y,z;
  Window temp=main_win;
  int type=MARKER;
  if(select_marker_type(&type)==0)return;
  if(new_float("Size: ",&size))return;
  if(new_int("Color: ",&color))return;
  if(new_int("Number of markers: ",&nm))return;
  if(new_int("Skip between: ", &nskip))return;
  for(i=0;i<nm;i++){
    get_data_xyz(&x,&y,&z,MyGraph->xv[0],MyGraph->yv[0],MyGraph->zv[0],
		 i*nskip);
    if(MyGraph->ThreeDFlag==0){
      xs=x;
      ys=y;
    }
    else{
      threed_proj(x,y,z,&xs,&ys);

    }
    add_grob(xs,ys,xe,ye,size,type,color);
  }
  redraw_all(); 
}

add_pntarr(type)
     int type;
{
  double size=.1;
  int i1,j1,i2,j2,color=0;
  float xe,ye,xs,ys;
  Window temp;
  int flag;
  temp=main_win;
  if(new_float("Size: ",&size))return;
  if(new_int("Color: ",&color))return;
  message_box(&temp,0,SCALEY-5*DCURY,"Choose start/end");
  flag=rubber(&i1,&j1,&i2,&j2,draw_win,1);
  XDestroyWindow(display,temp);
  XFlush(display);
  if(flag){
    scale_to_real(i1,j1,&xs,&ys);
    scale_to_real(i2,j2,&xe,&ye);
    if(i1==i2&&j1==j2)return;
    add_grob(xs,ys,xe,ye,size,type,color);
    redraw_all(); 
  }
}
  
edit_object()
{
  char ch,ans,str[80];
  int i,j,ilab=-1,flag,type,ip,jp;
  float x,y;
  float dist=1e20,dd;
  static char *list[]={"(M)ove","(C)hange","(D)elete"};
  static char key[]="mcd";
  static char title[]="Edit";
  Window temp=main_win;
  ch=(char)pop_up_list(&temp,title,list,key,3,9,0,10,10*DCURY+8,
		       edit_hint,info_pop,info_message);
  if(ch==27)return;
  temp=main_win;
  message_box(&temp,0,SCALEY-5*DCURY,"Choose Object");
  flag=get_mouse_xy(&i,&j,draw_win);
  XDestroyWindow(display,temp);
  XFlush(display);
  if(flag){
      scale_to_real(i,j,&x,&y);
      /* now search all labels to find the best */
      type=0;  /* label =  0, arrows, etc =1 */
      for(i=0;i<MAXLAB;i++){
	if(lb[i].use==1&&lb[i].w==draw_win){
	  dd=(x-lb[i].x)*(x-lb[i].x)+(y-lb[i].y)*(y-lb[i].y);
	  if(dd<dist){
	    ilab=i;
	    dist=dd;
	  }
	}
      }
      for(i=0;i<MAXGROB;i++){
	if(grob[i].use==1&&grob[i].w==draw_win){
	  dd=(x-grob[i].xs)*(x-grob[i].xs)+(y-grob[i].ys)*(y-grob[i].ys);
	  if(dd<dist){
	    ilab=i;
	    dist=dd;
	    type=1;
	  }
	}
      }
      if(ilab>=0&&type==0){
	switch(ch){
	case 'm':
	  sprintf(str,"Move %s ?", lb[ilab].s);
	  ans=(char)two_choice("Yes","No",str,"yn",0,0,main_win);
	  if(ans=='y'){
	    temp=main_win;
	    message_box(&temp,0,SCALEY-5*DCURY,"Click on new position");
	    flag=get_mouse_xy(&i,&j,draw_win);
	    XDestroyWindow(display,temp);
	    XFlush(display);
	    if(flag){
	      scale_to_real(i,j,&x,&y);
	      lb[ilab].x=x;
	      lb[ilab].y=y;
	      clr_scrn();
	      redraw_all();
	    }
	  }
	  break;
	case 'c':
	  sprintf(str,"Change %s ?", lb[ilab].s);
	  ans=(char)two_choice("Yes","No",str,"yn",0,0,main_win);
	  if(ans=='y'){
	    new_string("Text: ",lb[ilab].s);
            new_int("Size 0-4 :",&lb[ilab].size);
	    new_int("Font  0-times/1-symbol :",&lb[ilab].font);
	    if(lb[ilab].size>4)lb[ilab].size=4;
	    if(lb[ilab].size<0)lb[ilab].size=0;
	    clr_scrn();
	    redraw_all();
	  }
	  break;
	case 'd':
	  sprintf(str,"Delete %s ?", lb[ilab].s);
	  ans=(char)two_choice("Yes","No",str,"yn",0,0,main_win);
	  if(ans=='y'){
	    lb[ilab].w=0;
	    lb[ilab].use=0;
	    clr_scrn();
	    redraw_all();
	  }
	  break;
	}
      }
      if(ilab>=0&&type==1){
	switch(ch){
	case 'm':
	  sprintf(str,"Move graphic at (%f,%f)",
		  grob[ilab].xs,grob[ilab].ys);
	  ans=(char)two_choice("Yes","No",str,"yn",0,0,main_win);
	  if(ans=='y'){
	    temp=main_win;
	    message_box(&temp,0,SCALEY-5*DCURY,"Reposition");
	    flag=get_mouse_xy(&i,&j,draw_win);
	    XDestroyWindow(display,temp);
	    XFlush(display);
	    if(flag){
	      scale_to_real(i,j,&x,&y);
	      grob[ilab].xe=grob[ilab].xe-grob[ilab].xs+x;
	      grob[ilab].ye=grob[ilab].ye-grob[ilab].ys+y;
	      grob[ilab].xs=x;
	      grob[ilab].ys=y;
	      clr_scrn();
	      redraw_all();
	    }
	  }
	  break;
	case 'c':
	  sprintf(str,"Change graphic at (%f,%f)",
		  grob[ilab].xs,grob[ilab].ys);
	  ans=(char)two_choice("Yes","No",str,"yn",0,0,main_win);
	  if(ans=='y'){
            if(grob[ilab].type>=MARKER)
	      select_marker_type(&grob[ilab].type);
	    new_float("Size ",&grob[ilab].size);
            new_int("Color :", &grob[ilab].color);
	    clr_scrn();
	    redraw_all();
	  }
	  break;
	case 'd':
          sprintf(str,"Delete graphic at (%f,%f)",
		  grob[ilab].xs,grob[ilab].ys);
	  ans=(char)two_choice("Yes","No",str,"yn",0,0,main_win);
	  if(ans=='y'){
	    grob[ilab].w=0;
	    grob[ilab].use=0;
	    clr_scrn();
	    redraw_all();
	  }
	  break;
	}
      }

    }

}

  

do_gr_objs()
{
  char ch;
  static char *list[]={"(T)ext","(A)rrow","(P)ointer","(M)arker",
			 "(E)dit","(D)elete all","marker(S)"};
  static char key[]="tapmeds";
  static char title[]="Text,etc";
  Window temp=main_win;
  ch=(char)pop_up_list(&temp,title,list,key,7,10,0,10,10*DCURY+8,
		       text_hint,info_pop,info_message);
  switch(ch){
  case 't': 
    cput_text();
    break;
  case 'a':
     add_pntarr(ARROW);
    break;
  case 'p':
     add_pntarr(POINTER);
    break;
  case 'm':
     add_marker();
    break;
  case 's':
    add_markers();
    break;
  case 'e':
    edit_object();
    break;
  case 'd':
    destroy_label(draw_win);
    destroy_grob(draw_win);
    clr_scrn();
    redraw_all();
    break;
  }
}
do_windows()
{
 char ch;
 static char *list[]={"(C)reate","(K)ill all","(D)estroy","(B)ottom",
		"(A)uto","(M)anual"};
 static char key[]="ckdbam";
 static char title[]="Make window";

 Window temp=main_win;
 ch=(char)pop_up_list(&temp,title,list,key,6,11,0,10,14*DCURY+8,
		      half_hint,info_pop,info_message);
 switch(ch){
        case 27: break;
	
	case 'c': create_a_pop();
         		break;
 	case 'k': 
		if(yes_no_box())kill_all_pops();
		break;
	case 'b': 
		 XLowerWindow(display,draw_win);
		break;
	case 'd': destroy_a_pop();
		break;
	case 'm': set_restore(0);
		 break;
        case 'a': set_restore(1);
		 break;
        default: create_a_pop();
                 break;
	}
       /* 	XDestroyWindow(display,temp);   */
	
}

  set_restore(flag)
  {
   int i;
      for(i=0;i<MAXPOP;i++){
    if(graph[i].w==draw_win){
	    graph[i].Restore=flag;
	    graph[i].Nullrestore=flag;
	return;
	}
  }
  }

 destroy_a_pop()
 {
  int i;
  if(draw_win==graph[0].w)
  {
   respond_box(main_win,0,0,"Okay","Can't destroy big window!");
   return;
  }
  for(i=1;i<MAXPOP;i++)
   if(graph[i].w==draw_win)break;
  if(i>=MAXPOP)return;
   select_window(graph[0].w);
  graph[i].Use=0;
  destroy_label(graph[i].w);
  destroy_grob(graph[i].w);
  XDestroySubwindows(display,graph[i].w);
  XDestroyWindow(display,graph[i].w);
  num_pops--;
  }


init_grafs(x,y,w,h)
int x,y,w,h;
{
 int i;
 GrCol();
 for(i=0;i<MAXLAB;i++)
 {
  lb[i].use=0;
  lb[i].w=(Window)0;
 } 
 for(i=0;i<MAXGROB;i++){
   grob[i].w=(Window)0;
   grob[i].use=0;
 }
 init_bd();
 for(i=0;i<MAXFRZ;i++)
   frz[i].use=0;
 /* for(i=0;i<MAXNCLINE ... */
 for(i=0;i<MAXPOP;i++)
 graph[i].Use=0;
 init_all_graph();
  graph[0].w=XCreateSimpleWindow(display,main_win,x,y,w,h,2,GrFore,GrBack);
 graph[0].w_info=info_pop;
 info_message=graph[0].gr_info;
 graph[0].Use=1;
 graph[0].Restore=1;
 graph[0].Nullrestore=1;
 graph[0].x0=x;
 graph[0].y0=y;
 graph[0].Height=h;
 graph[0].Width=w;
 XSelectInput(display,graph[0].w,KeyPressMask|ButtonPressMask|ExposureMask|
	      ButtonReleaseMask|ButtonMotionMask| StructureNotifyMask);
 num_pops=1;
 XMapWindow(display,graph[0].w);
 draw_win=graph[0].w;
 current_pop=0;
 get_draw_area();
 select_sym(graph[0].w);
 BaseCol();
}
/*
 draw_help()
{
 	switch(help_menu){
		case MAIN_HELP: help(); break;
		case NUM_HELP : help_num();break;
	
		case FILE_HELP: help_file();break;
	
}
}
*/
 ps_restore()
{
 do_axes();
 restore(0,my_browser.maxrow);
  if(MyGraph->Nullrestore)restore_nullclines();
 redraw_dfield();
 draw_label(draw_win);
 draw_freeze(draw_win);
 ps_end();
}

 do_motion_events(ev)
     XEvent ev;
{
  int i=ev.xmotion.x;
  int j=ev.xmotion.y;
  float x,y;
  char buf[256];
  slider_motion(ev);
#ifdef AUTO
  auto_motion(ev);
#endif
  if(ev.xmotion.window==draw_win){
    scale_to_real(i,j,&x,&y);
    sprintf(buf,"x=%f y=%f ",x,y);
    canvas_xy(buf);
  }
}
 
 do_expose(ev)
 XEvent ev;
 {
  int i;
  int cp=current_pop;
  Window temp;
  temp=draw_win;
  expose_aplot(ev.xany.window);
  ani_expose(ev.xany.window);
  expose_my_browser(ev);
  /* draw_info_pop(ev.xany.window); */
  draw_eq_list(ev.xany.window);
  draw_eq_box(ev.xany.window);
  do_box_expose(ev.xany.window);
  expose_slides(ev.xany.window);
  menu_expose(ev.xany.window);
#ifdef AUTO
  display_auto(ev.xany.window);
#endif 
  /* if(ev.xexpose.window==menu_pop){
	draw_help();
 	
   return;
    }
    */
  GrCol();
     for(i=0;i<MAXPOP;i++){
       if((graph[i].Use)&&(ev.xexpose.window==graph[i].w_info)){
	 XClearWindow(display,graph[i].w_info);
	 if(i==0){
	   BaseCol();
	   XDrawString(display,graph[i].w_info,gc,5,CURY_OFF,
		       graph[i].gr_info,strlen(graph[i].gr_info));
	 }
	 else{
	   SmallBase();
	   XDrawString(display,graph[i].w_info,small_gc,0,CURY_OFFs,
		     graph[i].gr_info,strlen(graph[i].gr_info));
	   SmallGr();
	 }
	
       }
	if((graph[i].Use)&&(ev.xexpose.window==graph[i].w)){
	  /* redraw_dfield(); */
		current_pop=i;
		MyGraph=&graph[i];
		draw_win=graph[i].w;
  		get_draw_area();
 		do_axes();
  		if(graph[i].Restore)restore(0,my_browser.maxrow);
		draw_label(graph[i].w);
		draw_freeze(graph[i].w);
		if(graph[i].Nullrestore)restore_nullclines();
	}
   }
   draw_win=temp;
   MyGraph=&graph[cp];
   current_pop=cp;
   hi_lite(draw_win);
   get_draw_area();
   BaseCol();
 
	
	



  }


resize_all_pops(wid,hgt)
int wid,hgt;
{
 int i;
 int ox=graph[0].x0;
 int nx,ny,wp,hp;
 int oy=graph[0].y0;
 int ow=graph[0].Width,oh=graph[0].Height;
 int nw=wid-16-16*DCURX,nh=hgt-5*DCURY-16;
 XResizeWindow(display,graph[0].w,nw,nh);
 graph[0].Width=nw;
 graph[0].Height=nh;
  get_draw_area();
}

kill_all_pops()
{
 int i;
 select_window(graph[0].w);

 for(i=1;i<MAXPOP;i++)
 if(graph[i].Use){
   graph[i].Use=0; 
   destroy_label(graph[i].w);
   destroy_grob(graph[i].w);
   
	XDestroySubwindows(display,graph[i].w);
        XDestroyWindow(display,graph[i].w);
      
 }
 num_pops=1;
}



 create_a_pop()
 {
  int i,index;
  int x,y,xp,yp,h,w;
  int xg,yg,hg,wg,bw,de;
  Window root;
  Window temp=main_win;
  for(i=1;i<MAXPOP;i++)
  if(graph[i].Use==0)break;
  if(i>=MAXPOP)
  {
   respond_box(main_win,0,0,"Okay","Too many windows!");
   return;
  }
index=i;


graph[index].w=XCreateSimpleWindow(display,RootWindow(display,screen),0,0,200,200,2,GrFore,GrBack);
 graph[index].w_info=make_window(graph[index].w,10,0,40*DCURXs,DCURYs,0);
  copy_graph(index,current_pop);
  graph[index].Width=200;
  graph[index].Height=200;
  graph[index].x0=0;
  graph[index].y0=0;
  num_pops++;
  make_icon(graph_bits,graph_width,graph_height,graph[index].w);
  XSelectInput(display,graph[index].w,KeyPressMask|ButtonPressMask|ExposureMask|
	       ButtonReleaseMask|ButtonMotionMask);
  XMapWindow(display,graph[index].w);
  XRaiseWindow(display,graph[index].w);
  XSetWMProtocols(display, graph[index].w, &deleteWindowAtom, 1);
  select_window(graph[index].w);
 /*  XDestroyWindow(display,temp); */
}

GrCol()
{
 XSetForeground(display,gc,GrFore);
 XSetBackground(display,gc,GrBack);
 
}

BaseCol()
{
 XSetForeground(display,gc,MyForeColor);
 XSetBackground(display,gc,MyBackColor);
}

SmallGr()
{
 XSetForeground(display,small_gc,GrFore);
 XSetBackground(display,small_gc,GrBack);
}

SmallBase()
{
 XSetForeground(display,small_gc,MyForeColor);
 XSetBackground(display,small_gc,MyBackColor);
}


select_window(w)
Window w;
{
 int i;

 if(w==draw_win)return;
 GrCol();
 if(w==graph[0].w)current_pop=0;
 else{
 
  for(i=1;i<MAXPOP;i++)if((graph[i].Use)&&(w==graph[i].w))current_pop=i;
 }
 MyGraph=&graph[current_pop];
 lo_lite(draw_win);
 draw_win=w;
 hi_lite(w);
  XRaiseWindow(display,w); 
 get_draw_area();
BaseCol();
}
 
set_gr_fore()
{
 XSetForeground(display,gc,GrFore);
}
set_gr_back()
{
 XSetForeground(display,gc,GrBack);
}

hi_lite(wi)
Window wi;
{
  set_gr_fore();
  select_sym(wi);
 
}

lo_lite(wi)
Window wi;
{
 set_gr_back();
 bar(0,0,5,5,wi);
 
}

select_sym(w)
Window w;
{
 bar(0,0,5,5,w);
}
 
canvas_xy(buf)
char *buf;
{
  XClearWindow(display,MyGraph->w_info);
  strcpy(MyGraph->gr_info,buf);
  if(MyGraph->w_info==info_pop){
    BaseCol();
    XDrawString(display,MyGraph->w_info,gc,5,CURY_OFF,buf,strlen(buf));
  }
  else{
    SmallBase();
    XDrawString(display,MyGraph->w_info,small_gc,0,CURY_OFFs,buf,strlen(buf));
    /* SmallGr(); */
  }
}  

check_draw_button(ev)
XEvent ev;
{
 int k;
 char buf[256];
 char ch;
 int button;
 int i,j;
 float x,y;
 int flag=0;
 Window w;
 button=ev.xbutton.button;
 w=ev.xbutton.window;
 i=ev.xbutton.x;
 j=ev.xbutton.y;
 if(button==1){          /* select window   */

 for(k=1;k<MAXPOP;k++)
 if((graph[k].Use)&&(w==graph[k].w))flag=1;
   if((w==graph[0].w)||(flag==1))select_window(w);
  }
 else  /* any other button   */
 {
   if(w!=draw_win)return;
   scale_to_real(i,j,&x,&y);
 sprintf(buf,"x=%f y=%f ",x,y);
 canvas_xy(buf);
  }
}  

 
   




