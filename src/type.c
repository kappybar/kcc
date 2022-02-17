#include "kcc.h"

Type *new_type(TypeKind kind) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = kind;
    return  type;
}

Type *new_type_ptr(Type *ty) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TyPtr;
    type->ptr_to = ty;
    return type;
}

int ptr_to_size(Type *ty) {
    if (ty->kind == TyPtr) {
        switch (ty->ptr_to->kind) {
        case TyInt :
            return 8;
        case TyPtr :
            return 8;    
        }
    } else {
        error_type();
        return 0;
    }
}

void add_type(Node *node) {
    if (node->type) {
        return;
    }

    switch (node->kind) {
    case NdNum :
        node->type = new_type(TyInt);
        break;
    case NdLvar:
        node->type = node->obj->type;
        break;
    case NdAdd:
    case NdSub:
        add_type(node->lhs);
        add_type(node->rhs);
        
        if (node->lhs->type->kind == TyInt) {
            if (node->rhs->type->kind == TyInt) {
                node->type = new_type(TyInt);
            } else {
                node->type = new_type(TyPtr);
            }
        } else {
            if (node->rhs->type->kind == TyInt) {
                node->type = new_type(TyPtr);
            } else {
                error_type();
            }
        }
        break;
    case NdMul:
    case NdDiv:
        add_type(node->lhs);
        add_type(node->rhs);
        if (node->lhs->type->kind != TyInt || node->rhs->type->kind != TyInt) {
            error_type();
        }
        node->type = new_type(TyInt);
        break;
    case NdEq:
    case NdNeq:
    case NdLt:
    case NdLe:
        add_type(node->lhs);
        add_type(node->rhs);
        if (node->lhs->type->kind != node->rhs->type->kind) {
            error_type();
        }
        node->type = new_type(TyInt);
        break;
    case NdAssign:
        add_type(node->lhs);
        add_type(node->rhs);
        if (node->lhs->type->kind != node->rhs->type->kind) {
            error_type();
        }
        node->type = node->lhs->type;
    case NdDeref:
        add_type(node->lhs);
        if (node->lhs->type->kind != TyPtr) {
            error_type();
        }
        node->type = node->lhs->type->ptr_to;
        break;
    case NdRef:
        add_type(node->rhs);
        node->type = new_type_ptr(node->lhs->type);
        break;
    default:
        break;
    }
    return;
}