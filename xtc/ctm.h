#define MAXCTM 60 /* total number of continuous things  */
#define MAXFORMLEN 80

/*  Convolution and evaluation extensions   */

#define PERIODIC 0
#define ZERO 1
#define ODD 3
#define EVEN 2

/* boundary conditions   -- periodic,zero as above  */

#define NOFLUX 2
#define DYNAMIC 4
#define LEAKY 3

/* creation of arrays     */

#define NORMALIZE 1
#define PERIODIZE 2
#define PERNORM 3
#define NORM2D   4      
#define READFILE 5

typedef struct {
	double *u;
	int length,dim,type,special,bv;
	char *formula;
}CONTINUUM;

typedef struct {
  int bcl,bcr,parent,ix,ixx;
  char *sbc[6]; /* al,bl,cl,ar,br,cr  -- al*ux(0)+bl*u(0)=cl  */
  int *ibc[6]; /* formulae for bvp   */
}BDRYVAL;











