#ifndef STACK
#define STACK

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_STRING_LEN 64
#define MAX_STACK_SIZE 32

typedef unsigned int uint;

/* A value contains both a number
and a string value. Which of those
values is used is dictated by the
type. */
typedef struct {
    float number;
    char string[MAX_STRING_LEN];
    uint8_t type;
} value;

typedef struct {
    value items[MAX_STACK_SIZE];
    uint top;
} stack;

/**
 * @brief Initializes the provided stack.
 * @param s Stack
 */
void stack_init(stack *s) {
    s->top = -1;
}

/**
 * @param s Stack
 * @returns Whether the provided stack is
 * empty or not.
 */
bool stack_is_empty(stack *s) {
    return s->top <= -1;
}

/**
 * @param s Stack
 * @returns Whether the provided stack is
 * full or not.
 */
bool stack_is_full(stack *s) {
    return s->top >= MAX_STACK_SIZE - 1;
}

/**
 * @brief Pushes a value into the stack.
 * @param s Stack
 * @param v Value
 * @returns Whether the push was successful
 * or not
 */
bool stack_push(stack *s, value v) {
    /* Stack overflow */
    if (stack_is_full(s)) return false;
    s->top++;

    s->items[s->top].number = v.number;
    strcpy(s->items[s->top].string, v.string);
    s->items[s->top].type = v.type;

    return true;
}

/**
 * @brief Pops a value from the stack
 * @param s Stack
 * @returns The popped value
 */
value stack_pop(stack *s) {
    /* Stack underflow */
    if (stack_is_empty) return (value){0};
    value v = s->items[s->top--];
    return v;
}

/**
 * @brief Empties the stack
 * @param s Stack
 */
void stack_flush(stack *s) {
    s->top = -1;
}

#endif