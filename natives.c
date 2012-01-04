typedef void (*Native_f)(intptr_t **token);

// Nucleus

#define binary_op(name, op) void name(intptr_t **token) { stack[-1] = stack[-1] op stack[0]; stack -= 1; }

binary_op(plus, +);
binary_op(minus, -);
binary_op(times, *);
binary_op(divide, /);
binary_op(mod, %);
binary_op(less_than, <);
binary_op(equals, =);
binary_op(greater_than, >);
binary_op(and, &);
binary_op(or, |);
binary_op(xor, ^);

#define binary_op_with_const(name, op, c) void name(intptr_t **token) { *stack = *stack op c; }

binary_op_with_const(zero_less, <, 0);
binary_op_with_const(zero_equal, ==, 0);
binary_op_with_const(zero_greater, >, 0);
binary_op_with_const(one_plus, +, 1);
binary_op_with_const(one_minus, -, 1);
binary_op_with_const(two_plus, +, 2);
binary_op_with_const(two_minus, -, 2);
binary_op_with_const(two_divide, /, 2);

void plus_store(intptr_t **token) {
    *((intptr_t *) stack[0]) = *((intptr_t *) stack[0]) + stack[-1];
    stack -= 2;
}

void divide_mod(intptr_t **token) {
    ldiv_t result = ldiv(stack[-1], stack[0]);
    stack[-1] = result.rem;
    stack[0] = result.quot;
}

void times_divide(intptr_t **token) {
    stack[-2] = stack[-2] * stack[-1] / stack[0];
    stack -= 2;
}

void times_divide_mod(intptr_t **token) {
    ldiv_t result = ldiv(stack[-2] * stack[-1], stack[0]);
    stack[-2] = result.rem;
    stack[-1] = result.quot;
    stack -= 1;
}

void abs_(intptr_t **token) {
    *stack = abs(*stack);
}

void not(intptr_t **token) {
    *stack = !*stack; // TODO: Check
}

void store(intptr_t **token) {
    *((intptr_t *) stack[0]) = stack[-1];
    stack -= 2;
}

void fetch(intptr_t **token) {
    *stack = *((intptr_t *) *stack);
}

void c_store(intptr_t **token) {
    *((char *) stack[0]) = stack[-1];
    stack -= 2;
}

void c_fetch(intptr_t **token) {
    *stack = *((char *) *stack);
}

void cmove(intptr_t **token) {
    char *dst = (char *) stack[-1];
    char *src = (char *) stack[-2];
    char *end = (char *) dst + stack[0];
    while (dst < end) {
        *dst++ = *src++;
    }
}

void cmove_up(intptr_t **token) {
    char *dst = (char *) stack[-1] + stack[0] - 1;
    char *src = (char *) stack[-2] + stack[0] - 1;
    char *end = (char *) stack[-1];
    while (dst >= end) {
        *dst-- = *src--;
    }
}

void count(intptr_t **token) {
    *stack = *stack + sizeof(int);
    stack++;
    *stack = *((int *) *stack);
}

void fill(intptr_t **token) {
    memset((void *) stack[-2], stack[-1], stack[0]);
}

void sp0(intptr_t **token) {
    *(++stack) = (intptr_t) stack_bottom;
}

void dup(intptr_t **token) {
    *(++stack) = *stack;
}

void question_dup(intptr_t **token) {
    if (*stack) {
        dup(token);
    }
}

void depth(intptr_t **token) {
    *(++stack) = (stack - stack_bottom + 1);
}

void drop(intptr_t **token) {
    stack--;
}

void pick_store(intptr_t **token) {
    intptr_t value = stack[-1];
    int n = stack[0];
    stack -= 2;
    stack[-n] = value;
}

void roll(intptr_t **token) {
    int n = stack[0];
    stack--;
    intptr_t value = stack[-n];
    for (int i = -n; i < 0; i++) {
        stack[i] = stack[i + 1];
    }
    stack[0] = value;
}

void rot(intptr_t **token) {
    *(++stack) = 2;
    roll(token);
}

void swap(intptr_t **token) {
    intptr_t tmp = stack[0];
    stack[0] = stack[-1];
    stack[-1] = tmp;
}

void to_r(intptr_t **token) {
    *(++return_stack) = *stack--;
}

void from_r(intptr_t **token) {
    *(++stack) = *return_stack--;
}

void r_fetch(intptr_t **token) {
    *(++stack) = *return_stack;
}

// Additional

void tick(intptr_t **token) {
    *(++stack) = (*token)[2];
    *(++stack) = (*token)[3];
    *token += 2;
}

void emit(intptr_t **token) {
    putc(*stack--, stdout);
}

void branch(intptr_t **token) {
    *token += ((*token)[3] + 1) * 2;
}

void branch_cond(intptr_t **token) {
    if (*stack--) {
        *token += ((*token)[3] + 1) * 2;
    } else {
        *token = *token + 2;
    }
}

void dot_s(intptr_t **token) {
    for (intptr_t *s = stack_bottom; s <= stack; s++) {
        printf("%d ", (int) *s);
    }
}

char *get_native_name(Native_f func);

void dot_cs(intptr_t **token) {
    for (intptr_t *s = stack_bottom; s <= stack; s += 2) {
        switch (s[0]) {
        case tt_Int:
            printf("%d ", (int) s[1]);
            break;
        case tt_Native:
            printf("%s ", get_native_name((Native_f) s[1]));
            break;
        case tt_Symbol:
            for (int i = 0; i < dict->count; i++) {
                if (dict->values[i] == (void *) s[1]) {
                    printf("%s ", dict->keys[i]);
                }
            }
            break;
        }
    }
}

void dot_d(intptr_t **token) {
    printf("%d ", (int) *stack--);
}

void dot(intptr_t **token) {
    printf("%s ", (char *) *stack--);
}


// Compile words

void left_bracket(intptr_t **token) {
    if (is_compile_mode) {
        is_compile_mode = 0;
    }
}

void right_bracket(intptr_t **token) {
    if (!is_compile_mode) {
        is_compile_mode = 1;
    }
}

void literal(intptr_t **token) {
    stack++;
    stack[0] = stack[-1];
    stack[-1] = tt_Int;
}

void immediate(intptr_t **token) {
    if (is_compile_mode) {
        assert((stack - old_stack) % 2 == 0);
        for (intptr_t *s = stack; s > old_stack; s -= 2) {
            s[1] = s[-1];
            s[2] = s[0];
        }
        old_stack[1] = tt_Native;
        old_stack[2] = (intptr_t) immediate;
        stack += 2;
    }
}


struct {
    char *name;
    Native_f func;
} natives[] = {
    { "+", plus },
    { "+!", plus_store },
    { "-", minus },
    { "/", divide },
    { "mod", mod },
    { "/mod", divide_mod },
    { "*", times },
    { "*/", times_divide },
    { "*/mod", times_divide_mod },
    { "0<", zero_less },
    { "0=", zero_equal},
    { "0>", zero_greater },
    { "1+", one_plus },
    { "1-", one_minus },
    { "2+", two_plus },
    { "2-", two_minus },
    { "2/", two_divide },
    { "<", less_than},
    { "=", equals },
    { ">", greater_than },
    { "and", and},
    { "or", greater_than },
    { "xor", greater_than },
    { "abs", abs_ },
    { "not", not },

    { ">r", to_r },
    { "r>", from_r },
    { "r@", r_fetch },

    { "!", store },
    { "@", fetch },
    { "c!", c_store },
    { "c@", c_fetch },
    { "cmove", cmove },
    { "cmove_up", cmove },
    { "count", count },
    { "fill", fill },

    { "sp0", sp0 },
    { "dup", dup },
    { "?dup", question_dup },
    { "depth", depth },
    { "drop", drop },
    { "pick!", pick_store }, // Non-standard
    { "roll", roll },
    { "rot", rot},
    { "swap", swap},

    { "'", tick },
    { "emit", emit },

    { "branch", branch },
    { "branch?", branch_cond },
    { ".d", dot_d },
    { ".s", dot_s },
    { ".cs", dot_cs },
    { ".", dot },


    //    !  *  */  */MOD  +  +!  -  /  /MOD  0<  0=  0>  1+  1-  2+
    //           2-  2/  <  =  >  >R  ?DUP  @  ABS  AND  C!  C@  CMOVE
    //           CMOVE>  COUNT  D+  D<  DEPTH  DNEGATE  DROP  DUP  EXECUTE
    //           EXIT  FILL  I  J  MAX  MIN  MOD  NEGATE  NOT  OR  OVER  PICK
    //           R>  R@  ROLL  ROT  SWAP  U<  UM*  UM/MOD  XOR


    // Compile words
    { "[", left_bracket },
    { "]", right_bracket },
    { "literal", literal },
    { "immediate", immediate }
};

Native_f compile_natives[] = { left_bracket, right_bracket, literal, immediate };

int is_compile_native(Native_f func) {
    for (int i = 0; i < sizeof(compile_natives) / sizeof(Native_f); i++) {
        if (compile_natives[i] == func) {
            return 1;
        }
    }

    return 0;
}

char *get_native_name(Native_f func) {
    for (int i = 0; i < sizeof(natives) / sizeof(natives[0]); i++) {
        if (natives[i].func == func) {
            return natives[i].name;
        }
    }

    return NULL;
}
