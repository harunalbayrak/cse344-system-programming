all: program

program: 171044014_func_hw4.o 171044014_hw4.o
	gcc -g 171044014_func_hw4.o 171044014_hw4.o -o program -lrt -pthread

171044014_func_hw4.o:  171044014_func_hw4.c
	gcc -ansi -Wall -std=gnu99 -c 171044014_func_hw4.c

171044014_hw4.o:  171044014_hw4.c
	gcc -ansi -Wall -std=gnu99 -c 171044014_hw4.c

clear:
	rm *.o