function: main.o function.o
	gcc main.o function.o -o function
main.o: main.c function.h
	gcc -c main.c
function.o: function.c function.h
	gcc -c function.c
clean:
	rm -f *.o function