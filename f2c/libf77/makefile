.SUFFIXES: .c .obj
CC = gcc
OFLAGS = -m486 -O3 -malign-loops=2 -malign-jumps=2 -malign-functions=2 
CFLAGS = -mprobe -D__ST_MT_ERRNO__

%.obj: %.c; $(CC) $(OFLAGS) $(CFLAGS) -c $< -o $@ -Zomf
%.o: %.c; $(CC) $(OFLAGS) $(CFLAGS) -c $< -o $@

# compile, then strip unnecessary symbols
#	$(CC) -c -DSkip_f2c_Undefs $(CFLAGS) $*.c
#	ld -r -x -o $*.xxx $*.obj
#	mv $*.xxx $*.obj

MAIN =  main.obj getarg_.obj iargc_.obj
MAINO = main.o getarg_.o iargc_.o

MISC =	Version.obj s_rnge.obj abort_.obj \
	getenv_.obj signal_.obj s_stop.obj s_paus.obj system_.obj cabs.obj \
	sig_die.obj
POW =	pow_ci.obj pow_dd.obj pow_di.obj pow_hh.obj pow_ii.obj pow_ri.obj \
	pow_zi.obj pow_zz.obj
CX =	c_abs.obj c_cos.obj c_div.obj c_exp.obj c_log.obj c_sin.obj c_sqrt.obj
DCX =	z_abs.obj z_cos.obj z_div.obj z_exp.obj z_log.obj z_sin.obj z_sqrt.obj
REAL =	r_abs.obj r_acos.obj r_asin.obj r_atan.obj r_atn2.obj r_cnjg.obj \
	r_cos.obj r_cosh.obj r_dim.obj r_exp.obj r_imag.obj r_int.obj\
	r_lg10.obj r_log.obj r_mod.obj r_nint.obj r_sign.obj\
	r_sin.obj r_sinh.obj r_sqrt.obj r_tan.obj r_tanh.obj
DBL =	d_abs.obj d_acos.obj d_asin.obj d_atan.obj d_atn2.obj\
	d_cnjg.obj d_cos.obj d_cosh.obj d_dim.obj d_exp.obj\
	d_imag.obj d_int.obj d_lg10.obj d_log.obj d_mod.obj\
	d_nint.obj d_prod.obj d_sign.obj d_sin.obj d_sinh.obj\
	d_sqrt.obj d_tan.obj d_tanh.obj
INT =	i_abs.obj i_dim.obj i_dnnt.obj i_indx.obj i_len.obj i_mod.obj \
	i_nint.obj i_sign.obj
HALF =	h_abs.obj h_dim.obj h_dnnt.obj h_indx.obj h_len.obj h_mod.obj \
	h_nint.obj h_sign.obj
CMP =	l_ge.obj l_gt.obj l_le.obj l_lt.obj hl_ge.obj hl_gt.obj hl_le.obj \
	hl_lt.obj
EFL =	ef1asc_.obj ef1cmc_.obj
CHAR =	s_cat.obj s_cmp.obj s_copy.obj

OBJECTS = $(MISC) $(POW) $(CX) $(DCX) $(REAL) $(DBL) $(INT) $(HALF) $(CMP) \
	$(EFL) $(CHAR)

all: F77.lib $(MAIN) $(MAINO)

F77.lib: $(OBJECTS) 
	emxomfar r $@ $(OBJECTS)

Version.obj: Version.c
	$(CC) $(CFLAGS) -c Version.c -Zomf

# If your system lacks onexit() and you are not using an
# ANSI C compiler, then you should uncomment the following
# two lines (for compiling main.obj):
#main.obj: main.c
#	$(CC) -c -DNO_ONEXIT -DSkip_f2c_Undefs main.c
# On at least some Sun systems, it is more appropriate to
# uncomment the following two lines:
#main.obj: main.c
#	$(CC) -c -Donexit=on_exit -DSkip_f2c_Undefs main.c


clean:
	rm -f F77.a F77.lib *.obj
