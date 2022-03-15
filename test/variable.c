#include "test.h"

const char *args_reg64[6] = {"rdi", "rsi", "rdx", "rcx", "r8" , "r9" };
int hoge;
int x;
int y[10];
int memo[20];
char a[3];

int fib(int n){
    if (n <= 1) {
        return n;
    } else if (memo[n] != 0) {
        return memo[n];
    } else {
        return memo[n] = fib(n-1) + fib(n-2);
    }
}

int add(char a, char b) {
    return a + b;
}

char addsub(int a, char b, int c, char d,int e, char f){
    int g = 1;
    char h = 10;  
    return a - b + c - d + e + f - g + h;
}

extern int ex1;
extern int exf(int x);

int main() {

    ASSERT(0 , hoge);
    ASSERT(3 , ({x = 3; x;}));
    ASSERT(3 , ({y[1] = 3; y[1] + y[2];}));
    ASSERT(55, fib(10));
    ASSERT(2 , ({char a = 2; a;}));
    ASSERT(6 , ({ char hoge = 3, fuga = 3; add(hoge, fuga); }));
    ASSERT(9 , ({ char hoge = 3; int x = 3; int *y = &x; hoge + x + *y;}));
    ASSERT(9 , ({ int *y,x;char hoge = 3;x = 3;y = &x; hoge + x + *y;}));
    ASSERT(18, addsub(1,2,3,4,5,6));
    ASSERT(5, ({a[0] = -1;a[1] = 2;int x = 4; a[0]+a[1]+x; }));
    ASSERT(3 , ({int a[2];char b[2];int c;char d;a[0] = 1;a[1] = 2;b[0] = 3;b[1] = 4;c = 5;d = 6; -a[0]+a[1]-b[0]+b[1]-c+d;  }));
    ASSERT(1 , ({int a = 3;char b = 4;int res; if (a < b) {res = 1;} else { res = 0;} res;}));
    ASSERT(10, ({int a = 1,b = 1; a = b += 4; a+b;}));
    ASSERT(2, ({int a[5]; a[0] = 1, a[2] = 2; int *b = a; b += 2; *b;}));
    ASSERT(2 , ({int a = 5,b = 5; a = b -= 4; a+b;}));
    ASSERT(1, ({int a[5]; a[0] = 1, a[2] = 2; int *b = a+2; b -= 2; *b;}));
    ASSERT(10, ({int a = 2; a *= 5; a;}));
    ASSERT(2, ({int a = 10; a /= 5; a;}));
    ASSERT(3, ({int a = 13; a %= 5; a;}));
    ASSERT(16, ({int a = 2; a <<= 3; a;}));
    ASSERT(2, ({int a = 17; a >>= 3; a;}));
    ASSERT(1, ({int a = 3; a &= 5; a;}));
    ASSERT(15, ({int a = 10; a ^= 5; a;}));
    ASSERT(7, ({int a = 3; a |= 5; a;}));
    ASSERT(2, ({int a = 1; ++a;}));
    ASSERT(2, ({int a = 1,b = ++a; b;}));
    ASSERT(0, ({int a = 1; --a;}));
    ASSERT(0, ({int a = 1,b = --a; b;}));
    ASSERT(1, ({int a = 1;int b = a++; b;}));
    ASSERT(2, ({int a = 1;int b = a++; a;}));
    ASSERT(1, ({int a = 1;int b = a--; b;}));
    ASSERT(0, ({int a = 1;int b = a--; a;}));
    ASSERT(3, ({char a = 1,b = 1; add(++a,b);}));
    ASSERT(2, ({char a = 1,b = 1; add(a++,b);}));
    ASSERT(1, ({char a = 1,b = 1; add(--a,b);}));
    ASSERT(2, ({char a = 1,b = 1; add(a--,b);}));
    ASSERT(2, ({int a[2]; a[0]=1; ++a[0];}));
    ASSERT(2, ({int a[2]; a[0]=1; a[0]++; a[0];}));
    ASSERT(1, ({int a[3]={0,1,2};int *b = a; b++; *b;}));
    ASSERT(1, ({int a[3]={0,1,2};int *b = a; ++b; *b;}));
    ASSERT(0, ({int a[3]={0,1,2};int *b = a+1; b--; *b;}));
    ASSERT(0, ({int a[3]={0,1,2};int *b = a+1; --b; *b;}));
    ASSERT(0, ({int a[3]={0,1,2};int *b = a; b++[0];}));
    ASSERT(0, ({int *d;int x;d = &x;*(d++) = 0; x;}));
    ASSERT(0, ({int *d;int x;d = &x; !d;}));
    ASSERT(0, ({int a[1];int b=0;if (2 < 1 && a[100]) b = 1; b;}));
    ASSERT(1, ({int a[1];int b=0;if (1 < 2 || a[100]) b = 1; b;}));
    ASSERT(1, ex1);
    ASSERT(2, ({extern int ex2; ex2;}));
    ASSERT(3, ({extern int ex2; ex2=3;}));
    ASSERT(4, ({extern int ex2; add(ex1, ex2);}));
    ASSERT(2, exf(2));
    ASSERT(5, exf(5));
    ASSERT(0, ({int x = -1; x >= 0;}));


    return 0;
}