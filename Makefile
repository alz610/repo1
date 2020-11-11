all:
	gcc -g -Wall -fopenmp -c parse.c -o libparse.o 
	ar -rc libparse.a libparse.o

	gcc main.c -g -Wall -L. -fopenmp -lparse -o main.out


shared:
	gcc -g -c -Wall -fopenmp -fpic parse.c
	gcc -shared -o libparse.so parse.o -lgomp

	gcc -g -Wall -L. -fopenmp -o main.out main.c -lparse