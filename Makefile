.PHONY: test
.ONESHELL: test


all:
	gcc -g -c -Wall -fopenmp -fpic parse.c
	gcc -shared -o libparse.so parse.o -lgomp

	gcc -g -Wall -L. -fopenmp -o main.out main.c -lparse


test:
	# add the current directory to the environment variable LD_LIBRARY_PATH
	@export LD_LIBRARY_PATH=$$PWD:$$LD_LIBRARY_PATH
	./main.out