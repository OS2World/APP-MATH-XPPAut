/* command-line stuff for xpp */
#include <stdio.h>
#define NCMD 4  /* add new commands as needed  */

#define MAKEC 0
#define XORFX 1
#define SILENT 2  /* not working yet.... */
#define CONVERT 3
extern int got_file;
extern char this_file[100];
extern int XPPBatch;
extern int xorfix;
extern int silent;
extern int ConvertStyle;
typedef struct {
  char name[10];
  int len;

} VOCAB;

VOCAB my_cmd[NCMD]=
{
  "-m",2,
  "-xorfix",7,
  "-silent",7,
  "-convert",8,
 };

do_comline(argc,argv)
char **argv;
int argc;
{
 int i;
 for(i=1;i<argc;i++){
   parse_it(argv[i]);
 }
}


parse_it(com)
     char *com;
{
  int j;
  for(j=0;j<NCMD;j++)
    if(strncmp(com,my_cmd[j].name,my_cmd[j].len)==0)break;
  if(j<NCMD){
    switch(j){
    case MAKEC:
      printf(" C files are no longer part of this version. \n Sorry \n");
      break;
    case SILENT:
      XPPBatch=1;
      break;
    case XORFX:
      xorfix=1;
      break;
    case CONVERT:
      ConvertStyle=1;
    }
  }
  else {
    if(com[0]=='-'||got_file==1){
      printf(" xppaut filename -silent -xorfix -convert\n");
      exit(0);
    }
    else {
      strcpy(this_file,com);
      got_file=1;
    }
  }
}







