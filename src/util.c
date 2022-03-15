#include "kcc.h"

int counter() {
    static int count = 0;
    return count++;
}

int align_to(int x, int align) {
    return ((x + align - 1) / align) * align;
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char *line = loc;
    while (user_input < line && line[-1] != '\n') {
        line --;
    }

    char *end = loc;
    while (*end != '\n' && *end != '\0') {
        end++;
    }
    
    int line_num = 1;
    for (char *p = user_input; p < line; p++) {
        if (*p == '\n') line_num++;
    }

    int indent = fprintf(stderr, "%s:%d: ", input_path, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);
    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// builtin
const int SIZEOF_BUILTIN_VA_LIST = 24;
const int NAMELEN_BUILTIN_VA_LIST = 17;