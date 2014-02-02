#include <math.h>
#include <stdio.h>
/* Hard Copy file production  this is for postscript and HPGL   */

typedef struct {
  float xmin,xmax,ymin,ymax;
  float xscale,yscale,xoff,yoff;
  float tx,ty,angle,slant;  /* text attributes   */
  float linecol;
  int linewid;
  } DEVSCALE;

DEVSCALE ps_scale;
DEVSCALE hpgl_scale;

extern FILE *my_plot_file;  /* use the same file descriptor for both  */


/* PostScript device driver   */


ps_init(xlo,ylo,xhi,yhi)
     float xlo,ylo,xhi,yhi;
{
  ps_begin(xlo,ylo,xhi,yhi,6.0,6.0);
}

ps_begin(xlo,ylo,xhi,yhi,sx,sy)
     double xlo,ylo,xhi,yhi;
     float sx,sy;
{
  ps_scale.xmin=xlo;
  ps_scale.ymin=ylo;
  ps_scale.ymax=yhi;
  ps_scale.xmax=xhi;
  ps_scale.xoff=300;
  ps_scale.yoff=300;
  ps_scale.xscale=1800.*sx*.2/(xhi-xlo);
  ps_scale.yscale=1800.*sy*.2/(yhi-ylo);
  ps_set_text(0.0,0.0,18.0,18.0);
  
  fprintf(my_plot_file,"%s\n","%!");
  fprintf(my_plot_file,"20 dict begin\n");
  fprintf(my_plot_file,"gsave\n");
  fprintf(my_plot_file,"/m {moveto} def\n");
  fprintf(my_plot_file,"/l {lineto} def\n");
  fprintf(my_plot_file,"/C {setrgbcolor} def\n");
  fprintf(my_plot_file,"/G {setgray} def\n");
  fprintf(my_plot_file,"/S {stroke} def\n");
  fprintf(my_plot_file,"/HSB {sethsbcolor} def\n");
  fprintf(my_plot_file,"/RGB {setrgbcolor} def\n");
  fprintf(my_plot_file,"/FS {fill stroke} def\n");
   fprintf(my_plot_file,"540 82 translate\n");
  fprintf(my_plot_file,"90 rotate\n");
  fprintf(my_plot_file,".2 .2 scale\n");
  fprintf(my_plot_file,"/basefont /Times-Roman findfont def\n");
  ps_setline(0.0,4);
}

ps_convert(x,y,xs,ys)
     float x,y, *xs,*ys;
{
  *xs=(x-ps_scale.xmin)*ps_scale.xscale+ps_scale.xoff;
  *ys=(y-ps_scale.ymin)*ps_scale.yscale+ps_scale.yoff;
}

ps_boxit()
{
  int i=ps_scale.linewid;
  float z=ps_scale.linecol;
  ps_setline(0.0,10);
  ps_rect(ps_scale.xmin,ps_scale.ymin,
	  ps_scale.xmax-ps_scale.xmin,ps_scale.ymax-ps_scale.ymin);
  ps_setline(z,i);
}

ps_close()
 {
  fprintf(my_plot_file,"showpage\n");
  fprintf(my_plot_file,"grestore\n");
  fprintf(my_plot_file,"end\n");
  fclose(my_plot_file);
}

ps_setline(fill,thick)
     float fill;
     int thick;
{
  fprintf(my_plot_file,"%f G\n %d setlinewidth \n",fill,thick);
  ps_scale.linewid=thick;
  ps_scale.linecol=fill;
}
 
 ps_put_char( ch,x,y)
char ch;
float *x, *y;
 {
  float xp=*x,yp=*y;
  char str[4];
  str[0]=ch;
  str[1]='\0';
  ps_text(str,xp,yp,0);
 }




ps_text(str,xr,yr,icent)
     char *str;
     float xr,yr;
     int icent;  /* ignores for now  */
{
  double slant=.0174532*ps_scale.slant;
  float x,y;
  float sizex=ps_scale.tx,sizey=ps_scale.ty,rot=ps_scale.angle;
  double a=sizex*cos(slant),b=sizey*sin(slant),
  c=-sizex*sin(slant),d=sizey*cos(slant);
  ps_convert(xr,yr,&x,&y);
  fprintf(my_plot_file,"%d %d m\n",(int)x,(int)y);
  fprintf(my_plot_file,"gsave \n %f rotate \n",rot);
  fprintf(my_plot_file,"basefont [%.4f %.4f %.4f %.4f 0 0] makefont setfont\n"
	,a,b,c,d);
  fprintf(my_plot_file,"( %s ) show \n grestore\n",str);
}

ps_line(x1r,y1r,x2r,y2r)
     float x1r,y1r,x2r,y2r;
{
  float x1,y1,x2,y2;
  ps_convert(x1r,y1r,&x1,&y1);
  ps_convert(x2r,y2r,&x2,&y2);
  fprintf(my_plot_file,"%d %d m \n %d %d l S\n",
	  (int)x1,(int)y1,(int)x2,(int)y2);
}

ps_set_text(angle,slant,x_size,y_size)  
     float angle,slant,x_size,y_size;
{
 ps_scale.tx=x_size*5.0;
 ps_scale.ty=y_size*5.0;
 ps_scale.angle=angle;
 ps_scale.slant=slant;
}

ps_rect(x,y,wid,len)  
     float x,y,wid,len;
{
 float x1,y1,x2,y2;
 ps_convert(x,y,&x1,&y1);
 ps_convert(x+wid,y+len,&x2,&y2);
 fprintf(my_plot_file,"%d %d m \n %d %d l \n %d %d l \n %d %d l \n %d %d l \n S \n",
	 (int)x1,(int)y1,(int)x2,(int)y1,(int)x2,
	 (int)y2,(int)x1,(int)y2,(int)x1,(int)y1);
}

ps_bar(x,y,wid,len,fill,flag)
     float x,y,wid,len,fill;
     int flag;
{
    float x1,y1,x2,y2;
   fprintf(my_plot_file,"%f G\n",fill);
    ps_convert(x,y,&x1,&y1);
    ps_convert(x+wid,y+len,&x2,&y2);
   fprintf(my_plot_file,"%d %d m \n %d %d l \n %d %d l \n %d %d l \n FS\n",
	   (int)x1,(int)y1,(int)x2,(int)y1,(int)x2,(int)y2,(int)x1,(int)y2);
  
  if(flag){
    fprintf(my_plot_file,"0 G\n");
    ps_rect(x,y,wid,len);
  }

 }

ps_rgb_bar(x,y,wid,len,fill,flag,rgb)
     float x,y,wid,len,fill;
     int flag,rgb;
{
    float x1,y1,x2,y2;
    float r,g=0.0,b;
    if(rgb==2){
      ps_hsb_bar(x,y,wid,len,fill,flag);
      return;
    }
    if(fill<0.0)fill=0.0;
    if(fill>1.0)fill=1.0;
    switch(rgb)
      {
      case 0:b=(float)sqrt((double)(1.0-fill*fill));
      r=(float)sqrt((double)(fill*(2.0-fill)));
	break;
      case 1:
       if(fill>.4999)r=0.0;
	else r=(float)sqrt((float)(1.-4*fill*fill));
	g=(float)2*sqrt((double)fill*(1.-fill));
	
	if(fill<.5001)b=0.0;
	else b=(float)sqrt((float)(4*(fill-.5)*(1.5-fill)));
	break;
      }
   fprintf(my_plot_file,"%f %f %f RGB\n",r,g,b);
    ps_convert(x,y,&x1,&y1);
    ps_convert(x+wid,y+len,&x2,&y2);
/*   fprintf(my_plot_file,"%f %f m \n %f %f l \n %f %f l \n %f %f l \n FS\n",
	   x1,y1,x2,y1,x2,y2,x1,y2); */
  fprintf(my_plot_file,"%d %d m \n %d %d l \n %d %d l \n %d %d l \n FS\n",
	   (int)x1,(int)y1,(int)x2,(int)y1,(int)x2,(int)y2,(int)x1,(int)y2);
  if(flag){
    fprintf(my_plot_file,"0 G\n");
    ps_rect(x,y,wid,len);
  }

 }

ps_hsb_bar(x,y,wid,len,fill,flag)
     float x,y,wid,len,fill;
     int flag;
{
    float x1,y1,x2,y2;
   fprintf(my_plot_file,"%f 1.0 1.0 HSB\n",fill);
    ps_convert(x,y,&x1,&y1);
    ps_convert(x+wid,y+len,&x2,&y2);
  /* fprintf(my_plot_file,"%f %f m \n %f %f l \n %f %f l \n %f %f l \n FS\n",
	   x1,y1,x2,y1,x2,y2,x1,y2); */
  fprintf(my_plot_file,"%d %d m \n %d %d l \n %d %d l \n %d %d l \n FS\n",
	   (int)x1,(int)y1,(int)x2,(int)y1,(int)x2,(int)y2,(int)x1,(int)y2);
  if(flag){
    fprintf(my_plot_file,"0 G\n");
    ps_rect(x,y,wid,len);
  }

 }







