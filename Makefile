CC=mpiicpc


all: tests rt benchmarks
.PHONY : all	

.PHONY : tests
tests:
	cd tests; make

.PHONY : rt
rt:
	cd src; make

.PHONY : benchmarks
benchmarks: 
	cd benchmarks; make
	
.PHONY : clean
clean:
	rm -rf bin/*
	
