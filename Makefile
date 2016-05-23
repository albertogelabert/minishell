CC=gcc
CFLAGS=-c -Wall -std=c99
LDFLAGS= -lreadline

SOURCES=shell.c
LIBRARIES=shell.o
INCLUDES=shell.h
PROGRAMS=shell
OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES) 

%.o: %.c $(INCLUDES) 
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS)
