CC=mpiicpc
INC=-I../../../packages/gasnet/include -I../../../packages/gasnet/include/mpi-conduit -I../src
LIB=-L../../../packages/gasnet/lib

all: ../bin/dist-vector ../bin/dist-mmult

dist-mult: ../bin/dist-mmult
	
.PHONY : ../bin/runtime.o
../bin/runtime.o: 
	cd ../src; make

../bin/dist-vector: vector-dist.cpp ../src/coarray.h ../bin/runtime.o
	$(CC) $< ../bin/runtime.o -o $@ -g -std=c++11 $(INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt

../bin/dist-mmult: mmult-dist.cpp ../src/coarray.h ../bin/runtime.o
	$(CC) $< ../bin/runtime.o -o $@ -g -std=c++11 $(INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt
	
