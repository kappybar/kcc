#include "kcc.h"

void stack_pop(char *fmt, ...);
void stack_push(char *fmt, ...);
void gen_addr(Node *node);
void codegen_gvar_init(Type *type, Node *init);
void codegen_gvar(Obj *obj);
void codegen_expr(Node *node);
void codegen_stmt(Node *node);
void codegen_function(Obj *func);

const char *args_reg64[] = {"rdi", "rsi", "rdx", "rcx", "r8" , "r9" };
const char *args_reg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
const char *args_reg16[] = {"di" , "si" , "dx" , "cx" , "r8w", "r9w"};
const char *args_reg8[]  = {"dil", "sil", "dl" , "cl" , "r8b", "r9b"};
int depth = 0;
FILE *output_file;

void println(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(output_file, fmt, ap);
    fprintf(output_file, "\n");
}

void stack_pop(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(output_file, fmt, ap);
    fprintf(output_file, "\n");
    depth += 8;
    return;
}

void stack_push(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(output_file, fmt, ap);
    fprintf(output_file, "\n");
    depth -= 8;
    return;
}

void codegen_gvar_init(Type *type, Node *init) {
    switch (type->kind) {
    case TyAbsent:
    case TyFunc:
    case TyVoid :
        error("cannot initialize this type");
        break;
    case TyChar :
        println("  .byte %d", init->val);
        break;
    case TyShort :
        println("  .value %d", init->val);
        break;
    case TyInt :
        println("  .long %d", init->val);
        break;
    case TyLong :
        println("  .quad %d", init->val);
        break;
    case TyPtr : {
        if (init->kind == NdNum) {
            println("  .quad %d", init->val);
        } else if (init->kind == NdGvar) {
            char name[init->obj->len + 1];
            strncpy(name, init->obj->name, init->obj->len);
            name[init->obj->len] = '\0';
            println("  .quad %s", name);
        } else if (init->kind == NdRef) {
            char name[init->lhs->obj->len + 1];
            strncpy(name, init->lhs->obj->name, init->lhs->obj->len);
            name[init->lhs->obj->len] = '\0';
            println("  .quad %s", name);
        } else {
            error("cannot initialize this value");
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
            error("cannot initialize this type");
        }
        break;
        }
    case TyStruct : {
        error("THIS IS NOT IMPLEMENNTED");
    }
    }
}

void assign_label(Node *node) {
    char *s = calloc(10, sizeof(char));
    int cnt = counter();
    sprintf(s, ".L%d", cnt);
    node->label = s;
    return;
}

void codegen_switch(Node *node) {
    // codegen case label
    switch (node->then->kind) {
    case NdCase:
        assign_label(node->then);
        codegen_expr(node->then->cond);
        stack_pop("  pop rdi");
        stack_pop("  pop rax");
        println("  cmp rax, rdi");
        stack_push("  push rax");
        println("  je %s", node->then->label);
        break;
    case NdBlock:
        for (Node *nd = node->then->body;nd;nd = nd->next) {
            if (nd->kind == NdCase) {
                assign_label(nd);
                codegen_expr(nd->cond);
                stack_pop("  pop rdi");
                stack_pop("  pop rax");
                println("  cmp rax, rdi");
                stack_push("  push rax");
                println("  je %s", nd->label);
            }
        }
        break;
    default:
        break;
    }
    // codegen default jmp
    switch (node->then->kind) {
    case NdDefault:
        assign_label(node->then);
        println("  jmp %s", node->then->label);  
        break;
    case NdBlock:
        for (Node *nd = node->then->body;nd;nd = nd->next) {
            if (nd->kind == NdDefault) {
                assign_label(nd);
                println("  jmp %s", nd->label);
                break;
            }
        }
        break;
    default:
        break;
    }
    return;
}

void codegen_gvar(Obj *obj) {
    char s[100];
    strncpy(s, obj->name, obj->len);
    s[obj->len] = '\0';
    println(".data");
    println("%s:", s);
    codegen_gvar_init(obj->type, obj->init);
    return;
}

void codegen_load(Type *type, const char *reg64, const char *reg32, const char *src) {
    switch(type->kind) {
    case TyLong:
        println("  mov %s, QWORD PTR [%s]", reg64, src);
        break;
    case TyInt :
        println("  mov %s, DWORD PTR [%s]", reg32, src);
        break;
    case TyShort :
        println("  movsx %s, WORD PTR [%s]", reg32, src);
        break;
    case TyChar :
        println("  movsx %s, BYTE PTR [%s]", reg32, src);
        break;
    default : 
        println("  mov %s, [%s]", reg64, src);
        break;
    }
    return;
}

void codegen_struct_assign(Type *type, const char *src, const char *dst) {
    for (int i = 0;i < type->type_struct->size; i++) {
        println("  movsx r8d, BYTE PTR [%s+%d]", src, i);
        println("  mov [%s+%d], r8b ", dst, i);
    }
    return;
}

void codegen_store(Type *type, const char *reg64, const char *reg32, const char *reg16, const char *reg8, const char *dst) {
    switch (type->kind) {
    case TyLong :
        println("  mov [%s], %s", dst, reg64);
        break;
    case TyInt :
        println("  mov [%s], %s", dst, reg32);
        break;
    case TyShort :
        println("  mov [%s], %s", dst, reg16);
        break;
    case TyChar :
        println("  mov [%s], %s", dst, reg8);
        break;
    default:
        println("  mov [%s], %s", dst, reg64);
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
        println("  lea rax, [rbp%+d] # %s", -node->obj->offset, s);
        stack_push("  push rax");
        break; 
    case NdGvar:
        strncpy(s, node->obj->name, node->obj->len);
        s[node->obj->len] = '\0';
        println("  lea rax, [rip+%s]", s);
        stack_push("  push rax");
        break; 
    case NdDeref:
        codegen_expr(node->lhs);
        break;
    case NdMember:
        gen_addr(node->lhs);
        stack_pop("  pop rax");
        println("  add rax, %d", node->member->offset);
        stack_push("  push rax");
        break;
    default:
        error("not a lvalue"); 
    }
} 

void codegen_stmt(Node *node) {
    switch (node->kind) {
    case NdReturn:
        if (node->lhs) { 
            codegen_expr(node->lhs);
            stack_pop("  pop rax # return value "); 
        }
        println("  mov rsp, rbp");
        println("  pop rbp");
        println("  ret");
        return;
    case NdIf: {
        int cnt = counter();
        codegen_expr(node->cond);
        stack_pop("  pop rax");
        println("  cmp rax, 0");
        println("  je .Lelse%d", cnt);
        codegen_stmt(node->then);
        println("  jmp .Lend%d", cnt);
        println(".Lelse%d:", cnt);
        if (node->els) {
            codegen_stmt(node->els);
        }
        println(".Lend%d:", cnt);
        return;
    }
    case NdFor : {
        int cnt = counter();
        if (node->init) {
            codegen_expr(node->init);
            stack_pop("  pop rax # init%d", cnt);
        }
        println(".Lcond%d:", cnt);
        if (node->cond) {
            codegen_expr(node->cond);
            stack_pop("  pop rax # cond%d", cnt);
            println("  cmp rax, 0");
            println("  je .Lend%d", cnt);
        }
        codegen_stmt(node->then);
        if (node->inc) {
            codegen_expr(node->inc);
            stack_pop("  pop rax # inc%d", cnt);
        }
        println("  jmp .Lcond%d", cnt);
        println(".Lend%d:", cnt);
        return;
    }
    case NdDoWhile : {
        int cnt = counter();
        println(".Lcond%d:", cnt);
        codegen_stmt(node->then);
        codegen_expr(node->cond);
        stack_pop("  pop rax # cond%d", cnt);
        println("  cmp rax, 0");
        println("  jne .Lcond%d", cnt);
        println(".Lend%d:", cnt);
        return;
    }
    case NdSwitch : {
        int cnt = counter();
        codegen_expr(node->cond);
        codegen_switch(node);
        println("  jmp .Lend%d", cnt);  
        codegen_stmt(node->then);
        println(".Lend%d:", cnt);
        stack_pop("  pop rax"); 
        return;  
    }
    case NdCase : {
        if (!node->label) {
            error("case label not within switch statement\n");
        }
        println("%s:", node->label);
        codegen_stmt(node->then);
        return;
    }
    case NdDefault : {
        if (!node->label) {
            error("default label not within switch statement\n");
        }
        println("%s:", node->label);
        codegen_stmt(node->then);
        return;
    }
    case NdBlock :
        for (Node *cur = node->body;cur;cur = cur->next) {
            codegen_stmt(cur);
        }
        return;
    default :
        codegen_expr(node);
        stack_pop("  pop rax # expr -> stmt");
        return;
    }
}

void codegen_expr(Node *node) {
    switch (node->kind) {
    case NdNum:
        stack_push("  push %d", node->val);
        return;
    case NdLvar:
    case NdGvar:
        if (node->type->kind == TyArray || node->type->kind == TyStruct) {
            gen_addr(node);
        } else {
            gen_addr(node);
            stack_pop("  pop rax");
            codegen_load(node->type, "rax", "eax", "rax");
            stack_push("  push rax");
        }
        return;
    case NdPostInc: {
        gen_addr(node->lhs);
        stack_pop("  pop rax");
        codegen_load(node->lhs->type, "rdi", "edi", "rax");
        println("  mov rcx, rdi");
        println("  add rcx, %d", is_pointer(node->type) ? ptr_to_size(node->type) : 1);
        codegen_store(node->type, "rcx", "ecx", "cx", "cl", "rax");
        stack_push("  push rdi");
        return;
        }
    case NdPostDec: {
        gen_addr(node->lhs);
        stack_pop("  pop rax");
        codegen_load(node->lhs->type, "rdi", "edi", "rax");
        println("  mov rcx, rdi");
        println("  sub rcx, %d", is_pointer(node->type) ? ptr_to_size(node->type) : 1);
        codegen_store(node->type, "rcx", "ecx", "cx", "cl", "rax");
        stack_push("  push rdi");
        return;
        }
    case NdAssign:
        gen_addr(node->lhs);
        codegen_expr(node->rhs);
        stack_pop("  pop rdi # rhs "); 
        stack_pop("  pop rax # lhs addr "); 
        if (node->type->kind == TyStruct) {
            codegen_struct_assign(node->type, "rdi", "rax");
        } else {
            codegen_store(node->type, "rdi", "edi", "di", "dil", "rax");
        }
        stack_push("  push rdi");
        return;
    case NdDeref:
        codegen_expr(node->lhs);
        if (node->type->kind != TyArray) {
            stack_pop("  pop rax");
            codegen_load(node->type, "rax", "eax", "rax");
            stack_push("  push rax");
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
            stack_pop("  pop %s", args_reg64[i]);
        }
        char name[node->func_name_len + 1];
        strncpy(name, node->func_name, node->func_name_len);
        name[node->func_name_len] = '\0';

        if (depth % 16 != 0) {
            int diff = align_to(depth, 16) - depth;
            println("  sub rsp, %d # align", diff);
            println("  call %s", name);
            println("  add rsp, %d # align", diff);
        } else {
            println("  call %s", name);
        }

        stack_push("  push rax");
        return;
        }
    case NdMember : 
        gen_addr(node->lhs);
        stack_pop("  pop rax");
        println("  add rax, %d", node->member->offset);
        if (node->type->kind != TyArray) {
            codegen_load(node->member->type, "rax", "eax", "rax");
        }
        stack_push("  push rax");
        return;
    case NdStmtExpr :
        for (Node *nd = node->body;nd;nd = nd->next) {
            codegen_stmt(nd);
        }
        stack_push("  push rax # StmtExpr");
        return;
    case NdReturn:
    case NdBlock:
    case NdIf:
    case NdFor:
    case NdDoWhile:
    case NdSwitch:
    case NdCase:
    case NdDefault:
        error("expect expression, but statement");
    default:
        break;
    }

    codegen_expr(node->lhs);
    codegen_expr(node->rhs);
    stack_pop("  pop rdi # rhs"); 
    stack_pop("  pop rax # lhs"); 

    switch (node->kind) {
    case NdAdd:
        println("  add rax, rdi"); 
        break;
    case NdSub:
        println("  sub rax, rdi");
        break;
    case NdMul:
        println("  imul rax, rdi");
        break;
    case NdDiv:
        println("  cqo");
        println("  idiv rdi");
        break;
    case NdMod:
        println("  cqo");
        println("  idiv rdi");
        println("  mov rax, rdx");
        break;
    case NdEq:
        println("  cmp rax, rdi");
        println("  sete al");
        println("  movzb rax, al");
        break;
    case NdNeq:
        println("  cmp rax, rdi");
        println("  setne al");
        println("  movzb rax, al");
        break;
    case NdLe:
        println("  cmp rax, rdi");
        println("  setle al");
        println("  movzb rax, al");
        break;
    case NdLt:
        println("  cmp rax, rdi");
        println("  setl al");
        println("  movzb rax, al");
        break;
    case NdShl:
        println("  mov rcx, rdi");
        println("  shl rax, cl");
        break;
    case NdSar:
        println("  mov rcx, rdi");
        println("  sar rax, cl");
        break;
    case NdComma:
        println("  mov rax, rdi");
        break;
    default:
        error("cannot codegen");
    }

    stack_push("  push rax");
    return;
}

void codegen_function(Obj *func) {
    if (!func->is_defined) {
        return;
    }
    char name[func->len + 1];
    strncpy(name, func->name, func->len);
    name[func->len] = '\0';
    println(".text");
    println(".globl %s", name);
    println("%s:", name);

    // prologue
    // call                    depth -= 8;
    println("  push rbp"); // depth -= 8;
    println("  mov rbp, rsp");
    println("  sub rsp, %d", func->stack_size);
    
    int i = 0;
    for (Obj *arg = func->args;arg;arg = arg->next) {
        println("  lea rax, [rbp%+d]", -arg->offset);
        codegen_store(arg->type, args_reg64[i], args_reg32[i], args_reg16[i] ,args_reg8[i], "rax");
        i++;
    }

    for (Node *cur = func->body;cur;cur = cur->next) {
        codegen_stmt(cur);
    }
    assert(depth == 0);

    // epilogue
    println("  mov rsp, rbp");
    println("  pop rbp"); // depth += 8
    println("  ret\n");   // depth += 8
    return;
}

void open_file(char *path) {
    if (!path || strcmp(path, "-") == 0) {
        output_file = stdout;
        return;
    }
    output_file = fopen(path, "w");
    if (!output_file) {
        fprintf(stderr, "cannot open file\n");
        exit(1);
    }
    return;
}

void codegen(Obj *func, char *path) {
    open_file(path);
    println(".intel_syntax noprefix");

    for (Obj *fn = func;fn;fn = fn->next) {
        if (is_function(fn)) {
            codegen_function(fn);
        } else {
            codegen_gvar(fn);
        }
    }
    return;
}