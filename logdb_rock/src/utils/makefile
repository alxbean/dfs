#target
test: test.o logdb_queue.o
	cc -g -o test test.o logdb_queue.o -lpthread
test.o: ./include/logdb_queue.h test.c
	cc -g -c -I include test.c
logdb_queue.o: ./include/logdb_queue.h logdb_queue.c
	cc -g -c -I include logdb_queue.c

clean:
	rm -rf *.o

