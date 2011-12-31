typedef void (*Native_f)(Token **token);

void dup(Token **token) {
    *(++stack) = *stack;
}

void branch(Token **token) {
    *token += (*token)[1].value + 1;
}

void branch_cond(Token **token) {
    if (*stack--) {
        *token += (*token)[1].value + 1;
    } else {
        *token = *token + 1;
    }
}

void plus(Token **token) {
    stack[-1] = stack[-1] + stack[0];
    stack--;
}

void minus(Token **token) {
    stack[-1] = stack[-1] - stack[0];
    stack--;
}

void divide(Token **token) {
    stack[-1] = stack[-1] / stack[0];
    stack--;
}

void multiply(Token **token) {
    stack[-1] = stack[-1] * stack[0];
    stack--;
}

void dot_d(Token **token) {
    printf("%d ", (int) *stack--);
}

void dot(Token **token) {
    printf("%s ", (char *) *stack--);
}

struct {
    char *name;
    Native_f func;
} natives[] = {
    { "dup", dup },
    { "branch", branch },
    { "branch?", branch_cond },
    { "+", plus },
    { "-", minus },
    { "/", divide },
    { "*", multiply },
    { ".d", dot_d },
    { ".", dot }
};
