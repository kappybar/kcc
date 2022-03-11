#include "test.h"

int f(int x) {
    int res = 0;
    switch (x) {
    case 0: 
        res = 0;
        break;
    case 1:
        res = 1;
        break;
    case 2:
        res = 2;
        break;
    default:
        res = 10;
        break;
    }
    return res;
}

int g(int x, int y) {
    int res = 0;
    switch (x) {
    case 0: 
        switch (y) {
        case 0:
            res = 0;
            break;
        default:
            res = 1;
            break;
        }
        break;
    case 1:
        switch (y) {
        case 1:
            res = 2;
            break;
        default:
            res = 3;
            break;
        }
        break;
    default:
        res = 10;
        break;
    }
    return res;
}

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
    ASSERT(11, ({int i = 0;int s = 0; do {s++;} while (i++ < 10); s;}));
    ASSERT(11, ({int i = 0;int s = 0; do {s++;} while (i++ < 10); i;}));
    ASSERT(5, ({int i = 0; for (i = 0;i < 10; i++) { if (i == 5) break; } i;}));
    ASSERT(5, ({int i = 0; while (i < 10) { if (i == 5) break; i++;} i;}));
    ASSERT(5, ({int i = 0; do { if (i == 5) break; i++; } while (i < 10); i;}));
    ASSERT(3, ({int i = 0; switch (0) {case 0:i++; case 1:i++; case 2: i++;} i;}));
    ASSERT(2, ({int i = 0; switch (1) {case 0:i++; case 1:i++; case 2: i++;} i;}));
    ASSERT(1, ({int i = 0; switch (2) {case 0:i++; case 1:i++; case 2: i++;} i;}));
    ASSERT(0, ({int i = 0; switch (3) {case 0:i++; case 1:i++; case 2: i++;} i;}));
    ASSERT(1, ({int i = 0; switch (0) case 0: i++; i; }));
    ASSERT(0, ({int i = 0; switch (1) case 0: i++; i; }));
    ASSERT(0, ({int i = 0; switch (1) i = 3; i; }));
    ASSERT(3, ({int i = 0;int k = 0; switch (k) {case 0:i++; case 1:i++; case 2: i++;} i; }));
    ASSERT(2, ({int i = 0;int k = 1; switch (k) {case 0:i++; case 1:i++; case 2: i++;} i; }));
    ASSERT(6, ({int i = 0; switch (0) {case 0:i++;i++; case 1:i++;i++; case 2: i++; i++;} i; }));
    ASSERT(4, ({int i = 0; switch (1) {case 0:i++;i++; case 1:i++;i++; case 2: i++; i++;} i; }));
    ASSERT(1, ({int i = 0; switch (3) {case 0:i++;i++; case 1:i++;i++; case 2: i++; i++; default: i++;} i; }));
    ASSERT(3, ({int i = 0; switch (2) {case 0:i++;i++; case 1:i++;i++; case 2: i++; i++; default: i++;} i; }));
    ASSERT(2, ({int i = 0; switch (2) {case 0:i++;i++; default: i++; case 1:i++;i++; case 2: i++; i++; } i; }));
    ASSERT(5, ({int i = 0; switch (3) {case 0:i++;i++; default: i++; case 1:i++;i++; case 2: i++; i++; } i; }));
    ASSERT(7, ({int i = 0; switch (0) {case 0:i++;i++; default: i++; case 1:i++;i++; case 2: i++; i++; } i; }));
    ASSERT(2, ({int i = 0; switch (0) {case 0:{switch (1) {case 0:i++; case 1:i++;}} case 1:{switch (0) case 0:i++;}} i;}));
    ASSERT(2, ({int i = 0; switch (0) {case 0:i++;{case 1:i++;}} i;}));
    ASSERT(1, ({int i = 0; switch (1) {case 0:i++;{case 1:i++;}} i;}));
    ASSERT(0, ({int i = 0; switch (2) {case 0:i++;{case 1:i++;}} i;}));
    ASSERT(0, f(0));
    ASSERT(1, f(1));
    ASSERT(2, f(2));
    ASSERT(10, f(90));
    ASSERT(0, g(0, 0));
    ASSERT(1, g(0, 3));
    ASSERT(2, g(1, 1));
    ASSERT(3, g(1, 5));
    ASSERT(10, g(2, 5));


    return 0;
}