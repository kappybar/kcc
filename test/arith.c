#include "test.h"

int main() {
    ASSERT(10, 10);
    ASSERT(5 , 2 + 3);
    ASSERT(1,  3 - 2);
    ASSERT(6,  3 * 2);
    ASSERT(6 , 24 / 4);
    ASSERT(11, 5 - 3 - 1 + 10);
    ASSERT(24, 6 * 3 + 6);
    ASSERT(12, ((1 + 2) + 3 + 4) * 2 / 5 * 3);
    ASSERT(24, -8 * -3);
    ASSERT(12, (-2 *3 + + 9) * -4 * -1);
    ASSERT(1 , 1 == 1);
    ASSERT(1 , 1 != 0);
    ASSERT(1 , 0 < 1);
    ASSERT(1 , 0 <= 0);
    ASSERT(0 , 0 == 1);
    ASSERT(0 , 1 != 1);
    ASSERT(0 , 1 < 0);
    ASSERT(0 , 1 <= 0);
    ASSERT(2 , 30 % 4);
    ASSERT(1 , 100 % 3);
    ASSERT(4 , (0 >= 0) + (5 > 2) + (3 == 3) + (1 != 3) + (0 < 0) + (2 <= 1) + (3 != 3) + (1 == 3));
    ASSERT(3 , + + + + + 3);
    ASSERT(3 , (- + + - + - 3) + 6);
    ASSERT(8, 1 << 3);
    ASSERT(15, 120 >> 3);
    ASSERT(-15, -120 >> 3);
    ASSERT(64, 1 << 3 + 3);
    ASSERT(1, 1 << 2 < 1 << 3);
    ASSERT(0, 1 & 2);
    ASSERT(2, 2 & 2);
    ASSERT(1, 5 & 3);
    ASSERT(1, 2+3 & 1+2);
    ASSERT(3, 1 ^ 2);
    ASSERT(0, 2 ^ 2);
    ASSERT(6, 5 ^ 3);
    ASSERT(6, 2+3 ^ 1+2);
    ASSERT(3, 1 | 2);
    ASSERT(2, 2 | 2);
    ASSERT(7, 5 | 3);
    ASSERT(7, 2+3 | 1+2);
    return 0;
}