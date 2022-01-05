all: server client

server: serverFunctions.o serverMain.o linkedList.o helper.o
	gcc -g serverFunctions.o serverMain.o linkedList.o helper.o -o server -lrt -pthread

client: clientFunctions.o clientMain.o linkedList.o helper.o
	gcc -g clientFunctions.o clientMain.o linkedList.o helper.o -o client -lrt -pthread

serverFunctions.o: serverFunctions.c linkedList.c helper.c
	gcc -ansi -Wall -std=gnu99 -c serverFunctions.c linkedList.c helper.c

clientFunctions.o: clientFunctions.c helper.c
	gcc -ansi -Wall -std=gnu99 -c clientFunctions.c helper.c

linkedList.o: linkedList.c
	gcc -ansi -Wall -std=gnu99 -c linkedList.c

helper.o: helper.c
	gcc -ansi -Wall -std=gnu99 -c helper.c

serverMain.o: serverMain.c linkedList.c helper.c
	gcc -ansi -Wall -std=gnu99 -c serverMain.c linkedList.c helper.c

clientMain.o: clientMain.c
	gcc -ansi -Wall -std=gnu99 -c clientMain.c linkedList.c helper.c

clear:
	rm *.o