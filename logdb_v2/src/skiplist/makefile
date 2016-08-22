#target
skiplist: serial.o skiplist.o 
	cc -g -o skiplist skiplist.o serial.o
skiplist.o: skiplist.h skiplist.c
	cc -std=c99 -c -g  skiplist.c
serial.o: serial.h serial.c
	cc -std=c99 -c -g serial.c 

clean : 
	rm -rf *.o
