#ifndef LEXER
#define LEXER

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>

typedef unsigned long ulong;
typedef unsigned  int  uint;

typedef std::vector<std::string> string_vector;

/* A type that contains two values. */
template <typename L, typename R>
using pair = std::pair<L, R>;

/* A vector of pairs. */
template <typename L, typename R>
using pair_vector = std::vector<pair<L, R>>;

namespace Lexer {
    string_vector tokens; /* Token vector */
    pair_vector<ulong, ulong> rc_data; /* Row-column data */

    ulong row = 1; /* Line count; starts at one since documents
    do not start at zero lines */
    ulong column = 0; /* Character position in the line */

    char c;

    /**
     * @brief Reads and writes the next
     * character in the file to c whilst
     * advancing 'column'.
     * @param file File object
     * @returns Whether the character was actually
     * read or not
     */
    static bool getc(std::ifstream &file) {
        column++;
        if (file.get(c)) return true;
        return false;
    }

    /**
     * @brief Unreads the next character in the file
     * to c whilst regressing 'column'.
     * @param file File object
     */
    static void ungetc(std::ifstream &file) {
        column--;
        file.unget();
    }

    /**
     * @brief Adds a token to the token vector.
     * @param token_content Token content
     */
    static void add_token(const std::string &token_content) {
        tokens.push_back(token_content);
        rc_data.emplace_back(row, column);
    }

    /**
     * @brief Performs "lexer-ing". Returns the tokens
     * processed, alongside a symmetrical pair vector with
     * data pertaining to each token on row and column
     * @param input_path Input path (to source)
     * @returns Both the token vector and the row-column data vector
     * as a pair
     */
    auto process(const std::string &input_path) {
        tokens.clear();
        std::ifstream file(input_path);

        if (!file.is_open()) {
            /* The file failed to open */
            std::cout << "Could not open file \"" + input_path + "\"\n";
            std::exit(1);
        }

        /* Reset values */
        row = 1;
        column = 0;

        while (getc(file)) {
            /* Looping through each character in the input */

            /* Index overshoot occurs if the file is
            not at EOF when the loop stops */

            if (c == '\n') {
                /* New line; new row */
                row++;
                column = 0; /* Reset character position */
            }
            if (iswspace(c)) continue; /* Whitespace is skipped */

            if (c == '#') {
                /* Comment */
                while (getc(file) && !file.eof() && c != '\n');
                /* Unread so that the new cycle can see the newline */
                if (!file.eof()) ungetc(file);
                continue;
            }

            if (isdigit(c)) {
                /* Numeric */

                std::string s;
                s += c; /* Add the first digit */

                while (getc(file) && !file.eof() && isdigit(c) || c == '.') {
                    /* Continues while not at EOF and the character
                    is a number or dot(for floats) */
                    s += c;
                }

                if (!file.eof()) ungetc(file);

                add_token(s);
                continue;
            }
            if (isalpha(c)) {
                /* Alphabetic */

                std::string s;
                s += c;

                while (getc(file) && !file.eof() && isalnum(c) || c == '_') {
                    /* Continues while not at EOF and the character is
                    alphanumeric or an underscore */
                    s += c;
                }

                if (!file.eof()) ungetc(file);

                add_token(s);
                continue;
            }

            if (c == '"' || c == '\'') {
                /* String; both single and double
                quotes will suffice */
                char starting_quote = c; /* " or ' */

                std::string s;
                s += '"'; /* In token form, all strings
                are saved with a double quote, regardless
                or how they're actually written in source */

                while (getc(file) && !file.eof() && c != starting_quote) {
                    /* Continues while not at EOF and the character
                    does not match with the starting quote */

                    if (c == '\\') {
                        /* Escape sequence */
                        getc(file);

                        switch (c) {
                            case  'n': { s += '\n'; break; }
                            case  't': { s += '\t'; break; }
                            case  '"': { s +=  '"'; break; }
                            case '\\': { s += '\\'; break; }
                            default: { s += c; break; } /* No clue */
                        }
                    } else {
                        /* Just a normal character */
                        s += c;
                    }
                }

                /* No ending quote, since strings in token
                form always have a starting quote that no
                other token will have, it is redundant and
                unnecessary */

                add_token(s);
                continue;
            }

            if (c == '=' || c == '!' || c == '<' || c == '>') {
                std::string s;
                s += c;

                getc(file);
                if (c == '=') {
                    /* Two-character operator */
                    s += c;
                } else {
                    /* Just a single character */
                    if (!file.eof()) ungetc(file);
                }
                add_token(s);
                continue;
            }

            /* Everything else */
            std::string s(1, c);
            add_token(s);
        }

        file.close();

        /* Both tokens and rc_data are returned */
        pair<string_vector, pair_vector<ulong, ulong>> result(tokens, rc_data);
        return result;
    }
}

#endif