#include "test.h"

char xc1;
char xc2 = 1;
char *xc3 = &xc2;

short xs1;
short xs2 = 1;
short *xs3 = &xs2;

int xi1;
int xi2 = 1;
int *xi3 = &xi2;

long xl1;
long xl2 = 1;
long *xl3 = &xl2;

char addc(char x, char y) {
    return x + y;
}

short adds(short x, short y) {
    return x + y;
}

int addi(int x, int y) {
    return x + y;
}

long addl(long x, long y) {
    return x + y;
}

void swap(int *x, int *y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
    return;
}

int (*f1())();
int (*f2())[1];
void (*signal(int x, void (*y)(int z)))(int w);

typedef int I;
typedef short S;
typedef int IA[10];
typedef int IAA[10][10];

void set(IA a, int i) {
    a[i] = 1;
}

void set2(IAA a, int i, int j) {
    a[i][j] = 1;
}

int counter() {
    static int x = 0;
    return x++;
}

int main() {

    ASSERT(4 , ({char x; x = 4; x; }));
    ASSERT(0 , xc1);
    ASSERT(1 , xc2);
    ASSERT(1 , *xc3);
    ASSERT(6 , ({char x = adds(1,2); char y = adds(x,3); y;}));
    ASSERT(4 , ({char x, *y = &x; *y = 4; x; }));
    ASSERT(10, ({char arr[10]; int i;for (i = 0;i < 10; i=i+1) arr[i] = 1;char sum = 0;for (i = 0;i < 10;i = i+1) sum = sum + arr[i]; sum; }));
    ASSERT(4 , ({short x; x = 4; x; }));
    ASSERT(0 , xs1);
    ASSERT(1 , xs2);
    ASSERT(1 , *xs3);
    ASSERT(6 , ({short x = adds(1,2); short y = adds(x,3); y;}));
    ASSERT(4 , ({short x, *y = &x; *y = 4; x; }));
    ASSERT(10, ({ short arr[10]; int i;for (i = 0;i < 10; i=i+1) arr[i] = 1;short sum = 0;for (i = 0;i < 10;i = i+1) sum = sum + arr[i]; sum; }));
    ASSERT(4 , ({int x; x = 4; x; }));
    ASSERT(0 , xi1);
    ASSERT(1 , xi2);
    ASSERT(1 , *xi3);
    ASSERT(6 , ({int x = adds(1,2); short y = adds(x,3); y;}));
    ASSERT(4 , ({int x, *y = &x; *y = 4; x; }));
    ASSERT(10, ({ int arr[10]; int i;for (i = 0;i < 10; i=i+1) arr[i] = 1;short sum = 0;for (i = 0;i < 10;i = i+1) sum = sum + arr[i]; sum; }));
    ASSERT(4 , ({long x; x = 4; x; }));
    ASSERT(0 , xl1);
    ASSERT(1 , xl2);
    ASSERT(1 , *xl3);
    ASSERT(6 , ({long x = addl(1,2); long y = addl(x,3); y; }));
    ASSERT(4 , ({ long x, *y = &x; *y = 4; x; }));
    ASSERT(10, ({ long arr[10]; int i;for (i = 0;i < 10; i=i+1) arr[i] = 1;long sum = 0;for (i = 0;i < 10;i = i+1) sum = sum + arr[i]; sum;}));

    ASSERT(1 , ({char y; long x = sizeof(y); x;}));
    ASSERT(2 , ({short y; long x = sizeof(y); x;}));
    ASSERT(4 , ({int y; long x = sizeof(y); x;}));
    ASSERT(8 , ({long y; long x = sizeof(y); x;}));
    ASSERT(1 , sizeof(char));
    ASSERT(2 , sizeof(short));
    ASSERT(4 , sizeof(int));
    ASSERT(8 , sizeof(long));
    ASSERT(3, ({int a = 1,b = 3; swap(&a, &b); a;}));

    ASSERT(0, ({ int (*x)[1]; 0; }));
    ASSERT(0, ({ int (*x)(); 0; }));
    ASSERT(0, ({ int (**x)(); 0; }));
    ASSERT(0, ({ int (*x[1])(); 0; }));

    ASSERT(1, ({I x = 1; x;}));
    ASSERT(1, ({S x = 1; x;}));
    ASSERT(4, sizeof(I));
    ASSERT(2, sizeof(S));
    ASSERT(1, ({int I = 1;int y = 1; I * y;}));
    ASSERT(1, ({IA a; set(a,3); a[3];}));
    ASSERT(1, ({IAA a; set2(a,3,3); a[3][3];}));

    ASSERT(0, counter());
    ASSERT(1, counter());
    ASSERT(2, counter());

    ASSERT(0, (int)0);
    ASSERT(1, (char)1);
    ASSERT(0, (char)256);
    ASSERT(0, (short)(1 << 16));
    ASSERT(0, (int)(char)256);
    ASSERT(0, ({const int c = 0; c;}));
    ASSERT(0, ({int const const c = 0; c;}));
    
    return 0;
}