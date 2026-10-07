#include <iostream>
    #include "lib/lexer.hh"
    #include "lib/parser.hh"
    #include "lib/bytecode.hh"

int main(void) {
    auto [tokens, rc_data] = Lexer::process("main.qz");
    auto instruction_set = Parser::process(tokens, rc_data);

    Bytecode::process(instruction_set);

    return 0;
}