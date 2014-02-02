/* Modified by Klaus Gebhardt, 1997 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <math.h>
#include "struct.h"
#include "newhome.h"
#define RIGHT 6
#define LEFT 2
#define HOME 1
#define END 5
#define DEL 9 
#define BKSP 8
#define FINE 13
#define ESC 27
#define TAB 10
#define BAD 0


int do_calc();
int new_float();
int new_int();

double atof();
extern int SCALEX,SCALEY;
int done=0,bell_flag=1;
extern Display *display;
extern int screen;
int CURS_X,CURS_Y;
extern int DCURY,DCURX,CURY_OFF;
extern Window win,command_pop,info_pop,menu_pop,draw_win,main_win;
extern GC gc,gc_graph;
extern unsigned long MyBackColor,MyForeColor;
int xor_flag;

ping() 
{
  if(bell_flag)XBell(display,100);
}
blank_screen(w)
Window w;

{

 CURS_X=0;
 CURS_Y=0;
 xor_flag=0;
 XClearWindow(display,w);
}

 set_fore()
 {
  XSetForeground(display,gc,MyForeColor);
 }

 set_back()
 {
  XSetForeground(display,gc,MyBackColor);
 }

showchar(ch,col,row,or)
int col,row;
Window or;
char ch;
{
 char bob[2];
 bob[0]=ch;
 chk_xor();
 XDrawString(display,or,gc,col,row+CURY_OFF,bob,1);

}


chk_xor()
 {
  if(xor_flag==1)
  XSetFunction(display,gc,GXxor);
   else
    XSetFunction(display,gc,GXcopy);
 }











  
 set_gcurs( y,  x)
int y,x;
{
 CURS_X=x;
 CURS_Y=y;
}

clr_command()
{
 blank_screen(command_pop);
}


gputs(string,win)
char *string;
Window win;
{
int xloc=CURS_X*DCURX,yloc=CURS_Y*DCURY;
 ftext( xloc, yloc, string,win );
 CURS_X+=strlen(string);

}


err_msg(string)
char *string;
{
 respond_box(main_win,0,0,"OK",&string,1);
}




put_command(string)
char *string;
{
 clr_command();
 ftext(0,0,string,command_pop);
 CURS_X=strlen(string);
}


    get_key_press(ev)
    XKeyEvent *ev;
    {
     int maxlen=64;
      char buf[65];
      XComposeStatus comp;
       KeySym ks;
     
    
       XLookupString(ev,buf,maxlen,&ks,&comp);
  /*      printf("h=%d e=%d ks=%d \n",XK_Home,XK_End,ks); */
	if(ks==XK_Escape)return(ESC);
	if((ks==XK_Return)||(ks==XK_KP_Enter)||
 		(ks==XK_Linefeed))return(FINE);
	else if	(((ks>=XK_KP_Space)&&(ks<=XK_KP_9))
		|| ((ks>=XK_space)&&(ks<=XK_asciitilde)))
		return((int)buf[0]);
     /*   else if ((ks>=XK_Shift_L)&&(ks<=XK_Hyper_R)) return(0);
	else if ((ks>=XK_F1)&&(ks<=XK_F35))  return(0); */

	else if (ks==XK_BackSpace) return(BKSP);
        else if (ks==XK_Delete) return(DEL);
	else if (ks==XK_Tab) return(TAB);
        else if (ks==XK_Home)return(HOME);
        else if (ks==XK_End)return(END);
        else if (ks==XK_Left)return(LEFT);
        else if (ks==XK_Right)return(RIGHT);
        else {
        
         return(BAD);
         }
    }

 


 get_mouse_xy(x,y,w)
 Window w;
 int *x,*y;
 {
  int no_but=1;
  XEvent ev;
  while(no_but)
  {
   XNextEvent(display,&ev);
   switch(ev.type)
   {
    case Expose: do_expose(ev);
		 break;
		 
    case ButtonPress:
          if(ev.xbutton.window!=w)return(0);
          no_but=0;
          *x=ev.xbutton.x;
          *y=ev.xbutton.y;
          return(1);
          
    }
   }
	return(0);
 }
               
     

ftext(x,y,string,o)
int x,y;
 Window o;
char *string;
{
   chk_xor();
   XDrawString(display,o,gc,x,y+CURY_OFF,string,strlen(string));
}
 




bar(x,y,x2,y2,w)
int x,y,x2,y2;
Window w;
{
 XFillRectangle(display,w,gc,x,y,x2-x,y2-y);
}






new_float(name,value)
char *name;
double *value;
 {  int temp;
    int flag;
    int done;
    double newz;
   char tvalue[200];
   sprintf(tvalue,"%.16g",*value);
    done=new_string(name,tvalue);
	
  if(done==0||strlen(tvalue)==0)return(0);

    /*
    if(tvalue[0]=='%')
    {
     flag=do_calc(&tvalue[1],&newz);
     if(flag!=-1)*value=newz;
     return(0);
    }
   */
     *value=atof(tvalue);
 
   return(done);

 }
 

do_calc(s,v)
char *s;
double *v;
{
 return(1);
}
 


 new_int(name,value)
 char *name;
 int *value;
 {
   char svalue[200];
   int done;
    
   sprintf(svalue,"%d",*value);
  done=new_string(name,svalue); 
  if(done==0||strlen(svalue)==0)return(0);
   *value=atoi(svalue);
   return(done);
 }
  
   


odisplay_command(name,value)
char *name,*value;
{
 int l=strlen(name);
 int m=strlen(value);

 set_fore();
 bar(0,0,l*DCURX,DCURY+4,command_pop);
 set_back();
 XDrawString(display,command_pop,gc,0,CURY_OFF,name,l);
 set_fore();
 if(m>0){
 XDrawString(display,command_pop,gc,l*DCURX,CURY_OFF,value,m);
 showchar('_',DCURX*(l+m),0,command_pop);
 }
}

display_command(name,value,pos,col)
char *name,*value;
int pos,col;
{
 int l=strlen(name);
 int m=strlen(value);

 set_fore();
 bar(0,0,l*DCURX,DCURY+4,command_pop);
 set_back();
 XDrawString(display,command_pop,gc,0,CURY_OFF,name,l);
 set_fore();
 if(m>0){
 XDrawString(display,command_pop,gc,l*DCURX,CURY_OFF,value,m);
 /* showchar('_',DCURX*(l+m),0,command_pop); */
 put_cursor_at(command_pop,DCURX*l,pos);
 }
}


 oedit_window(w,pos,value,col,done,ch)
 Window w;
 int *pos,*col,*done;
 char *value;
 int ch;
 {
	*done=0;
 	 switch(ch){
			case 0: break;     /* junk key  */
			case 27: *done=-1;  /* quit without saving */
				 break;
			case 13: *done=1;
				 break;   /* save this guy */
			case 9:  while(*pos>0)do_backspace(pos,value,col,w);
				 break;
 			case 8:  if(*pos>0)do_backspace(pos,value,col,w);
				else ping();
				break;
			case 10: *done=2; break;
			default:
				if(*col<(SCALEX-DCURX)){
				set_back();
				showchar('_',*col,0,w);
				set_fore();
				showchar(ch,*col,0,w);
				*col+=DCURX;
				showchar('_',*col,0,w);
                                value[*pos]=ch;
                                *pos=*pos+1;
				value[*pos]='\0';
				}
				else
				ping();
				break;
			} /* end key cases */
}

 do_backspace(pos,value,col,w)
 int *pos,*col;
 char *value;
 Window w;
 {
	char oldch;
 				*pos=*pos-1;
				oldch=value[*pos];
				value[*pos]='\0';
				if(*col<(SCALEX-DCURX))
				set_back();
				showchar('_',*col,0,w);
				*col=*col-DCURX;
				showchar(oldch,*col,0,w);
				set_fore();
				showchar('_',*col,0,w);
}

 edit_command_string(ev,name,value,done,pos,col)
 XEvent ev;
 char *name,*value;
 int *done,*pos,*col;
{
  char ch,oldch;
  switch(ev.type){
 		case ConfigureNotify:
		case Expose:
 		case MapNotify:
			do_expose(ev);
			if(ev.xexpose.window==command_pop)
  			display_command(name,value,*pos,*col);
 			break;
		case ButtonPress:
 			if(ev.xbutton.window==command_pop)
 	  		XSetInputFocus(display,command_pop,
			  RevertToParent,CurrentTime);
			break;
 		case KeyPress:
      		 ch=get_key_press(&ev);
		 edit_window(command_pop,pos,value,col,done,ch);

		

		} /* end event cases */
  }


 new_string(name,value)
char *name;
char *value;
{
 char old_value[80];
 char ch,oldch;
 int done=0;
 int pos=strlen(value);
 int col=(pos+strlen(name))*DCURX;
 
 XEvent ev;
 strcpy(old_value,value);
 clr_command();
 display_command(name,value,pos,col);
 while(done==0){
	XNextEvent(display,&ev);
	edit_command_string(ev,name,value,&done,&pos,&col);
 	} 
	clr_command();
	if(done==1)return(1);
	if(done==2)return(2);
	strcpy(value,old_value);
	return(0);
}

       
      
/*************   New Edit window    **********************
*  
*      Based on the very simplest of line editors. Can insert in middle
*      with this !!
**********************************************************/

 edit_window(w,pos,value,col,done,ch)
 Window w;
 int *pos,*col,*done;
 char *value;
 int ch;
 {
   int col0=*col-*pos*DCURX;
        
   *done=0;
  /*  printf(" po=%d cl=%d ch=%d ||%s|| c0=%d\n",*pos,*col,ch,value,col0); */
   switch(ch){
   case LEFT: 
     if(*pos>0){ *pos=*pos-1; *col-=DCURX;}
     else ping();
     break;
   case RIGHT: 
     if(*pos<strlen(value)){ *pos=*pos+1; *col+=DCURX;}
     else ping();
     break;
   case HOME: { *pos=0; *col=col0;}
     break;
   case END:  { *pos=strlen(value); *col=*pos*DCURX+col0;}
     break;
   case BAD: return;    /* junk key  */
   case ESC: *done=-1;  /* quit without saving */
     return;
   case FINE: *done=1;
     return;   /* save this guy */
   case BKSP:  
   case DEL:	
     if(*pos>0){
       memmov(&value[*pos-1],&value[*pos],strlen(value)-*pos+1);
       *pos=*pos-1;
       *col-=DCURX;
     }
     else ping();
     break;
   case TAB: *done=2; return;
   default:
     if( (ch>=' ') && (ch <= '~')){
       movmem(&value[*pos+1],&value[*pos],strlen(value)-*pos+1);
       value[*pos]=ch;
       *pos=*pos+1;
       *col+=DCURX;
     }
     break;
   } /* end key cases */
   /* Now redraw everything !!  */
   clr_line_at(w,col0,0,strlen(value));
   put_string_at(w,col0,value,0);
   put_cursor_at(w,col0,*pos);
/*   printf(" on ret %d %d %d %s %d\n",*pos,*col,ch,value,col0);*/
   
   XFlush(display);
   
 }

clr_line_at(w,col0,pos,n)
     Window w;
     int col0,pos,n;
{
  XClearArea(display,w,col0+pos*DCURX,0,(n+2)*DCURX,2*DCURY,False);
}

put_cursor_at(w,col0,pos)
     Window w;
     int pos,col0;
{
  int x1=col0+pos*DCURX;
  int x2=x1+1;
    int y1=DCURY-2,y2=2;
  /* XDrawString(display,w,gc,col0+pos*DCURX-1,DCURY,"^",1);*/
   XDrawLine(display,w,gc,x1,y1,x1,y2);
  XDrawLine(display,w,gc,x2,y1,x2,y2);
 }

put_string_at(w,col,s,off)
     Window w;
     char *s;
     int off,col;
{
 int l=strlen(s)-off;
 
 XDrawString(display,w,gc,col,CURY_OFF,s+off,l);
}
              
movmem(s1,s2,len)
     char *s1,*s2;
     int len;
{
  int i;
  for(i=len-1;i>=0;i--)
    s1[i]=s2[i];
}

memmov(s1,s2,len)
     char *s1,*s2;
     int len;
{
  int i;
  for(i=0;i<len;i++)
    s1[i]=s2[i];
}














