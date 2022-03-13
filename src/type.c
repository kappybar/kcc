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

Type *new_type_struct(Struct *s) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TyStruct;
    type->type_struct = s;
    return type;
}

Type *new_type_fun(Type *return_ty, Type *params_ty) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TyFunc;
    type->return_ty = return_ty;
    type->params = params_ty;
    return type;
}

Type *copy_type(Type *ty) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = ty->kind;
    type->next = ty->next ? copy_type(ty->next) : NULL;

    type->ptr_to = ty->ptr_to ? copy_type(ty->ptr_to) : NULL;
    type->return_ty = ty->return_ty ? copy_type(ty->return_ty) : NULL;
    type->params = ty->params ? copy_type(ty->params) : NULL;

    type->array_size = ty->array_size;
    type->type_struct = ty->type_struct;

    type->is_typdef = ty->is_typdef;
    return type;
}

Type *find_return_type(char *func_name, int func_name_len) {
    for (Obj *fn = globals;fn;fn = fn->next) {
        if (is_function(fn) && fn->len == func_name_len && strncmp(func_name, fn->name, func_name_len) == 0) {
            return fn->type->return_ty;
        }
    }
    error("implicit declaraton of function\n");
    return NULL;
}

Node *zeros_like(Type *type) {
    switch (type->kind) {
    case TyLong:
    case TyInt :
    case TyShort :
    case TyChar :
    case TyPtr :
        return new_node_num(0);
    case TyArray : {
        Node head;
        Node *cur = &head;
        for (int i = 0;i < type->array_size; i++) {
            cur->next = zeros_like(type->ptr_to);
            cur = cur->next;
        }
        Node *node = new_node(NdInit);
        node->body = head.next;
        return node;
    }
    case TyStruct : {
        error("THIS IS NOT IMPLEMENTED");
        return NULL;
    }
    case TyVoid : {
        error("void type has no value\n");
        return NULL;
    }
    case TyFunc : 
        error("func type has no value\n");
        return NULL;
    case TyAbsent : 
        error("absent type has no value\n");
        return NULL;
    }
}

int alignment(Type *ty) {
    switch (ty->kind) {
    case TyVoid:
        return 0;
    case TyLong:
    case TyInt:
        return 4;
    case TyShort:
        return 2;
    case TyChar:
        return 1;
    case TyPtr:
        return 4;
    case TyArray:
        return alignment(ty->ptr_to);
    case TyStruct:
        return ty->type_struct->align;
    case TyFunc:
    case TyAbsent:
        error("cannot alignment this type\n");
        return 0;
    }
}

int sizeof_type(Type *ty) {
    switch (ty->kind) {
    case TyVoid:
        return 1;
    case TyLong:
        return 8;
    case TyInt:
        return 4;
    case TyShort:
        return 2;
    case TyChar:
        return 1;
    case TyPtr:
        return 8;
    case TyArray:
        return ty->array_size * sizeof_type(ty->ptr_to);
    case TyStruct:
        return ty->type_struct->size;
    case TyFunc:
    case TyAbsent:
        error("cannot get size of this type\n");
        return 0;
    }
}

int ptr_to_size(Type *ty) {
    if (ty->ptr_to) {
        return sizeof_type(ty->ptr_to);
    } else {
        error("type error\n");
        return 0;
    }
}

bool is_integer(Type *ty) {
    if (!ty) {
        return false;
    }
    return (ty->kind == TyLong || ty->kind == TyInt || ty->kind == TyChar || ty->kind == TyShort);
}

bool is_pointer(Type *ty) {
    if (!ty) {
        return false;
    }
    return (ty->kind == TyPtr || ty->kind == TyArray); 
    // implicity type conversion array -> ptr
}

bool is_function(Obj *obj) {
    return obj->type->kind == TyFunc;
}

bool same_type(Type *ty1, Type *ty2) {
    if (!ty1 || !ty2) {
        return false;
    }
    if (ty1->kind == ty2->kind) {
        return true;
    }
    if (is_integer(ty1) && is_integer(ty2)) {
        return true;
    } 
    if (is_pointer(ty1) && is_pointer(ty2)) {
        return true;
    }
    return false;
}

Type *fill_absent_type(Type *type_absent, Type *type_fill) {
    switch (type_absent->kind) {
    case TyVoid:
    case TyInt:
    case TyChar:
    case TyShort:
    case TyLong:
    case TyStruct:
        return type_absent;
    case TyPtr:
    case TyArray:
        type_absent->ptr_to = fill_absent_type(type_absent->ptr_to, type_fill);
        return type_absent;
    case TyAbsent:
        type_absent = type_fill;
        return type_absent;
    case TyFunc:
        type_absent->return_ty = fill_absent_type(type_absent->return_ty, type_fill);
        return type_absent;
    }
}

void add_type(Node *node) {
    if (!node || node->type) {
        return;
    }

    add_type(node->lhs);
    add_type(node->rhs);
    add_type(node->cond);
    add_type(node->then);
    add_type(node->init);
    add_type(node->inc);
    add_type(node->els);
    for (Node *nd = node->body;nd;nd = nd->next) {
        add_type(nd);
    }
    for (Node *nd = node->arguments;nd;nd = nd->next) {
        add_type(nd);
    }

    switch (node->kind) {
    case NdNum :
        node->type = new_type(TyInt);
        break;
    case NdLvar:
    case NdGvar:
        node->type = node->obj->type;
        break;
    case NdAdd:
    case NdSub:
        if (!is_integer(node->rhs->type)) {
            Node *tmp = node->rhs;
            node->rhs = node->lhs;
            node->lhs = tmp;
        }
        if (is_integer(node->lhs->type) && is_integer(node->rhs->type)) {
            node->type = new_type(TyInt);
        } else if (is_integer(node->rhs->type)) {
            switch (node->lhs->type->kind) {
            case TyArray :
                node->type = new_type_array(node->lhs->type->ptr_to, ptr_to_size(node->lhs->type));
                break;
            case TyPtr :
                node->type = new_type_ptr(node->lhs->type->ptr_to);
                break;
            default:
                error("type error : cannot add this two type\n");    
            }
        } else {
            error("type error : cannot add this two type\n");
        }

        break;
    case NdMul:
    case NdDiv:
    case NdMod:
    case NdShl:
    case NdSar:
    case NdAnd:
    case NdXor:
    case NdOr:
    case NdLAnd:
    case NdLOr:
        if (!is_integer(node->lhs->type) || !is_integer(node->rhs->type)) {
            error("type error : cannot operate this two type\n");
        }
        node->type = new_type(TyInt);
        break;
    case NdCond:
        if (!same_type(node->then->type, node->els->type)) {
            error("type error : type mismatch in conditional expr\n");
        }
        node->type = node->then->type;
        break;
    case NdEq:
    case NdNeq:
    case NdLt:
    case NdLe:
        if (!same_type(node->lhs->type, node->rhs->type)) {
            error("type error : cannot compare this two type\n");
        }
        node->type = new_type(TyInt);
        break;
    case NdPostDec:
    case NdPostInc:
        if (!is_integer(node->lhs->type) && node->lhs->type->kind != TyPtr) {
            error("type error : cannot compare this two type\n");
        }
        node->type = node->lhs->type;
        break;
    case NdAssign:
        if (!same_type(node->lhs->type, node->rhs->type)) {
            error("type error : cannot assign different type\n");
        }
        node->type = node->lhs->type;
        break;
    case NdDeref:
        if (node->lhs->type->kind != TyPtr && node->lhs->type->kind != TyArray) {
            error("type error : cannot deref this type\n");
        }
        node->type = node->lhs->type->ptr_to;
        break;
    case NdRef:
        node->type = new_type_ptr(node->lhs->type);
        break;
    case NdBlock:
    case NdFor:
    case NdReturn:
    case NdIf:
    case NdDoWhile:
    case NdSwitch:
    case NdCase:
    case NdDefault:
    case NdBreak:
    case NdContinue:
    case NdInit:
        node->type = new_type(TyVoid);
        break;
    case NdFuncall:
        node->type = find_return_type(node->func_name, node->func_name_len); 
        break;
    case NdMember:
        node->type = node->member->type;
        break;
    case NdStmtExpr: {
        Node *nd;
        for (nd = node->body;nd->next;nd = nd->next);
        node->type = nd->type;
        break;
        }
    case NdComma:
        node->type = node->rhs->type;
        break;
    }
        
    return;
}