.SUFFIXES: .c .obj

.c.obj:
	$(CC) $(CFLAGS) -c -O3 $<

all: scimark2 

CC = gcc
LDFLAGS = 

OBJS = FFT.obj kernel.obj Stopwatch.obj Random.obj SOR.obj SparseCompRow.obj \
	array.obj MonteCarlo.obj LU.obj 

scimark2 : scimark2.obj $(OBJS)
	$(CC) $(CFLAGS) -o scimark2 scimark2.obj $(OBJS) $(LDFLAGS) -lm

clean:
	del $(OBJS) scimark2
