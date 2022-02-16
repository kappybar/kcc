#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

//
// tokenize.c
//

typedef enum {
    TkReserved, // reserved word
    TkNum,      // number
    TkIdent,    // Identifier
    TkKeyword,  // return
    TkEof       // end of file
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;   // 次のToken
    char *str;     // このTokenの文字列
    int len;       // このTokenの長さ

    int val;       // kindがTkNumであるときの数字 
};

Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

//
// parse.c
//

typedef struct Obj Obj;

struct Obj {
    Obj *next;
    char *name;
    int len;
    int offset;
};

extern Obj *locals;

typedef enum {
    NdNum,     // num
    NdAdd,     // +
    NdSub,     // -
    NdMul,     // *
    NdDiv,     // /
    NdEq,      // ==
    NdNeq,     // !=
    NdLt,      // <
    NdLe,      // <=
    NdAssign,  // =
    NdLvar,    // Local variable
    NdReturn,  // return
    NdIf,      // if
    NdFor,     // for
    NdBlock,   // { .. }
    NdDeref,   // *
    NdRef      // &
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *next;
    Node *lhs; // left hand side
    Node *rhs; // right hand side

    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    Node *body;

    int val;   // kindがNdNumのとき、その値
    Obj *obj;  // 
};

typedef struct Function Function;

struct Function {
    Node *body;
    Obj *locals;
    int stack_size;
};

bool equal(Token *token, char *s);
bool at_eof(Token *token);
bool consume(Token **token, char *op);
int expect_number(Token **token);
Function *parse(Token **token);

//
// codegen.c
//

void codegen_function(Function *func);

//
// util.c
//

int counter();
void error_tokenize(char *p);
void error_parse(Token *token);
void error_codegen();

//
// debug.c
//

void display_token(Token *token);
void display_function(Function *func);
