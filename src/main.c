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
    
    return 0;
}