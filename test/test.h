#define ASSERT(x, y) assert(x, y, #y)
void assert(int expected, int actual, char *code);

#include "../src/help.h"