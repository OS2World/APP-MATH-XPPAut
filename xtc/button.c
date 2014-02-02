#include <X11/Xlib.h>
#include <stdio.h>
#include "struct.h"


extern Display *display;
extern int screen;
extern int DCURY,DCURX,CURY_OFF,xor_flag;
extern GC gc;
extern unsigned long MyBackColor,MyForeColor;

Window make_window();

redraw_all(b)
BUTLIST b;
{
 int i;
 XClearWindow(display,b.home);
 for(i=0;i<b.n;i++){
   XClearWindow(display,b.w[i]);
   XDrawString(display,b.w[i],gc,DCURX/2,DCURY+2,b.names[i],strlen(b.names[i]));
 }
}
draw_buttons(b,w)
     BUTLIST b;
     Window w;
{
  int i;
  for(i=0;i<b.n;i++)if(w==b.w[i])
    XDrawString(display,w,gc,DCURX/2,DCURY+2,b.names[i],strlen(b.names[i]));
}
  
do_button_events(b,ev)
     BUTLIST b;
     XEvent ev;

{
  int i;
  switch(ev.type){
  case Expose:
  case MapNotify:
    draw_buttons(b,ev.xexpose.window);
    break;
  case KeyPress:
    return(get_key_press(&ev));
    break;
  case ButtonPress:
    do_graph_buttons(ev.xbutton.window);
    for(i=0;i<b.n;i++)
      if(ev.xbutton.window==b.w[i])
	return((int)b.key[i]);
    break;
  case EnterNotify:
    for(i=0;i<b.n;i++)if(ev.xcrossing.window==b.w[i])
      XSetWindowBorderWidth(display,
			    b.w[i],2);
    break;
  case LeaveNotify:
    for(i=0;i<b.n;i++)if(ev.xcrossing.window==b.w[i])
      XSetWindowBorderWidth(display,
			    b.w[i],1);
    break;
  }
  return(0);
}

make_buttons(n,names,key,b,home)
BUTLIST *b;
int n;
char **names,*key;
Window home;
{
  int i;
  b->w = (Window *) malloc(n*sizeof(Window));
  b->names=names;
  b->n=n;
  b->key=key;
  b->home=home;
  for(i=0;i<n;i++){
    b->w[i]=make_window(home,DCURX,DCURY+i*(DCURY+8),
			DCURX*(strlen(b->names[i])+2),DCURY+4,1);
    XSelectInput(display,b->w[i],BUT_MASK);
  }
}

  





