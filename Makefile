CC = gcc
CFLAGS = -g -Wall -I.
LDFLAGS = -lpthread -lm -lX11 -ldl
RAYLIB_HEADERS = raylib.h
OBJECTS = sound.o


main: main.c $(RAYLIB_HEADERS) $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $< libraylib.a -o $@

$(OBJECTS) : %.o : %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $< -o $@

.PHONY: clean

clean: 
	rm -f main main.o $(OBJECTS)
