include ../config.mk

build: libGM

obj = ../commen/commen.o gobject.o mea.o reader.o vertex.o loader.o modelstore.o configuration.o \
	tokenizer.o solid.o mesh.o matter.o iterator.o stlreader.o matter.o


CFLAGS += $(DEFS) $(WARN) $(OPT) $(INCLUDE)


libGM: $(obj) 
	ar -cr ../../lib/libGM.a $(obj)

clean:
	rm -f $(obj)
