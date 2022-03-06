#include "test.h"

int main() {

    ASSERT(7,  ({ int foo; int bar; foo = 3;bar = 4; foo + bar;}));
    ASSERT(6 , ({ int a;int b; a = b = 3; a+b;}));
    ASSERT(9 , ({ int a;int b;int c;int d; a = b = 3;c = d = 2; a * b + c - d;}));
    ASSERT(7 , ({ int a;int b; a = 3; b = 4; a+b;}));
    ASSERT(11, ({ int a; 5 * 3 - 4;}));
    ASSERT(12, ({ int bar; if (1) bar = 12; else bar = 3; bar;}));
    ASSERT(1 , ({int foo; foo = 3;int bar;bar = 4;int res; if (foo == 3) res = 1; else res = 0; res;}));
    ASSERT(10, ({int foo;int bar; foo = 3;bar = 4; if (foo == 3) bar = bar + 2; foo +bar + 1;}));
    ASSERT(13, ({ int a; int i; a = 3; for(i = 0;i < 10;i = i + 1) a = a + 1; a;}));
    ASSERT(10, ({int i; i = 3; for(;i < 10;) i = i + 1; i;}));
    ASSERT(14, ({ int i;int a;int b;int c; i=2;{a=3;b=4;c=5;} i+a+b+c; }));
    ASSERT(3 , ({ int a; a = 2;if (a == 2) {a = 3;} else {a = 5;} a;}));
    ASSERT(5 , ({int a; a = 3;if (a == 2) {a = 3;} else {a = 5;} a;}));
    ASSERT(7 , ({ int a;int b;int i; a = 4;b = 3; for(i = 0;i < 10;i = i + 1) {a = a + 1; b = b - 1;} a + b;}));
    ASSERT(3 , ({ int a; a = 3;;;;; a;}));
    ASSERT(7 , ({int a = 3,b = 4,c;c = a + b; c; }));
    ASSERT(4, ({ 1,2,3,4;}));
    ASSERT(2, ({int i = 0; i=i+1,i=i+1; i;}));
    ASSERT(1, ({int a[3] = {1,2,3}; a[1,2] = 1; a[2];}));

    return 0;
}