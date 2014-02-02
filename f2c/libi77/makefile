.SUFFIXES: .c .obj
CC = gcc
OFLAGS = -m486 -O3 -malign-loops=2 -malign-jumps=2 -malign-functions=2 
CFLAGS = -mprobe -DNON_UNIX_STDIO -D__ST_MT_ERRNO__

all: I77.lib

%.obj: %.c; $(CC) $(OFLAGS) $(CFLAGS) -c $< -o $@ -Zomf
%.o: %.c; $(CC) $(OFLAGS) $(CFLAGS) -c $< -o $@

# compile, then strip unnecessary symbols
#	$(CC) -c -DSkip_f2c_Undefs $(CFLAGS) $*.c
#	ld -r -x -o $*.xxx $*.o
#	mv $*.xxx $*.o

OBJ =	Version.obj backspace.obj close.obj dfe.obj dolio.obj due.obj \
	endfile.obj err.obj fmt.obj fmtlib.obj iio.obj ilnw.obj inquire.obj \
	lread.obj lwrite.obj open.obj rdfmt.obj rewind.obj rsfe.obj rsli.obj \
	rsne.obj sfe.obj sue.obj typesize.obj uio.obj util.obj wref.obj \
	wrtfmt.obj wsfe.obj wsle.obj wsne.obj xwsne.obj

I77.lib: $(OBJ) 
	emxomfar r $@ $(OBJ)

Version.obj: Version.c
	$(CC) $(CFLAGS) -c Version.c -Zomf

clean:
	rm -f $(OBJ) I77.a I77.lib


backspace.obj:	fio.h
close.obj:	fio.h
dfe.obj:	fio.h
dfe.obj:	fmt.h
due.obj:	fio.h
endfile.obj:	fio.h rawio.h
err.obj:	fio.h rawio.h
fmt.obj:	fio.h
fmt.obj:	fmt.h
iio.obj:	fio.h
iio.obj:	fmt.h
ilnw.obj:	fio.h
ilnw.obj:	lio.h
inquire.obj:	fio.h
lread.obj:	fio.h
lread.obj:	fmt.h
lread.obj:	lio.h
lread.obj:	fp.h
lwrite.obj:	fio.h
lwrite.obj:	fmt.h
lwrite.obj:	lio.h
open.obj:	fio.h rawio.h
rdfmt.obj:	fio.h
rdfmt.obj:	fmt.h
rdfmt.obj:	fp.h
rewind.obj:	fio.h
rsfe.obj:	fio.h
rsfe.obj:	fmt.h
rsli.obj:	fio.h
rsli.obj:	lio.h
rsne.obj:	fio.h
rsne.obj:	lio.h
sfe.obj:	fio.h
sue.obj:	fio.h
uio.obj:	fio.h
util.obj:	fio.h
wref.obj:	fio.h
wref.obj:	fmt.h
wref.obj:	fp.h
wrtfmt.obj:	fio.h
wrtfmt.obj:	fmt.h
wsfe.obj:	fio.h
wsfe.obj:	fmt.h
wsle.obj:	fio.h
wsle.obj:	fmt.h
wsle.obj:	lio.h
wsne.obj:	fio.h
wsne.obj:	lio.h
xwsne.obj:	fio.h
xwsne.obj:	lio.h
xwsne.obj:	fmt.h
