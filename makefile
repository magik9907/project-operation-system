readfile: main.o readFile.o
	gcc main.o readFile.o -o readfile
main.o: main.c readFile.h
	gcc -c main.c
readFile: readFile.c readFile.h
	gcc -c readFile.c
clean:
	rm -f *.o readfile