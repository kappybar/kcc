#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

typedef struct Token Token;
typedef struct Type Type;
typedef struct Obj Obj;
typedef struct Node Node;


//
// tokenize.c
//

typedef enum {
    TkReserved, // reserved word
    TkNum,      // number
    TkIdent,    // Identifier
    TkKeyword,  // return
    TkString,   // string ".."
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
    TyChar,
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
int alignment(Type *ty);
bool is_integer(Type *ty);
bool is_pointer(Type *ty);
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
    bool is_global;
    bool is_string;
    char *str;

    // Function
    bool is_function;
    Node *body;
    Obj *locals;
    Obj *args;
    Type *return_type;
    int stack_size;
};

extern Obj *locals;
extern Obj *globals;

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
    NdGvar,     // Global variable
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
    Node *arguments;
};


bool equal(Token *token, char *s);
bool at_eof(Token *token);
bool consume(Token **token, char *op);
int expect_number(Token **token);
Obj *parse(Token **token);

//
// codegen.c
//

void codegen(Obj *func);

//
// util.c
//

int counter();
int align_to(int x, int align);
void error_tokenize(char *p);
void error_parse(Token *token, const char *fmt, ...);
void error_codegen();
void error_type(char *fmt, ...);

//
// debug.c
//

void display_token(Token *token);
void display_function(Obj *func);
void display_program(Obj *func);
