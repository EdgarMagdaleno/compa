all:
	gcc -g -ggdb *.c -o compa -lm
	./compa
	./asm asm.we
	./vm