#include "kcc.h"

int counter() {
    static int count = 0;
    return count++;
}

int align_to(int x, int align) {
    return ((x + align - 1) / align) * align;
}

void error_tokenize(char *p) {
    fprintf(stderr, "%s\n", p);
    fprintf(stderr, "^ tokenize error\n");
    exit(1);
}

void error_parse(Token *token, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char s[100];
    strncpy(s, "^ parse error ", 14);
    strcpy(s+14, fmt);
    fprintf(stderr, "%s\n", token->str);
    vfprintf(stderr, s, ap);
    exit(1);
}

void error_codegen() {
    fprintf(stderr, "codegen error\n");
    exit(1);
}

void error_type(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    exit(1);
}