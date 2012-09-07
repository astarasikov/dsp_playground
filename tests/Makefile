TESTS=conv_2d conv_raw
CXX ?= g++
#CXFLAGS=-std=c++0x -Wall -pg
CXFLAGS=-O3 -std=c++0x -Wall

CXFILES = $(patsubst %,%.cc,$(TESTS))
OBJFILES = $(patsubst %.cc,%.o,$(CXFILES))

all: $(TESTS)

$(TESTS): $(OBJFILES)
	$(CXX) $(CXFLAGS) -o $@ $<

$(OBJFILES): %.o: %.cc
	$(CXX) $(CXFLAGS) -c $< -o $@

clean:
	rm -f $(TESTS)
	rm -f *.o
