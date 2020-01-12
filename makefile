EXEC=projet
SOURCES= terminal.c editor.c  main.c
OBJECTS=$(SOURCES:.c=.o)
CC=gcc
CFLAGS= -lm

.PHONY: default clean

default: $(EXEC)

helpers.o: helpers.c helpers.h
editor.o: editor.h editor.c terminal.c
terminal.o: terminal.h terminal.c 
main.o: main.c   terminal.h editor.h



%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^

clean:
	rm -rf $(EXEC) $(OBJECTS) $(SOURCES:.c=.c~) $(SOURCES:.c=.h~) Makefile~
