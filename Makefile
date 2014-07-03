all:
	g++ coarray.cpp -I../../packages/gasnet/include/ -I../../packages/gasnet/include/smp-conduit -L../../packages/gasnet/lib/ -lpthread -lgasnet-smp-par -lrt
