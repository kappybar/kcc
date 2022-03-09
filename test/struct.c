#include "test.h"

struct s1 {
    int m1;
    int m2;
    char m3;
}; 

struct s2 {
    struct s3 {
        int m1;
    } m2;
    int m3;
    int m4;
    struct s1 m5;
};

struct s4 {
    struct s2 *m1;
};

struct s5 {
    char m1[5];
};

union u1 {
    int m1;
    int m2;
};

union u2 {
    int m1;
    char m2[4];
};

enum e1 {
    E_0,
    E_1,
    E_2,
};

enum e2 {
    E_3,
    E_4
};

enum e1 eg;

int main() {

    ASSERT(0, ({struct s1 a; 0;}));
    ASSERT(0, ({struct s2 g; 0;}));
    ASSERT(4, ({struct s1 z; z.m1 = 4; z.m1; }));
    ASSERT(7, ({struct s1 w; w.m1 = 4; w.m2 = 3; w.m1 + w.m2; }));
    ASSERT(6, ({struct s1 k; k.m1 = 1; k.m2 = 2; k.m3 = 3; k.m1 + k.m2 + k.m3;}));
    ASSERT(1, ({struct s2 x;x.m5.m1 = 1; x.m5.m1; }));
    ASSERT(1, ({struct s1 x;x.m2 = 1; x.m2; }));
    ASSERT(2, ({struct s1 x, *y = &x; y->m1 = 2; x.m1; }));
    ASSERT(2, ({struct s2 x, *y = &x; y->m5.m1 = 2; y->m5.m1; }));
    ASSERT(2, ({ struct s2 x; struct s4 y, *z = &y; y.m1 = &x; z->m1->m3 = 2; z->m1->m3; }));
    ASSERT(3, ({ struct s5 x; x.m1[0] = 1; x.m1[1] = 2; x.m1[0]+x.m1[1];}));
    ASSERT(4, ({ union u1 x; x.m1 = 4; x.m2; }));
    ASSERT(4, ({ union u2 x; x.m1 = 4; x.m2[0]; }));
    ASSERT(0, ({ union u2 x;x.m1 = 4; x.m2[1];}));
    ASSERT(1, ({struct s1 x, y; x.m1 = 1; y = x; y.m1; }));
    ASSERT(4, ({struct s1 x, y; x.m1 = 3; x.m2 = 1; y = x; y.m1 + y.m2; }));
    ASSERT(0, E_0);
    ASSERT(1, E_1);
    ASSERT(0, ({enum e1 x;x = E_0;x;}));
    ASSERT(1, ({enum e1 x;x = E_1;x;}));
    ASSERT(1, ({int x;x = E_1;x;}));
    ASSERT(1, ({eg = E_1;eg;}));
    ASSERT(4, sizeof(enum e1));
    


    return 0;
}