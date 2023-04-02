include ../config.mk

build: libGMC

obj = ../commen/commen.o gmath.o modelcreator.o shapes.o creator_cube.o creator_lemvos.o creator_test.o \
	sequencer.o


CFLAGS += $(DEFS) $(WARN) $(COMPILE) $(OPT) -I../gm -I../gv -I../../include -I../commen

libGMC: $(obj) 
	ar -cr ../../lib/libGMC.a $(obj)


clean:
	rm -f $(obj)
