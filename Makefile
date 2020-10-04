all:
	gcc -g -fopenmp main.c -o main.out

nonparallel:
	gcc -g main.c -o main.out

.PHONY: test
test:
	./main.out message.txt a