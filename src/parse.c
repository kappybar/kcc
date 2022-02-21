#include "kcc.h"

Obj *locals;
Function *functions;

Node *primary(Token **token);
Node *unary(Token **token);
Node *add(Token **token);
Node *new_add(Node *lhs, Node *rhs);
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
    if ((*token)->kind != TkNum) error_parse(*token, "expect number\n");
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

Token *expect_ident(Token **token) {
    if ((*token)->kind != TkIdent) {
        error_parse(*token, "expect ident\n");
        return NULL;
    }    
    Token *token_return = *token;
    *token = (*token)->next;
    return token_return;
}

bool consume_keyword(Token **token, char *keyword) {
    if ((*token)->kind != TkKeyword || !equal(*token, keyword)) return false;
    *token = (*token)->next;
    return true;
}

void expect_keyword(Token **token, char *keyword) {
    if ((*token)->kind != TkKeyword || !equal(*token, keyword)) error_parse(*token, "expect keyword\n");
    *token = (*token)->next;
    return;
}

void expect(Token **token, char *op) {
    if ((*token)->kind != TkReserved || !equal(*token, op)) error_parse(*token, "expect %s\n", op);
    *token = (*token)->next;
    return;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdNum;
    node->val  = val;
    return node;
}

Obj *find_obj(Token *token, bool should_exist) {
    char *name = token->str;
    int len = token->len;
    for (Obj *obj = locals;obj; obj = obj->next) {
        if (len == obj->len && strncmp(name, obj->name, len) == 0) {
            if (!should_exist) {
                error_parse(token, "redeclaration\n");
            }
            return obj;
        }
    }
    if (should_exist) {
        error_parse(token, "undefinde variable\n");
    }
    return NULL;
}

void locals_reverse(void) {
    Obj *next = NULL;
    Obj *temp = NULL;
    for (Obj *obj = locals;obj;obj = temp) {
        temp = obj->next;
        obj->next = next;
        next = obj;
    }
    locals = next;
    return;
}

Obj *new_obj(Token *token, Type *type) {
    Obj *obj = calloc(1, sizeof(Obj));
    obj->next = locals;
    obj->len = token->len;
    obj->name = token->str;
    obj->type = type;
    locals = obj;
    return obj;
}
 
Node *new_node_lvar(Obj *obj) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdLvar;
    node->obj = obj;
    node->type = copy_type(obj->type);
    return node;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// argument = expr ("," expr) *
Node *argument(Token **token) {
    Node head;
    Node *cur = &head;
    cur->next = expr(token);
    cur = cur->next;
    while(consume(token, ",")) {
        cur->next = expr(token);
        cur = cur->next;
    }
    return head.next;
}

// primary =   number 
//           | "(" expr ")"
//           | ident "(" argument ")"
//           | ident "[" expr "]"
//           | ident 
Node *primary(Token **token) {
    if (consume(token, "(")) {
        Node *node = expr(token);
        expect(token, ")");
        return node;
    }
    Token *token_ident = consume_ident(token);
    if (token_ident) {
        // funccall
        if (consume(token, "(")) {
            Node *node = new_node(NdFuncall, NULL, NULL);
            node->func_name = token_ident->str;
            node->func_name_len = token_ident->len;
            if (consume(token, ")")) {
                return node;
            }
            node->arguments = argument(token);
            expect(token, ")");
            return node;
        }
        // variable
        Obj *obj = find_obj(token_ident, true); // should_exist = false
        Node *node = new_node_lvar(obj);
        add_type(node);
        if (consume(token, "[")) {
            Node *index = expr(token);
            node = new_add(node, index);
            node = new_node(NdDeref, node, NULL);
            expect(token, "]");
        }
        return node;
    }
    int val = expect_number(token);
    return new_node_num(val);
}

// postfix = primary ( "[" expr "]" | "(" argument ")" )*
Node *postfix(Token **token) {
    Node *node = primary(token);
    while (1) {
        if (consume(token, "[")) {
            Node *index = expr(token);
            node = new_add(node, index);
            node = new_node(NdDeref, node, NULL);
            expect(token, "]");
            continue;
        }
        if (consume(token, "(")) {
            error_parse(*token, "function pointer is not supported now\n");
            continue;
        }

        break;
    }

    return node;
}

// unary =   ("+" | "-") unary
//         | ("*" | "&") unary
//         | "sizeof" unary
//         | postfix
Node *unary(Token **token) {
    if (consume(token, "+")) {
        return unary(token);
    } else if (consume(token, "-")) {
        return new_node(NdSub, new_node_num(0), unary(token));
    } else if (consume(token, "*")) {
        return new_node(NdDeref, unary(token), NULL);
    } else if (consume(token, "&")) {
        return new_node(NdRef, unary(token), NULL);
    } else if (consume_keyword(token, "sizeof")) {
        Node *node = unary(token);
        add_type(node);
        if (!node->type) {
            error_parse(*token, "expected expression\n");
        }
        return new_node_num(sizeof_type(node->type));
    } else {
        return postfix(token);
    }
}

// mul = unary ("*" unary | "/" unary) *
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

Node *new_add(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    if (lhs->type->kind == TyInt && (rhs->type->kind == TyPtr || rhs->type->kind == TyArray) ) {
        Node *tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }
    if (lhs->type->kind == TyInt && rhs->type->kind == TyInt) {
        return new_node(NdAdd, lhs, rhs);
    }
    else if ( (lhs->type->kind == TyPtr || lhs->type->kind == TyArray) && rhs->type->kind == TyInt) {
        int size = ptr_to_size(lhs->type);
        rhs = new_node(NdMul, rhs, new_node_num(size));
        return new_node(NdAdd, lhs , rhs);
    } 
    else {
        error_type("type error : cannot add this two type\n");
        return NULL;
    }
}

Node *new_sub(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    if (lhs->type->kind == TyInt && (rhs->type->kind == TyPtr || rhs->type->kind == TyArray)) {
        Node *tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }
    if (lhs->type->kind == TyInt && rhs->type->kind == TyInt) {
        return new_node(NdSub, lhs, rhs);
    }
    else if ((lhs->type->kind == TyPtr || lhs->type->kind == TyArray) && rhs->type->kind == TyInt) {
        int size = ptr_to_size(lhs->type);
        rhs = new_node(NdMul, rhs, new_node_num(size));
        return new_node(NdSub, lhs , rhs);
    } 
    else {
        int size = ptr_to_size(lhs->type);
        Node *node = new_node(NdSub, lhs, rhs);
        return new_node(NdDiv, node, new_node_num(size));
    }
}

// add = mul ("+" mul | "-" mul) *
Node *add(Token **token) {
    Node *node = mul(token);

    while (1) {
        if (consume(token, "+")) {
            node = new_add(node, mul(token));
            continue;
        }
        if (consume(token, "-")) {
            node = new_sub(node, mul(token));
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

// declspec = "int"
Type *declspec(Token **token) {
    expect_keyword(token, "int");
    return new_type(TyInt);
}

// direct_decl =   ident
//               | direct_decl "[" number "]" 
Obj *direct_decl(Token **token, Type *type) {
    Token *token_ident = expect_ident(token);
    find_obj(token_ident, false); // should_exist = false

    while (consume(token, "[")) {
        int size = expect_number(token);
        type = new_type_array(type, size);
        expect(token, "]");
    }

    Obj *obj = new_obj(token_ident, type);
    return obj;
}

// declaration = declspec "*"* direct_decl ";"
Node *declaration(Token **token) {
    Type *type = declspec(token);
    while (consume(token, "*")) {
        type = new_type_ptr(type);
    }
    Node *node = new_node_lvar( direct_decl(token, type) );
    expect(token, ";");
    return node;
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

// compound_stmt = (declaration | stmt)* "}"
Node *compound_stmt(Token **token) {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while(!consume(token, "}")) {
        if (equal(*token, "int")) {
            cur->next = declaration(token);
            cur = cur->next;
        } else {
            cur->next = stmt(token);
            cur = cur->next;
        }
    }

    return head.next;
}

// decl = declspec "*"* direct_decl
Obj *decl(Token **token) {
    Type *type = declspec(token);
    while(consume(token, "*")) {
        type = new_type_ptr(type);
    }
    return direct_decl(token, type);
}

// param = decl ("," decl) * 
Obj *param(Token **token) {
    decl(token);

    while (consume(token, ",")) {
        decl(token);
    }

    for (Obj *arg = locals;arg;arg = arg->next) {
        // implicit array to ptr type coversion
        if (arg->type->kind == TyArray) {
            arg->type->kind = TyPtr;
        }
    }

    locals_reverse();

    return locals;
}

void allocate_stack_offset(Function *func) {
    int stack_size = 0;
    for (Obj *obj = func->locals;obj;obj = obj->next) {
        int size = sizeof_type(obj->type);
        stack_size += size;
        obj->offset = stack_size;
    }
    func->stack_size = align_to(stack_size, 16);
    return;
}

// func_def = declspec "*"* ident "(" param ")"  "{" compound_stmt
void func_def(Token **token) {
    Function *func = calloc(1, sizeof(Function));

    func->return_type = declspec(token);
    while(consume(token, "*")) {
        func->return_type = new_type_ptr(func->return_type);
    }
    Token *token_ident = expect_ident(token);
    
    // param
    expect(token, "(");
    if (!consume(token, ")")) {
        func->args = param(token);
        expect(token, ")");
    }

    // set function
    func->next = functions;
    functions = func;
    func->name = token_ident->str;
    func->name_len = token_ident->len;

    // body
    expect(token, "{");
    Node *node = compound_stmt(token);

    // add type
    for (Node *cur = node;cur;cur = cur->next) {
        add_type(cur);
    }

    func->body = node;
    func->locals = locals;
    allocate_stack_offset(func);
    locals = NULL;
    return;
}

// program = func_def *
Function *program(Token **token) {

    while (!at_eof(*token)) {
        func_def(token);
    }

    return functions;
}



Function *parse(Token **token) {
    return program(token);
}