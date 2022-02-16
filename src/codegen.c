#include "kcc.h"

void gen_addr(Node *node) {
    char s[100];
    switch (node->kind) {
    case NdLvar:
        strncpy(s, node->obj->name, node->obj->len);
        s[node->obj->len] = '\0';
        printf("  lea rax, [rbp%+d] # %s\n", -node->obj->offset, s);
        printf("  push rax\n");
        break;   
    default:
        error_codegen(); 
    }

} 

void codegen(Node *node) {
    switch (node->kind) {
    case NdNum:
        printf("  push %d\n", node->val);
        return;
    case NdLvar:
        gen_addr(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case NdAssign:
        gen_addr(node->lhs);
        codegen(node->rhs);
        printf("  pop rdi # rhs \n"); 
        printf("  pop rax # lhs addr \n"); 
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case NdReturn:
        codegen(node->lhs);
        printf("  pop rax # return value \n"); 
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    default:
        break;
    }

    codegen(node->lhs);
    codegen(node->rhs);
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
    printf(".globl main\n");
    printf("main:\n");

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", func->stack_size);

    for (Node *cur = func->body;cur;cur = cur->next) {
        codegen(cur);
        printf("  pop rax\n");
    }

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
}