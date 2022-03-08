#include "kcc.h"

Struct *user_structs;
Scope *locals = &(Scope){};
Scope *fun_locals = &(Scope){};
Obj *globals;

// new scope
Scope *next_locals();
void prev_locals(Scope *new_locals);

// find obj
Obj *find_lvar(Token *token, bool should_exist);
Obj *find_gvar(Token *token);
Obj *find_obj(Token *token, bool should_exist);

// new obj
void add_lvar(Obj *obj);
void add_gvar(Obj *obj);
Obj *new_fun(Token *token, Type *return_ty, Obj *params);
Obj *new_string(Token *token);

// new node
Node *new_node_num(int val);
Node *new_node_lvar(Obj *obj);
Node *new_node_gvar(Obj *obj);
Node *new_node_obj(Obj *obj);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

// expr
Node *argument(Token **token);
Node *primary(Token **token);
Node *postfix(Token **token);
Node *unary(Token **token);
Node *mul(Token **token);
Node *new_add(Node *lhs, Node *rhs);
Node *new_sub(Node *lhs, Node *rhs);
Node *add(Token **token);
Node *shift(Token **token);
Node *relational (Token **token);
Node *equality(Token **token);
Node *assign(Token **token);
Node *expr(Token **token);

// stmt
Node *stmt(Token **token);
Node *compound_stmt(Token **token);

// declare
Type *typename(Token **token);
Type *typespec(Token **token);
Obj *params(Token **token);
Type *declspec(Token **token);
Obj *direct_decl(Token **token, Type *type);
Obj *declarator(Token **token, Type *type);
Node *declaration(Token **token, bool is_global);
Struct *struct_union_declaration(Token **token, Token *token_ident, bool is_union);
void def(Token **token);

bool equal(Token *token, char *s) {
    return (memcmp(token->str, s, token->len) == 0) && s[token->len] == '\0';
}

bool at_eof(Token *token) {
    return token->kind == TkEof;
}

int expect_number(Token **token) {
    if ((*token)->kind != TkNum) error_parse(*token, "expect number\n");
    int val = (*token)->val;
    *token = (*token)->next;
    return val;
}

Token *consume_string(Token **token) {
    if ((*token)->kind != TkString) return NULL;
    Token *token_return = *token;
    *token = (*token)->next;
    return token_return;
}

bool consume(Token **token, char *op) {
    if ((*token)->kind != TkReserved || !equal(*token, op)) return false;
    *token = (*token)->next;
    return true;
}

void expect(Token **token, char *op) {
    if ((*token)->kind != TkReserved || !equal(*token, op)) error_parse(*token, "expect %s\n", op);
    *token = (*token)->next;
    return;
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

Scope *next_locals() {
    Scope *new_locals = calloc(1, sizeof(Scope));
    new_locals->prev = locals;
    locals = new_locals;
    return new_locals;
}

void prev_locals(Scope *new_locals) {
    locals = new_locals->prev;
    new_locals->prev = fun_locals;
    fun_locals = new_locals;
    return;
}

Obj *find_lvar(Token *token, bool should_exist) {
    char *name = token->str;
    int len = token->len;
    for (Scope *scope = locals;scope; scope = scope->prev) {
        for (Obj *obj = scope->objs;obj; obj = obj->next  ) {
            if (len == obj->len && strncmp(name, obj->name, len) == 0) {
                return obj;
            }
        }
        if (!should_exist) {
            return NULL;
        }
    }
    return NULL;
}

Obj *find_gvar(Token *token) {
    char *name = token->str;
    int len = token->len;
    for (Obj *obj = globals;obj; obj = obj->next) {
        if (len == obj->len && strncmp(name, obj->name, len) == 0) {
            return obj;
        }
    }
    return NULL;
}

Obj *find_obj(Token *token, bool should_exist) {
    Obj *lvar = find_lvar(token, should_exist);
    if (lvar) {
        if (should_exist) return lvar;
        else error_parse(token, "redeclaration\n");
    }
    Obj *gvar = find_gvar(token);
    if (gvar) {
        if (should_exist) return gvar;
        else error_parse(token, "redeclaration\n");
    }
    if (should_exist) error_parse(token, "undefinde variable\n");
    return NULL;
}

Struct *find_struct(Token *token) {
    for (Struct *s = user_structs;s;s = s->next) {
        if (s->name_len == token->len && strncmp(s->name, token->str, token->len) == 0 ) {
            return s;
        }
    }
    return NULL;
}

Obj *find_member(Struct *st, Token *token) {
    for (Obj *obj = st->member;obj;obj = obj->next) {
        if (obj->len == token->len && strncmp(obj->name, token->str, obj->len) == 0) {
            return obj;
        }
    }
    error_parse(token, "this struct has no member name\n");
    return NULL;
}

void add_lvar(Obj *obj) {
    obj->next = locals->objs;
    obj->is_global = false;
    locals->objs = obj;
    return;
}

void add_gvar(Obj *obj) {
    obj->next = globals;
    obj->is_global = true;
    globals = obj;
    return;
}

Obj *new_obj(Token *token, Type *type) {
    if (type->kind == TyVoid) {
        error_type("cannot define void object\n");
    }
    Obj *obj = calloc(1, sizeof(Obj));
    obj->name = token->str;
    obj->len = token->len;
    obj->type = type;
    return obj;
}

Obj *new_fun(Token *token, Type *return_ty, Obj *params) {
    Obj *fn = calloc(1, sizeof(Obj));
    fn->len = token->len;
    fn->name = token->str;
    fn->return_type = return_ty;
    fn->args = params;
    fn->type = new_type_fun(return_ty);
    return fn;
}

char *unique_str_name() {
    int cnt = counter();
    char *name = calloc(10, sizeof(char));
    int len = sprintf(name, ".L%d", cnt);
    name[len] = '\0';
    return name;
}

Obj *new_string(Token *token) {
    Obj *obj = calloc(1, sizeof(Obj));
    obj->name = unique_str_name();
    obj->len = strlen(obj->name);
    obj->type = token->type;

    Node head;
    Node *cur = &head;
    for (int i = 0;i < obj->type->array_size - 1; i++) {
        cur->next = new_node_num(token->str[i]);
        cur = cur->next;
    }
    cur->next = new_node_num(0);
    cur = cur->next;

    obj->init = new_node(NdInit, NULL, NULL);
    obj->init->body = head.next;
    return obj;
}

Struct *new_struct(Token *token_ident, Obj *member) {
    Struct *new_struct_ = calloc(1, sizeof(Struct));
    new_struct_->member = member;
    new_struct_->name = token_ident->str;
    new_struct_->name_len = token_ident->len;

    int offset = 0;
    for (Obj *obj = new_struct_->member;obj;obj = obj->next) {
        int align = alignment(obj->type);
        offset = align_to(offset, align);
        obj->offset = offset;
        offset += sizeof_type(obj->type);
        if (new_struct_->align < align) {
            new_struct_->align = align;
        }
    }
    new_struct_->size = offset;

    new_struct_->next = user_structs;
    user_structs = new_struct_;
    return new_struct_;
}

Struct *new_union(Token *token_ident, Obj *member) {
    Struct *new_union_ = calloc(1, sizeof(Struct));
    new_union_->member = member;
    new_union_->name = token_ident->str;
    new_union_->name_len = token_ident->len;

    for (Obj *obj = new_union_->member;obj;obj = obj->next) {
        int align = alignment(obj->type);
        int size = sizeof_type(obj->type);
        if (new_union_->align < align) {
            new_union_->align = align;
        }
        if (new_union_->size < size) {
            new_union_->size = size;
        }
        obj->offset = 0;
    }

    new_union_->next = user_structs;
    user_structs = new_union_;
    return new_union_;
}
 
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdNum;
    node->val  = val;
    return node;
}

Node *new_node_lvar(Obj *obj) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdLvar;
    node->obj = obj;
    node->type = copy_type(obj->type);
    return node;
}

Node *new_node_gvar(Obj *obj) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdGvar;
    node->obj = obj;
    node->type = copy_type(obj->type);
    return node;
}

Node *new_node_obj(Obj *obj) {
    if (obj->is_global) {
        return new_node_gvar(obj);
    } else {
        return new_node_lvar(obj);
    }
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

bool is_typename(Token *token) {
    char *typenames[] = {"void", "long", "int", "char", "short", "struct", "union"};
    for (int i = 0;i < sizeof(typenames) / sizeof(*typenames); i++) {
        if (equal(token, typenames[i])) {
            return true;
        }
    }
    return false;
}

// argument = assign ("," assign) *
Node *argument(Token **token) {
    Node head;
    Node *cur = &head;
    cur->next = assign(token);
    cur = cur->next;
    while(consume(token, ",")) {
        cur->next = assign(token);
        cur = cur->next;
    }
    return head.next;
}

// primary =   number 
//           | "(" expr ")"
//           | "(" "{" stmt+ "}" ")"
//           | ident "(" argument ")"
//           | ident 
//           | string
Node *primary(Token **token) {
    if (consume(token, "(")) {
        if (consume(token , "{")) {
            Node *body = compound_stmt(token);
            Node *node = new_node(NdStmtExpr, NULL, NULL);
            node->body = body;
            expect(token, ")");
            return node;
        } else {
            Node *node = expr(token);
            expect(token, ")");
            return node;
        }
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
        Obj *obj = find_obj(token_ident, true); // should_exist = true
        Node *node = new_node_obj(obj);
        return node;
    }
    Token *token_string = consume_string(token);
    if (token_string) {
        Obj *obj = new_string(token_string);
        add_gvar(obj);
        Node *node = new_node_obj(obj);
        return node;
    }
    int val = expect_number(token);
    return new_node_num(val);
}

// postfix = primary ( "[" expr "]" | "(" argument ")" | "." ident | "->" ident )*
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
        if (consume(token, ".")) {
            add_type(node);
            Struct *st = node->type->type_struct;
            Token *member = expect_ident(token);
            node = new_node(NdMember, node, NULL);
            node->member = find_member(st, member);
            continue;
        }
        if (consume(token, "->")) {
            node = new_node(NdDeref, node, NULL);
            add_type(node);
            Struct *st = node->type->type_struct;
            Token *member = expect_ident(token);
            node = new_node(NdMember, node, NULL);
            node->member = find_member(st, member);
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
//         | "sizeof" "(" typename ")"
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
        if (equal(*token, "(") && is_typename((*token)->next)) {
            // "(" typename ")"
            expect(token, "(");
            Type *type = typename(token);
            expect(token, ")");
            return new_node_num(sizeof_type(type));
        } else {
            // "sizeof" unary
            Node *node = unary(token);
            add_type(node);
            if (!node->type) {
                error_parse(*token, "expected expression\n");
            }
            return new_node_num(sizeof_type(node->type));
        }
    } else {
        return postfix(token);
    }
}

// mul = unary ("*" unary | "/" unary | "%" unary) *
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
        if (consume(token, "%")) {
            node = new_node(NdMod, node, unary(token));
            continue;
        }
        break;
    }

    return node;
}

Node *new_add(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    if (!is_integer(rhs->type)) {
        Node *tmp = rhs;
        rhs = lhs;
        lhs = tmp;
    }
    if (is_integer(lhs->type) && is_integer(rhs->type)) {
        return new_node(NdAdd, lhs, rhs);
    } else if (is_integer(rhs->type) && is_pointer(lhs->type)) {
        int size = ptr_to_size(lhs->type);
        rhs = new_node(NdMul, rhs, new_node_num(size));
        return new_node(NdAdd, lhs , rhs);
    } else {
        error_type("type error : cannot add this two type\n");
        return NULL;
    }
}

Node *new_sub(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);


    if (is_integer(lhs->type) && is_integer(rhs->type)) {
        return new_node(NdSub, lhs, rhs);
    } else if (is_pointer(lhs->type) && is_integer(rhs->type)) {
        int size = ptr_to_size(lhs->type);
        rhs = new_node(NdMul, rhs, new_node_num(size));
        return new_node(NdSub, lhs , rhs);
    } else if (is_pointer(lhs->type) && is_pointer(rhs->type)) {
        int size = ptr_to_size(lhs->type);
        Node *node = new_node(NdSub, lhs, rhs);
        return new_node(NdDiv, node, new_node_num(size));
    } else {
        error_type("type error : cannot sub this two type\n");
        return NULL;
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

// shift = add ("<<" add | ">>" add)*
Node *shift(Token **token) {
    Node *node = add(token);

    while (1) {
        if (consume(token, "<<")) {
            node = new_node(NdShl, node, add(token));
            continue;
        }
        if (consume(token, ">>")) {
            node = new_node(NdSar, node, add(token));
            continue;
        }
        break;
    }

    return node;
}

// relational = shift ( "<" shift | ">" shift | "<=" shift | ">=" shift)*
Node *relational (Token **token) {
    Node *node = shift(token);

    while(1) {
        if (consume(token, "<")) {
            node = new_node(NdLt, node, shift(token));
            continue;
        }
        if (consume(token, ">")) {
            node = new_node(NdLt, shift(token), node);
            continue;
        }
        if (consume(token, "<=")) {
            node = new_node(NdLe, node, shift(token));
            continue;
        }
        if (consume(token, ">=")) {
            node = new_node(NdLe, shift(token), node);
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

// expr = assign (, assing)*
Node *expr(Token **token) {
    Node *node = assign(token);
    while (consume(token, ",")) {
        node = new_node(NdComma, node, assign(token));
    }
    return node;
}

// stmt =   expr? ";" 
//        | "return" expr? ";"
//        | "if" "(" expr ")" stmt ("else" stmt) ?
//        | "while" "(" expr ")" stmt 
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | "{" compound_stmt
Node *stmt(Token **token) {
    if (consume_keyword(token, "return")) {
        if (consume(token, ";")) {
            Node *node = new_node(NdReturn, NULL, NULL);
            return node;
        } else {
            Node *node = new_node(NdReturn, expr(token), NULL);
            expect(token, ";");
            return node;
        }
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

    // block scope
    Scope *new_locals = next_locals();

    while(!consume(token, "}")) {
        if (is_typename(*token)) {
            cur->next = declaration(token, false);
            cur = cur->next;
        } else {
            cur->next = stmt(token);
            cur = cur->next;
        }
    }

    // block scope
    prev_locals(new_locals);

    return head.next;
}

void add_param_to_locals(Obj *param) {
    if (param->next) {
        add_param_to_locals(param->next);
        add_lvar(param);
    } else {
        add_lvar(param);
    }
    return;
}

// params = declspce declarator ("," declspec declarator) * 
Obj *params(Token **token) {
    Type *type = declspec(token);
    Obj head;
    Obj *cur = &head;
    cur->next = declarator(token, type);
    cur = cur->next;

    while (consume(token, ",")) {
        Type *ty = declspec(token);
        cur->next = declarator(token, ty);
        cur = cur->next;
    }

    for (Obj *arg = head.next;arg;arg = arg->next) {
        // implicit array to ptr type coversion
        if (arg->type->kind == TyArray) {
            arg->type->kind = TyPtr;
        }
    }

    add_param_to_locals(head.next);
    return locals->objs;
}

// typename = typespec
Type *typename(Token **token) {
    return typespec(token);
}

// typespec =   "void"
//            | "long"
//            | "int" 
//            | "short"
//            | "char" 
//            | "struct" ident ("{" struct_union_declaration "}")? 
//            | "union" ident  ("{" struct_union_declaration "}")?
Type *typespec(Token **token) {
    if (consume_keyword(token, "void")) {
        return new_type(TyVoid);
    }
    if (consume_keyword(token, "long")) {
        return new_type(TyLong);
    }
    if (consume_keyword(token, "int")) {
        return new_type(TyInt);
    }
    if (consume_keyword(token, "short")) {
        return new_type(TyShort);
    } 
    if (consume_keyword(token, "char")) {
        return new_type(TyChar);
    }
    if (consume_keyword(token, "struct")) {
        Token *token_ident = expect_ident(token);
        if (consume(token, "{")) {
            struct_union_declaration(token, token_ident, false);
            expect(token, "}");
        }
        Struct *s = find_struct(token_ident);
        if (!s) {
            error_parse(*token, "undefined type");
        } 
        return new_type_struct(s);
    }
    expect_keyword(token, "union");
    Token *token_ident = expect_ident(token);
    if (consume(token, "{")) {
        struct_union_declaration(token, token_ident, true);
        expect(token, "}");
    }
    Struct *s = find_struct(token_ident);
    if (!s) {
        error_parse(*token, "undefined type");
    } 
    return new_type_struct(s);
}

// declspec = typespec
Type *declspec(Token **token) {
    return typespec(token);
}

// direct_decl =   ident
//               | ident ("[" number "]")* 
//               | ident ("(" params? ")")?
Obj *direct_decl(Token **token, Type *type) {
    Token *token_ident = expect_ident(token);

    // function
    if (consume(token, "(")) {
        Obj *param = NULL;
        if (!consume(token, ")")) {
            param = params(token);
            expect(token, ")");
        }
        Obj *fn = new_fun(token_ident, type, param);
        add_gvar(fn);
        return fn;
    }

    // obj
    while (consume(token, "[")) {
        int size = expect_number(token);
        type = new_type_array(type, size);
        expect(token, "]");
    }

    return new_obj(token_ident, type);
}

// initializer = assign
//               "{" (initializer ",")* initializer? "}"
Node *initializer(Token **token) {
    if (consume(token, "{")) {
        Node head;
        head.next = NULL;
        Node *cur = &head;

        int i = 0;
        while(!consume(token, "}")) {
            if (i++ > 0) {
                expect(token, ",");
            }
            cur->next = initializer(token);
            cur = cur->next;
        }

        Node *node = new_node(NdInit, NULL, NULL);
        node->body = head.next;
        return node;
    } else {
        return assign(token);
    }
}

Node *init_assign(Node *var, Node *init_value) {
    add_type(var);
    add_type(init_value);
    switch (init_value->kind) {
    case NdInit : {
        if (var->type->kind == TyArray) {
            Node head;
            head.next = NULL;
            Node *cur = &head;
            int i = 0;
            for (Node *nd = init_value->body;nd;nd = nd->next) {
                Node *var_i = new_node(NdDeref, new_add(var, new_node_num(i++)), NULL);
                cur->next = init_assign(var_i, nd);
                cur = cur->next;
            }
            Node *node = new_node(NdBlock, NULL, NULL);
            node->body = head.next;
            return node;
        } else {
            return init_assign(var, init_value->body);
        }
    }
    case NdGvar : {
        // char *s = "abc"; -> "abc" Global variable
        // char s[4] = "abc"; -> "abc" initial variable
        // In later case, I have to initialize s by "abc"
        if (var->type->kind == TyArray && var->type->ptr_to->kind == TyChar) {
            return init_assign(var, init_value->obj->init);
        }
    }
    default : 
        return new_node(NdAssign, var, init_value);
    }
}

// declarator = "*"* direct_decl
Obj *declarator(Token **token, Type *type) {
    while (consume(token, "*")) {
        type = new_type_ptr(type);
    }
    return direct_decl(token, type);
}

// declaration = declspec declarator ("=" initializer)? ("," declarator ("=" initializer)? )* ";"
Node *declaration(Token **token, bool is_global) {
    Type *base_type = declspec(token);

    Node head;
    Node *cur = &head;

    int i = 0;
    while(!consume(token, ";")) {
        if (i++ > 0) {
            expect(token, ",");
        }

        Obj *obj = declarator(token, base_type);
        add_lvar(obj);
        Node *var = new_node_obj(obj);
        if (consume(token, "=")) {
            cur->next = init_assign(var, initializer(token));
            cur = cur->next;
        } else {
            cur->next = var;
            cur = cur->next;
        }
    
    }

    Node *node = new_node(NdBlock, NULL, NULL);
    node->body = head.next;
    return node;
}

// struct_union_declaration = (declspce declarator ";")* 
Struct *struct_union_declaration(Token **token, Token *token_ident, bool is_union) {
    Obj head;
    Obj *cur = &head;

    while (1) {
        if (is_typename(*token)) {
            Type *type = declspec(token);
            cur->next = declarator(token, type);
            cur = cur->next;
            expect(token, ";");
            continue;
        }
        break;
    }

    if (is_union) {
        return new_union(token_ident, head.next);
    } else {
        return new_struct(token_ident, head.next);
    }
}

void allocate_stack_offset(Obj *func) {
    int stack_size = 0;
    for (Scope *scope = func->locals;scope;scope = scope->prev) {
        for (Obj *obj = scope->objs;obj;obj = obj->next) {
            int size = sizeof_type(obj->type);
            stack_size = align_to(stack_size, alignment(obj->type));
            stack_size += size;
            obj->offset = stack_size;
        }
    }
    func->stack_size = align_to(stack_size, 16);
    return;
}

// definition = declspec declarator "{" compound_stmt 
//              declspec declarator ("=" initializer)? ";"
//              declspec ";"
void def(Token **token) {
    Type *type = declspec(token);
    if (consume(token, ";")) {
        return;
    }
    Obj *fn = declarator(token, type);

    if (is_function(fn)) {
        if (consume(token, ";")) {
            locals->prev = fun_locals;
            fun_locals = locals;
            fn->locals = fun_locals;
            fn->is_defined = false;
        } else {
            // body
            expect(token, "{");
            Node *node = compound_stmt(token);

            // add type
            for (Node *cur = node;cur;cur = cur->next) {
                add_type(cur);
            }

            locals->prev = fun_locals;
            fun_locals = locals;

            fn->body = node;
            fn->locals = fun_locals;
            fn->is_defined = true;
            allocate_stack_offset(fn);
        }
    } else {
        add_gvar(fn);
        // global variable initialization
        if (consume(token, "=")) {
            fn->init = initializer(token);
        } else {
            fn->init = zeros_like(fn->type);
        }
        expect(token, ";");
    }

    
    locals = &(Scope){};
    fun_locals = &(Scope){};
    return;
}

// program = def *
Obj *program(Token **token) {

    while (!at_eof(*token)) {
        def(token);
    }

    return globals;
}



Obj *parse(Token **token) {
    return program(token);
}