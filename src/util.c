#include "kcc.h"

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