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
assert 10 "int main(){return 10;}"
assert 5  "int main(){return 2 + 3;}"
assert 11 "int main(){return 5 - 3 - 1 + 10;}"
assert 6  "int main(){ return 24 /    4;}"
assert 24 "int main(){ return 6 * 3 + 6;}"
assert 12 "int main(){ return ((1 + 2) + 3 + 4) * 2 / 5 * 3;}"
assert 24 "int main(){ return -8 * -3;}"
assert 12 "int main(){ return (  -2 *3 + + 9) * -4 * -1;}"
assert 1 "int main(){ return 1 == 1;}"
assert 1 "int main(){ return 1 != 0;}"
assert 1 "int main(){ return 0 < 1;}"
assert 1 "int main(){ return 0 <= 0;}"
assert 4 "int main(){ return (0 >= 0) + (5 > 2) + (3 == 3) + (1 != 3) + (0 < 0) + (2 <= 1) + (3 != 3) + (1 == 3);}"
assert 7 "int main(){ int foo; int bar; foo = 3;bar = 4; return foo + bar;}"
assert 6 "int main(){ int a;int b; a = b = 3; return a+b;}"
assert 9 "int main(){ int a;int b;int c;int d; a = b = 3;c = d = 2; return a * b + c - d;}"
assert 7 "int main(){ int a;int b; a = 3; b = 4; return a+b;}"
assert 11 "int main(){ int a; return 5 * 3 - 4;a = 4 + 3;}"
assert 12 "int main(){ int bar; if (1) bar = 12; else bar = 3; return bar;}"
assert 10 "int main(){int foo;int bar; foo = 3;bar = 4; if (foo == 3) bar = bar + 2; return foo +bar + 1;}"
assert 13 "int main(){ int a; int i; a = 3; for(i = 0;i < 10;i = i + 1) a = a + 1; return a;}"
assert 10 "int main(){int i; i = 3; for(;i < 10;) i = i + 1; return i;}"
assert 14 "int main(){ int i;int a;int b;int c; i=2;{a=3;b=4;c=5;} return i+a+b+c; }"
assert 3 "int main() { int a; a = 2;if (a == 2) {a = 3;} else {a = 5;} return a;}"
assert 5 "int main() {int a; a = 3;if (a == 2) {a = 3;} else {a = 5;} return a;}"
assert 7 "int main() { int a;int b;int i; a = 4;b = 3; for(i = 0;i < 10;i = i + 1) {a = a + 1; b = b - 1;} return a + b; }"
assert 3 "int main() { int a; a = 3;;;;; return a;}"
assert 3 "int main() {int x;int *y; x = 3;y = &x; return *y;}"
assert 3 "int main() { int x;int *y;int **z; x = 3;y = &x; z = &y; return **z;}"
assert 10 "int main() { int x;int *y;int **z; x = 4;y = &x; z = &y; **z = 10; return x;}"
assert 19 "int main() { int foo;int bar;int i; foo = 3;bar = 4; for (i = 0;i < 10;i = i + 1) if(i > 5) foo = foo + bar; return foo; }"
assert 8 "int main() {int a;int b;a = sizeof b; return a;}"
assert 8 "int main() {int a;int b;a = sizeof (b); return a;}"
assert 8 "int main() {int a;int *b;a = sizeof (b); return a;}"
assert 8 "int main() {int a; a = sizeof sizeof sizeof a; return a;}"
assert 8 "int main() {int a; a = sizeof(sizeof(a)); return a;}"
assert 80 "int main() {int a[10];int b; b = sizeof(a); return b;}"
assert 3 "int main(){int a[10]; *(a + 1) = 3; return *(a + 1);}"
assert 4 "int main(){int a[2];*a = 1;*(a + 1) = 3;int *p;p = a; return *p + *(p+1);}"
assert 3 "int main(){int a[2];int *b;b = &a; *(b+2) = 3; return *(a+2);}"
assert 10 "int main(){int *a[4];int *b;int c;b = &c; *(a + 3) = b; *b = 10; return **(a + 3);  }"
assert 3 "int main(){int a[3]; a[0] = 3; return a[0];}"
assert 55 "int main() {int foo[10];int i; for(i = 0;i < 10;i = i + 1) {foo[i] = i + 1;} int sum;sum = 0; for (i = 0;i < 10; i = i+ 1) sum = sum + foo[i]; return sum;}"