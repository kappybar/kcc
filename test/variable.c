#include "test.h"

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

    return 0;
}