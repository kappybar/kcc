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
    TkReserved, // 予約語
    TkNum,      // 数字
    TkEof       // 改行
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;   // 次のToken
    char *str;     // このTokenの文字列
    int len;       // このTokenの長さ

    int val;       // kindがTokinNumであるときの数字 
};

void display_token(Token *token);
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

//
// parse.c
//

typedef enum {
    NdNum,  // num
    NdAdd,  // +
    NdSub,  // -
    NdMul,  // *
    NdDiv,  // /
    NdEq,   // ==
    NdNeq,  // !=
    NdLt,   // <
    NdLe,   // <=
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs; // left hand side
    Node *rhs; // right hand side

    int val;   // kindがNdNumのとき、その値
};

bool equal(Token *token, char *s);
bool at_eof(Token *token);
bool consume(Token **token, char *op);
int expect_number(Token **token);
Node *new_node_num(int val);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *primary(Token **token);
Node *unary(Token **token);
Node *add(Token **token);
Node *mul(Token **token);
Node *relational (Token **token);
Node *equality(Token **token);
Node *expr(Token **token);
Node *parse(Token **token);

//
// codegen.c
//

void codegen(Node *node);

//
// util.c
//

void error_tokenize(char *p);
void error_parse(Token *token);
void error_codegen();