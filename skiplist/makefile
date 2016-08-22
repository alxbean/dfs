#target
skiplist: serial.o skiplist.o 
	cc -o skiplist skiplist.o serial.o
skiplist.o: skiplist.h skiplist.c
	cc -std=c99 -c skiplist.c
serial.o: serial.h serial.c
	cc -std=c99 -c serial.c 

clean : 
	rm -rf *.o
