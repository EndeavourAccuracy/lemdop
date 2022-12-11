# Makefile for lemdop 0.7b by Norbert de Jonge
#
# cppcheck --language=c --std=c99 --verbose lemdop.c
# Also try the line below with clang instead of gcc.
#
all:
	gcc -O2 -Wno-unused-result -std=c99 -g -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes lemdop.c -o lemdop `sdl2-config --cflags --libs` -lSDL2_ttf -lSDL2_image -lm
