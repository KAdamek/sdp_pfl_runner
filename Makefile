INC := -I${CUDA_HOME}/include
LIB := -L${CUDA_HOME}/lib64 -lcudart -lcuda

GCC = g++
NVCC = ${CUDA_HOME}/bin/nvcc


NVCCFLAGS = -O3 -arch=sm_70 --ptxas-options=-v -Xcompiler -Wextra -lineinfo
GCC_OPTS =-O3 -Wall -Wextra $(INC)

PFPROTOTYPECLASS = pfl_runner.exe
PFPROTOTYPEBUILD = pfl_builder.exe

all: clean pflr pflb

pflr: Makefile
	$(GCC) -o $(PFPROTOTYPECLASS) PFL_runner.cpp $(GCC_OPTS) -std=c++11
	
pflb: Makefile
	$(GCC) -o $(PFPROTOTYPEBUILD) PFL_builder.cpp $(GCC_OPTS) -std=c++11

clean:	
	rm -f *.o *.exe
	



