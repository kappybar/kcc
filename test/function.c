#include "test.h"

int ten() {
    return 10;
} 

int ten2() {
    return ten();
}

int id(int x) {
    return x;
}

int add(int x1, int x2, int x3, int x4, int x5, int x6) {
    int k; 
    k = x1 + x2 + x3 + x4 + x5 + x6; 
    return k;
}

int set5(int *x) {
    *x = 5;
}

int fib(int x) {
    if (x <= 1) {
        return x;
    } else {
        return fib(x-1) + fib(x-2);
    }
}

int add_arr(int x[2]){ 
    return x[0] + x[1]; 
} 

int add_arr2(int x[2], int y) {
    return x[0] + x[1] + y;
}

int add_arr3(int a[2], int c, int b[2]) { 
    int k; 
    k = 4;
    return k + c + a[0] + b[1]; 
}

int *ptr(int *a){ 
    return a; 
} 

int index(int mat[2][2],int i,int j) {
    return mat[i][j];
}

char getsi(const char *s, int i){ 
    return s[i];
}

int main() {

    ASSERT(10, ({int a; a = ten(); a;}));
    ASSERT(10, ({int a; a = ten2(); a;}));
    ASSERT(1 , id(1));
    ASSERT(21, ({int s; s = add(1,2,3,4,5,6); s;}));
    ASSERT(5 , ({int s;int *t; t = &s; set5(t); s; }));
    ASSERT(55, fib(10));
    ASSERT(5 , ({int x[2];x[0] = 2;x[1] = 3; add_arr(x); }));
    ASSERT(8 , ({int x[2];int y; x[0] = 2; y = x[1] = 3; add_arr2(x,y); }));
    ASSERT(13, ({int a[2];int b[2];int y; a[0] = b[1] = y = 3; add_arr3(a, y, b); }));
    ASSERT(3 , ({ int c[2]; ptr(c)[1] = 3; c[1]; }));
    ASSERT(3 , ({ int a[2][2]; a[1][0] = 3; index(a,1,0); }));
    ASSERT(97, getsi("abc", 0));
    ASSERT(98, getsi("abc", 1));
    ASSERT(0, getsi("abc", 3));

    return 0;
}