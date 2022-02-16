#include "kcc.h"

void display_token(Token *token) {
    for (Token *cur = token;cur;cur = cur->next) {
        char s[cur->len + 1];
        s[cur->len] = '\0';
        switch (cur->kind) {
        case TkReserved:
            strncpy(s, cur->str, cur->len);
            fprintf(stderr, "Reserved : %s\n", s);
            break;
        case TkNum:
            fprintf(stderr, "Num : %d\n", cur->val);
            break;
        case TkEof:
            fprintf(stderr, "eof\n");
            break;
        }
    }
    return;
}

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

        error_tokenize(p);
    }

    new_token(TkEof, cur, p);
    return head.next;
}