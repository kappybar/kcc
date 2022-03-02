#include "test.h"

short x;
short adds(short x, short y) {
    return x + y;
}

long xl1;
long xl2 = 7; 
long addl(long x, long y) {
    return x + y;
}

char xc3 = 1;
short xs3 = 1;
int xi3 = 1;
long xl3 = 1; 

int main() {

    ASSERT(4 , ({short x; x = 4; x; }));
    ASSERT(0 , x);
    ASSERT(6 , ({short x = adds(1,2); short y = adds(x,3); y;}));
    ASSERT(4 , ({ short x, *y = &x; *y = 4; x; }));
    ASSERT(10, ({ short arr[10]; int i;for (i = 0;i < 10; i=i+1) arr[i] = 1;short sum = 0;for (i = 0;i < 10;i = i+1) sum = sum + arr[i]; sum; }));
    ASSERT(4 , ({long x; x = 4; x; }));
    ASSERT(0 , xl1);
    ASSERT(7 , xl2);
    ASSERT(6 , ({long x = addl(1,2); long y = addl(x,3); y; }));
    ASSERT(4 , ({ long x, *y = &x; *y = 4; x; }));
    ASSERT(10, ({ long arr[10]; int i;for (i = 0;i < 10; i=i+1) arr[i] = 1;long sum = 0;for (i = 0;i < 10;i = i+1) sum = sum + arr[i]; sum;}));
    ASSERT(4 , xc3 + xs3 + xi3 + xl3);
    ASSERT(1 , ({char y; long x = sizeof(y); x;}));
    ASSERT(2 , ({short y; long x = sizeof(y); x;}));
    ASSERT(4 , ({int y; long x = sizeof(y); x;}));
    ASSERT(8 , ({long y; long x = sizeof(y); x;}));
    
    return 0;
}