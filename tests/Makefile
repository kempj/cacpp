CC=mpiicpc
INC=-I../../../packages/gasnet/include -I../../../packages/gasnet/include/mpi-conduit -I../src
LIB=-L../../../packages/gasnet/lib

all: ../bin/driver ../bin/mmult ../bin/mmult-local ../bin/vector-local
	
gather: ../bin/gather-test

../bin/gather-test: gather-tests.cpp ../src/coarray.h ../bin/runtime.o ../src/collectives.h
	$(CC) $< ../bin/runtime.o -o $@ -g -std=c++11 $(INC) $(MPI_INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt

.PHONY : ../bin/runtime.o
../bin/runtime.o: 
	cd ../src; make

../bin/driver: driver.cpp ../src/coarray.h ../bin/runtime.o
	$(CC) $< ../bin/runtime.o -o $@ -g -std=c++11 $(INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt

../bin/mmult: mmult.cpp ../src/coarray.h
	$(CC) $< -o $@ -g $(INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt
	
../bin/mmult-local: mmult-local.cpp ../src/coarray.h ../bin/runtime.o
	$(CC) $< ../bin/runtime.o -o $@ -g -std=c++11 $(INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt

../bin/vector-local: mmult-local.cpp ../src/coarray.h ../bin/runtime.o
	$(CC) $< ../bin/runtime.o -o $@ -g -std=c++11 $(INC) $(LIB) -lpthread -lgasnet-mpi-par -lammpi -lrt

clean:
	rm -rf *.o cacpp
