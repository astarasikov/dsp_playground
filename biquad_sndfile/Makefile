APPNAME=biquad_filter
CC ?= gcc
CFLAGS=-std=c99 -Wall -Werror -Wextra -pg -lsndfile

CFILES = biquad_sndfile.c
OBJFILES = $(patsubst %.c,%.o,$(CFILES))

all: $(APPNAME)

$(APPNAME): $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $(OBJFILES)

$(OBJFILES): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(APPNAME)
	rm -f *.o
	rm -f gmon.out
