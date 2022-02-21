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

Type *find_return_type(char *func_name, int func_name_len) {
    for (Function *fn = functions;fn;fn = fn->next) {
        if (fn->name_len == func_name_len && strncmp(func_name, fn->name, func_name_len) == 0) {
            return fn->return_type;
        }
    }
    error_type("implicit declaraton of function\n");
    return NULL;
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
        error_type("type error\n");
        return 0;
    }
}

bool same_type(Type *ty1, Type *ty2) {
    if (ty1->kind == ty2->kind ) return true;
    if ((ty1->kind == TyPtr && ty2->kind == TyArray) || (ty1->kind == TyPtr && ty2->kind == TyArray)) return true;
    return false;
}

void add_type(Node *node) {
    if (!node || node->type) {
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
        
        if (node->rhs->type->kind != TyInt) {
            Node *tmp = node->rhs;
            node->rhs = node->lhs;
            node->lhs = tmp;
        }
        if (node->lhs->type->kind == TyInt && node->rhs->type->kind == TyInt) {
            node->type = new_type(TyInt);
        }
        else if (node->rhs->type->kind == TyInt) {
            switch (node->lhs->type->kind) {
            case TyArray :
                node->type = new_type_array(node->lhs->type->ptr_to, ptr_to_size(node->lhs->type));
                break;
            case TyPtr :
                node->type = new_type_ptr(node->lhs->type->ptr_to);
                break;
            default:
                break;
            }
        } else {
            error_type("type error : cannot add this two type\n");
        }

        break;
    case NdMul:
    case NdDiv:
        add_type(node->lhs);
        add_type(node->rhs);
        if (node->lhs->type->kind != TyInt || node->rhs->type->kind != TyInt) {
            error_type("type error : cannot mul this two type\n");
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
            error_type("type error : cannot compare this two type\n");
        }
        node->type = new_type(TyInt);
        break;
    case NdAssign:
        add_type(node->lhs);
        add_type(node->rhs);

        if (!same_type(node->lhs->type, node->rhs->type)) {
            error_type("type error : cannot assign different type\n");
        }
        node->type = node->lhs->type;
        break;
    case NdDeref:
        add_type(node->lhs);
        if (node->lhs->type->kind != TyPtr && node->lhs->type->kind != TyArray) {
            error_type("type error : cannot deref this type\n");
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
    case NdFuncall:
        node->type = find_return_type(node->func_name, node->func_name_len); 
        break;
    }
    return;
}