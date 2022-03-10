#include "kcc.h"

Struct *defined_structs;
Enum *defined_enums;
Typdef *defined_typdefs;
Scope *locals = &(Scope){};
Scope *fun_locals = &(Scope){};
Obj *globals;

Node *const_eval(Node *node);

// new scope
Scope *next_locals();
void prev_locals(Scope *new_locals);

// find 
Obj *find_lvar(Token *token, bool redeclaration);
Obj *find_gvar(Token *token);
Obj *find_obj(Token *token, bool redeclaration);
Struct *find_struct(Token *token);
Obj *find_member(Struct *st, Token *token);
Enum *find_enum(Token *token);
Obj *find_enum_const(Token *token);
Type *find_typdef(Token *token);

// new obj
void add_lvar(Obj *obj);
void add_gvar(Obj *obj);
Obj *new_obj(Token *token, Type *type);
Obj *new_fun(Token *token, Type *return_ty, Obj *params, Type *params_ty);
Obj *new_string(Token *token);
Struct *new_struct_or_union(Token *token_ident);
void add_member_struct(Struct *st, Obj *member);
void add_member_union(Struct *st, Obj *member);
Enum *new_enum(Token *token);
void add_typdef(Obj *obj);

// new node
Node *new_node_num(int val);
Node *new_node_lvar(Obj *obj);
Node *new_node_gvar(Obj *obj);
Node *new_node_obj(Obj *obj);
Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_unary(NodeKind kind, Node *lhs);
Node *new_node(NodeKind kind);

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
Node *const_expr(Token **token);
Node *expr(Token **token);

// stmt
Node *stmt(Token **token);
Node *compound_stmt(Token **token);

// initialize
Node *initializer(Token **token);
Node *init_assign(Node *var, Node *init_value);

// declare
Struct *struct_spec(Token **token);
void struct_union_declaration(Token **token, Struct *st, bool is_union);
Enum *enum_spec(Token **token);
void enum_list(Token **token, Enum *enm);
Type *typename(Token **token);
Type *typespec(Token **token);
Type *params(Token **token);
Type *declspec(Token **token);
Obj *direct_declarator(Token **token, Type *type);
Obj *declarator(Token **token, Type *type);
Node *declaration(Token **token, bool is_global);
void def(Token **token);

bool equal(Token *token, char *s) {
    return (memcmp(token->str, s, token->len) == 0) && s[token->len] == '\0';
}

bool at_eof(Token *token) {
    return token->kind == TkEof;
}

int expect_number(Token **token) {
    if ((*token)->kind != TkNum) error_at((*token)->str, "expect number");
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
    if ((*token)->kind != TkReserved || !equal(*token, op)) error_at((*token)->str, "expect %s", op);
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
        error_at((*token)->str, "expect ident");
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
    if ((*token)->kind != TkKeyword || !equal(*token, keyword)) error_at((*token)->str, "expect keyword %s", keyword);
    *token = (*token)->next;
    return;
}

Type *consume_typdef(Token **token) {
    Type *type = find_typdef(*token);
    if (!type) {
        return NULL;
    }
    *token = (*token)->next;
    return type;
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

Obj *find_lvar(Token *token, bool redeclaration) {
    char *name = token->str;
    int len = token->len;
    for (Obj *obj = locals->objs;obj; obj = obj->next) {
        if (len == obj->len && strncmp(name, obj->name, len) == 0) {
            if (redeclaration) {
                error_at(token->str, "redeclaration");
            }
            return obj;
        }
    }
    for (Scope *scope = locals->prev;scope; scope = scope->prev) {
        for (Obj *obj = scope->objs;obj; obj = obj->next  ) {
            if (len == obj->len && strncmp(name, obj->name, len) == 0) {
                return obj;
            }
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

Obj *find_obj(Token *token, bool redeclaration) {
    Obj *lvar = find_lvar(token, redeclaration);
    if (lvar) {
        return lvar;
    }
    Obj *gvar = find_gvar(token);
    if (gvar) {
        return gvar;
    }
    return NULL;
}

Struct *find_struct(Token *token) {
    for (Struct *s = defined_structs;s;s = s->next) {
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
    error_at(token->str, "this struct has no member of this name\n");
    return NULL;
}

Enum *find_enum(Token *token) {
    for (Enum *enm = defined_enums;enm;enm = enm->next) {
        if (enm->name_len == token->len && strncmp(enm->name, token->str, token->len) == 0) {
            return enm;
        }
    }
    return NULL;
}

Obj *find_enum_const(Token *token) {
    for (Enum *enm = defined_enums;enm;enm = enm->next) {
        for (Obj *obj = enm->enum_list;obj;obj = obj->next) {
            if (obj->len == token->len && strncmp(obj->name, token->str, token->len) == 0) {
                return obj;
            }
        }
    }
    error_at(token->str, "undefinesd variable\n");
    return NULL;
}

Type *find_typdef(Token *token) {
    for (Typdef *typdef = defined_typdefs;typdef;typdef = typdef->next) {
        if (typdef->name_len == token->len && strncmp(typdef->name, token->str, token->len) == 0) {
            return typdef->type;
        }
    }
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
        error_at(token->str, "cannot define void object");
    }
    Obj *obj = calloc(1, sizeof(Obj));
    obj->name = token->str;
    obj->len = token->len;
    obj->type = type;
    return obj;
}

Obj *new_fun(Token *token, Type *return_ty, Obj *params, Type *params_ty) {
    Obj *fn = calloc(1, sizeof(Obj));
    fn->len = token->len;
    fn->name = token->str;
    fn->args = params;
    fn->type = new_type_fun(return_ty, params_ty);
    return fn;
}

char *unique_str_name() {
    int cnt = counter();
    char *name = calloc(10, sizeof(char));
    int len = sprintf(name, ".LC%d.str", cnt);
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

    obj->init = new_node(NdInit);
    obj->init->body = head.next;
    return obj;
}

Struct *new_struct_or_union(Token *token_ident) {
    Struct *st = calloc(1, sizeof(Struct));
    st->name = token_ident->str;
    st->name_len = token_ident->len;
    st->align = 1;

    st->next = defined_structs;
    defined_structs = st;
    return st;
}

void add_member_struct(Struct *st, Obj *member) {
    st->member = member;
    
    // calculate offset, align, size
    int offset = 0;
    for (Obj *obj = st->member;obj;obj = obj->next) {
        int align = alignment(obj->type);
        offset = align_to(offset, align);
        obj->offset = offset;
        offset += sizeof_type(obj->type);
        if (st->align < align) {
            st->align = align;
        }
    }
    st->size = offset;

    return;
}

void add_member_union(Struct *uni, Obj *member) {
    uni->member = member;

    // calculate offset, align, size
    for (Obj *obj = uni->member;obj;obj = obj->next) {
        int align = alignment(obj->type);
        int size = sizeof_type(obj->type);
        if (uni->align < align) {
            uni->align = align;
        }
        if (uni->size < size) {
            uni->size = size;
        }
        obj->offset = 0;
    }

    return;
}

Enum *new_enum(Token *token) {
    Enum *enm = calloc(1, sizeof(Enum));
    enm->name = token->str;
    enm->name_len = token->len;

    enm->next = defined_enums;
    defined_enums = enm;
    return enm;
}

void add_typdef(Obj *obj) {
    Typdef *typdef = calloc(1, sizeof(Typdef));
    typdef->name = obj->name;
    typdef->name_len = obj->len;
    typdef->type = obj->type;

    typdef->next = defined_typdefs;
    defined_typdefs = typdef;
    return;
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
    return node;
}

Node *new_node_gvar(Obj *obj) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = NdGvar;
    node->obj = obj;
    return node;
}

Node *new_node_obj(Obj *obj) {
    if (obj->is_global) {
        return new_node_gvar(obj);
    } else {
        return new_node_lvar(obj);
    }
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_unary(NodeKind kind, Node *lhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    return node;
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

bool is_typename(Token *token) {
    char *typenames[] = {"void", "long", "int", "char", "short", "struct", "union", "enum"};
    for (int i = 0;i < sizeof(typenames) / sizeof(*typenames); i++) {
        if (equal(token, typenames[i])) {
            return true;
        }
    }
    if (find_typdef(token) && !find_obj(token, false)) {
        return true;
    }
    return false;
}

Node *const_eval(Node *node) {
    switch (node->kind) {
    case NdNum:
        return node;
    case NdAdd: {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val + rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdSub: {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val - rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdMul: {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val * rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdDiv : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            if (rhs->val == 0) {
                error("zero division error");
            }
            return new_node_num(lhs->val / rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdMod : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            if (rhs->val == 0) {
                error("zero division error");
            }
            return new_node_num(lhs->val % rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdEq : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val == rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdNeq : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val != rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdLt : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val < rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdLe : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val <= rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdShl : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val << rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdSar : {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        add_type(lhs);
        add_type(rhs);
        if (is_integer(lhs->type) && is_integer(rhs->type)) {
            return new_node_num(lhs->val >> rhs->val);
        } else {
            error("not a compile time constant");
        }
        break;
    }
    case NdComma: {
        Node *lhs = const_eval(node->lhs);
        Node *rhs = const_eval(node->rhs);
        return rhs;
    }
    default: {
        error("not a compile time constant");
    }
    }
    return NULL;
}

Node *search_case_node(Node *node) {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    switch (node->kind) {
    case NdCase:
        cur->next = node;
        cur = cur->next;
        break;
    case NdBlock:
        for (Node *nd = node->body;nd;nd = nd->next) {
            if (nd->kind == NdCase) {
                cur->next = nd;
                cur = cur->next;
            }
        }
        break;
    default:
        break;
    }

    return head.next;
}

void assign_case_label(Node *node) {
    for (Node *nd = node->cases;nd;nd = nd->next) {
        char *s = calloc(10, sizeof(char));
        int cnt = counter();
        sprintf(s, ".L%d", cnt);
        nd->label = s;
    }
    return;
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
            Node *node = new_node(NdStmtExpr);
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
            Node *node = new_node(NdFuncall);
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
        Obj *obj = find_obj(token_ident, false); // redeclaration = false
        if (!obj) {
            obj = find_enum_const(token_ident);
            return new_node_num(obj->enum_value);
        }
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

// postfix = primary ( 
//                      "[" expr "]"     | 
//                      "(" argument ")" | 
//                      "." ident        |
//                      "->" ident 
//                    )*
//                    ("++" | "--")?
Node *postfix(Token **token) {
    Node *node = primary(token);
    while (1) {
        if (consume(token, "[")) {
            Node *index = expr(token);
            node = new_add(node, index);
            node = new_node_unary(NdDeref, node);
            expect(token, "]");
            continue;
        }
        if (consume(token, ".")) {
            add_type(node);
            Struct *st = node->type->type_struct;
            Token *member = expect_ident(token);
            node = new_node_unary(NdMember, node);
            node->member = find_member(st, member);
            continue;
        }
        if (consume(token, "->")) {
            node = new_node_unary(NdDeref, node);
            add_type(node);
            Struct *st = node->type->type_struct;
            Token *member = expect_ident(token);
            node = new_node_unary(NdMember, node);
            node->member = find_member(st, member);
            continue;
        }
        if (consume(token, "(")) {
            error_at((*token)->str, "function pointer is not supported now\n");
            continue;
        }

        break;
    }

    if (consume(token, "++")) {
        node = new_node_unary(NdPostInc, node);
    } else if (consume(token, "--")) {
        node = new_node_unary(NdPostDec, node);
    }

    return node;
}

// unary =   ("+" | "-" | "*" | "&") unary
//         | ("++" | "--") unary
//         | "sizeof" "(" typename ")"
//         | "sizeof" unary
//         | postfix
Node *unary(Token **token) {
    if (consume(token, "+")) {
        return unary(token);
    } else if (consume(token, "-")) {
        return new_node_binary(NdSub, new_node_num(0), unary(token));
    } else if (consume(token, "*")) {
        return new_node_unary(NdDeref, unary(token));
    } else if (consume(token, "&")) {
        return new_node_unary(NdRef, unary(token));
    } else if (consume(token, "++")) {
        Node *node = unary(token);
        return new_node_binary(NdAssign, node, new_add(node, new_node_num(1)));
    } else if (consume(token, "--")) {
        Node *node = unary(token);
        return new_node_binary(NdAssign, node, new_sub(node, new_node_num(1)));
    } else if (consume_keyword(token, "sizeof")) {
        if (equal(*token, "(") && is_typename((*token)->next)) {
            // "(" typename ")"
            expect(token, "(");
            Type *type = typename(token);
            expect(token, ")");
            Node *node = new_node_num(sizeof_type(type));
            node->type = new_type(TyLong);
            return node;
        } else {
            // "sizeof" unary
            Node *node = unary(token);
            add_type(node);
            node = new_node_num(sizeof_type(node->type));
            node->type = new_type(TyLong);
            return node;
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
            node = new_node_binary(NdMul, node, unary(token));
            continue;
        }
        if (consume(token, "/")) {
            node = new_node_binary(NdDiv, node, unary(token));
            continue;
        }
        if (consume(token, "%")) {
            node = new_node_binary(NdMod, node, unary(token));
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
        return new_node_binary(NdAdd, lhs, rhs);
    } else if (is_integer(rhs->type) && is_pointer(lhs->type)) {
        int size = ptr_to_size(lhs->type);
        rhs = new_node_binary(NdMul, rhs, new_node_num(size));
        return new_node_binary(NdAdd, lhs , rhs);
    } else {
        error("type error : cannot add this two type\n");
        return NULL;
    }
}

Node *new_sub(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);


    if (is_integer(lhs->type) && is_integer(rhs->type)) {
        return new_node_binary(NdSub, lhs, rhs);
    } else if (is_pointer(lhs->type) && is_integer(rhs->type)) {
        int size = ptr_to_size(lhs->type);
        rhs = new_node_binary(NdMul, rhs, new_node_num(size));
        return new_node_binary(NdSub, lhs , rhs);
    } else if (is_pointer(lhs->type) && is_pointer(rhs->type)) {
        int size = ptr_to_size(lhs->type);
        Node *node = new_node_binary(NdSub, lhs, rhs);
        return new_node_binary(NdDiv, node, new_node_num(size));
    } else {
        error("type error : cannot sub this two type\n");
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
            node = new_node_binary(NdShl, node, add(token));
            continue;
        }
        if (consume(token, ">>")) {
            node = new_node_binary(NdSar, node, add(token));
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
            node = new_node_binary(NdLt, node, shift(token));
            continue;
        }
        if (consume(token, ">")) {
            node = new_node_binary(NdLt, shift(token), node);
            continue;
        }
        if (consume(token, "<=")) {
            node = new_node_binary(NdLe, node, shift(token));
            continue;
        }
        if (consume(token, ">=")) {
            node = new_node_binary(NdLe, shift(token), node);
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
            node = new_node_binary(NdEq, node, relational(token));
            continue;
        } 
        if (consume(token, "!=")) {
            node = new_node_binary(NdNeq, node, relational(token));
            continue;
        }
        break;
    }

    return node;
}

// assign = equality ("="  assign | 
//                    "+=" assign | 
//                    "-=" assign | 
//                    "*=" assign | 
//                    "/=" assign | 
//                    "%=" assign | 
//                    "<<=" assign | 
//                    ">>=" assign)?
Node *assign(Token **token) {
    Node *node = equality(token);
    
    if (consume(token, "=")) {
        node = new_node_binary(NdAssign, node, assign(token));
    }
    if (consume(token, "+=")) {
        node = new_node_binary(NdAssign, node,  new_add(node, assign(token)));
    }
    if (consume(token, "-=")) {
        node = new_node_binary(NdAssign, node,  new_sub(node, assign(token)));
    }
    if (consume(token, "*=")) {
        node = new_node_binary(NdAssign, node,  new_node_binary(NdMul, node, assign(token)));
    }
    if (consume(token, "/=")) {
        node = new_node_binary(NdAssign, node,  new_node_binary(NdDiv, node, assign(token)));
    }
    if (consume(token, "%=")) {
        node = new_node_binary(NdAssign, node,  new_node_binary(NdMod, node, assign(token)));
    }
    if (consume(token, "<<=")) {
        node = new_node_binary(NdAssign, node,  new_node_binary(NdShl, node, assign(token)));
    }
    if (consume(token, ">>=")) {
        node = new_node_binary(NdAssign, node,  new_node_binary(NdSar, node, assign(token)));
    }


    return node;
}

// const_expr = equality
Node *const_expr(Token **token) {
    return equality(token);
}

// expr = assign (, assign)*
Node *expr(Token **token) {
    Node *node = assign(token);
    while (consume(token, ",")) {
        node = new_node_binary(NdComma, node, assign(token));
    }
    return node;
}

// stmt =   expr? ";" 
//        | "return" expr? ";"
//        | "if" "(" expr ")" stmt ("else" stmt) ?
//        | "while" "(" expr ")" stmt 
//        | "do" stmt "while" "(" expr ")"
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | "switch" "(" expr ")" stmt
//        | "case" const_expr ":" stmt
//        | "{" compound_stmt
Node *stmt(Token **token) {
    if (consume_keyword(token, "return")) {
        if (consume(token, ";")) {
            Node *node = new_node(NdReturn);
            return node;
        } else {
            Node *node = new_node_unary(NdReturn, expr(token));
            expect(token, ";");
            return node;
        }
    }
    if (consume_keyword(token, "if")) {
        expect(token, "(");
        Node *node = new_node(NdIf);
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
        Node *node = new_node(NdFor);
        node->cond = expr(token);
        expect(token, ")");
        node->then = stmt(token);
        return node;
    }
    if (consume_keyword(token, "do")) {
        Node *node = new_node(NdDoWhile);
        node->then = stmt(token);
        expect_keyword(token, "while");
        expect(token, "(");
        node->cond = expr(token);
        expect(token, ")");
        return node;
    }
    if (consume_keyword(token, "for")) {
        expect(token, "(");
        Node *node = new_node(NdFor);
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
    if (consume_keyword(token, "switch")) {
        Node *node = new_node(NdSwitch);
        expect(token, "(");
        node->cond = expr(token);
        expect(token, ")");
        node->then = stmt(token);
        node->cases = search_case_node(node->then);
        assign_case_label(node);
        return node;
    }
    if (consume_keyword(token, "case")) {
        Node *node = new_node(NdCase);
        node->cond = const_expr(token);
        node->cond = const_eval(node->cond);
        expect(token, ":");
        node->then = stmt(token);
        return node;
    }
    if (consume(token, "{")) {
        Node *node = new_node(NdBlock);
        node->body = compound_stmt(token);;
        return node;
    }
    if (consume(token, ";")) {
        Node *node = new_node(NdBlock);
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
        if (is_typename(*token) || equal(*token, "typedef")) {
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
Type *params(Token **token) {
    Type *type = declspec(token);

    Obj head;
    Obj *cur = &head;
    Type head_ty;
    Type *cur_ty = &head_ty;
    
    cur->next = declarator(token, type);
    cur_ty->next = cur->next->type;
    cur = cur->next;
    cur_ty = cur_ty->next;

    while (consume(token, ",")) {
        Type *ty = declspec(token);

        cur->next = declarator(token, ty);
        cur_ty->next = cur->next->type;
        cur = cur->next;
        cur_ty = cur_ty->next;

    }

    for (Obj *arg = head.next;arg;arg = arg->next) {
        // implicit array to ptr type coversion
        if (arg->type->kind == TyArray) {
            arg->type->kind = TyPtr;
        }
    }

    add_param_to_locals(head.next);
    return head_ty.next;
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

        Node *node = new_node(NdInit);
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
                Node *var_i = new_node_unary(NdDeref, new_add(var, new_node_num(i++)));
                cur->next = init_assign(var_i, nd);
                cur = cur->next;
            }
            Node *node = new_node(NdBlock);
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
        return new_node_binary(NdAssign, var, init_value);
    }
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
//            | struct_spec
//            | enum_spec
//            | typedef_name
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
    if (equal(*token, "struct") || equal(*token, "union")) {
        return new_type_struct(struct_spec(token));
    }
    if (equal(*token, "enum")) {
        enum_spec(token);
        return new_type(TyInt);
    }
    Type *type = consume_typdef(token);
    if (!type) {
        error_at((*token)->str, "unknown type name");
    }
    return type;
}

// struct_spec = "struct" ident ("{" struct_union_declaration "}")? 
//               "union"  ident ("{" struct_union_declaration "}")? 
Struct *struct_spec(Token **token) {
    if (consume_keyword(token, "struct")) {
        Token *token_ident = expect_ident(token);
        Struct *st = find_struct(token_ident);
        if (!st) {
            st = new_struct_or_union(token_ident);
        } 
        if (consume(token, "{")) {
            struct_union_declaration(token, st, false);
            expect(token, "}");
        }
        return st;
    }
    if (consume_keyword(token, "union")) {
        Token *token_ident = expect_ident(token);
        Struct *uni = find_struct(token_ident);
        if (!uni) {
            uni = new_struct_or_union(token_ident);
        } 
        if (consume(token, "{")) {
            struct_union_declaration(token, uni, true);
            expect(token, "}");
        }
        return uni;
    }
    error_at((*token)->str, "this is not sturct or union type");
    return NULL;
}


// struct_union_declaration = (declspce declarator ";")* 
void struct_union_declaration(Token **token, Struct *st, bool is_union) {
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
        add_member_union(st, head.next);
    } else {
        add_member_struct(st, head.next);
    }
}


// enum_spec = "enum" ident ("(" enum_list ")")?
Enum *enum_spec(Token **token) {
    expect_keyword(token, "enum");
    Token *token_ident = expect_ident(token);
    Enum *enm = find_enum(token_ident);
    if (!enm) {
        enm = new_enum(token_ident);
    }
    if (consume(token, "{")) {
        enum_list(token, enm);
        expect(token, "}");
    }
    return enm;
}

// enum_list = ident ("," ident)* ","?
void enum_list(Token **token, Enum *enm) {
    int value = 0;
    Obj head;
    Obj *cur = &head;

    do {
        Token *token_ident = consume_ident(token);
        if (!token_ident) {
            break;
        }

        Obj *obj = new_obj(token_ident, new_type(TyInt));
        if (consume(token, "=")) {
            Node *node = const_expr(token);
            node = const_eval(node);
            value = node->val;
        }
        obj->enum_value = value++;

        cur->next = obj;
        cur = cur->next;
    } while (consume(token, ","));

    enm->enum_list = head.next;
    return;
}

// declspec = (typespec | "typedef")*
Type *declspec(Token **token) {
    Type *type = NULL;
    bool is_typdef = false;

    while (1) {
        if (consume_keyword(token, "typedef")) {
            is_typdef = true;
            continue;
        }
        if (is_typename(*token)) {
            if (find_typdef(*token) && type) {
                break;
            } else if (type) {
                error_at((*token)->str, "too many type name");
            }
            type = typespec(token);
            continue;
        }
        break;
    }

    if (!type) {
        error_at((*token)->str, "empty type name");
    }
    type->is_typdef = is_typdef;

    return type;
}

// direct_declarator =   ident
//                     | ident ("[" number "]")* 
//                     | ident "(" params? ")"?
//                     | "(" declarator ")"
//                     | "(" declarator ")" ("[" number "]")*
//                     | "(" declarator ")" "(" params? ")"*
Obj *direct_declarator(Token **token, Type *type) {
    if (consume(token, "(")) {
        Obj *obj = declarator(token, new_type(TyAbsent));
        expect(token, ")");

        // function
        if (consume(token, "(")) {
            Obj *param = NULL;
            Type *param_ty = NULL;
            if (!consume(token, ")")) {
                param_ty = params(token);
                param = locals->objs;
                expect(token, ")");
            }
            Type *ty = new_type_fun(type, param_ty);
            obj->type = fill_absent_type(obj->type, ty);
            return obj;
        }

        // obj
        while (consume(token, "[")) {
            int size = expect_number(token);
            type = new_type_array(type, size);
            expect(token, "]");
        }

        obj->type = fill_absent_type(obj->type, type);
        return obj;
    } else {
        Token *token_ident = expect_ident(token);

        // function
        if (consume(token, "(")) {
            Obj *param = NULL;
            Type *param_ty = NULL;
            if (!consume(token, ")")) {
                param_ty = params(token);
                param = locals->objs;
                expect(token, ")");
            }
            Obj *fn = new_fun(token_ident, type, param, param_ty);
            return fn;
        }

        // obj
        if (!type->is_typdef) {
            find_obj(token_ident, true); // redeclaration = true
        }
        while (consume(token, "[")) {
            int size = expect_number(token);
            type = new_type_array(type, size);
            expect(token, "]");
        }

        return new_obj(token_ident, type);
    }
}

// declarator = "*"* direct_declarator
Obj *declarator(Token **token, Type *type) {
    while (consume(token, "*")) {
        type = new_type_ptr(type);
    }
    return direct_declarator(token, type);
}

// declaration = declspec declarator ("=" initializer)? ("," declarator ("=" initializer)? )* ";"
Node *declaration(Token **token, bool is_global) {
    Type *base_type = declspec(token);

    Node head;
    head.next = NULL;
    Node *cur = &head;

    int i = 0;
    while(!consume(token, ";")) {
        if (i++ > 0) {
            expect(token, ",");
        }

        Obj *obj = declarator(token, base_type);

        if (base_type->is_typdef) {
            add_typdef(obj);
        } else {
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
    
    }
    
    Node *node = new_node(NdBlock);
    node->body = head.next;
    return node;
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
        add_gvar(fn);
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
    } else if (type->is_typdef) {
        add_typdef(fn);
        expect(token, ";");
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