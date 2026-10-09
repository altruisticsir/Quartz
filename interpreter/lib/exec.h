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
        static char _buf[MAX_STRING_LEN]; \
        snprintf(_buf, sizeof(_buf), format, __VA_ARGS__); \
        _buf; \
    })

    #if defined(SANDBOXED)
        #define OUT(content) printf("%s\n", content);

        /**
         * @brief Returns the value of a value object
         * (number or string) as a string
         */
        static const char* val(value *v) {
            if (v->type == NUMBER) {
                return FORMAT("%g", v->number);
            } else {
                return v->string;
            }
        }
    #else
        static const char* val(value *v) {}
    #endif

    /* Prints <g>[label]:</g> content */
    #if defined(SANDBOXED) && defined(VERBOSE)
        #define LOG(label, content) OUT(FORMAT("%s[%s]:%s %s", GREEN, label, RST, content));
    #else
        #define LOG(label, content);
    #endif


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
value variables[32]; /* Holds variables and function + table pointers */

stack tables; /* Holds actual table content */

void exec_init() {
    stack_init(&s);
    stack_init(&tables);
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

            LOG(
                "number",
                FORMAT("pushed number [%g] to stack", converter.f)
            );

            stack_push(&s, (value){
                .number = converter.f,
                .type = NUMBER
            });

            break;
        }
        case STRING: {
            char str[MAX_STRING_LEN];
            uint len = 0; /* String length */

            /* Keep scanning string until terminator */
            for (;;) {
                str[len++] = pgm[pgm_index++];
                if (pgm[pgm_index] == STRING_END) break;
            }
            str[len++] = '\0';

            value result = {.type = STRING};
            strncpy(result.string, str, sizeof(result.string));

            LOG(
                "string",
                FORMAT("pushed string [%s] to stack", result.string)
            );

            stack_push(&s, result);

            break;
        }
        case TABLE: {
            value table_pointer = stack_pop(&s);
            int element_count = 0;

            for (;;) {
                /* Process one element */
                while (pgm[pgm_index] != SET) {
                    exec_step();
                }

                LOG(
                    "table",
                    "pushed value into table"
                );

                value element = stack_pop(&s);
                element.pointer = (uint)table_pointer.number;
                element.index = element_count++;

                stack_push(&tables, element);

                /* Still on element byte; skip first and then check */
                if (pgm[++pgm_index] == TABLE_END) break;
            }

            break;
        }
        case PRINT: {
            value content = stack_pop(&s);
            OUT(
                val(&content)
            );
            break;
        }
        case FETCH: {
            value variable_id = stack_pop(&s);
            value fetched = variables[(uint)variable_id.number];
            stack_push(&s, fetched);
            LOG(
                "variables",
                FORMAT("fetched and pushed value [%s] to stack", val(&fetched))
            )
            break;
        }
        case SET: {
            /* Save a variable with a value */
            value variable_id = stack_pop(&s);
            value content = stack_pop(&s);

            LOG(
                "variables",
                FORMAT("Set variable [%s] to [*%g]", val(&content), variable_id.number)
            )

            /* Store */
            variables[(uint)variable_id.number] = content;

            break;
        }
        case INDEX_ACCESS: {
            /* From variable */
            value table_pointer = stack_pop(&s);
            value index = stack_pop(&s);

            stack_push(&s, stack_find(&tables, table_pointer, index));

            break;
        }
        case ADD:
        case SUB:
        case MUL:
        case DIV: {
            value right = stack_pop(&s);
            value left  = stack_pop(&s);

            value result = {0};
            result.type = NUMBER;

            if (left.type == STRING || right.type == STRING) {
                if (byte == ADD) {
                    /* Attempt to concatenate string and number
                    (either left or right), convert number to string */
                    if (left.type == NUMBER) {
                        strncpy(left.string, FORMAT("%g", left.number), sizeof(left.string));
                        left.type = STRING;
                    }
                    if (right.type == NUMBER) {
                        strncpy(right.string, FORMAT("%g", right.number), sizeof(right.string));
                        right.type = STRING;
                    }

                    strncpy(result.string, FORMAT("%s%s", left.string, right.string), sizeof(result.string));
                    result.type = STRING;
                    
                    LOG(
                        "string",
                        FORMAT("pushed string [%s] to stack", result.string)
                    );
                } else {
                    result.type = NIL;
                    LOG(
                        "nil",
                        "pushed nil to stack"
                    );
                }
            } else {
                /* Basic number <=> number math */
                if (byte == ADD) result.number = left.number + right.number;
                if (byte == SUB) result.number = left.number - right.number;
                if (byte == MUL) result.number = left.number * right.number;
                if (byte == DIV) result.number = left.number / right.number;

                LOG(
                    "number",
                    FORMAT("pushed number [%g] to stack", result.number)
                );
            }
            
            stack_push(&s, result);
            break;
        }
    }
}

#endif