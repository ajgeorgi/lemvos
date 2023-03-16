include ../../config.mk
# DEFS+= -DCHECK 

obj= hmatrix.o ../../commen/commen.o


CPPFLAGS += $(DEFS) $(WARN) $(CPPCOMPILE) $(OPT) -I. -I../../../include -I../../commen
CFLAGS += $(DEFS) $(WARN) $(COMPILE) $(OPT) -I. -I../../../include

CC=gcc

all : hmatrix.o main.o libMat
	g++ hmatrix.o main.o -o matrix

hmatrix.o : hmatrix.cc hmatrix.h
	g++ -c $(CPPFLAGS) hmatrix.cc

main.o : main.cc hmatrix.h
	g++ -c $(CPPFLAGS) main.cc
	
libMat: $(obj) 
	ar -cr ../../../lib/libMat.a $(obj)
	cp -f hmatrix.h ../../../include/

clean:
	rm -f matrix *.o *~
