all: player

player: 171044014_hw3.o 
	gcc -g 171044014_hw3.o -o player -lrt -pthread

171044014_hw3.o:  171044014_hw3.c
	gcc -ansi -Wall -std=gnu99 -c 171044014_hw3.c

clear:
	rm *.o