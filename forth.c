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

typedef struct {
    TokenType type;
    intptr_t value;
} Token;

#define logf(...) fprintf(stderr, __VA_ARGS__)

#include "natives.c"

void eval_tokens(Token *token) {
    //logf("eval_tokens: %p %d %s\n", token, token->type, (char *) token->value);

    for (; token->type != tt_None; token++) {
        switch (token->type) {
        case tt_Native:
            ((Native_f) token->value)(&token);
            break;
        case tt_Symbol: {
            //logf("symbol\n");
            Token *def = hash_get(dict, (char *) token->value);
            if (def) {
                eval_tokens(def + 2);
                break;
            } else {
                //logf("%s\n", (char *) token->value);
            }
        }
        default:
            //logf("default\n");
            *(++stack) = token->value;
        }
    }
}

void parse_token(char *str, Token *token) {
    //logf("parse_token: %s\n", str);

    if (strcmp(str, ":") == 0 ) {
        token->type = tt_Definition;
        return;
    }

    for (int i = 0; i < sizeof(natives) / sizeof(natives[0]); i++) {
        if (strcmp(str, natives[i].name) == 0) {
            token->type = tt_Native;
            token->value = (intptr_t) natives[i].func;
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
        token->type = tt_Int;
        token->value = atoi(str);
        return;
    }

    token->type = tt_Symbol;
    token->value = (intptr_t) strdup(str);
}

Token *parse(char *str) {
    //logf("parse: %s\n", str);

    int num_tokens = 128;
    Token *tokens = calloc(num_tokens, sizeof(Token));
    Token *current_token = tokens;
    for (char *s = str; *s != 0; s++) {
        if (isspace(*s)) {
            *s = 0;
            parse_token(str, current_token);
            current_token++;
            while (isspace(*++s));
            str = s;
        }
    }

    return tokens;
}

void eval(char *str) {
    //logf("eval: %s\n", str);

    Token *token = parse(str);
    if (token->type == tt_Definition) {
        assert(token[1].type = tt_Symbol);
        assert(token[2].type != tt_None);
        hash_set(dict, (char *) token[1].value, token);
    } else {
        eval_tokens(token);
    }
}

int main() {
    stack = stack_bottom = calloc(STACK_SIZE, sizeof(intptr_t));
    dict = calloc(1, sizeof(Hash));

    const int BUFFER_SIZE = 1024;
    char str[BUFFER_SIZE];

    while(fgets(str, BUFFER_SIZE, stdin)) {
        eval(str);
    }

    return 0;
}
