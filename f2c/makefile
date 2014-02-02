# Makefile: f2c for os2
# (c) Klaus Gebhardt, 1997

include ../makeversions

all: f2cdll.a f2cdll.lib
	cd src & $(MAKE) all & cd ..
	cd libF77 & $(MAKE) all & cd ..
	cd libI77 & $(MAKE) all & cd ..
	gcc -o f2c.dll f2c.def libF77/F77.lib libI77/I77.lib -Zdll -Zomf -Zcrtdll

clean:
	cd src & $(MAKE) clean & cd ..
	cd libF77 & $(MAKE) clean & cd ..
	cd libI77 & $(MAKE) clean & cd ..
	rm -f *.a *.lib *.dll *.exe

install: all
	cp src/f2c.h ../include
	cp src/f2c.exe h:/bin
	cp f2cdll.a f2cdll.lib ../lib
	cp f2c.dll h:/apps/science/octave-$(OCTVER)/dll

f2c.imp: f2c.def
	emximp -o f2c.imp f2c.def

f2cdll.a: f2c.imp
	emximp -o f2cdll.a f2c.imp

f2cdll.lib: f2c.imp
	emximp -o f2cdll.lib f2c.imp
