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
typedef struct Struct Struct;
typedef struct Obj Obj;
typedef struct Scope Scope;
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
    Token *next;   
    char *str;     
    int len;       
    Type *type;

    int val;       
};

Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

//
// type.c
//

typedef enum {
    TyLong,
    TyInt,
    TyShort,
    TyChar,
    TyPtr,
    TyArray,
    TyStruct,
} TypeKind;


struct Type {
    TypeKind kind;
    Type *ptr_to;

    size_t array_size;
    Struct *type_struct;
};

struct Struct {
    char *name;
    int name_len;
    Struct *next;
    Obj *member;
    int align;
    int size;
};

Type *new_type(TypeKind kind);
Type *new_type_ptr(Type *ty);
Type *new_type_array(Type *ty, size_t size);
Type *new_type_struct(Struct *s);
Type *copy_type(Type *ty);
Node *zeros_like(Type *type);
int alignment(Type *ty);
bool is_integer(Type *ty);
bool is_pointer(Type *ty);
int sizeof_type(Type *ty);
int ptr_to_size(Type *ty);
void add_type(Node *node);

//
// parse.c
//


struct Scope {
    Obj *objs;
    Scope *prev;
};

struct Obj {
    Obj *next;
    Type *type;
    char *name;
    int len;
    int offset;
    bool is_global;

    Node *init;

    // Function
    bool is_function;
    Node *body;
    Scope *locals;
    Obj *args;
    Type *return_type;
    int stack_size;
};

extern Scope *locals;
extern Scope *fun_locals;
extern Obj *globals;

typedef enum {
    NdNum,      // num
    NdAdd,      // +
    NdSub,      // -
    NdMul,      // *
    NdDiv,      // /
    NdMod,      // %
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
    NdFuncall,  // funtion call
    NdInit,     // { .. , .. }
    NdMember,   //  .
    NdStmtExpr, // ({ .. })
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

    int val;   
    Obj *obj;   
    Type *type;

    char *func_name;
    int func_name_len;
    Node *arguments;

    Obj *member;
};


bool equal(Token *token, char *s);
bool at_eof(Token *token);
bool consume(Token **token, char *op);
int expect_number(Token **token);
// new node
Node *new_node_num(int val);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
// parse
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

void display_obj(Obj *obj);
void display_token(Token *token);
void display_node(Node *node, int indent);
void display_function(Obj *func);
void display_program(Obj *func);
