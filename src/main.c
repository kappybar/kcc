#include "kcc.h"

int main(int argc,char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"specify program");
        exit(1);
    }

    // tokenize
    Token *token = tokenize(argv[1]);
    // for debug
    fprintf(stderr, "tokenize finish \n");
    display_token(token);

    // parse
    Function *func = parse(&token);
    // for debug
    fprintf(stderr, "parse finish\n");
    display_function(func);

    // codegen
    printf(".intel_syntax noprefix\n");
    codegen_function(func);
    // printf(".globl main\n");
    // printf("main:\n");

    // // explicit main function
    // // prologue
    // int offset = locals ? locals->offset + 8 : 0;
    // printf("  push rbp\n");
    // printf("  mov rbp, rsp\n");
    // printf("  sub rsp, %d\n", offset);

    // for (Node *cur = node;cur;cur = cur->next) {
    //     codegen(cur);
    //     printf("  pop rax\n");
    // }

    // // epilogue
    // printf("  mov rsp, rbp\n");
    // printf("  pop rbp\n");
    // printf("  ret\n");
    return 0;
}