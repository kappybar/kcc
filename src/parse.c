#include "kcc.h"

Obj *locals;

Node *primary(Token **token);
Node *unary(Token **token);
Node *add(Token **token);
Node *mul(Token **token);
Node *relational (Token **token);
Node *assign(Token **token);
Node *equality(Token **token);
Node *expr(Token **token);
Node *stmt(Token **token);
Node *compound_stmt(Token **token);
Function *program(Token **token);

bool equal(Token *token, char *s) {
    return (memcmp(token->str, s, token->len) == 0) && s[token->len] == '\0';
}

bool at_eof(Token *token) {
    return token->kind == TkEof;
}

// tokenの種類が数字なら、その値を返し、トークンを一つ先に進める。
int expect_number(Token **token) {
    if ((*token)->kind != TkNum) error_parse(*token);
    int val = (*token)->val;
    *token = (*token)->next;
    return val;
}

// tokenがopと一致していたらtrueを返し、トークンを一つ進める。
bool consume(Token **token, char *op) {
    if ((*token)->kind != TkReserved || !equal(*token, op)) return false;
    *token = (*token)->next;
    return true;
}

Token *consume_ident(Token **token) {
    if ((*token)->kind != TkIdent) return NULL;
    Token *token_return = *token;
    *token = (*token)->next;
    return token_return;
}

bool consume_keyword(Token **token, char *keyword) {
    if ((*token)->kind != TkKeyword || !equal(*token, keyword)) return false;
    *token = (*token)->next;
    return true;
}

void expect(Token **token, char *op) {
    if ((*token)->kind != TkReserved || !equal(*token, op)) error_parse(*token);
    *token = (*token)->next;
    return;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdNum;
    node->val  = val;
    return node;
}

Obj *find_obj(char *name, int len) {
    for (Obj *obj = locals;obj; obj = obj->next) {
        if (len == obj->len && strncmp(name, obj->name, len) == 0) {
            return obj;
        }
    }
    return NULL;
}

Obj *new_obj(Token *token) {
    Obj *obj = calloc(1, sizeof(Obj));
    obj->next = locals;
    obj->len = token->len;
    obj->name = token->str;
    obj->offset = locals ? locals->offset + 8 : 0;
    locals = obj;
    return obj;
}
 
Node *new_node_lvar(Token *token) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdLvar;
    Obj *obj = find_obj(token->str, token->len);
    if (obj) {
        node->obj = obj;
    } else {
        node->obj = new_obj(token);
    }
    return node;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// primary = number | ident | "(" expr ")"
Node *primary(Token **token) {
    if (consume(token, "(")) {
        Node *node = expr(token);
        expect(token, ")");
        return node;
    }
    Token *token_ident = consume_ident(token);
    if (token_ident) {
        Node *node = new_node_lvar(token_ident);
        return node;
    }
    int val = expect_number(token);
    return new_node_num(val);
}

// unary = ("+" | "-")? primary
Node *unary(Token **token) {
    if (consume(token, "+")) {
        return primary(token);
    } else if (consume(token, "-")) {
        return new_node(NdSub, new_node_num(0), primary(token));
    } else {
        return primary(token);
    }
}

// mul = primary ("*" unary | "/" unary) *
Node *mul(Token **token) {
    Node *node = unary(token);

    while (1) {
        if (consume(token, "*")) {
            node = new_node(NdMul, node, unary(token));
            continue;
        }
        if (consume(token, "/")) {
            node = new_node(NdDiv, node, unary(token));
            continue;
        }
        break;
    }

    return node;
}

// add = mul ("+" mul | "-" mul) *
Node *add(Token **token) {
    Node *node = mul(token);

    while (1) {
        if (consume(token, "+")) {
            node = new_node(NdAdd, node, mul(token));
            continue;
        }
        if (consume(token, "-")) {
            node = new_node(NdSub, node, mul(token));
            continue;
        }
        break;
    }

    return node;
}

// relational = add ( "<" add | ">" add | "<=" add | ">=" add)*
Node *relational (Token **token) {
    Node *node = add(token);

    while(1) {
        if (consume(token, "<")) {
            node = new_node(NdLt, node, add(token));
            continue;
        }
        if (consume(token, ">")) {
            node = new_node(NdLt, add(token), node);
            continue;
        }
        if (consume(token, "<=")) {
            node = new_node(NdLe, node, add(token));
            continue;
        }
        if (consume(token, ">=")) {
            node = new_node(NdLe, add(token), node);
            continue;
        }
        break;
    }

    return node;
}

// equality = relational ( "==" relational | "!=" relational) * 
Node *equality(Token **token) {
    Node *node = relational(token);

    while (1) {
        if (consume(token, "==")) {
            node = new_node(NdEq, node, relational(token));
            continue;
        } 
        if (consume(token, "!=")) {
            node = new_node(NdNeq, node, relational(token));
            continue;
        }
        break;
    }

    return node;
}

// assign = equality ("=" assign) ?
Node *assign(Token **token) {
    Node *node = equality(token);
    
    if (consume(token, "=")) {
        node = new_node(NdAssign, node, assign(token));
    }

    return node;
}

// expr = assign
Node *expr(Token **token) {
    return assign(token);
}

// stmt =   expr? ";" 
//        | "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt) ?
//        | "while" "(" expr ")" stmt 
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | "{" compound_stmt
Node *stmt(Token **token) {
    if (consume_keyword(token, "return")) {
        Node *node = new_node(NdReturn, expr(token), NULL);
        expect(token, ";");
        return node;
    }
    if (consume_keyword(token, "if")) {
        expect(token, "(");
        Node *node = new_node(NdIf, NULL, NULL);
        node->cond = expr(token);
        expect(token, ")");
        node->then = stmt(token);
        if (consume_keyword(token, "else")) {
            node->els = stmt(token);
        }
        return node;
    }
    if (consume_keyword(token, "while")) {
        expect(token, "(");
        Node *node = new_node(NdFor, NULL, NULL);
        node->cond = expr(token);
        expect(token, ")");
        node->then = stmt(token);
        return node;
    }
    if (consume_keyword(token, "for")) {
        expect(token, "(");
        Node *node = new_node(NdFor, NULL, NULL);
        if (!consume(token, ";")) {
            node->init = expr(token);
            expect(token, ";");
        }
        if (!consume(token, ";")) {
            node->cond = expr(token);
            expect(token, ";");
        }
        if (!consume(token, ")")) {
            node->inc = expr(token);
            expect(token, ")");
        }
        node->then = stmt(token);
        return node;
    }
    if (consume(token, "{")) {
        Node *node = new_node(NdBlock, NULL, NULL);
        node->body = compound_stmt(token);;
        return node;
    }
    if (consume(token, ";")) {
        Node *node = new_node(NdBlock, NULL, NULL);
        return node;
    }
    Node *node = expr(token);
    expect(token, ";");
    return node;
}

// compound_stmt = stmt* "}"
Node *compound_stmt(Token **token) {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while(!consume(token, "}")) {
        cur->next = stmt(token);
        cur = cur->next;
    }

    return head.next;
}

// program = "{" compound_stmt
Function *program(Token **token) {
    expect(token, "{");

    Node *node = compound_stmt(token);
    if (!at_eof(*token)) {
        printf("err");
        error_parse(*token);
    }

    Function *func = calloc(1, sizeof(Function));
    func->body = node;
    func->locals = locals;
    func->stack_size = locals ? locals->offset + 8 : 0;
    locals = NULL;
    return func;
}

Function *parse(Token **token) {
    return program(token);
}