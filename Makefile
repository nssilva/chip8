CFLAGS = -O2 -Wall
all: chip8
test: all
chip8: chip8.c chip8.h
clean:
	rm -f chip8 *.o
