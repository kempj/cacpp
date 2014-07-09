all:
	mpicxx driver.cpp -o cacpp -I../../packages/gasnet/include/ -I../../packages/gasnet/include/mpi-conduit/ -L../../packages/gasnet/lib/ -lpthread -lgasnet-mpi-par -lammpi -lrt
