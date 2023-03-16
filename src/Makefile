build: GC

GC: 
	make -C matrix/src
	make -C gm
	make -C gmc
	make -C gp
	make -C gc
	make -C gv



clean:
	make -C matrix/src clean
	make -C gm clean
	make -C gmc clean
	make -C gp clean	
	make -C gc clean
	make -C gv clean

test:
	make -C gc test
	make -C gv test
