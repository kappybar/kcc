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
        while (isspace(*p)) p++;
        
        // num
        if (isdigit(*p)) {
            cur = new_token(TkNum, cur, p);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
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
    return head.next;
}