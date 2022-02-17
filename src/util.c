#include "kcc.h"

int counter() {
    static int count = 0;
    return count++;
}

void error_tokenize(char *p) {
    fprintf(stderr, "%s\n", p);
    fprintf(stderr, "^ tokenize error\n");
    exit(1);
}

void error_parse(Token *token) {
    fprintf(stderr, "%s\n", token->str);
    fprintf(stderr, "^ parse error");
    exit(1);
}

void error_codegen() {
    fprintf(stderr, "codegen error\n");
    exit(1);
}

void error_type() {
    fprintf(stderr, "type error\n");
    exit(1);
}