include ../config.mk

build: gv test


obj = ../commen/commen.o widget.o soliddialog.o popup.o main.o viewer.o controller.o menu.o \
      projection.o aperal.o tabsheet.o input.o oceandialog.o meshdialog.o table.o combobox.o \
      modeldia.o progress.o

CFLAGS += $(DEFS) $(WARN) $(COMPILE) $(OPT) $(INCLUDE) -I../gm -I../gmc -I../gp
LIBS = -L../../lib $(LSTATIC) -lGMC -lGM -lGP -lhpdfs -L/lib64/ $(LSTATIC) -lz -L/usr/lib64/ $(LSTATIC) -lpng16 

gv: $(obj) 
	$(CC) $(obj) $(LFLAGS) -o ../../bin/gv -lm $(X11LIBSDIR) -lX11  $(LIBS)

test: gv
	export MALLOC_TRACE=$(MALLOC_TRACE); cd ../../$(TESTDIR); ../bin/gv -l $(TESTFILE)

clean:
	rm -f $(obj)
