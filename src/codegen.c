#include "kcc.h"

void codegen_expr(Node *node);
void codegen_stmt(Node *node);

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
        gen_addr(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
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
        // not safe
        // node->lhs type should be pointer type
        codegen_expr(node->lhs);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case NdRef:
        gen_addr(node->lhs);
        return;
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
    printf(".globl main\n");
    printf("main:\n");

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", func->stack_size);

    for (Node *cur = func->body;cur;cur = cur->next) {
        codegen_stmt(cur);
    }

    // epilogue
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
}