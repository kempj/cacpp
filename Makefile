all: driver.cpp coarray.h
	mpiicpc driver.cpp -o cacpp -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

mmult: mmult.cpp coarray.h
	mpiicpc mmult.cpp -o mmult -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
	
mm-local-2D: mmult-local.cpp coarray.h
	mpiicpc mmult-local.cpp -o mmult-local -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
