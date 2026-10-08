#include <iostream>
    #include "lib/lexer.hh"
    #include "lib/parser.hh"
    #include "lib/bytecode.hh"
    #include "lib/output.hh"

int main(void) {
    auto [tokens, rc_data] = Lexer::process("main.qz");
    auto instruction_set = Parser::process(tokens, rc_data);
    auto byte_set = Bytecode::process(instruction_set);

    Output::write("bin/qz.bin", byte_set);

    return 0;
}