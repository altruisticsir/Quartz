#ifndef EXECUTOR
#define EXECUTOR

#include <stdint.h>
#include <stdbool.h>
#include "stack.h"

#define MAX_PGM_SIZE 256

typedef unsigned long ulong;

/* Program is stored as bytes */
uint8_t pgm[MAX_PGM_SIZE] = {0};
    /* Tracks program size(how many bytes
    are in the program)*/
    ulong pgm_size  = 0;
    /* Where in the program the executor is */
    ulong pgm_index = 0;

stack s;

void exec_init() {
    stack_init(&s);
}

void exec_write(uint8_t byte) {
    printf("%02X ", byte);
}

#endif