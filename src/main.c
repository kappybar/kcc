#include "kcc.h"

char *input_path;
char *output_path;
FILE *stdin_file;
FILE *stdout_file;
FILE *stderr_file;

void parse_args(int argc, char **argv) {
    for (int i = 0;i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (argv[i+1]) {
                output_path = argv[i+1];
            } else {
                fprintf(stderr, "specify output file\n");
                exit(1);
            }
            i ++;
            continue;
        }
        input_path = argv[i];
    }
    if (!input_path) {
        fprintf(stderr, "sppecify input file\n");
        exit(1);
    }
    return;
}

int main(int argc,char **argv) {
    #ifdef KCC_
    stdin_file = fopen("in_file.txt", "r");
    if (!stdin_file) error("cannot open in_file");

    stdout_file = fopen("out_file.txt", "w");
    if (!stdout_file) error("cannot open out_file");

    stderr_file = fopen("err_file.txt", "w");
    if (!stderr_file) error("cannot open err_file");
    #endif

    parse_args(argc, argv);

    // tokenize
    Token *token = tokenize_file(input_path);
    #ifdef DEBUG_
    fprintf(stderr, "tokenize finish \n");
    display_token(token);
    #endif

    // parse
    Obj *func = parse(&token);
    #ifdef DEBUG_
    fprintf(stderr, "parse finish\n");
    display_program(func);
    #endif

    // codegen
    codegen(func, output_path);
    
    return 0;
}