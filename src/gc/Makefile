include ../config.mk

build: gc test

obj = ../commen/commen.o main.o

CC = gcc
CFLAGS += $(DEFS) $(WARN) $(COMPILE) $(OPT) -I../gm -I../gmc -I../commen

gc: $(obj) 
	gcc $(obj) $(LFLAGS) -o ../../bin/gc $(LSHARED) -lm $(LSTATIC) -L../../lib -lGMC -lGM

test: gc
	export MALLOC_TRACE=$(MALLOC_TRACE); cd ../../$(TESTDIR); ../bin/gc $(TEST)

clean:
	rm -f $(obj)
