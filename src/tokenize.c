#include "kcc.h"

bool startwith(char *p, char *q) {
    return strncmp(p, q, strlen(q)) == 0;
}

int isreserved(char *p) {
    if (startwith(p, "<=") || startwith(p, ">=") || startwith(p, "==") || startwith(p, "!=") ) {
        return 2;
    }
    if (ispunct(*p)) {
        return 1;
    }
    return 0;
}

bool is_ident1(char *p) {
    return ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_');
}

bool is_ident2(char *p) {
    return is_ident1(p) || ('0' <= *p && *p <= '9');
}

bool is_keyword(Token *token) {
    char *keywords[] = {"return", "if", "else", "while", "for", "int", "sizeof", "char"};
    for (int i = 0;i < 8; i++) {
        if (token->len == strlen(keywords[i]) && strncmp(token->str, keywords[i], token->len) == 0) {
            return true;
        }
    }
    return false;
}

void convert_keyword(Token *token) {
    for (Token *cur = token;cur;cur = cur->next) {
        if (is_keyword(cur)) {
            cur->kind = TkKeyword;
        }
    }
    return;
}

char read_escaped_char(char *p) {
    switch (*p) {
    case 'a': return '\a';
    case 'b': return '\b';
    case 't': return '\t';
    case 'n': return '\n';
    case 'v': return '\v';
    case 'f': return '\f';
    case 'r': return '\r';
    case 'e': return 27;
    default : return *p;
    }
}

char *read_string_end(char *start) {
    char *p = start+1;
    while ((*p) != '"') {
        if (*p == '\0' || *p == '\n') {
            error_tokenize(p);
        }
        if (*p == '\\') {
            p++;
        }
        p++;
    }
    return p;
}

Token *read_string(Token *cur, char *p) {
    char *start = p;
    char *end = read_string_end(start);
    char *buf = calloc(end - start, sizeof(char));
    int len = 0;

    for (p = start+1;p < end;) {
        if (*p == '\\') {
            buf[len++] = read_escaped_char(p + 1);
            p += 2;
        } else {
            buf[len++] = *p++;
        }
    }

    cur = new_token(TkString, cur, start);
    cur->str = buf;
    cur->type = new_type_array(new_type(TyChar), len + 1);
    cur->len = end - start + 1;
    // printf("%d", cur->len);
    return cur;
}

Token *read_char(Token *cur, char *p) {
    if (*(p + 1) == '\\') {
        cur = new_token(TkNum, cur, p);
        cur->val = read_escaped_char(p + 2);
        cur->len = 4;
        p += 3;
    } else {
        cur = new_token(TkNum, cur, p);
        cur->val = *(p + 1);
        cur->len = 3;
        p += 2;
    } 

    if (*p != '\'') {
        error_tokenize(p);
    }

    return cur;
}

// curに新しいTokenを繋げる。
// head -> ... -> cur
// head -> ... -> cur -> token　になる。
// tokenを返す。
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *token = calloc(1, sizeof(Token));
    token->kind  = kind;
    token->str   = str;
    cur->next    = token;
    return token;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        
        // num
        if (isdigit(*p)) {
            cur = new_token(TkNum, cur, p);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        // char literal 'a'
        if (*p == '\'') {
            cur = read_char(cur, p);
            p += cur->len;
            continue;
        }

        // string literal "a"
        if (*p == '"') {
            cur = read_string(cur, p);
            p += cur->len;
            continue;
        }

        // reserved
        int len = isreserved(p);
        if (len > 0) {
            cur = new_token(TkReserved, cur, p);
            cur->len = len;
            p += len;
            continue;
        }

        // ident
        if (is_ident1(p)) {
            cur = new_token(TkIdent, cur, p);
            char *start = p;
            while(is_ident2(p)) p++;
            cur->len = p - start;
            continue;
        }

        error_tokenize(p);
    }

    new_token(TkEof, cur, p);
    convert_keyword(head.next);
    return head.next;
}