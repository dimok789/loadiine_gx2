CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES:= $(wildcard source/*.c) 
OBJ_FILES = $(patsubst source/%.c,obj/%.o,$(SOURCES))
EXECUTABLE=UdpDebugReader

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

obj/%.o: source/%.c
	mkdir obj; $(CC) $(CFLAGS) -c -o $@ $<
