#include "common.hh"
#include "lex/lex.hh"
#include <iostream>
#include <print>

auto main() -> int {
    auto and_ = Token(TokenKind::And, 0, 3);
    auto or_  = Token(TokenKind::Or, 4, 6);
    auto plus = Token(TokenKind::Plus, 7, 8);

    std::println("{}, {}, {}", and_, or_, plus);

    return 0;
}
