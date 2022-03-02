#include "kcc.h"

int main(int argc,char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"specify file");
        exit(1);
    }

    // tokenize
    Token *token = tokenize_file(argv[1]);
    // for debug
    fprintf(stderr, "tokenize finish \n");
    display_token(token);

    // parse
    Obj *func = parse(&token);
    // for debug
    fprintf(stderr, "parse finish\n");
    display_program(func);

    // codegen
    printf(".intel_syntax noprefix\n");
    codegen(func);
    
    return 0;
}