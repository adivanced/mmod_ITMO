.PHONY: all

all:
	nasm -f elf64 idw.asm -g -F dwarf -o idw.o
	g++ main.cpp -g3 -c
	g++ main.o idw.o -g3 -o main

main: main.o idw.o
	g++ main.o idw.o -g3 -o main

main.o: main.cpp kd_tree.h idw.h settings.h
	g++ main.cpp -g3 -c

idw.o: idw.asm
	nasm -f elf64 idw.asm -g -F dwarf -o idw.o