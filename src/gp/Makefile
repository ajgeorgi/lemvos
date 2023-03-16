include ../config.mk

build: libGP

obj = ../commen/commen.o texprinter.o pdfprinter.o exporter.o

CC = gcc
CFLAGS += $(DEFS) $(WARN) $(COMPILE) $(OPT) -I../gm -I../../include -I../commen

libGP: $(obj) 
	ar -cr ../../lib/libGP.a $(obj)


clean:
	rm -f $(obj)
