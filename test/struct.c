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

enum e4 {
    E7 = 3,
    E8 = 1+3,
    E9 = 3-1,
    E10 = 1*3,
    E11 = 5/2,
    E12 = 5%2,
    E13 = 1 == 1,
    E14 = 1 != 1,
    E15 = 1 < 3,
    E16 = 1 <= 1,
    E17 = 1 << 3,
    E18 = 10 >> 1,
    E19 = 10 & 7,
    E20 = 10 ^ 7,
    E21 = 10 | 7,
    E22 = 1 && 1,
    E23 = 0 && 1,
    E24 = 0 || 0,
    E25 = 1 || 0,
    E26 = 1 ? 2 : 3,
    E27 = 0 ? 2 : 3,
    E28 = (1+2*3)+3-(1-3),
};

struct Node {
    struct Node *lhs;
    struct Node *rhs;
};

typedef struct List List;

struct List {
    int car;
    List *cdr;
};

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
    ASSERT(4, ({struct s1 x; x.m1 = 3; x.m1++; x.m1; }));
    ASSERT(4, ({struct s1 x; x.m1 = 3; ++x.m1;}));
    ASSERT(0, E_0);
    ASSERT(1, E_1);
    ASSERT(0, ({enum e1 x;x = E_0;x;}));
    ASSERT(1, ({enum e1 x;x = E_1;x;}));
    ASSERT(1, ({int x;x = E_1;x;}));
    ASSERT(1, ({eg = E_1;eg;}));
    ASSERT(4, sizeof(enum e1));
    ASSERT(1, ({ struct s6 *x; struct s6 {int a;}; struct s6 y; x = &y; x->a = 1; x->a; }));
    ASSERT(16, sizeof(struct Node));
    ASSERT(0, ({struct Node l, r, nd;nd.lhs = &l;nd.rhs = &r; 0;}));
    ASSERT(0, ({enum e3 x; enum e3 {E5, E6}; x = E5; x;}));
    ASSERT(2, ({List a,b; a.car = 1; b.car = 2; a.cdr = &b; a.cdr->car;}));
    ASSERT(3, E7);
    ASSERT(4, E8);
    ASSERT(2, E9);
    ASSERT(3, E10);
    ASSERT(2, E11);
    ASSERT(1, E12);
    ASSERT(1, E13);
    ASSERT(0, E14);
    ASSERT(1, E15);
    ASSERT(1, E16);
    ASSERT(8, E17);
    ASSERT(5, E18);
    ASSERT(2, E19);
    ASSERT(13, E20);
    ASSERT(15, E21);
    ASSERT(1, E22);
    ASSERT(0, E23);
    ASSERT(0, E24);
    ASSERT(1, E25);
    ASSERT(2, E26);
    ASSERT(3, E27);
    ASSERT(12, E28);



    return 0;
}

