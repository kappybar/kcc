CC = gcc

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

ASMS2 = $(SRCS:.c=2.s)

TEST_SRCS = $(wildcard test/*.c)
TESTS = $(TEST_SRCS:.c=.exe)

DEBUGS = $(subst main.o,main_debug.o,$(OBJS))

kcc : $(OBJS)
	$(CC) -o kcc $(OBJS) 

test/%.exe: kcc test/%.c
	$(CC) -o test/$*.i -E -P -C test/$*.c 
	./kcc -o test/$*.s test/$*.i
	$(CC) -o $@ test/$*.s -xc test/common

test : $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

debug : $(DEBUGS)
	$(CC) -o debug $(DEBUGS) 

src/main_debug.o : src/main.c 
	$(CC)    -c -o src/main_debug.o $< -DDEBUG_  

src/%2.s: kcc src/%.c 
	$(CC) -o src/$*2.i -E -P -C src/$*.c -DKCC_
	./kcc -o src/$*2.s src/$*2.i 

kcc2 : $(ASMS2)
	$(CC) -o kcc2 $(ASMS2) 

$(OBJS) : src/kcc.h src/help.h

clean : 
	rm kcc src/*.o tmp.s tmp debug test/*.exe test/*.s test/*.i src/*.i src/*.s src/*.o

.PHONY: clean tests