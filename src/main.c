#include "kcc.h"

int main(int argc,char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"specify program");
        exit(1);
    }

    // tokenize
    Token *token = tokenize(argv[1]);
    fprintf(stderr, "tokenize finish \n");
    display_token(token);

    // parse
    Node *node = parse(&token);
    fprintf(stderr, "parse finish\n");

    // codegen
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    codegen(node);
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}