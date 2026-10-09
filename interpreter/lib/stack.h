#ifndef STACK
#define STACK

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef SANDBOXED
    #define MAX_STRING_LEN 512
    #define MAX_STACK_SIZE 256
#else
    #define MAX_STRING_LEN 64
    #define MAX_STACK_SIZE 32
#endif

typedef unsigned int uint;

/* A value contains both a number
and a string value. Which of those
values is used is dictated by the
type. */
typedef struct {
    float number;
    char string[MAX_STRING_LEN];

    /* If a value is stored
    anywhere, then this contains
    the parent pointer */
    uint pointer;
    /* Index position of a value
    in a table */
    int index;

    uint8_t type;
} value;

/* LIFO stack */
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
    return s->top == -1;
}

/**
 * @param s Stack
 * @returns Whether the provided stack is
 * full or not.
 */
bool stack_is_full(stack *s) {
    return s->top == MAX_STACK_SIZE - 1;
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

    s->items[s->top].pointer = v.pointer;
    s->items[s->top].index = v.index;

    return true;
}

/**
 * @brief Pops a value from the stack
 * @param s Stack
 * @returns The popped value
 */
value stack_pop(stack *s) {
    /* Stack underflow */
    if (stack_is_empty(s)) return (value){0};
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

/**
 * @brief Given a pointer and an index, a value
 * with matching parameters will be found and returned.
 * If none are found, then a default value is returned.
 */
value stack_find(stack *s, value p, value i) {
    uint pointer = (uint)p.number;
    uint index = (uint)i.number - 1; /* 1-indexed in .qz source, 0-indexed here */
    
    for (int i = 0; i <= s->top; i++) {
        if (
               s->items[i].pointer == pointer
            && s->items[i].index   == index
        ) {
            /* Pointer and index match what was
            provided, return */
            return s->items[i];
        }
    }
    /* Nothing found, default fallback */
    return (value){0};
}

#endif