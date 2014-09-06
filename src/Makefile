CC=mpiicpc
INC=-I../../../packages/gasnet/include -I../../../packages/gasnet/include/mpi-conduit -I../src
LIB=-L../../../packages/gasnet/lib


all: ../bin/runtime.o
	

../bin/runtime.o: runtime.cpp runtime.h
	$(CC) $< -c -g -std=c++11 -o $@ $(INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt

clean:
	rm -rf ../bin/runtime.o 
