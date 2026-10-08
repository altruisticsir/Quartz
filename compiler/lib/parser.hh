#ifndef PARSER
#define PARSER

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "output.hh"

typedef unsigned long ulong;
typedef unsigned  int  uint;

typedef std::vector<std::string> string_vector;

/* A type that contains two values. */
template <typename L, typename R>
using pair = std::pair<L, R>;

/* A vector of pairs. */
template <typename L, typename R>
using pair_vector = std::vector<pair<L, R>>;

/* vector start */

    /* Initially always false */
    template <typename T>
    struct is_vector : std::false_type {};

    /* Returns true if the provided type matches
    std::vector<T, Alloc> */
    template <typename T, typename Alloc>
    struct is_vector<std::vector<T, Alloc>> : std::true_type {};

    /* Vector checker */
    template <typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;

/* end */

/**
 * @param str String
 * @returns Whether the provided string is
 * completely numeric or not
 */
static bool is_numeric(const std::string &str) {
    std::istringstream iss(str);
    double d;
    return (iss >> d) && (iss.eof());
}

/**
 * @brief Replaces all instances of a specified
 * value in a provided vector with something new
 * @param vector Target vector
 * @param old_value Old
 * @param new_value New
 */
template <typename V, typename T>
static void replace(V &vector, T old_value, T new_value) {
    std::replace(vector.begin(), vector.end(), old_value, new_value);
}

/**
 * @param vector Target vector
 * @param target Target value
 * @returns Whether a specified value exists
 * in the provided vector
 */
template <typename V, typename T>
static bool find(V &vector, T target) {
    auto iterator = std::find(vector.begin(), vector.end(), target);
    return (iterator != vector.end());
}

/**
 * @brief Pushes items into a provided vector.
 * @param vector Target vector
 * @param args The items; takes
 * any quantity
 */
template <typename V, typename... Args>
static void push(V &vector, const Args&... args) {
    auto push_handler = [&vector](const auto &x) {
        using T = std::decay_t<decltype(x)>;

        if constexpr(is_vector_v<T>) {
            for (auto p : x) push(vector, p);
        } else vector.push_back(x);
    };
    (push_handler(args), ...);
}

/**
 * @brief Pops the top value from a
 * provided vector.
 * @param vector Target vector
 * @returns Popped value
 */
template <typename V>
static auto pop(V &vector) {
    auto back = vector.back();
    vector.pop_back();
    return back;
}

/**
 * @returns A unique ID value that has not
 * been issued before
 */
static uint unique_id() {
    static uint ids_issued = 0; /* Initialized once */
    return ids_issued++;
}

namespace Parser {
    ulong index = 0;

    string_vector tokens;
    pair_vector<ulong, ulong> rc_data;

    string_vector instruction_set;
    string_vector tables;

    /* List of words that are keywords;
    will be used in evaluate to determine 
    what is an identifier and what is not */
    string_vector keywords = {
        "let"
    };

    static void parse_chunk();

    /**
     * @brief Skips the index by
     * a specified amount.
     * @param by Skip by (default 0)
     */
    static void skip(uint by = 0) {
        index += by;
        if (index >= tokens.size()) Output::error("unpaired paren/bracket");
    }

    /**
     * @brief Returns the token content
     * with a specified offset.
     * @param offset Offset (default 0)
     */
    static std::string peek(uint offset = 0) {
        if (index + offset >= tokens.size()) {
            /* Out of bounds, fall back to maximum value */
            return tokens[tokens.size() - 1];
        }
        return tokens[index + offset];
    }

    /**
     * @brief Combines peek and skip.
     * @param offset Offset (default 0)
     */
    static std::string get(uint offset = 0) {
        skip(offset);
        return peek();
    }

    /**
     * @brief Processes the row-column data
     * at the current index and returns a clean
     * string format of the information inside.
     * @returns (row x, column y)
     */
    static std::string get_rc_data() {
        const std::string &row = std::to_string(rc_data[index].first);
        const std::string &column = std::to_string(rc_data[index].second);
        return std::string("(line:" + row + " char:" + column + ")");
    }

    /**
     * @brief Checks if the token at a specific offset
     * matches what is expected. If not, then the program
     * will be terminated.
     * @param expected What was expected at the index(+offset)
     * @param offset Offset
     */
    static void expect(const std::string &expected, uint offset = 0) {
        if (get(offset) != expected) {
            Output::error("Expected \"" + expected + "\", got \"" + get() + "\" instead " + get_rc_data());
            std::exit(1);
        }
    }

    /**
     * @brief Pushes instructions into the
     * instruction set.
     * @param args The arguments; takes
     * any quantity
     */
    template <typename... Args>
    static void add_instructions(const Args&... args) {
        /* Deduces the type of the provided value
        and appropriately handles it */
        auto instruction_handler = [](const auto &x) {
            /* Type of x */
            using T = std::decay_t<decltype(x)>;
            
            if constexpr (is_vector_v<T>) {
                /* Current argument is a vector; loop through
                and add each element inside to the instruction
                set */
                for (auto i : x) push(instruction_set, i);

            } else if constexpr (
                   std::is_same_v<T, std::string>
                || std::is_same_v<T, const char*>
                || std::is_convertible_v<T, std::string>
            ) {
                /* Current argument is a string; add to the
                instruction set directly */
                push(instruction_set, x);

            } else {
                /* Unknown */
                Output::error("Attempted to add an unhandled type to the instruction set");
            }
        };

        /* Runs each provided argument
        through the lambda function above */
        (instruction_handler(args), ...);
    }

    /**
     * @brief Rearranges an expression to conform to
     * LIFO stack standards. For example, 3 * 3 + 2
     * becomes 3 3 * 2 +.
     */
    static string_vector evaluate() {
        if (peek() != "{") {
            string_vector stack;
            string_vector queue;

            /* Defining precedence */
            std::unordered_map<std::string, uint> precedence = {
                {"+", 1},
                {"-", 1},
                {"*", 2},
                {"/", 2},
            };
            bool expecting_operator = false; /* If there is an operator or value when
            there shouldn't be, the loop will stop */

            /**
             * @brief Checks that the next token matches with what is expected
             * (operators and values). If not, then give the go-ahead to break
             * the loop
             * @param expected Expected state
             * @returns Whether or not the loop this lambda is called in
             * should break
             */
            auto op_check = [&expecting_operator](bool expected) {
                if (expecting_operator != expected) {
                    /* Loop will break */
                    index--;
                    return true;
                }
                expecting_operator = !expected;
                return false;
            };

            while (index < tokens.size()) {
                std::string t = get();

                if (is_numeric(t)) {
                    /* Number literal */
                    if (op_check(false)) break;

                    /* NUMBER x */
                    push(stack, "NUMBER", t);

                } else if (t[0] == '"') {
                    /* String literal */
                    if (op_check(false)) break;

                    push(stack, "STRING", t, "STRING_END");

                    /* Quotation mark is
                    retained so that the
                    instruction => byte
                    converter can tell what
                    is a string and what is
                    not */

                } else if (t == "nil") {
                    /* Nil value */
                    if (op_check(false)) break;
                    push(stack, "NIL");

                } else if (t == "(") {
                    /* Opening paren; capture content */
                    push(queue, t);
                    
                } else if (t == ")") {
                    /* Closing paren; dump what was captured
                    onto the stack */
                    while (queue.size() > 0 && queue.back() != "(") {
                        /* Pop from queue and add to stack */
                        push(stack, pop(queue));
                    }

                    /* Try to find an opening paren */

                    if (find(queue, "(")) queue.pop_back();
                    else break;

                } else if (precedence.count(t)) {
                    /* Token is some kind of operator +-/* */
                    if (op_check(true)) break;

                    while (queue.size() > 0) {
                        const std::string &top = queue.back();
                        if (!precedence.count(top)) break;

                        if (precedence[top] >= precedence[t]) {
                            push(stack, pop(queue));
                        } else break;
                    }

                    push(queue, t);
                } else {
                    if (find(keywords, t)) {
                        index--;
                        break;
                    }
                    if (t == "}") {
                        index--;
                        break;
                    }
                    if (t == ",") break;

                    if (op_check(false)) break;

                    if (peek(1) != "[") {
                        /* Definitely a variable;
                        add fetch commands from ID */

                        /* TABLE_FETCH if the variable name is linked to
                        a table, otherwise regular FETCH */
                        push(stack, "NUMBER", "*" + t, find(tables, t) ? "TABLE_FETCH" : "FETCH");
                    } else {
                        /* Index access from a table */
                        /* evaluate() is the index in question */
                        skip(2);
                        push(stack, evaluate(), "NUMBER", "*" + t, "INDEX_ACCESS");
                    }
                }

                index++;
            }

            /* Dump what's left of the queue
            into the stack */
            while (queue.size() > 0) {
                push(stack, pop(queue));
            }

            replace(stack, "+", "ADD");
            replace(stack, "-", "SUB");
            replace(stack, "*", "MUL");
            replace(stack, "/", "DIV");

            return stack;
        } else {
            /* Table ID, <start> ... SET ... SET <end> TABLE_END NUMBER *x SET */
            
            /* Evaluating a table */
            string_vector stack;

            skip(1); /* skip { */

            /* & indicates a table pointer. Tables are stored separately from variables. */
            push(stack, "NUMBER", "&" + std::to_string(unique_id()));

            /* Pushes a table pointer to the stack
            (so just the table ID) */
            push(stack, "TABLE");

            while (peek() != "}") {
                /* While the table isn't at its end */
                push(stack, evaluate(), "SET"); /* Set lets the 
                interpreter know when to push an element onto the table */
                skip(1); /* to , or } */
            }

            push(stack, "TABLE_END");
            /* When fetching a table, it still uses *.
            But, it uses TABLE_FETCH instead of FETCH. */

            return stack;
        }
    }
    
    /**
     * @brief Captures the contents of an entire
     * block of code(within {}).
     * @returns The block
     */
    static void collect_block() {
        expect("{");
        uint depth = 0;

        while (index < tokens.size()) {
            const std::string &t = get();
            parse_chunk();

            if (t == "{") depth++;
            if (t == "}") depth--;

            if (depth == 0) {
                /* All blocks escaped, stop collecting */
                return;
            }

            index++;
        }
        Output::error("Block has no end " + get_rc_data());
    }

    /**
     * @brief Parses a single token alongside any
     * other tokens required for its functionality
     */
    static void parse_chunk() {
        /* Loop through each token */
        const std::string &t = tokens[index];
            
        if (t == "let") {
            /* Variable assignment */
            const std::string &name = get(1);
            expect("=", 1);
            skip(1); /* Skip '=' */

            auto evaluation = evaluate();

            if (evaluation[0] == "TABLE") {
                /* Assignment of a table;
                register the table by variable
                name */
                push(tables, name);
            }

            add_instructions(
                evaluation,
                "NUMBER", "*" + name,
                "SET"
            );
        }
        if (t == "for") {
            /* For loop */
            /* Loop variable name(like i)*/
            const std::string &name = get(1);
            expect("=", 1);
            skip(1);

            /* Start value */
            string_vector start_evaluation = evaluate();
            expect("to", 1);
            skip(1);

            /* End value */
            string_vector end_evaluation = evaluate();
            skip(1);

            add_instructions(
                end_evaluation,
                start_evaluation,
                "NUMBER",
                "*" + name, /* * indicates a variable pointer. */
                "FOR"
            );

            collect_block();
            add_instructions("LOOP_END");
        }
    }

    string_vector process(string_vector t, pair_vector<ulong, ulong> r) {
        index  = 0;
        instruction_set.clear();

        tokens = t;
        rc_data = r;

        /* START_PGM and END_PGM are repeated
        so that the interpreter has NO doubt on
        the intent of the program structure */
        for (int i = 0; i < 8; i++)
            add_instructions("START_PGM");

        while (index < tokens.size()) {
            parse_chunk();
            index++;
        }

        for (int i = 0; i < 8; i++)
            add_instructions("END_PGM");

        for (auto i : instruction_set)
            std::cout << i << ' ';
        
        return instruction_set;
    }
}

#endif