# kcc (WIP)

A toy self-hosted C compiler written in C, this compiler works on only Linux

## Build
```
make kcc
```

## Usage
```
sh kcc.sh -o <output-file> <input-file>
```

## Run Examles
```
sh kcc.sh -o fib examples/fib.c
./fib
```

## Features
### Implemented
+ `+`,`-`,`*`,`/`,`%`, ...expression
+ `if`, `for`, `while`, `switch`, ...statement
+ `int`, `char`, `short`, `long`, `void`, `struct`, `union`, `enum`,`typedef`
+ `static`, `extern`
+ variable length arguments fuction
+ string, char literal
+ block, line comment

### Unimplemented
+ preprocess
+ struct, union initialization
+ no-length array initialization
+ some global variable initialization
+ `float`, `double`, `unsigned`, `_Bool`
+ bitfield
+ some type cast
+ `goto`
+ `inline` function
+ function with 7 or more arguments
+ function pointer
+ variable length array
+ number literal
+ some error handling
+ tag scope

## Test
build the 1st, 2nd and 3rd generation compilers and test them.
```
make test_all
```

## References
+ https://github.com/rui314/chibicc
+ https://www.sigbus.info/compilerbook
+ [BNF](https://cs.wmich.edu/~gupta/teaching/cs4850/sumII06/The%20syntax%20of%20C%20in%20Backus-Naur%20form.htm)
+ [Compiler Explorer](https://godbolt.org/)