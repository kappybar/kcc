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
assert 10 10
assert 5 "2 + 3"
assert 11 "5 - 3 - 1 + 10"
assert 6  " 24 /    4"
assert 24 " 6 * 3 + 6"
assert 12 " ((1 + 2) + 3 + 4) * 2 / 5 * 3"
assert 24 " -8 * -3"
assert 12 "(  -2 *3 + + 9) * -4 * -1"
assert 1 " 1 == 1"
assert 1 " 1 != 0"
assert 1 " 0 < 1"
assert 1 " 0 <= 0"
assert 4 " (0 >= 0) + (5 > 2) + (3 == 3) + (1 != 3) + (0 < 0) + (2 <= 1) + (3 != 3) + (1 == 3)"



