wish: main.o modes.o commands.o
	gcc -o wish main.o modes.o commands.o

commands.o: commands.c
	gcc -g -c commands.c

modes.o: modes.c
	gcc -g -c modes.c
	
main.o: main.c
	gcc -g -c main.c