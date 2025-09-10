#include "lex/lex.hh"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== Lex Module Functional Demo ===" << std::endl;

    // Demo 1: Basic tokenization
    std::cout << "\n1. Basic Tokenization Demo:" << std::endl;
    std::string_view source1 = "fn main() { let x = 42 + 13; }";
    std::cout << "Source: " << source1 << std::endl;

    Lexer lexer1(source1);
    std::vector<Token> tokens;

    while (true) {
        Token token = lexer1.next();
        tokens.push_back(token);
        if (token.kind == TokenKind::Eof) {
            break;
        }
    }

    std::cout << "Tokens found: " << tokens.size() - 1 << " (excluding EOF)" << std::endl;
    for (size_t i = 0; i < tokens.size() - 1; ++i) {
        const auto& token = tokens[i];
        std::cout << "  [" << i << "] " << "Kind: " << static_cast<int>(token.kind)
                  << ", Start: " << token.start << ", End: " << token.end << ", Text: '"
                  << source1.substr(token.start, token.end - token.start) << "'" << std::endl;
    }

    // Demo 2: Keyword vs Identifier recognition
    std::cout << "\n2. Keyword vs Identifier Demo:" << std::endl;
    std::string_view source2 = "fn function if identifier";
    std::cout << "Source: " << source2 << std::endl;

    Lexer lexer2(source2);
    while (true) {
        Token token = lexer2.next();
        if (token.kind == TokenKind::Eof) {
            break;
        }

        std::string_view token_text = source2.substr(token.start, token.end - token.start);
        if (token.kind == TokenKind::Fn) {
            std::cout << "  '" << token_text << "' -> Keyword (fn)" << std::endl;
        } else if (token.kind == TokenKind::If) {
            std::cout << "  '" << token_text << "' -> Keyword (if)" << std::endl;
        } else if (token.kind == TokenKind::Id) {
            std::cout << "  '" << token_text << "' -> Identifier" << std::endl;
        }
    }

    // Demo 3: Number literals
    std::cout << "\n3. Number Literals Demo:" << std::endl;
    std::string_view source3 = "123 0xFF 0b1010 123.45 1.23e-4";
    std::cout << "Source: " << source3 << std::endl;

    Lexer lexer3(source3);
    while (true) {
        Token token = lexer3.next();
        if (token.kind == TokenKind::Eof) {
            break;
        }

        std::string_view token_text = source3.substr(token.start, token.end - token.start);
        switch (token.kind) {
        case TokenKind::Int:
            std::cout << "  '" << token_text << "' -> Integer literal" << std::endl;
            break;
        case TokenKind::IntHex:
            std::cout << "  '" << token_text << "' -> Hex integer literal" << std::endl;
            break;
        case TokenKind::IntBin:
            std::cout << "  '" << token_text << "' -> Binary integer literal" << std::endl;
            break;
        case TokenKind::Real:
            std::cout << "  '" << token_text << "' -> Real number literal" << std::endl;
            break;
        case TokenKind::RealSci:
            std::cout << "  '" << token_text << "' -> Scientific notation literal" << std::endl;
            break;
        default:
            break;
        }
    }

    // Demo 4: Operators
    std::cout << "\n4. Operators Demo:" << std::endl;
    std::string_view source4 = "+ += ++ == != -> =>";
    std::cout << "Source: " << source4 << std::endl;

    Lexer lexer4(source4);
    while (true) {
        Token token = lexer4.next();
        if (token.kind == TokenKind::Eof) {
            break;
        }

        std::string_view token_text = source4.substr(token.start, token.end - token.start);
        std::cout << "  '" << token_text << "' -> " << lexeme(token.kind) << std::endl;
    }

    std::cout << "\nLex demo completed successfully!" << std::endl;
    return 0;
}
