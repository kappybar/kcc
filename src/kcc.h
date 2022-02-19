#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

typedef struct Token Token;
typedef struct Type Type;
typedef struct Obj Obj;
typedef struct Node Node;
typedef struct Function Function;


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
// type.c
//

typedef enum {
    TyInt,
    TyPtr,
    TyArray,
} TypeKind;


struct Type {
    TypeKind kind;
    Type *ptr_to;

    size_t array_size;
};

Type *new_type(TypeKind kind);
Type *new_type_ptr(Type *ty);
Type *new_type_array(Type *ty, size_t size);
Type *copy_type(Type *ty);
int sizeof_type(Type *ty);
int ptr_to_size(Type *ty);
void add_type(Node *node);

//
// parse.c
//


struct Obj {
    Obj *next;
    Type *type;
    char *name;
    int len;
    int offset;
};

extern Obj *locals;

typedef enum {
    NdNum,      // num
    NdAdd,      // +
    NdSub,      // -
    NdMul,      // *
    NdDiv,      // /
    NdEq,       // ==
    NdNeq,      // !=
    NdLt,       // <
    NdLe,       // <=
    NdAssign,   // =
    NdLvar,     // Local variable
    NdReturn,   // return
    NdIf,       // if
    NdFor,      // for
    NdBlock,    // { .. }
    NdDeref,    // *
    NdRef,      // &
    NdFuncall  // funtion call
} NodeKind;

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
    Type *type;

    char *func_name;
    int func_name_len;
};


struct Function {
    Function *next;
    Node *body;
    Obj *locals;
    Type *return_type;

    char *name;
    int name_len;
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

void codegen(Function *func);

//
// util.c
//

int counter();
void error_tokenize(char *p);
void error_parse(Token *token);
void error_codegen();
void error_type();

//
// debug.c
//

void display_token(Token *token);
void display_function(Function *func);
void display_program(Function *func);
