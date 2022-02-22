#include "kcc.h"

void stack_pop(char *fmt, ...);
void stack_push(char *fmt, ...);
void gen_addr(Node *node);
void codegen_gvar(Obj *obj);
void codegen_expr(Node *node);
void codegen_stmt(Node *node);
void codegen_function(Obj *func);

const char *args_reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
const char *args_reg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
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

void codegen_gvar(Obj *obj) {
    char s[100];
    strncpy(s, obj->name, obj->len);
    s[obj->len] = '\0';
    printf(".data\n");
    printf("%s:\n", s);
    printf("  .zero %d\n", sizeof_type(obj->type));
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
            if (node->type->kind == TyInt) {
                printf("  mov eax, [rax]\n");
            } else {
                printf("  mov rax, [rax]\n");
            }
            stack_push("  push rax\n");
        }
        return;
    case NdAssign:
        gen_addr(node->lhs);
        codegen_expr(node->rhs);
        stack_pop("  pop rdi # rhs \n"); 
        stack_pop("  pop rax # lhs addr \n"); 
        if (node->type->kind == TyInt) {
            printf("  mov [rax], edi\n");
        } else {
            printf("  mov [rax], rdi\n");
        }
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
            stack_pop("  pop %s\n", args_reg[i]);
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
        if (arg->type->kind == TyInt) {
            printf("  mov [rax], %s\n", args_reg32[i++]);
        } else {
            printf("  mov [rax], %s\n", args_reg[i++]);
        }
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