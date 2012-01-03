#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "hash.c"

const int STACK_SIZE = 1024;
intptr_t *stack_bottom;
intptr_t *stack;

Hash* dict;

typedef enum {
    tt_None = 0,
    tt_Symbol,
    tt_Int,
    tt_Native,
    tt_Definition
} TokenType;

#define logf(...) fprintf(stderr, __VA_ARGS__)

#include "natives.c"

void eval_tokens(intptr_t *token) {
    //logf("eval_tokens: %p\n", token);

    for (; token[0] != tt_None; token += 2) {
        switch (token[0]) {
        case tt_Native:
            ((Native_f) token[1])(&token);
            break;
        case tt_Symbol: {
            //logf("symbol\n");
            intptr_t *def = hash_get(dict, (char *) token[1]);
            if (def) {
                eval_tokens(def + 4);
                break;
            } else {
            //    logf("%s\n", (char *) token[1]);
            }
        }
        default:
            //logf("default\n");
            *(++stack) = token[1];
        }
    }
}

void parse_token(char *str, intptr_t *token) {
    //logf("parse_token: %s\n", str);

    if (strcmp(str, ":") == 0 ) {
        token[0] = tt_Definition;
        return;
    }

    for (int i = 0; i < sizeof(natives) / sizeof(natives[0]); i++) {
        if (strcmp(str, natives[i].name) == 0) {
            token[0] = tt_Native;
            token[1] = (intptr_t) natives[i].func;
            return;
        }
    }

    int is_num = 1;
    char *s = str[0] == '-' ? str + 1 : str;
    while (*s) {
        is_num = is_num && isdigit(*s);
        s++;
    }
    if (is_num && s > str && *s != '-') {
        token[0] = tt_Int;
        token[1] = atoi(str);
        return;
    }

    token[0] = tt_Symbol;
    token[1] = (intptr_t) strdup(str);
}

intptr_t *parse(char *str) {
    //logf("parse: %s\n", str);

    int num_tokens = 128;
    intptr_t *tokens = calloc(num_tokens, sizeof(intptr_t) * 2);
    intptr_t *current_token = tokens;
    for (char *s = str; *s != 0; s++) {
        if (isspace(*s)) {
            *s = 0;
            parse_token(str, current_token);
            current_token += 2;
            while (isspace(*++s));
            str = s;
        }
    }

    return tokens;
}

void eval(char *str) {
    //logf("eval: %s\n", str);

    intptr_t *token = parse(str);
    if (token[0] == tt_Definition) {
        assert(token[2] = tt_Symbol);
        assert(token[4] != tt_None);
        hash_set(dict, (char *) token[3], token);
    } else {
        eval_tokens(token);
    }
}

int main() {
    stack = stack_bottom = calloc(STACK_SIZE, sizeof(intptr_t));
    stack--;
    dict = calloc(1, sizeof(Hash));

    const int BUFFER_SIZE = 1024;
    char str[BUFFER_SIZE];

    while(fgets(str, BUFFER_SIZE, stdin)) {
        eval(str);
    }

    return 0;
}
