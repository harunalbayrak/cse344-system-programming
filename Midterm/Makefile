all: program nurse vaccinator citizen

program: 171044014_func_midterm.o 171044014_midterm.o
	gcc -g 171044014_func_midterm.o 171044014_midterm.o -o program -lrt -pthread

nurse: nurse.o
	gcc -g nurse.o -o nurse -lrt -pthread

vaccinator: vaccinator.o
	gcc -g vaccinator.o -o vaccinator -lrt -pthread

citizen: citizen.o
	gcc -g citizen.o -o citizen -lrt -pthread

nurse.o : nurse.c
	gcc -ansi -Wall -std=gnu99 -c nurse.c

vaccinator.o : vaccinator.c
	gcc -ansi -Wall -std=gnu99 -c vaccinator.c

citizen.o : citizen.c
	gcc -ansi -Wall -std=gnu99 -c citizen.c

171044014_func_midterm.o:  171044014_func_midterm.c
	gcc -ansi -Wall -std=gnu99 -c 171044014_func_midterm.c

171044014_midterm.o:  171044014_midterm.c
	gcc -ansi -Wall -std=gnu99 -c 171044014_midterm.c

clear:
	rm *.o