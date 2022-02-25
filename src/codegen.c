#include "kcc.h"

void stack_pop(char *fmt, ...);
void stack_push(char *fmt, ...);
void gen_addr(Node *node);
void codegen_gvar_init(Type *type, Node *init);
void codegen_gvar(Obj *obj);
void codegen_expr(Node *node);
void codegen_stmt(Node *node);
void codegen_function(Obj *func);

const char *args_reg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
const char *args_reg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
const char *args_reg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
int depth = 0;

void stack_pop(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    depth += 8;
    return;
}

void stack_push(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    depth -= 8;
    return;
}

void codegen_gvar_init(Type *type, Node *init) {
    switch (type->kind) {
    case TyChar :
        printf("  .byte %d\n", init->val);
        break;
    case TyInt :
        printf("  .long %d\n", init->val);
        break;
    case TyPtr : {
        if (init->kind == NdNum) {
            printf("  .quad %d\n", init->val);
        } else if (init->kind == NdGvar) {
            char name[init->obj->len + 1];
            strncpy(name, init->obj->name, init->obj->len);
            name[init->obj->len] = '\0';
            printf("  .quad %s\n", name);
        } else if (init->kind == NdRef) {
            char name[init->lhs->obj->len + 1];
            strncpy(name, init->lhs->obj->name, init->lhs->obj->len);
            name[init->lhs->obj->len] = '\0';
            printf("  .quad %s\n", name);
        } else {
            error_codegen();
        }
        break;
    }
    case TyArray : {
        if (init->kind == NdInit) {
            Node *node = init->body;
            for (int i = 0;i < type->array_size; i++) {
                if (node) {
                    codegen_gvar_init(type->ptr_to, node);
                    node = node->next;
                } else {
                    codegen_gvar_init(type->ptr_to, zeros_like(type->ptr_to));
                }
            }
        } else if (init->kind == NdGvar) {
            codegen_gvar_init(type, init->obj->init);
        } else {
            error_codegen();
        }
        break;
        }
    case TyStruct : {
        error_type("THIS IS NOT IMPLEMENNTED");
    }
    }
}

void codegen_gvar(Obj *obj) {
    char s[100];
    strncpy(s, obj->name, obj->len);
    s[obj->len] = '\0';
    printf(".data\n");
    printf("%s:\n", s);
    codegen_gvar_init(obj->type, obj->init);
    return;
}

void codegen_load(Type *type, const char *reg64, const char *reg32, const char *src) {
    switch(type->kind) {
    case TyInt :
        printf("  mov %s, [%s]\n", reg32, src);
        break;
    case TyChar :
        printf("  movsx %s, BYTE PTR [%s]\n", reg32, src);
        break;
    default : 
        printf("  mov %s, [%s]\n", reg64, src);
        break;
    }
    return;
}

void codegen_store(Type *type, const char *reg64, const char *reg32, const char *reg8, const char *dst) {
    switch (type->kind) {
    case TyInt :
        printf("  mov [%s], %s\n", dst, reg32);
        break;
    case TyChar :
        printf("  mov [%s], %s\n", dst, reg8);
        break;
    default:
        printf("  mov [%s], %s\n", dst, reg64);
        break;
    }
    return;
}

void gen_addr(Node *node) {
    char s[100];
    switch (node->kind) {
    case NdLvar:
        strncpy(s, node->obj->name, node->obj->len);
        s[node->obj->len] = '\0';
        printf("  lea rax, [rbp%+d] # %s\n", -node->obj->offset, s);
        stack_push("  push rax\n");
        break; 
    case NdGvar:
        strncpy(s, node->obj->name, node->obj->len);
        s[node->obj->len] = '\0';
        printf("  lea rax, [rip+%s]\n", s);
        stack_push("  push rax\n");
        break; 
    case NdDeref:
        codegen_expr(node->lhs);
        break;
    default:
        error_codegen(); 
    }
} 

void codegen_stmt(Node *node) {
    switch (node->kind) {
    case NdReturn:
        codegen_expr(node->lhs);
        stack_pop("  pop rax # return value \n"); 
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case NdIf: {
        int cnt = counter();
        codegen_expr(node->cond);
        stack_pop("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lelse%d\n", cnt);
        codegen_stmt(node->then);
        printf("  jmp .Lend%d\n", cnt);
        printf(".Lelse%d:\n", cnt);
        if (node->els) {
            codegen_stmt(node->els);
        }
        printf(".Lend%d:\n", cnt);
        return;
    }
    case NdFor : {
        int cnt = counter();
        if (node->init) {
            codegen_expr(node->init);
            stack_pop("  pop rax # init%d\n", cnt);
        }
        printf(".Lcond%d:\n", cnt);
        if (node->cond) {
            codegen_expr(node->cond);
            stack_pop("  pop rax # cond%d\n", cnt);
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", cnt);
        }
        codegen_stmt(node->then);
        if (node->inc) {
            codegen_expr(node->inc);
            stack_pop("  pop rax # inc%d\n", cnt);
        }
        printf("  jmp .Lcond%d\n", cnt);
        printf(".Lend%d:\n", cnt);
        return;
    }
    case NdBlock :
        for (Node *cur = node->body;cur;cur = cur->next) {
            codegen_stmt(cur);
        }
        return;
    default :
        codegen_expr(node);
        stack_pop("  pop rax # expr -> stmt\n");
        return;
    }
}

void codegen_expr(Node *node) {
    switch (node->kind) {
    case NdNum:
        stack_push("  push %d\n", node->val);
        return;
    case NdLvar:
    case NdGvar:
        if (node->type->kind == TyArray) {
            gen_addr(node);
        } else {
            gen_addr(node);
            stack_pop("  pop rax\n");
            codegen_load(node->type, "rax", "eax", "rax");
            stack_push("  push rax\n");
        }
        return;
    case NdAssign:
        gen_addr(node->lhs);
        codegen_expr(node->rhs);
        stack_pop("  pop rdi # rhs \n"); 
        stack_pop("  pop rax # lhs addr \n"); 
        codegen_store(node->type, "rdi", "edi", "dil", "rax");
        stack_push("  push rdi\n");
        return;
    case NdDeref:
        codegen_expr(node->lhs);
        if (node->type->kind != TyArray) {
            stack_pop("  pop rax\n");
            printf("  mov rax, [rax]\n");
            stack_push("  push rax\n");
        }
        return;
    case NdRef:
        gen_addr(node->lhs);
        return;
    case NdFuncall: {
        int num_of_arg = 0;
        for (Node *nd = node->arguments;nd;nd = nd->next) {
            ++num_of_arg;
            codegen_expr(nd);
        }
        for (int i = num_of_arg - 1;i >= 0; i--) {
            stack_pop("  pop %s\n", args_reg64[i]);
        }
        char name[node->func_name_len + 1];
        strncpy(name, node->func_name, node->func_name_len);
        name[node->func_name_len] = '\0';

        if (depth % 16 != 0) {
            int diff = align_to(depth, 16) - depth;
            printf("  sub rsp, %d # align\n", diff);
            printf("  call %s\n", name);
            printf("  add rsp, %d # align\n", diff);
        } else {
            printf("  call %s\n", name);
        }

        stack_push("  push rax\n");
        return;
        }
    case NdReturn:
    case NdBlock:
    case NdIf:
    case NdFor:
        error_codegen();
    default:
        break;
    }

    codegen_expr(node->lhs);
    codegen_expr(node->rhs);
    stack_pop("  pop rdi # rhs \n"); 
    stack_pop("  pop rax # lhs \n"); 

    switch (node->kind) {
    case NdAdd:
        printf("  add rax, rdi\n"); 
        break;
    case NdSub:
        printf("  sub rax, rdi\n");
        break;
    case NdMul:
        printf("  imul rax, rdi\n");
        break;
    case NdDiv:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case NdMod:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  mov rax, rdx\n");
        break;
    case NdEq:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case NdNeq:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case NdLe:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    case NdLt:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    default:
        error_codegen();
    }

    stack_push("  push rax\n");
    return;
}

void codegen_function(Obj *func) {
    char name[func->len + 1];
    strncpy(name, func->name, func->len);
    name[func->len] = '\0';
    printf(".text\n");
    printf(".globl %s\n", name);
    printf("%s:\n", name);

    // prologue
    // call                    depth -= 8;
    printf("  push rbp\n"); // depth -= 8;
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", func->stack_size);
    
    int i = 0;
    for (Obj *arg = func->args;arg;arg = arg->next) {
        printf("  lea rax, [rbp%+d]\n", -arg->offset);
        codegen_store(arg->type, args_reg64[i], args_reg32[i], args_reg8[i], "rax");
        i++;
    }

    for (Node *cur = func->body;cur;cur = cur->next) {
        codegen_stmt(cur);
    }
    assert(depth == 0);

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n"); // depth += 8
    printf("  ret\n\n");   // depth += 8
    return;
}


void codegen(Obj *func) {
    for (Obj *fn = func;fn;fn = fn->next) {
        if (fn->is_function) {
            codegen_function(fn);
        } else {
            codegen_gvar(fn);
        }
    }
    return;
}