#include "test.h"

int main() {

    ASSERT(3 , ({int x;int *y; x = 3;y = &x; *y;}));
    ASSERT(3 , ({ int x;int *y;int **z; x = 3;y = &x; z = &y; **z;}));
    ASSERT(10, ({ int x;int *y;int **z; x = 4;y = &x; z = &y; **z = 10; x;}));
    ASSERT(19, ({ int foo;int bar;int i; foo = 3;bar = 4; for (i = 0;i < 10;i = i + 1) if(i > 5) foo = foo + bar; foo; }));
    ASSERT(4 , ({int a;int b;a = sizeof b; a;}));
    ASSERT(4 , ({int a;int b;a = sizeof (b); a;}));
    ASSERT(8 , ({int a;int *b;a = sizeof (b); a;}));
    ASSERT(8 , ({int a; a = sizeof sizeof sizeof a; a;}));
    ASSERT(8 , ({int a; a = sizeof(sizeof(a)); a;}));
    ASSERT(40, ({int a[10];int b; b = sizeof(a); b;}));
    ASSERT(3 , ({int a[10]; *(a + 1) = 3; *(a + 1);}));
    ASSERT(4 , ({int a[2];*a = 1;*(a + 1) = 3;int *p;p = a; *p + *(p+1);}));
    ASSERT(3 , ({int a[2];int *b;b = &a; *(b+2) = 3;  *(a+2);}));
    ASSERT(10, ({int *a[4];int *b;int c;b = &c; *(a + 3) = b; *b = 10; **(a + 3);  }));
    ASSERT(3 , ({int a[3]; a[0] = 3; a[0];}));
    ASSERT(55, ({int foo[10];int i; for(i = 0;i < 10;i = i + 1) {foo[i] = i + 1;} int sum;sum = 0; for (i = 0;i < 10; i = i+ 1) sum = sum + foo[i]; sum;}));
    ASSERT(5 , ({ int x[2];x[0] = 3;x[1] = 2; int k; k = x[0]+x[1]; k; }));
    ASSERT(10, ({ int a[2][2]; a[1][1] = 10; a[1][1]; }));
    ASSERT(1 , ({ int a[1][1]; **a = 1; **a; }));
    ASSERT(3 , ({int a = 3, *b = &a; *b; }));

    return 0;
}