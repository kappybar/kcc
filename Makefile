CC = gcc

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

kcc : $(OBJS)
	$(CC) -o kcc $(OBJS) 

$(OBJS) : src/kcc.h

clean : 
	rm kcc src/*.o tmp.s tmp

.PHONY: clean