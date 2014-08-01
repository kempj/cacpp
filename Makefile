CC=mpicxx
all: driver.cpp coarray.h
	$(CC) driver.cpp -o cacpp -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

mmult: mmult.cpp coarray.h
	$(CC) mmult.cpp -o mmult -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
	
mm-local-2D: mmult-local.cpp coarray.h
	$(CC) mmult-local.cpp -o mmult-local -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

vector-local-2D: mmult-local.cpp coarray.h
	$(CC) vector-local.cpp -o vector-local -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
