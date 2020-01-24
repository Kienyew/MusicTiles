CC = gcc
CFLAGS = -g -Wall -I.
LDFLAGS = -lpthread -lm -lX11 -ldl
RAYLIB_HEADERS = raylib.h physac.h raudio.h raymath.h rlgl.h
main: main.c $(RAYLIB_HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) $< libraylib.a -o $@


.PHONY: clean

clean: 
	echo 'clean'
