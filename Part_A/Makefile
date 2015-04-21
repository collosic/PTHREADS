# 
# Make file for Lab 1 - ICS 53
# Christian Collosi
# 11233529 - UC Irvine
#

C = gcc
CFLAGS = -I.

DEPS = pthreads.h
OBJ = pthreads.o

%.o: %.c $(DEPS)
	$(CC) -g -c -Wall -pthreads -o $@ $< $(CFLAGS)

pthreads: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:	
	rm -f pthreads output.txt *.o
