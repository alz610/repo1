
all:
	gcc -g -c -Wall -fopenmp -fpic parse.c
	gcc -shared -o libparse.so parse.o -lgomp

	gcc -g -Wall -L. -fopenmp -o test0.out test0.c -lparse
	gcc -g -Wall -L. -fopenmp -o test1.out test1.c -lparse