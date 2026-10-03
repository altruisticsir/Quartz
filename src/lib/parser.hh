#ifndef PARSER
#define PARSER

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>

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

    /* Returns false for any type that is not
    a vector */
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

namespace Parser {
    ulong index = 0;

    string_vector tokens;
    pair_vector<ulong, ulong> rc_data;

    string_vector instruction_set;

    /**
     * @brief Skips the index by
     * a specified amount.
     * @param by Skip by (default 0)
     */
    static void skip(uint by = 0) {
        index += by;
    }

    /**
     * @brief Returns the token content
     * with a specified offset.
     * @param offset Offset (default 0)
     */
    static std::string peek(uint offset = 0) {
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
        return std::string("(row " + row + ", column " + column + ")");
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
            std::cout << "Expected \"" << expected << "\", got \"" << get() << "\" instead " << get_rc_data() << '\n';
            std::exit(1);
        }
    }

    /**
     * @brief Rearranges an expression to conform to
     * LIFO stack standards. For example, 3 * 3 + 2
     * becomes 3 3 * 2 +.
     */
    static string_vector evaluate() {
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
                stack.push_back("NUMBER");
                stack.push_back(t);

            } else if (t[0] == '"') {
                /* String literal */
                if (op_check(false)) break;

                stack.push_back("STRING");
                stack.push_back(t);
                stack.push_back("STRING_END");

                /* Quotation mark is
                retained so that the
                instruction => byte
                converter can tell what
                is a string and what is
                not */

            } else if (t == "nil") {
                /* Nil value */
                if (op_check(false)) break;
                stack.push_back("NIL");

            } else if (t == "(") {
                /* Opening paren; capture content */
                queue.push_back(t);
                
            } else if (t == ")") {
                /* Closing paren; dump what was captured
                onto the stack */
                while (queue.size() > 0 && queue.back() != "(") {
                    /* Pop from queue and add to stack */
                    stack.push_back(queue.back());
                    queue.pop_back();
                }

                /* Try to find an opening paren */
                auto search = std::find(queue.begin(), queue.end(), "(");
                if (search != queue.end()) queue.pop_back();
                else break;

            } else if (precedence.count(t)) {
                /* Token is some kind of operator +-/* */
                if (op_check(true)) break;

                while (queue.size() > 0) {
                    const std::string &top = queue.back();
                    if (!precedence.count(top)) break;

                    if (precedence[top] >= precedence[t]) {
                        stack.push_back(queue.back());
                        queue.pop_back();
                    } else break;

                    index++;
                }

                queue.push_back(t);
            } else {
                /* Some unknown symbol */
                if (op_check(false)) break;
            }

            index++;
        }

        /* Dump what's left of the queue
        into the stack */
        while (queue.size() > 0) {
            stack.push_back(queue.back());
            queue.pop_back();
        }

        replace(stack, "+", "ADD");
        replace(stack, "-", "SUB");
        replace(stack, "*", "MUL");
        replace(stack, "/", "DIV");

        return stack;
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
                for (auto i : x) instruction_set.push_back(i);

            } else if constexpr (
                   std::is_same_v<T, std::string>
                || std::is_same_v<T, const char*>
                || std::is_convertible_v<T, std::string>
            ) {
                /* Current argument is a string; add to the
                instruction set directly */
                instruction_set.push_back(x);

            } else {
                /* Unknown */
                std::cout << "Attempted to add unhandled type to instruction set\n";
            }
        };

        /* Runs each provided argument
        through the lambda function above */
        (instruction_handler(args), ...);
    }

    void process(string_vector t, pair_vector<ulong, ulong> r) {
        index  = 0;

        tokens = t;
        rc_data = r;

        while (index < tokens.size()) {
            /* Loop through each token */
            const std::string &t = tokens[index];
            
            if (t == "let") {
                /* Variable assignment */
                const std::string &name = get(1);
                expect("=", 1);
                skip(1); /* Skip '=' */

                add_instructions(evaluate(), "*" + name, "SET");
            }

            index++;
        }
        for (auto i : instruction_set) std::cout << i << '\n';
    }
}

#endif