#!/bin/sh

assert() {
    expected="$1"
    input="$2"

    ./kcc "$input" > tmp.s
    cc -o tmp tmp.s    
    ./tmp
    actual="$?"
    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$inupt => $expected expected, but got $actual"
        exit 1
    fi
}

make kcc
assert 10 "10;"
assert 5 "2 + 3;"
assert 11 "5 - 3 - 1 + 10;"
assert 6  " 24 /    4;"
assert 24 " 6 * 3 + 6;"
assert 12 " ((1 + 2) + 3 + 4) * 2 / 5 * 3;"
assert 24 " -8 * -3;"
assert 12 "(  -2 *3 + + 9) * -4 * -1;"
assert 1 " 1 == 1;"
assert 1 " 1 != 0;"
assert 1 " 0 < 1;"
assert 1 " 0 <= 0;"
assert 4 " (0 >= 0) + (5 > 2) + (3 == 3) + (1 != 3) + (0 < 0) + (2 <= 1) + (3 != 3) + (1 == 3);"
assert 7 " foo = 3;bar = 4;foo + bar;"
assert 6 " a = b = 3;a+b;"
assert 9 " a = b = 3;c = d = 2;a * b + c - d;"
assert 7 "a = 3; b = 4; return a+b;"
assert 11 "return 5 * 3 - 4;a = 4 + 3;"
assert 12 "if (1) 12; else 3;"
assert 10 "foo = 3;bar = 4; if (foo == 3) bar = bar + 2; foo +bar + 1;"
assert 13 "a = 3; for(i = 0;i < 10;i = i + 1) a = a + 1; return a;"
assert 10 "i = 3; for(;i < 10;) i = i + 1; return i;"



