#include <iostream>
    #include "lib/lexer.hh"
    #include "lib/parser.hh"

int main(void) {
    auto [tokens, rc_data] = Lexer::process("main.qz");
    Parser::process(tokens, rc_data);
    return 0;
}