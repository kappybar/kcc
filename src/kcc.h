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
typedef struct Enum Enum;
typedef struct Typdef Typdef;
typedef struct Obj Obj;
typedef struct Scope Scope;
typedef struct Node Node;

//
// main.c
//

extern char *input_path;

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
    
    // TkNum
    int val;       
};

extern char *user_input;
Token *tokenize_file(char *path);

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
    TyVoid,
    TyFunc,
    TyAbsent
} TypeKind;


struct Type {
    TypeKind kind;
    Type *next;

    // TyPtr, TyArray
    Type *ptr_to;

    // TyFunc
    Type *return_ty;
    Type *params;

    // TyArray
    size_t array_size;

    // TyStruct
    Struct *type_struct;

    // 
    bool is_typdef;
};

Type *new_type(TypeKind kind);
Type *new_type_ptr(Type *ty);
Type *new_type_array(Type *ty, size_t size);
Type *new_type_struct(Struct *s);
Type *new_type_fun(Type *return_ty, Type *params_ty);
Node *zeros_like(Type *type);
Type *fill_absent_type(Type *type_absent, Type *type_fill);
int alignment(Type *ty);
bool is_integer(Type *ty);
bool is_pointer(Type *ty);
bool is_function(Obj *obj);
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
    
    // global var
    Node *init;

    // TyFunc
    bool is_defined;
    Node *body;
    Scope *locals;
    Obj *args;
    int stack_size;
     
    // Enum const
    int enum_value;
};

struct Struct {
    char *name;
    int name_len;
    Struct *next;
    Obj *member;
    int align;
    int size;
};

struct Enum {
    char *name;
    int name_len;
    Enum *next;
    Obj *enum_list;
};

struct Typdef {
    char *name;
    int name_len;
    Typdef *next;
    Type *type;
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
    NdShl,      // <<
    NdSar,      // >> shift arithmetic right
    NdPostInc,  // post ++
    NdPostDec,  // post --
    NdAssign,   // =
    NdLvar,     // Local variable
    NdGvar,     // Global variable
    NdReturn,   // return
    NdIf,       // if
    NdFor,      // for
    NdDoWhile,  // do while 
    NdSwitch,   // switch
    NdCase,     // case
    NdDefault,  // default
    NdBreak,    // break
    NdContinue, // continue
    NdBlock,    // { .. }
    NdDeref,    // *
    NdRef,      // &
    NdFuncall,  // funtion call
    NdInit,     // { .. , .. }
    NdMember,   //  .
    NdStmtExpr, // ({ .. })
    NdComma,    // ,
} NodeKind;

struct Node {
    NodeKind kind;
    Node *next;
    Type *type;
    Node *lhs; // left hand side
    Node *rhs; // right hand side
    
    // NdIf, NdFor, NdDoWhile, NdSwitch
    Node *cond;
    Node *then;
    Node *els;

    // NdFor
    Node *init;
    Node *inc;
    
    // NdBlock, NdStmtExpr, NdInit
    Node *body;
    
    // NdNum
    int val;  

    // NdLvar, NdGvar 
    Obj *obj;   
    
    // NdFuncall
    char *func_name;
    int func_name_len;
    Node *arguments;
    
    // NdMember
    Obj *member;

    // NdCase, NdDefault, NdBreak, NdContinue
    char *label;
};

// new node
Node *new_node_num(int val);
Node *new_node(NodeKind kind);
// parse
Obj *parse(Token **token);

//
// codegen.c
//

void codegen(Obj *func, char *path);

//
// util.c
//

int counter();
int align_to(int x, int align);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

//
// debug.c
//

void display_obj(Obj *obj);
void display_token(Token *token);
void display_node(Node *node, int indent);
void display_type(Type *type);
void display_function(Obj *func);
void display_program(Obj *func);
