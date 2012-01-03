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
    *((int *) stack[0]) = *((int *) stack[0]) + stack[-1];
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
    *((int *) stack[0]) = stack[-1];
    stack -= 2;
}

void fetch(intptr_t **token) {
    *stack = *((int *) *stack);
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

// TODO:
void to_r(intptr_t **token) { }
void from_r(intptr_t **token) { }
void r_fetch(intptr_t **token) { }


// Additional

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

void dot_d(intptr_t **token) {
    printf("%d ", (int) *stack--);
}

void dot(intptr_t **token) {
    printf("%s ", (char *) *stack--);
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

    { "dup", dup },
    { "?dup", question_dup },
    { "depth", depth },
    { "drop", drop },
    { "roll", roll },
    { "rot", rot},
    { "swap", swap},

    { "branch", branch },
    { "branch?", branch_cond },
    { ".d", dot_d },
    { ".s", dot_s },
    { ".", dot }


    //    !  *  */  */MOD  +  +!  -  /  /MOD  0<  0=  0>  1+  1-  2+
    //           2-  2/  <  =  >  >R  ?DUP  @  ABS  AND  C!  C@  CMOVE
    //           CMOVE>  COUNT  D+  D<  DEPTH  DNEGATE  DROP  DUP  EXECUTE
    //           EXIT  FILL  I  J  MAX  MIN  MOD  NEGATE  NOT  OR  OVER  PICK
    //           R>  R@  ROLL  ROT  SWAP  U<  UM*  UM/MOD  XOR
};
