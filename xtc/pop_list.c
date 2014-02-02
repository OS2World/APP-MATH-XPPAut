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
extern Window menu_pop,main_win,info_pop,draw_win;
extern int DCURY,DCURX,CURY_OFF,xor_flag;
extern GC gc;
extern unsigned long MyBackColor,MyForeColor;

char *get_next(),*get_first();

Window make_window();

/*  This is a string box widget which handles a list of 
	editable strings  
 */

typedef struct {
		Window base,ok,cancel;
		Window win[MAX_N_SBOX];
		char name[MAX_N_SBOX][MAX_LEN_SBOX],
		     value[MAX_N_SBOX][MAX_LEN_SBOX];
		int n,hot;
		} STRING_BOX;

/*  This is a new improved pop_up widget */
typedef struct {
		Window base,tit;
		Window *w;
		char *title;
		char **entries;
		int n,max;
		char *key;
		int hot;
		} POP_UP;
		
   





do_string_box(n,row,col,title,names,values,maxchar)
int n,row,col,maxchar;
char **names,values[][MAX_LEN_SBOX],*title;

{
 STRING_BOX sb;
 int i,status;
 int colm,pos;
 XEvent ev;
  for(i=0;i<n;i++){
	sprintf(sb.name[i],"%s:",names[i]);
	strcpy(sb.value[i],values[i]);
	}
  sb.n=n;
  sb.hot=0;
  make_sbox_windows(&sb,row,col,title,maxchar);
 XSelectInput(display,sb.cancel,BUT_MASK);
  	 XSelectInput(display,sb.ok,BUT_MASK);

  pos=strlen(sb.value[0]);
 colm=(pos+strlen(sb.name[0]))*DCURX;
  while(1){
   status=s_box_event_loop(&sb,&pos,&colm);
  if(status!=-1)break;
  }
  XSelectInput(display,sb.cancel,EV_MASK);
  	 XSelectInput(display,sb.ok,EV_MASK);


 XDestroySubwindows(display,sb.base);
 XDestroyWindow(display,sb.base);

  if(status==FORGET_ALL) return(status);
  for(i=0;i<n;i++)strcpy(values[i],sb.value[i]);
  return(status);
   
	
 }

expose_sbox(sb,w,pos,col)
STRING_BOX sb;
Window w;
int pos,col;
{
 int i,flag;
 int l,m;
 if(w==sb.ok){XDrawString(display,w,gc,0,CURY_OFF,"Ok",2);return;}
 if(w==sb.cancel){XDrawString(display,w,gc,0,CURY_OFF,"Cancel",6);
		   return; 
                 }
 for(i=0;i<sb.n;i++){
 if(w!=sb.win[i])continue;
 flag=0;
 if(i==sb.hot)flag=1;
 do_hilite_text(sb.name[i],sb.value[i],flag,w,pos,col);
}
}

do_hilite_text(name,value,flag,w,pos,col)
char *name,*value;
Window w;
int flag;
int pos,col;
{
 int l=strlen(name);
 int m=strlen(value);
 if(flag){
 set_fore();
 bar(0,0,l*DCURX,DCURY+4,w);
 set_back();
 }
 XDrawString(display,w,gc,0,CURY_OFF,name,l);
 set_fore();
 if(m>0){
 XDrawString(display,w,gc,l*DCURX,CURY_OFF,value,m);
 }
 /* if(flag) showchar('_',DCURX*(l+m),0,w); */
 if(flag) put_cursor_at(w,DCURX*l,pos);
}

  
reset_hot(inew,sb)
int inew;
STRING_BOX *sb;
{
 int i=sb->hot;
 sb->hot=inew;
 XClearWindow(display, sb->win[inew]);
 do_hilite_text(sb->name[inew],sb->value[inew],1,sb->win[inew],
		strlen(sb->value[inew]),0);
 XClearWindow(display, sb->win[i]);
 do_hilite_text(sb->name[i],sb->value[i],0,sb->win[i],
		strlen(sb->value[i]),0);
 }

 new_editable(sb,inew,pos,col,done,w)
 int inew;
 STRING_BOX *sb;
 int *pos,*col,*done;
 Window *w;
 {
  int i;
  reset_hot(inew,sb);
  *pos=strlen(sb->value[inew]);
  *col=(*pos+strlen(sb->name[inew]))*DCURX;
  *done=0;
  *w=sb->win[inew];
  }
 

s_box_event_loop(sb,pos,col)
     STRING_BOX *sb;
     int *col,*pos;
{
 XEvent ev;
 int status=-1,inew;
 int nn=sb->n;
 int done=0,i;
 char ch;
 int ihot=sb->hot;
 Window wt;
 Window w=sb->win[ihot];   /* active window   */
 char *s;
 s=sb->value[ihot];
  
 XNextEvent(display,&ev);
 switch(ev.type){
 	case ConfigureNotify:
	case Expose:
	case MapNotify:
		do_expose(ev);  /*  menus and graphs etc  */
		expose_sbox(*sb,ev.xany.window,*pos,*col);
		break;

	case ButtonPress:
		if(ev.xbutton.window==sb->ok){status=DONE_ALL;break;}
		if(ev.xbutton.window==sb->cancel){status=FORGET_ALL;break;}
			for(i=0;i<nn;i++)
 		{
			if(ev.xbutton.window==sb->win[i]){
		    XSetInputFocus(display,sb->win[i],
			RevertToParent,CurrentTime);
			 if(i!=sb->hot)new_editable(sb,i,pos,col,&done,&w);
			 break;
			}
                  }
                  break;

        case EnterNotify:
	wt=ev.xcrossing.window;
        if(wt==sb->ok||wt==sb->cancel)
	 XSetWindowBorderWidth(display,wt,2);
	break;

	case LeaveNotify:
	wt=ev.xcrossing.window;
        if(wt==sb->ok||wt==sb->cancel)
	 XSetWindowBorderWidth(display,wt,1);
	break;
	
	case KeyPress:
	ch=get_key_press(&ev);
	edit_window(w,pos,s,col,&done,ch);
       if(done!=0){
			if(done==DONE_ALL){status=DONE_ALL;break;}
		   inew=(sb->hot+1)%nn;
		   new_editable(sb,inew,pos,col,&done,&w);
		  }
        break;
		   
	}
       return(status);
     }           	
        
	

 
  



 

	
 
 
make_sbox_windows(sb,row,col,title,maxchar)
int row,col,maxchar;
char *title;
STRING_BOX *sb;
{
 int width,height;
 int i;
 int xpos,ypos,status,n=sb->n;
   int xstart,ystart;

 XTextProperty winname;
   XSizeHints size_hints;
   Window base,w;
   width=(maxchar+4)*col*DCURX;
   height=(row+4)*(DCURY+16);
   base=make_window(DefaultRootWindow(display),0,200,width,height,4);
 XStringListToTextProperty(&title,1,&winname);
 size_hints.flags=PPosition|PSize|PMinSize|PMaxSize;
 size_hints.x=0;
 size_hints.y=0;
 size_hints.width=width;
 size_hints.height=height;
 size_hints.min_width=width;
 size_hints.min_height=height;
 size_hints.max_width=width;
 size_hints.max_height=height;
 XSetWMProperties(display,base,&winname,NULL,NULL,0,&size_hints,NULL,NULL);
 sb->base=base;
 
  ystart=DCURY;
   xstart=DCURX;
  for(i=0;i<n;i++) {
   xpos=xstart+(maxchar+4)*DCURX*(i/row);
  ypos=ystart+(i%row)*(DCURY+10);
  sb->win[i]=make_window(base,xpos,ypos,maxchar*DCURX,DCURY,1);
  }
  
 ypos=height-2*DCURY;
  xpos=(width-12*DCURX)/2;
  (sb->ok)=make_window(base,xpos,ypos,2*DCURX,DCURY,1);
  (sb->cancel)=make_window(base,xpos+4*DCURX,ypos,6*DCURX,DCURY,1);
  XRaiseWindow(display,base);
 }

 



 
 

   
  
		




Window make_window(root,x,y,width,height,bw)
	Window root;
	int x,y,width,height,bw;
	{
	 Window win;
	 win=XCreateSimpleWindow(display,root,x,y,width,height,
		bw,MyForeColor,MyBackColor);
	 XSelectInput(display,win,EV_MASK);
	 XMapWindow(display,win);
         return(win);
         }


    expose_resp_box(button,message,wb,wm,w,nm)
    Window w,wb,wm;
     int nm;
    char *button,**message;
   {
     int i;
	if(w==wb)ftext(0,0,button,wb);
	if(w==wm){
	  for(i=0;i<nm;i++)ftext(0,i*DCURY,message[i],wm);
	}
   }


    respond_box(w,x,y,button,message,nm)
    char *button,**message;
    Window w;
    int x,y,nm;
    {
     int l1;
      int l2=strlen(button);
      int width;
      int height;
      int done=0,i;
      XEvent ev;
      Window wmain,wb,wm;
      l1=strlen(message[0]);
      for(i=0;i<nm;i++){
	if(strlen(message[i])>l1)l1=strlen(message[i]);
      }
      width=l1;
      if(l1<l2)width=l2;
      width=width+4;
      height=4*DCURY+DCURY*nm;
       wmain=make_window(w,x,y,width*DCURX,height,4);
       wm=make_window(wmain,((width-l1)*DCURX)/2,DCURY/2,l1*DCURX,nm*DCURY,0);
       wb=make_window(wmain,((width-l2)*DCURX)/2,2*DCURY+nm*DCURY,l2*DCURX,DCURY,1);
            ping();
      XSelectInput(display,wb,BUT_MASK);
       while(!done)
       {
	XNextEvent(display,&ev);
        switch(ev.type){
			case Expose:
			case MapNotify:
				do_expose(ev);
				expose_resp_box(button,message,wb,
				wm,ev.xexpose.window,nm);
				break;
			case KeyPress:
				done=1;
				break;
			case ButtonPress:
			     if(ev.xbutton.window==wb)done=1;
			     break;
			case EnterNotify:
			     if(ev.xcrossing.window==wb)
			     XSetWindowBorderWidth(display,wb,2);
			     break;
			case LeaveNotify:
			     if(ev.xcrossing.window==wb)
			     XSetWindowBorderWidth(display,wb,1);
			     break;
			
			}
	}
			
		    XSelectInput(display,wb,EV_MASK);
	  	    XDestroyWindow(display,wmain); 
    }
			    
			

 
       
       
       
      
      
      
    

    message_box(w,x,y,message)
	Window *w;
	int x,y;
	char *message;
	{
	 int wid=strlen(message)*DCURX;
	 int hgt=4*DCURY;
	 Window z;
	 z=make_window(*w,x,y,wid+50,hgt,4);
         XSelectInput(display,z,0);
	 ftext(25,2*DCURY,message,z);
	 ping();
	 *w=z;
	         }   


 expose_choice(choice1,choice2,msg,c1,c2,wm,w)
 Window c1,c2,wm,w;
 char *choice1,*choice2,*msg;
{
 	if(w==wm)ftext(0,0,msg,wm);
	if(w==c1)ftext(0,0,choice1,c1);
	if(w==c2)ftext(0,0,choice2,c2);
}

 two_choice(choice1,choice2,string,key,x,y,w)
 char *choice1,*choice2,*string,*key;
 int x,y;
  Window w;
 {
  Window base,c1,c2,wm;
  XEvent ev;
  int not_done=1;
  int value=0;
  int l1=strlen(choice1)*DCURX;
  int l2=strlen(choice2)*DCURX;
  int lm=strlen(string)*DCURX;
  int tot=lm,xm,x1,x2;
  
   if(lm<(l1+l2+4*DCURX))tot=(l1+l2+4*DCURX);
   tot=tot+6*DCURX;
    xm=(tot-lm)/2;
	x1=(tot-l1-l2-4*DCURX)/2;
	x2=x1+l1+4*DCURX;
        base=make_window(w,x,y,tot,5*DCURY,4);
	c1=make_window(base,x1,3*DCURY,l1+DCURX,DCURY+4,1);
        c2=make_window(base,x2,3*DCURY,l2+DCURX,DCURY+4,1);
	  XSelectInput(display,c1,BUT_MASK);
 	 XSelectInput(display,c2,BUT_MASK);

	wm=make_window(base,xm,DCURY/2,lm+2,DCURY,0);
	ping();

        while(not_done){
        XNextEvent(display,&ev);
		switch(ev.type){
		case Expose:
		case MapNotify:
				do_expose(ev);
				expose_choice(choice1,choice2,string,c1,c2,wm,
				ev.xexpose.window);
				break;

		case ButtonPress:
			 if(ev.xbutton.window==c1){
				value=(int)key[0];
				not_done=0;
				}
			 if(ev.xbutton.window==c2){
				value=(int)key[1];
				not_done=0;
				}
				break;
		case KeyPress: value=get_key_press(&ev);
				not_done=0;
				break;
		case EnterNotify:
			     if(ev.xcrossing.window==c1||
				ev.xcrossing.window==c2	)
			     XSetWindowBorderWidth(display,
				ev.xcrossing.window,2);
			     break;
		case LeaveNotify:
			    if(ev.xcrossing.window==c1||
				ev.xcrossing.window==c2	)
			     XSetWindowBorderWidth(display,
				ev.xcrossing.window,1);

			     break;

				}
	}
	  XSelectInput(display,c1,EV_MASK);
  	 XSelectInput(display,c2,EV_MASK);

	XDestroySubwindows(display,base);
 	XDestroyWindow(display,base);
   	return(value);
   }
	

  yes_no_box()
  {
   char ans;
   ans=(char)two_choice("YES","NO","Are you sure?","yn",0,0,main_win);
   if(ans=='y')return(1);
   return(0);
  }
	
  
/*  new pop_up_list   */

pop_up_list(root,title,list,key,n,max,def,x,y)
int def,n,max,x,y;
 char *title,**list,*key;
Window *root;

 {
  POP_UP p;
  XEvent ev;
	Window w;
	Cursor txt;
        int i,done=0,value;
	int width=DCURX*(max+5);
	int length=(DCURY+6)*(n+2);
	w=make_window(*root,x,y,width,length,2);
	txt=XCreateFontCursor(display,XC_hand2);
 	 XDefineCursor(display,w,txt);
  p.base=w;
  p.entries=list;
  p.title=title;
  p.n=n;
  p.max=max;
  p.key=key;
  p.hot=def;
  value=(int)key[def];
  p.w=(Window *)malloc(n * sizeof(Window));
  p.tit=make_window(w,0,0,width,DCURY+7,0);
  for(i=0;i<n;i++){
	p.w[i]=make_window(w,DCURX,DCURY+10+i*(DCURY+6),DCURX*(max+3),DCURY+3,0);
	 XSelectInput(display,p.w[i],BUT_MASK);
	}

  while(!done)
       {
	XNextEvent(display,&ev);
        switch(ev.type){
			case Expose:
			case MapNotify:
				do_expose(ev);
				draw_pop_up(p,ev.xexpose.window);
				break;
			case KeyPress:
			 value=get_key_press(&ev);
				done=1;
				break;
			case ButtonPress:
			     for(i=0;i<n;i++){
			     if(ev.xbutton.window==p.w[i]){
				value=(int)p.key[i];
				done=1;
				}
			     }	
   			         
			     break;
			case EnterNotify:
			     for(i=0;i<p.n;i++)if(ev.xcrossing.window==p.w[i])
			     XSetWindowBorderWidth(display,
			      p.w[i],1);
			     break;
			case LeaveNotify:
 			for(i=0;i<p.n;i++)if(ev.xcrossing.window==p.w[i])
			     XSetWindowBorderWidth(display,
			      p.w[i],0);
			     break;
			   			
			}
	}

	for(i=0;i<n;i++) XSelectInput(display,p.w[i],EV_MASK);
	 XDestroySubwindows(display,p.base);
	 XDestroyWindow(display,p.base);
        if(value==13)value=(int)key[def];
	return(value);

  }

 draw_pop_up(p,w)
 POP_UP p;
 Window w;
 {
  int i,xi,yi;

  if(w==p.tit){
	set_fore();
	bar(0,0,DCURX*(p.max+5),(DCURY+7),w);
 	set_back();
        ftext(DCURX*2,4,p.title,w);
	set_fore();
	 return;
         }
   for(i=0;i<p.n;i++){
	if(w==p.w[i]){
		      ftext(DCURX/2,3,p.entries[i],w);
		      if(i==p.hot)ftext(DCURX*(p.max+1),4,"X",w);
		      return;
		      }
	 
    }
}

   

		

  
   

/*   Note that this will be improved later -- it is pretty dumb  

pop_up_list(root,title,list,key,n,max,def,x,y)
int def,n,max,x,y;
 char *title,**list,*key;
Window *root;
{
	Window w;
	Cursor txt;
	XEvent ev;
	int width=DCURX*(max+4);
	int not_done=1;
	int value=-1;
	int com;
	int length=(DCURY+6)*(n+2);
	w=make_window(*root,x,y,width,length,2);
 	txt=XCreateFontCursor(display,XC_hand2);
 	 XDefineCursor(display,w,txt);

        draw_pop_list(w,title,list,n,width,def);
        while(not_done)
        {
	 XNextEvent(display,&ev);
	 switch(ev.type) {
		case ConfigureNotify:
		case Expose:
		case MapNotify:
		do_expose(ev);
	 	if(ev.xany.window==w)draw_pop_list(w,title,list,n,width,def);
		break;
		case KeyPress:
		           value=get_key_press(&ev);
			   not_done=0;
			   break;
		case ButtonPress:
			   if(ev.xbutton.window==w){
				com=ev.xbutton.y/(DCURY+8)-1;
				if(com>-1&&com<n){
					value=(int)key[com];
					not_done=0;
				
					}
			   }
				break;
			}
         }
         if(value==13&&def>=0)value=key[def];
        XDestroyWindow(display,w);
	*root=w;
	 return(value);
     }	
	
	
   draw_pop_list(w,title,list,n,max,def)
   Window w;
   char **list,*title;
   int n,max;
   int def;
   {
   
	int i,xi,yi;
 	xi=2*DCURX;
        yi=4;
        set_fore();
	bar(0,0,max,(DCURY+7),w);
 	set_back();
        ftext(xi,yi,title,w);
	set_fore();
	yi+=(DCURY+8);
	for(i=0;i<n;i++)
	{
	ftext(xi,yi,list[i],w);
	yi+=(DCURY+8);
	}
        if(def<0)return;
	xi=DCURX;
	yi=(DCURY+8)*(def+1);
	rectangle(xi,yi+5,xi+max-2*DCURX,yi+DCURY+5,w);
   } 	 


   */ 


   
  
