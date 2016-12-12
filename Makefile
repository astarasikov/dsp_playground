APPNAME=convolution
CXX ?= g++
CXFLAGS=-std=c++0x -Wall -O3

#CXFILES = main.cc
CXFILES = bmp-fft.cc
OBJFILES = $(patsubst %.cc,%.o,$(CXFILES))

all: $(APPNAME)

$(APPNAME): $(OBJFILES)
	$(CXX) -o $@ $(OBJFILES)

$(OBJFILES): %.o: %.cc
	$(CXX) $(CXFLAGS) -c $< -o $@

clean:
	rm -f $(APPNAME)
	rm -f *.o
