#include "kcc.h"

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
        case TkEof:
            fprintf(stderr, "eof\n");
            break;
        }
    }
    return;
}

void display_obj(Obj *obj) {
    char s[obj->len + 1];
    strncpy(s, obj->name, obj->len);
    s[obj->len] = '\0';
    fprintf(stderr, "(%s, offset : %d)", s, obj->offset);
    return;
}

void display_node(Node *node, int indent);

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
        fprintf(stderr, "Num : %d\n", node->val);
        break;
    case NdLvar:
        fprintf(stderr, "Lvar : ");
        display_obj(node->obj);
        fprintf(stderr, "\n");
        break;
    case NdAdd:
        fprintf(stderr, "Add\n");
        display_child(node, indent);
        break;
    case NdSub:
        fprintf(stderr, "Sub\n");
        display_child(node, indent);
        break;
    case NdMul:
        fprintf(stderr, "Mul\n");
        display_child(node, indent);
        break;
    case NdDiv:
        fprintf(stderr, "Div\n");
        display_child(node, indent);
        break;
    case NdEq:
        fprintf(stderr, "Eq\n");
        display_child(node, indent);
        break;
    case NdNeq:
        fprintf(stderr, "Neq\n");
        display_child(node, indent);
        break;
    case NdLe:
        fprintf(stderr, "Le\n");
        display_child(node, indent);
        break;
    case NdLt:
        fprintf(stderr, "Lt\n");
        display_child(node, indent);
        break;
    case NdAssign:
        fprintf(stderr, "Assign\n");
        display_child(node, indent);
        break;
    case NdReturn:
        fprintf(stderr, "Return\n");
        display_node(node->lhs, indent + 1);
        break;  
    case NdIf:
        fprintf(stderr, "If\n");
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
        fprintf(stderr, "For\n");
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
    default:
        break;
    }
    return;
}

void display_function(Function *func) {
    for (Node *cur = func->body ;cur; cur = cur->next) {
        display_node(cur, 0);
        fprintf(stderr, "\n");
    }
    return;
}