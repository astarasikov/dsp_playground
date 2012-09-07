TESTS=conv_2d conv_2d_par conv_raw
CXX ?= g++
CXFLAGS=-O3 -fopenmp -Wall

CXFILES = ${TESTS:=.cc}

all: ${TESTS}

${TESTS}: ${CXFILES}
	$(CXX) $(CXFLAGS) -o $@ $@.cc

clean:
	rm -f $(TESTS)
	rm -f *.o
