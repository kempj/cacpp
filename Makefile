CC=mpiicpc

all: driver
	
runtime.o: runtime.cpp runtime.h
	$(CC) runtime.cpp -c -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

driver: driver.cpp coarray.h runtime.o
	$(CC) driver.cpp runtime.o -o cacpp -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

mmult: mmult.cpp coarray.h
	$(CC) mmult.cpp -o mmult -g -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
	
mm-local-2D: mmult-local.cpp coarray.h runtime.o
	$(CC) mmult-local.cpp runtime.o -o mmult-local -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

vector-local-2D: mmult-local.cpp coarray.h runtime.o
	$(CC) vector-local.cpp runtime.o -o vector-local -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

dist-vector: vector-dist.cpp coarray.h runtime.o
	$(CC) vector-dist.cpp runtime.o -o dist-vector -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

dist-mmult: mmult-dist.cpp coarray.h runtime.o
	$(CC) mmult-dist.cpp runtime.o -o dist-mmult -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
	
clean:
	rm -rf *.o cacpp
