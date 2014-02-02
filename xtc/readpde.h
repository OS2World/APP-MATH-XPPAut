#define NCMD 17

#define TIME 0
#define SPACE 1
#define PAR 2
#define CPAR 3
#define WEIGHT 4
#define LOAD 5
#define SET 6
#define VAR 7
#define CVAR 8
#define FUN 9
#define FIX 10
#define CFIX 11
#define BDRY 12
#define DONE 13
#define CAUX 14
#define AUX 15
#define COMMENT 16    /*   always last    */
 

#define NKER 5 

#define NONE 0
#define NORMAL 1
#define PERIODIZE 2
#define PERNORM 3
#define READFILE 4
 
#define NBCS 5

#define PERIODIC 0
#define NOFLUX 2
#define ZERO 1
#define LEAKY 3
#define DYNAMIC 4
 
#define NSET 23

#define NPERIOD 0
#define GRID 1
#define LENGTH 2
#define FAST 3
#define METHOD 4
#define DELTA_T 5
#define TFINAL 6
#define TRANS 7
#define NOUT 8
#define PLOTVAR 9 
#define EXTEND 10
#define BUFSIZE 11
#define RGB 12
#define DTMIN 13
#define DTMAX 14
#define TOLERANCE 15
#define EPSILON 16
#define JACUSE 17
#define MAXITER 18 
#define TSTART 19
#define MAXDERIV 20
#define BOUNDSET 21
#define SEED 22

typedef struct {
  char name[10];
  int len;
 } VOCAB;

VOCAB my_cmd[NCMD]=
{
 "TIME",4,     
 "SPACE",5,
 "PAR",3,
 "CPAR",4,
 "WEIGHT",6,
 "LOAD",4,
 "SET",3,
 "VAR",3,
 "CVAR",4,
 "FUN",3,
 "FIX",3,
 "CFIX",4,
 "BDRY",4,
 "DONE",4,
 "CAUX",4,
 "AUX",3,
 "#",1
 };



 VOCAB my_ker[NKER]=
{
 "NONE",4,
 "NORMAL",5,
 "PERIODIZE",9,
 "PERNORM",7,
 "READFILE",8
 };


 VOCAB my_bcs[NBCS]=
{
 "PERIODIC",8,
 "ZERO",4,
 "NOFLUX",6,
 "LEAKY",5,
 "DYNAMIC",7
 };
 


VOCAB my_set[NSET]=
{
"NPERIOD",7,
  "GRID",4,
 "LENGTH",6,
 "FAST",4,
 "METHOD",6,
 "DELTA_T",7,
 "TFINAL",6,
 "TRANS",5,
 "NOUT",4,
 "PLOTVAR",7,
 "EXTEND",6,
 "BUFSIZE",7,
 "RGB",3,
 "DTMIN",5,
 "DTMAX",5,
 "TOLERANCE",9,
 "EPSILON",7,
 "JACUSE",6,
 "MAXITER",7,
 "TSTART",6,
 "MAXDERIV",8,
 "BOUND",5,
 "SEED",4,
};
 
 
















