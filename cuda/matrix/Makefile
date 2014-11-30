APPNAME=matrix
CC=nvcc
CFLAGS=
LDFLAGS=
JULIA=/Applications/Julia.app/Contents/Resources/julia/bin/julia

CFILES = \
	matrix.cu

OBJFILES=$(patsubst %.cu,%.o,$(CFILES))

all: $(APPNAME)

$(APPNAME): $(OBJFILES)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJFILES)

$(OBJFILES): %.o: %.cu
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(APPNAME) *.o *.csv || true

verify:
	$(JULIA) --no-history -f verify.jl

run:
	make clean
	make all
	./$(APPNAME) tiled 128
	make verify
