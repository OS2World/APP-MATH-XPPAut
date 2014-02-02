#define LPAREN 1
#define COMMENT 2
#define SPACE 3
#define EQUAL 4

#define COMMAND -1
#define FIXED 0
#define FUNCTION 1
#define IC 2
#define MAP 3
#define ODE 4
#define VEQ 5
#define MARKOV_VAR 6
#define AUX_VAR 7
#define TABLE 8

#define NAMLEN 10
#define MAXARG 10

typedef struct var_info {
  char lhs[256],rhs[256],args[MAXARG][NAMLEN+1];
  int type,nargs;
  double value;
  struct var_info *next,*prev;
} VAR_INFO;

int start_var_info=0;
VAR_INFO *my_varinfo;










