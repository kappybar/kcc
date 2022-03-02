#include "test.h"

// line comment

/*
  nested comment
*/

int four() {
    return 4;
}

char hoge1[2] = "a";
char *hoge2 = "a";
char hoge3[2] = {97,0};
char hoge4[2][2] = {{97,0},{97,0}};
char hoge5[2][2] = {"a","a"};
char *hoge6[2] = {"a","a"};
int hoge7[2][2] = {{97,0},{97,0}};
int a1[3] = {1};
int a2[3][3] = {{1},{2},{}};
int x; 
int *y = &x;
char get(char *s){ return s[0]; }

int main() {

    ASSERT(48,  ({ char a = '0';a;}));
    ASSERT(97,  ({char *x ="abc"; x[0];}));
    ASSERT(50,  ({char *b = "012"; b[2];}));
    ASSERT(4 ,  sizeof("abc"));
    ASSERT(98,  "abc"[1]);
    ASSERT(0 ,  ""[0]);
    ASSERT(0 ,  ""[0]);
    ASSERT(3 , ({int a[3] = {1,2}; a[0]+a[1]; }));
    ASSERT(3 , ({int a = {3}; a; }));
    ASSERT(10,  ({ int a[2][2] = {{1,2},{3,4}}; a[0][0]+a[0][1]+a[1][0]+a[1][1]; }));
    ASSERT(98,  ({ char a[4] = "abc"; a[1]; }));
    ASSERT(98,  ({ char *a[4] = {"abc"}; a[0][1]; }));
    ASSERT(98,  ({ char a[1][4] = {"abc"}; a[0][1]; }));
    ASSERT(9 ,  ({int y = 4; int x[3] = {1, y, four()}; x[0]+x[1]+x[2];}));
    ASSERT(97,  hoge1[0]);
    ASSERT(97,  hoge2[0]);
    ASSERT(97,  hoge3[0]);
    ASSERT(97,  hoge4[0][0]);
    ASSERT(97,  hoge5[0][0]);
    ASSERT(97,  hoge6[0][0]); 
    ASSERT(97,  hoge7[0][0]);
    ASSERT(97,  ({char hoge[2] = "a"; hoge[0]; }));
    ASSERT(97, ({char *hoge = "a"; hoge[0];}));
    ASSERT(97,  ({ char *hoge[2] = {"a","a"}; hoge[0][0]; }));
    ASSERT(3 , ({a1[1]=2; a1[0]+a1[1]; }));
    ASSERT(6 , ({a2[2][2] = 3; a2[0][0]+a2[1][0]+a2[2][2]; }));
    ASSERT(3 , ({ *y = 3; x; }));
    ASSERT(97, get("abc"));

    ASSERT(7  , "\a"[0]);
    ASSERT(8  , "\b"[0]);
    ASSERT(9  , "\t"[0]);
    ASSERT(10 , "\n"[0]);
    ASSERT(11 , "\v"[0]);
    ASSERT(12 , "\f"[0]);
    ASSERT(13 , "\r"[0]);
    ASSERT(27 , "\e"[0]);
    ASSERT(7  , '\a');
    ASSERT(8  , '\b');
    ASSERT(9  , '\t');
    ASSERT(10 , '\n');
    ASSERT(11 , '\v');
    ASSERT(12 , '\f');
    ASSERT(13 , '\r');
    ASSERT(27 , '\e');
    ASSERT(7  , "\ak\b"[0]);
    ASSERT(107, "\ak\b"[1]);
    ASSERT(0  , "\0"[0]);
    ASSERT(0  , '\0');

    return 0;
}