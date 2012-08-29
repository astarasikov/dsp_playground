APPNAME=tonegen
CXX ?= g++
CXFLAGS=-Wall -pg
LDFLAGS=-lopenal -lm

CXFILES = ToneGen.cc SoundOpenAl.cc
OBJFILES = $(patsubst %.cc,%.o,$(CXFILES))

all: $(APPNAME)

$(APPNAME): $(OBJFILES)
	$(CXX) $(LDFLAGS) -o $@ $(OBJFILES)

$(OBJFILES): %.o: %.cc
	$(CXX) $(CXFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	rm -f $(APPNAME)
	rm -f *.o
