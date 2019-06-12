prog: mirror-threaded.o
	gcc mirror-threaded.o -lpthread -lwiringPi
mirror-threaded.o: mirror-threaded.c
	gcc -c mirror-threaded.c