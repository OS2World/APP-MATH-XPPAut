#	Makefile for f2c, a Fortran 77 to C converter
#          (portiert auf OS/2, Klaus Gebhardt)

CC=gcc
# CFLAGS = -O -m486
CFLAGS = -g 

%.o: %.c; $(CC) $(CFLAGS) -c $< -o $@

OBJECTSd = main.o init.o gram.o lex.o proc.o equiv.o data.o \
	format.o expr.o exec.o intr.o io.o misc.o error.o \
	mem.o names.o output.o p1output.o pread.o put.o \
	putpcc.o vax.o formatdata.o parse_args.o niceprintf.o \
	cds.o sysdep.o version.o
OBJECTS = $(OBJECTSd) # malloc.o

all: f2c.exe f2c.1

f2c.exe : $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o f2c.exe

gram.c:	gram.head gram.dcl gram.expr gram.exec gram.io defs.h tokdefs.h
	sed <tokdefs.h "s/#define/%token/" >gram.in
	cat gram.head gram.dcl gram.expr gram.exec gram.io >>gram.in
	@echo YACC: 4 shift/reduce expected...
	$(YACC) $(YFLAGS) gram.in
	sed "s/^# line.*/\\/* & *\\//" y.tab.c >gram.c
	rm -f gram.in y.tab.c

$(OBJECTSd): defs.h ftypes.h defines.h machdefs.h sysdep.h

tokdefs.h: tokens
	grep -n . <tokens | sed -f tokdefs.sed >tokdefs.h

cds.o: sysdep.h
exec.o: p1defs.h names.h
expr.o: output.h niceprintf.h names.h
format.o: p1defs.h format.h output.h niceprintf.h names.h iob.h
formatdata.o: format.h output.h niceprintf.h names.h
gram.o: p1defs.h
init.o: output.h niceprintf.h iob.h
intr.o: names.h
io.o: names.h iob.h
lex.o : tokdefs.h p1defs.h
main.o: parse.h usignal.h
mem.o: iob.h
names.o: iob.h names.h output.h niceprintf.h
niceprintf.o: defs.h names.h output.h niceprintf.h
output.o: output.h niceprintf.h names.h
p1output.o: p1defs.h output.h niceprintf.h names.h
parse_args.o: parse.h
proc.o: tokdefs.h names.h niceprintf.h output.h p1defs.h
put.o: names.h pccdefs.h p1defs.h
putpcc.o: names.h
vax.o: defs.h output.h pccdefs.h
output.h: niceprintf.h

put.o putpcc.o: pccdefs.h

f2c.t: f2c.1t
	troff -man f2c.1t >f2c.t

f2c.1: f2c.1t
	nroff -man f2c.1t 2>NUL >f2c.1

clean:
	rm -f gram.c *.o f2c.exe tokdefs.h f2c.t f2c.1

b = Notice README cds.c data.c defines.h defs.h equiv.c error.c \
	exec.c expr.c f2c.h format.c format.h formatdata.c \
	ftypes.h gram.dcl gram.exec gram.expr gram.head gram.io \
	init.c intr.c io.c iob.h lex.c machdefs.h main.c \
	malloc.c mem.c memset.c misc.c names.c names.h niceprintf.c \
	niceprintf.h output.c output.h p1defs.h p1output.c \
	parse.h parse_args.c pccdefs.h pread.c proc.c put.c putpcc.c \
	sysdep.c sysdep.h tokens usignal.h vax.c version.c xsum.c


xsum.exe : xsum.c
	$(CC) $(CFLAGS) -o xsum.exe xsum.c

#Check validity of transmitted source...
#On non-Unix systems that end lines with carriage-return/newline pairs,
#use "make xsumr.out" rather than "make xsum.out".  The -r flag ignores
#carriage-return characters.
xsumr.out: xsum.exe $b
	xsum $b >xsum1.out
	cmp xsum0.out xsum1.out && mv xsum1.out xsumr.out

#Erzeuge den File xsum0.out
xsum0.out: xsum.exe $b
	xsum $b >xsum0.out
