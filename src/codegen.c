#include "kcc.h"

void codegen_expr(Node *node);
void codegen_stmt(Node *node);

const char *args_reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_addr(Node *node) {
    char s[100];
    switch (node->kind) {
    case NdLvar:
        strncpy(s, node->obj->name, node->obj->len);
        s[node->obj->len] = '\0';
        printf("  lea rax, [rbp%+d] # %s\n", -node->obj->offset, s);
        printf("  push rax\n");
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
        printf("  pop rax # return value \n"); 
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case NdIf: {
        int cnt = counter();
        codegen_expr(node->cond);
        printf("  pop rax\n");
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
            printf("  pop rax # init%d\n", cnt);
        }
        printf(".Lcond%d:\n", cnt);
        if (node->cond) {
            codegen_expr(node->cond);
            printf("  pop rax # cond%d\n", cnt);
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", cnt);
        }
        codegen_stmt(node->then);
        if (node->inc) {
            codegen_expr(node->inc);
            printf("  pop rax # inc%d\n", cnt);
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
        printf("  pop rax # expr -> stmt\n");
        return;
    }
}

void codegen_expr(Node *node) {
    switch (node->kind) {
    case NdNum:
        printf("  push %d\n", node->val);
        return;
    case NdLvar:
        if (node->type->kind == TyArray) {
            gen_addr(node);
        } else {
            gen_addr(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
        }
        return;
    case NdAssign:
        gen_addr(node->lhs);
        codegen_expr(node->rhs);
        printf("  pop rdi # rhs \n"); 
        printf("  pop rax # lhs addr \n"); 
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case NdDeref:
        codegen_expr(node->lhs);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
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
            printf("  pop %s\n", args_reg[i]);
        }
        char name[node->func_name_len + 1];
        strncpy(name, node->func_name, node->func_name_len);
        name[node->func_name_len] = '\0';
        printf("  call %s\n", name);
        printf("  push rax\n");
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
    printf("  pop rdi # rhs \n"); 
    printf("  pop rax # lhs \n"); 

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

    printf("  push rax\n");
    return;
}

void codegen_function(Function *func) {
    char name[func->name_len + 1];
    strncpy(name, func->name, func->name_len);
    name[func->name_len] = '\0';
    printf(".globl %s\n", name);
    printf("%s:\n", name);

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", func->stack_size);
    
    int i = 0;
    for (Obj *arg = func->args;arg;arg = arg->next) {
        printf("  lea rax, [rbp%+d]\n", -arg->offset);
        printf("  mov [rax], %s\n", args_reg[i++]);
    }

    for (Node *cur = func->body;cur;cur = cur->next) {
        codegen_stmt(cur);
    }

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n\n");
    return;
}

void codegen(Function *func) {
    for (Function *fn = func;fn;fn = fn->next) {
        codegen_function(fn);
    }
    return;
}