#include "test.h"

void vfprintf_test1(FILE *file, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(file, fmt, ap);
    return;
}

void vfprintf_test2(FILE *file, char *fmt, ...) {
    int x;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(file, fmt, ap);
    return;
}


int main() {
    FILE *file = fopen("a.txt", "w");
    fprintf(file, "THIS IS PPRINTF %d %d %d\n", 1, 2, 3);
    vfprintf_test1(file, "THIS IS VFPRITF TEST %d %d %d %d\n", 1, 2, 3, 4);
    vfprintf_test2(file, "THIS IS VFPRITF TEST %d %d %d %d\n", 1, 2, 3, 4);

    fclose(file);
    return 0;
}