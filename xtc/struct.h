
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


typedef struct {
  Window *w;
  char **names;
  char *key;
  int n;
  Window home;
}BUTLIST;



