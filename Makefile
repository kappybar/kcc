CC = gcc

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
DEBUGS = $(subst main.o,main_debug.o,$(OBJS))

kcc : $(OBJS)
	$(CC) -o kcc $(OBJS) 

debug : $(DEBUGS)
	$(CC) -o debug $(DEBUGS) 

src/main_debug.o : src/main.c 
	$(CC)    -c -o src/main_debug.o $< -DDEBUG_  

$(OBJS) : src/kcc.h

clean : 
	rm kcc src/*.o tmp.s tmp debug

.PHONY: clean