CC = gcc

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

ASMS2 = $(SRCS:.c=2.s)
ASMS3 = $(SRCS:.c=3.s)

TEST_SRCS = $(wildcard test/*.c)
TESTS = $(TEST_SRCS:.c=.exe)

TESTS2 = $(TEST_SRCS:.c=2.exe)
TESTS3 = $(TEST_SRCS:.c=3.exe)

DEBUGS = $(subst main.o,main_debug.o,$(OBJS))

kcc : $(OBJS)
	$(CC) -o kcc $(OBJS) 

debug : $(DEBUGS)
	$(CC) -o debug $(DEBUGS) 

src/main_debug.o : src/main.c 
	$(CC)    -c -o src/main_debug.o $< -DDEBUG_  

# 2nd generation
src/%2.s: kcc src/%.c 
	$(CC) -o src/$*2.i -E -P -C src/$*.c -DKCC_
	./kcc -o src/$*2.s src/$*2.i 

kcc2 : $(ASMS2) io_file
	$(CC) -o kcc2 $(ASMS2) 

# 3rd generation
src/%3.s: kcc2 src/%.c 
	$(CC) -o src/$*3.i -E -P -C src/$*.c -DKCC_
	./kcc2 -o src/$*3.s src/$*3.i 

kcc3 : $(ASMS3) io_file
	$(CC) -o kcc3 $(ASMS3) 

$(OBJS) : src/kcc.h src/help.h

io_file:
	touch in_file.txt
	touch out_file.txt
	touch err_file.txt

# test1
test/%.exe: kcc test/%.c
	$(CC) -o test/$*.i -E -P -C test/$*.c 
	./kcc -o test/$*.s test/$*.i
	$(CC) -o $@ test/$*.s -xc test/common

test : $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

# test2
test/%2.exe: kcc2 test/%.c 
	$(CC) -o test/$*2.i -E -P -C test/$*.c 
	./kcc2 -o test/$*2.s test/$*2.i
	$(CC) -o $@ test/$*2.s -xc test/common

test2 : $(TESTS2)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

# test3
test/%3.exe: kcc3 test/%.c 
	$(CC) -o test/$*3.i -E -P -C test/$*.c 
	./kcc3 -o test/$*3.s test/$*3.i
	$(CC) -o $@ test/$*3.s -xc test/common

test3 : $(TESTS3)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

test_all: test test2 test3

clean : 
	rm kcc kcc2 kcc3 src/*.o tmp.s tmp debug test/*.exe test/*.s test/*.i src/*.i src/*.s src/*.o *.txt

.PHONY: clean tests