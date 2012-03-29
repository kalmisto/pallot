CFLAGS=-W -ggdb -O2 `sdl-config --cflags`
CC=gcc

.PHONY: default

default: pallot

pallot: pallot.c
	$(CC) $(CFLAGS) pallot.c -o demo `sdl-config --libs` -lm -lSDL_image
