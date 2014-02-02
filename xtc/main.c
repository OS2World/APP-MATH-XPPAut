#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/bitmaps/icon>
#include <math.h>
#include <stdio.h>
#include "xtc.bit"
#include "graph.bit"
#include "turing.bit"
#include <X11/cursorfont.h>
#define BITMAPDEPTH 1
#define TOO_SMALL 0
#define BIG_ENOUGH 1
#include "struct.h"



Window init_win();
extern char this_file[100];
extern int MyRGB;
char big_font_name[100],small_font_name[100];
Window main_win,command_pop,menu_pop,graph_3d,graph_2d;
GC gc,gc_graph,small_gc;
unsigned long Black,White;
unsigned long MyBackColor,MyForeColor;
unsigned long GrFore,GrBack;
Display *display;
int screen;
int SCALEX,SCALEY;
int PaperWhite=1;  /* Change to 0 for Black background   */
int DCURYb,DCURXb,CURY_OFFb;
int DCURYs,DCURXs,CURY_OFFs;
int DCURY,DCURX,CURY_OFF;
 XFontStruct *big_font,*small_font;

int COLOR;
int color_min,color_scale;

BUTLIST main_buttons;


main(argc,argv)
     char **argv;
     int argc;
{
     static char *lefty[]={"Init. data","3D graphs","2D Graphs","Numerics",
		       "File","Redraw","More time","Erase","Go","Window",
			     "Parameter","Turing"};
     static char *left_key="i32nfrmegwpt";
     char xtc_title[60];
     char *icon_name="xtc";
     char *win_name;
     unsigned int min_width=400,min_hgt=330;
     unsigned int x=0,y=0;
     load_the_file(argc,argv);
     strcpy(big_font_name,"fixed");
     strcpy(small_font_name,"fixed");
     SCALEX=550;
     SCALEY=380;
     if(strlen(this_file)<50)
     sprintf(xtc_title,"XTC << %s",this_file);
     else sprintf(xtc_title,"Ahhhhh...XTC");
     win_name=xtc_title;
      main_win=init_win(4,icon_name,win_name,
   x,y,min_width,min_hgt,argc,argv);
     
XSelectInput(display,main_win,ExposureMask|KeyPressMask|ButtonPressMask|
              StructureNotifyMask);
load_fonts();
DCURXb=XTextWidth(big_font,"W",1);
DCURYb=big_font->ascent+big_font->descent;
CURY_OFFb=big_font->ascent+1;
DCURXs=XTextWidth(small_font,"W",1);
DCURYs=small_font->ascent+small_font->descent;
CURY_OFFs=small_font->ascent+1;


 Black=BlackPixel(display,screen);
White=WhitePixel(display,screen);  
/*  Switch for reversed video  */
MyForeColor=GrFore=Black;
MyBackColor=GrBack=White;
/*  This is reversed  
MyForeColor=White;
MyBackColor=Black; */
if(PaperWhite==0){
GrFore=White;
GrBack=Black; 
}
getGC(&gc);
getGC(&gc_graph);
getGC(&small_gc);
COLOR=1;
  if(DefaultDepth(display,screen)==4)color16();
     if(DefaultDepth(display,screen)>=8)color256(MyRGB);
     if(DefaultDepth(display,screen)<4){COLOR=0;make_greys();}
   
set_big_font(); 
/* set_small_font(); */

XSetFont(display,small_gc,small_font->fid);
XMapWindow(display,main_win);

 make_pops();
 make_buttons(12,lefty,left_key,&main_buttons,menu_pop);
 init_3d();
 init_2d();

do_events(min_width,min_hgt);
} 

flush_display()
{
 XFlush(display);
}
rdb()
{
 redraw_all(main_buttons);
}

redo_color(flag)
int flag;
{
 if(flag<0||flag>2)return;
  if(DefaultDepth(display,screen)>=8)color256(flag);
}
 
main_command(com)
     int com;
{
  char ch=(char)com;
  switch(ch){
  case 'i':
    com_integrate(); rdb();
    break;

  case '3':
    com_3d(); rdb();
    break;
  case '2':
    com_2d(); rdb();
    break;
  case 'n':
    com_numerics(); rdb();
    break;
  case 'f':
    com_file(); rdb();
    break;
  case 'r':
    com_redraw(); rdb();
    break;
  case 'm':
    com_continue(); rdb();
    break;
  case 'e':
    com_erase(); rdb();
    break;
  case 'g':
	solve_them();
    break;
  case 'w':
    com_window(); rdb();
    break;
  case 'p':
    com_parameter(); rdb();
    break;
  case 't':
    com_turing(); rdb();
    break;
  }
}

do_events(min_wid,min_hgt)
unsigned int min_wid,min_hgt;  
{
 XEvent report;
 int window_size,com;
 char string[100];
 char ch;
 int i1=-1,i2;
 int used;
  
while(1)
{
  XNextEvent(display,&report);
  main_command(do_button_events(main_buttons,report));
   
  switch(report.type){
  case Expose:
  case MapNotify:
    break;
  case ConfigureNotify:
    if(report.xconfigure.window==main_win){
      SCALEX=report.xconfigure.width;
      SCALEY=report.xconfigure.height;
      if((SCALEX<min_wid) || (SCALEY<min_hgt)){
	window_size=TOO_SMALL;
	SCALEX=min_wid;
	SCALEY=min_hgt;}
      else {
	window_size=BIG_ENOUGH;
	XResizeWindow(display,command_pop,SCALEX-4,DCURY+1);
	XResizeWindow(display,graph_3d,SCALEX-16*DCURX-8,SCALEY-DCURY-8);
	get_3d_area();

      }
    }
    if(report.xconfigure.window==graph_2d){
      get_2d_area();

    }
    break;

  case KeyPress:

               break;
case ButtonPress:

		 break;


 } /* end switch */
 } /* end while */
}


do_expose(ev)
XEvent(ev);
{
}





set_big_font()
{
 DCURX=DCURXb;
 DCURY=DCURYb;
 CURY_OFF=CURY_OFFb;
 XSetFont(display,gc,big_font->fid);
}

set_small_font()
{
  DCURX=DCURXs;
 DCURY=DCURYs;
 CURY_OFF=CURY_OFFs;
 XSetFont(display,gc,small_font->fid);
}


bye_bye()
{
   XUnloadFont(display,big_font->fid);
  XUnloadFont(display,small_font->fid);
 XFreeGC(display,gc);
XCloseDisplay(display);
 exit(1);
}











 Window init_win(bw,icon_name,win_name,
                 x,y,min_wid,min_hgt,argc,argv)
 int argc;
 char **argv;
 unsigned int min_wid,min_hgt,bw;
 char *icon_name,*win_name;
 {
  Window wine;
  int count;
  unsigned dp_h,dp_w;
  Pixmap icon_map;
 XIconSize *size_list;
  XSizeHints size_hints;
 char *display_name=NULL;
 if((display=XOpenDisplay(display_name))==NULL)exit(-1);
/*   Remove after debugging is done    
 XSynchronize(display,1); */
 screen=DefaultScreen(display);
 dp_w=DisplayWidth(display,screen);
 dp_h=DisplayHeight(display,screen);
 if(SCALEX>dp_w)SCALEX=dp_w;
 if(SCALEY>dp_h)SCALEY=dp_h;
 wine=XCreateSimpleWindow(display,RootWindow(display,screen),
     x,y,SCALEX,SCALEY,bw,MyForeColor,
     MyBackColor);
 XGetIconSizes(display,RootWindow(display,screen),&size_list,&count);
 icon_map=XCreateBitmapFromData(display,wine,
 xtc_bits,xtc_width,xtc_height);

#ifdef X11R3
  size_hints.flags=PPosition|PSize|PMinsize;
 size_hints.x=x;
 size_hints.y=y;
 size_hints.width=width;
 size_hints.height=height;
 size_hints.min_width=min_wid;
 size_hints.min_height=min_hgt;
#else 

 size_hints.flags=PPosition|PSize|PMinSize;
 size_hints.min_width=min_wid;
 size_hints.min_height=min_hgt;
#endif

#ifdef X11R3
 XSetStandardProperties(display,wine,win_name, icon_name,icon_map,argv,
  argc,&size_hints);
#else 
 {
  XWMHints wm_hints;
  XClassHint class_hints;
  XTextProperty winname,iconname;
 if(XStringListToTextProperty(&icon_name,1,&iconname)==0)
 {
 printf("failure for iconname\n");
 exit(-1);
 }
if(XStringListToTextProperty(&win_name,1,&winname)==0)
 {
 printf("failure for winname\n");
 exit(-1);
 }
 
 wm_hints.initial_state=NormalState;
 wm_hints.input=True;
 wm_hints.icon_pixmap=icon_map;
 wm_hints.flags=StateHint|IconPixmapHint|InputHint;
 class_hints.res_name="base";
 class_hints.res_class="Basicwin";
 XSetWMProperties(display,wine,&winname,&iconname,argv,argc,&size_hints,&wm_hints,&class_hints);
}
#endif
 return(wine);
}

getGC(gc)
GC *gc;
{
 unsigned long valuemask=0;
 XGCValues values;
 unsigned int lw=6;
 int ls=LineOnOffDash;
 int cs=CapRound;
 int js=JoinRound;
 int dash_off=0;
 static char dash[]={12,24};
 int ll=2;
 *gc=XCreateGC(display,main_win,valuemask,&values);
 XSetForeground(display,*gc,MyForeColor);
/* XSetLineAttributes(display,*gc,lw,ls,cs,js);
 XSetDashes(display,*gc,dash_off,dash,ll); */
}

 load_fonts()

 {


  if((big_font=XLoadQueryFont(display,big_font_name))==NULL)
 {
  printf("Failed for  font %s\n",big_font_name);
  exit(-1);
 }
 /*  if((small_font=XLoadQueryFont(display,"fixed"))==NULL)
 {
  printf("Failed for fixed font\n");
  exit(-1);
 } */
  if((small_font=XLoadQueryFont(display,small_font_name))==NULL)
 {
  printf("Failed for fixed font %s\n",small_font_name);
  exit(-1);
 }

}


make_pops()

{  int x,y,h,w,bw,d;
   Window wn;
   Cursor cursor;
   XGetGeometry(display,main_win,&wn,&x,&y,&w,&h,&bw,&d);
 menu_pop=XCreateSimpleWindow(display,main_win,
     0,DCURY+6,16*DCURX,13*(DCURY+8),2,MyForeColor,
     MyBackColor);
 command_pop=XCreateSimpleWindow(display,main_win,0,0,w-2,DCURY+4,2,
MyForeColor,
     MyBackColor);
 graph_3d=XCreateSimpleWindow(display,main_win,DCURX*16+6,DCURY+6,
			      w-DCURX*16-8,h-DCURY-8,2,GrFore,GrBack);
 cursor=XCreateFontCursor(display,XC_hand2);
 XDefineCursor(display,menu_pop,cursor);
XSelectInput(display,menu_pop,KeyPressMask|ButtonPressMask|ExposureMask);
XSelectInput(display,command_pop,KeyPressMask|ButtonPressMask|ExposureMask);
 XSelectInput(display,graph_3d,KeyPressMask|ButtonPressMask|ExposureMask);
 XMapWindow(display,graph_3d);
 XMapWindow(display,command_pop);
 XMapWindow(display,menu_pop);
 make_2d_window();
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

make_2d_window()
{
  graph_2d=XCreateSimpleWindow(display,RootWindow(display,screen),
			       200,200,200,200,2,GrFore,GrBack);
  make_icon(graph_bits,graph_width,graph_height,graph_2d);
  XSelectInput(display,graph_2d,KeyPressMask|ButtonPressMask|ExposureMask|
StructureNotifyMask);
  XMapWindow(display,graph_2d);
  XRaiseWindow(display,graph_2d);
  title_2d("Two-D View",graph_2d);
}

make_turing_window(w)
 Window *w;
{
 *w=XCreateSimpleWindow(display,RootWindow(display,screen),
			       200,200,200,200,2,GrFore,GrBack);
  make_icon(turing_bits,turing_width,turing_height,*w);
  XSelectInput(display,*w,KeyPressMask|ButtonPressMask|ExposureMask|
StructureNotifyMask);
  XMapWindow(display,*w);
  XRaiseWindow(display,*w);
  title_2d("Stability",*w);
}

 title_2d(string,w)
 char *string;
 Window w;
 {
  XTextProperty wname,iname;
   XStringListToTextProperty(&string,1,&wname);
  XStringListToTextProperty(&string,1,&iname);
 XSetWMProperties(display,w,&wname,&iname,NULL,0,NULL,NULL,NULL);
}





