all: driver.cpp
	mpiicpc driver.cpp -o cacpp -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt

mmult: mmult.cpp
	mpiicpc mmult.cpp -o mmult -g -std=c++11 -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
