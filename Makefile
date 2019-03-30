CCC   = gcc
RM    = rm -f

demo1: demo/demo1.c spool.c spool.h
	$(CCC) -std=c11 -o $@ demo/demo1.c spool.c

run:
	./demo1

clean:
	$(RM) demo1
