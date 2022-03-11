#include "kcc.h"

void display_space(int size);
void display_token(Token *token);
void display_type(Type *type);
void display_lvar(Obj *obj);
void display_gvar(Obj *obj);
void display_obj(Obj *obj);
void display_node(Node *node, int indent);
void display_child(Node *node, int indent);
void display_function(Obj *func);

void display_token(Token *token) {
    for (Token *cur = token;cur;cur = cur->next) {
        char s[cur->len + 1];
        s[cur->len] = '\0';
        switch (cur->kind) {
        case TkReserved:
            strncpy(s, cur->str, cur->len);
            fprintf(stderr, "Reserved : %s\n", s);
            break;
        case TkNum:
            fprintf(stderr, "Num : %d\n", cur->val);
            break;
        case TkIdent:
            strncpy(s, cur->str, cur->len);
            fprintf(stderr, "Ident : %s\n", s);
            break;
        case TkKeyword:
            strncpy(s, cur->str, cur->len);
            fprintf(stderr, "Keyword : %s\n", s);
            break;
        case TkString:
            strncpy(s, cur->str, cur->len);
            fprintf(stderr, "String : %s\n", s);
            break;
        case TkEof:
            fprintf(stderr, "eof\n");
            break;
        }
    }
    return;
}

void display_type_internal(Type *type) {
    if (!type) return;
    switch (type->kind) {
    case TyAbsent:
        fprintf(stderr, " absent");
        return;
    case TyVoid:
        fprintf(stderr, " void");
        return;
    case TyLong:
        fprintf(stderr, " long");
        return;   
    case TyInt:
        fprintf(stderr, " int");
        return;
    case TyShort:
        fprintf(stderr, " short");
        return;
    case TyChar:
        fprintf(stderr, " char");
        return;
    case TyArray:
        fprintf(stderr, " array of");
        display_type_internal(type->ptr_to);
        return;
    case TyPtr:
        fprintf(stderr, " ptr to");
        display_type_internal(type->ptr_to);
        return;
    case TyStruct:
        fprintf(stderr, " struct");
        return;
    case TyFunc:
        fprintf(stderr, " func(");
        for (Type *ty = type->params;ty;ty = ty->next) {
            display_type_internal(ty);
            if (ty->next) {
                fprintf(stderr, ",");
            }
        }
        fprintf(stderr, ")");
        display_type_internal(type->return_ty);
        return;
    }
}

void display_type(Type *type) {
    if (!type) return;
    fprintf(stderr, " { type : ");
    display_type_internal(type);
    fprintf(stderr, " }");
    return;
}

void display_lvar(Obj *obj) {
    char s[obj->len + 1];
    strncpy(s, obj->name, obj->len);
    s[obj->len] = '\0';
    fprintf(stderr, "(%s, local var,  offset : %d,", s, obj->offset);
    display_type(obj->type);
    fprintf(stderr, ")");
    return;
}

void display_gvar(Obj *obj) {
    char s[obj->len + 1];
    strncpy(s, obj->name, obj->len);
    s[obj->len] = '\0';
    fprintf(stderr, "(%s, global var", s);
    display_type(obj->type);
    fprintf(stderr, ")\n Init-Val : \n");
    display_node(obj->init, 2);
    return;
}

void display_obj(Obj *obj) {
    if (obj->is_global) {
        display_gvar(obj);
    } else {
        display_lvar(obj);
    }
    return;
}


void display_space(int size) {
    char s[size + 1];
    for (int i = 0;i < size; i++) s[i] = ' ';
    s[size] = '\0';
    fprintf(stderr, "%s", s);
    return;
}

void display_child(Node *node, int indent) {
    display_node(node->lhs, indent + 1);
    display_node(node->rhs, indent + 1);
    return;
}

void display_node(Node *node, int indent) {
    display_space(indent);
    switch (node->kind) {
    case NdNum:
        fprintf(stderr, "Num : %d", node->val);
        display_type(node->type);
        fprintf(stderr, "\n");
        break;
    case NdLvar:
        fprintf(stderr, "Lvar : ");
        display_lvar(node->obj);
        display_type(node->type);
        fprintf(stderr, "\n");
        break;
     case NdGvar:
        fprintf(stderr, "Gvar : ");
        display_gvar(node->obj);
        display_type(node->type);
        fprintf(stderr, "\n");
        break;
    case NdAdd:
        fprintf(stderr, "Add");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdSub:
        fprintf(stderr, "Sub");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdMul:
        fprintf(stderr, "Mul");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdDiv:
        fprintf(stderr, "Div");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdMod:
        fprintf(stderr, "Mod");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdEq:
        fprintf(stderr, "Eq");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdNeq:
        fprintf(stderr, "Neq");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdLe:
        fprintf(stderr, "Le");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdLt:
        fprintf(stderr, "Lt");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdShl : 
        fprintf(stderr, "Shift Left");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdSar : 
        fprintf(stderr, "Shift Arithmetic Right");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdPostInc:
        fprintf(stderr, "Post Inc");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_node(node->lhs, indent + 1);
        break;
    case NdPostDec:
        fprintf(stderr, "Post Dec");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_node(node->lhs, indent + 1);
        break;
    case NdAssign:
        fprintf(stderr, "Assign");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_child(node, indent);
        break;
    case NdReturn:
        fprintf(stderr, "Return");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_node(node->lhs, indent + 1);
        break;  
    case NdIf:
        fprintf(stderr, "If");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_node(node->cond, indent + 1);
        display_space(indent);
        fprintf(stderr, "Then\n");
        display_node(node->then, indent + 1);
        if (node->els) {
            display_space(indent);
            fprintf(stderr, "Else\n");
            display_node(node->els, indent + 1);
        }
        break;  
    case NdFor:
        fprintf(stderr, "For");
        display_type(node->type);
        fprintf(stderr, "\n");
        if (node->init) {
            display_space(indent);
            fprintf(stderr, "Init\n");
            display_node(node->init, indent + 1);
        }
        if (node->cond) {
            display_space(indent);
            fprintf(stderr, "Cond\n");
            display_node(node->cond, indent + 1);
        }
        if (node->inc) {
            display_space(indent);
            fprintf(stderr, "Inc\n");
            display_node(node->inc, indent + 1);
        }
        display_space(indent);
        fprintf(stderr, "Body\n");
        display_node(node->then, indent + 1);
        break; 
    case NdDoWhile:
        fprintf(stderr, "Do While");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_space(indent);
        fprintf(stderr, "Cond\n");
        display_node(node->cond, indent + 1);
        display_space(indent);
        fprintf(stderr, "Then\n");
        display_node(node->then, indent + 1);
        break;
    case NdSwitch:
        fprintf(stderr, "Switch");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_space(indent);
        fprintf(stderr, "Cond\n");
        display_node(node->cond, indent + 1);
        display_space(indent);
        fprintf(stderr, "Then\n");
        display_node(node->then, indent + 1);
        break;
    case NdCase:
        fprintf(stderr, "Case");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_space(indent);
        fprintf(stderr, "Cond\n");
        display_node(node->cond, indent + 1);
        display_space(indent);
        fprintf(stderr, "Then\n");
        display_node(node->then, indent + 1);
        break;
    case NdDefault:
        fprintf(stderr, "Default");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_space(indent);
        fprintf(stderr, "Then\n");
        display_node(node->then, indent + 1);
        break;
    case NdBreak:
        fprintf(stderr, "Break");
        display_type(node->type);
        fprintf(stderr, "\n");
        break;
    case NdBlock:
        fprintf(stderr, "Block");
        display_type(node->type);
        fprintf(stderr, "\n");
        for (Node *cur = node->body;cur;cur = cur->next) {
            display_node(cur, indent + 1);
            fprintf(stderr, "\n");
        }
        break;
    case NdDeref:
        fprintf(stderr, "Deref");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_node(node->lhs, indent + 1);
        break;
    case NdRef:
        fprintf(stderr, "Ref\n");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_node(node->lhs, indent + 1);
        break;
    case NdFuncall: {
        char name[node->func_name_len + 1];
        strncpy(name, node->func_name, node->func_name_len);
        name[node->func_name_len] = '\0';
        fprintf(stderr, "Funcall (name : %s)", name);
        display_type(node->type);
        fprintf(stderr, "\n");
        display_space(indent + 1);
        fprintf(stderr, "Args\n");
        for (Node *nd = node->arguments;nd;nd = nd->next) {
            display_node(nd, indent + 2);
        }
        break;
        }
    case NdInit : {
        fprintf(stderr, "Init\n");
        for (Node *nd = node->body;nd;nd = nd->next) {
            display_node(nd, indent + 1);
        }
        break;
        }
    case NdMember : {
        fprintf(stderr, "Member");
        display_type(node->type);
        fprintf(stderr, "\n");
        display_node(node->lhs, indent + 1);
        display_space(indent + 1);
        display_obj(node->member);
        fprintf(stderr, "\n");
        break;
        }
    case NdStmtExpr : {
        fprintf(stderr, "StmtExpr");
        display_type(node->type);
        fprintf(stderr, "\n");
        for (Node *nd = node->body;nd;nd = nd->next) {
            display_node(nd, indent + 1);
        }
        break;
        }
    case NdComma : 
        fprintf(stderr, "Comma\n");
        display_child(node, indent);
        break;
    }
    
    return;
}

void display_function(Obj *func) {
    char s[100];
    strncpy(s, func->name, func->len);
    s[func->len] = '\0';
    fprintf(stderr, "Function : (name : %s, ", s);
    display_type(func->type);
    fprintf(stderr, ")\n");
    display_space(1);
    fprintf(stderr, "Args : \n");
    for (Obj *arg = func->args;arg;arg = arg->next) {
        display_space(2);
        display_obj(arg);
        fprintf(stderr, "\n");
    }
    for (Node *cur = func->body ;cur; cur = cur->next) {
        display_node(cur, 1);
        fprintf(stderr, "\n");
    }
    return;
}

void display_program(Obj *func) {
    for (Obj *fn = func;fn;fn = fn->next) {
        if (is_function(fn)) {
            display_function(fn);
        } else {
            fprintf(stderr, "Gvar : ");
            display_gvar(fn);
            fprintf(stderr, "\n");
        }
    }
    return;
}