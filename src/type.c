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

Type *new_type_array(Type *ty, size_t size) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TyArray;
    type->ptr_to = ty;
    type->array_size = size;
    return type;
}

Type *copy_type(Type *ty) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = ty->kind;
    type->ptr_to = ty->ptr_to ? copy_type(ty->ptr_to) : NULL;
    type->array_size = ty->array_size;
    return type;
}

int sizeof_type(Type *ty) {
    switch (ty->kind) {
    case TyInt:
        return 8;
    case TyPtr:
        return 8;
    case TyArray:
        return ty->array_size * sizeof_type(ty->ptr_to);
    }
}

int ptr_to_size(Type *ty) {
    if (ty->ptr_to) {
        return sizeof_type(ty->ptr_to);
    } else {
        error_type();
        return 0;
    }
}

void add_type(Node *node) {
    if (!node && node->type) {
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
                node->type = new_type_ptr(node->rhs->type->ptr_to);
            }
        } else {
            if (node->rhs->type->kind == TyInt) {
                node->type = new_type_ptr(node->lhs->type->ptr_to);
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
        add_type(node->lhs);
        node->type = new_type_ptr(node->lhs->type);
        break;
    case NdBlock:
        for (Node *nd = node->body;nd;nd = nd->next) {
            add_type(nd);
        }
        break;
    case NdFor:
        add_type(node->cond);
        add_type(node->then);
        add_type(node->init);
        add_type(node->inc);
        break;
    case NdReturn:
        add_type(node->lhs);
        break;
    case NdIf:
        add_type(node->cond);
        add_type(node->then);
        if (node->els) add_type(node->els);
        break;
    }
    return;
}