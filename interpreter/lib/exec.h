#ifndef EXECUTOR
#define EXECUTOR

#include <stdint.h>
#include <stdbool.h>
#include "stack.h"
#include "codes.h"

#ifdef SANDBOXED
    #include "stdlib.h"

    /* ANSI color codes */
        #define RST        "\033[0m"
        #define RED        "\033[31m"
        #define GREEN      "\033[32m"
        #define YELLOW     "\033[33m"
        #define BLUE       "\033[34m"
        #define MAGENTA    "\033[35m"
        #define CYAN       "\033[36m"
        #define WHITE      "\033[37m"
        #define BOLD       "\033[1m"
        #define DIM        "\033[2m"
        #define EFFECT_RST "\033[22m"

    /**
     * @brief Formats a string(with a provided format)
     * and returns the result.
     */
    #define FORMAT(format, ...) ({  \
        static char _buf[256]; \
        snprintf(_buf, sizeof(_buf), format, __VA_ARGS__); \
        _buf; \
    })

    /* Prints <g>[label]:</g> content */
    #define LOG(label, content) printf("%s[%s]:%s %s\n", GREEN, label, RST, content);

    /**
     * @brief Prints <r>[error]:</r> content
     * and terminates the program
     */
    #define ERROR(content) ({ \
        printf("%s[error]:%s %s\n", RED, RST, content); \
        exit(1); \
    })
#endif

#define MAX_PGM_SIZE 256

typedef unsigned long ulong;
typedef unsigned int uint;

/* Program is stored as bytes */
uint8_t pgm[MAX_PGM_SIZE] = {0};
    /* Tracks program size(how many bytes
    are in the program)*/
    uint pgm_size  = 0;

    /* Where in the program the executor is */
    uint pgm_index = 0;

    /* Determines whether to program is being
    written to or not */
    bool pgm_writing = false;

        /* Keeps track of how many times START_PGM/END_PGM
        show up in a row, uninterrupted */
        uint8_t start_row = 0;
        uint8_t end_row = 0;

stack s;

void exec_init() {
    stack_init(&s);
}

void exec_write(uint8_t byte) {
    if (byte == START_PGM) {
        end_row = 0; /* START_PGM != END_PGM, reset */
        if (start_row >= MARKER_ROW) {
            /* Enough START_PGM bytes were encountered
            in a row, switch to writing mode */
            pgm_writing = true;

            start_row = 0;
            pgm_size  = 0; /* Delete previous program */

            stack_flush(&s);

            /* Don't write this byte!
            Because writing mode is on, without this
            return, this START_PGM byte will be written
            when it shouldn't be */
            return;

        } else start_row++;

    } else if (byte == END_PGM) {
        start_row = 0; /* END_PGM != START_PGM, reset */
        if (end_row >= MARKER_ROW) {
            /* Enough END_PGM bytes were encountered
            in a row, switch to run mode */
            pgm_writing = false;

            end_row = 0;
            pgm_size -= MARKER_ROW; /* Get rid of extraneous END_PGM bytes */
            pgm_index = 0;

            return;

        } else end_row++;
    } else {
        start_row = 0;
        end_row   = 0;
    }
    /* Write to the program, if writing mode is on */
    if (pgm_writing) pgm[pgm_size++] = byte;
}

/**
 * @brief Parses one byte in the program, alongside
 * extra bytes required for the byte's function to
 * work.
 */
void exec_step(void) {
    /* In writing mode; should not
    run */
    if (pgm_writing) return;
    if (pgm_index > pgm_size) {
        #ifdef SANDBOXED
            /* In sandbox mode the program
            should stop executing when it ends */
            exit(0);
        #else
            /* Clear the stack */
            stack_flush(&s);

            /* Loop back to the start
            of the program */
            pgm_index = 0;
        #endif
    }
    uint8_t byte = pgm[pgm_index++];

    switch (byte) {
        case NUMBER: {
            /* Four bytes; 32-bit float
            (single precision) */

            /* Extract from little endian */
            uint8_t endian_bytes[4] = {0};

            for (int i = 0; i < 4; i++) {
                /* Go through the next four bytes
                and store them in endian_bytes */
                endian_bytes[i] = pgm[pgm_index++];
            }

            union {
                uint8_t b[4];
                float f; /* Final float */
            } converter;

            for (int i = 0; i < 4; i++) {
                /* Apply byte values to the converter */
                converter.b[i] = endian_bytes[i];
            }

            #ifdef SANDBOXED
                LOG(
                    "number",
                    FORMAT("pushed number [%g] to stack", converter.f)
                );
            #endif

            stack_push(&s, (value){
                .number = converter.f,
                .type = NUMBER
            });

            break;
        }
    }
}

#endif