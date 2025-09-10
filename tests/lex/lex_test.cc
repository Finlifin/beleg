#include <gtest/gtest.h>
#include "lex/lex.hh"

class LexTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Setup code for each test
    }

    void TearDown() override {
        // Cleanup code for each test
    }
};

// Test TokenKind lexeme function
TEST_F(LexTest, TokenKindLexeme) {
    EXPECT_EQ(lexeme(TokenKind::Plus), "+");
    EXPECT_EQ(lexeme(TokenKind::Minus), "-");
    EXPECT_EQ(lexeme(TokenKind::Star), "*");
    EXPECT_EQ(lexeme(TokenKind::Slash), "/");
    EXPECT_EQ(lexeme(TokenKind::Eq), "=");
    EXPECT_EQ(lexeme(TokenKind::EqEq), "==");
    EXPECT_EQ(lexeme(TokenKind::LParen), "(");
    EXPECT_EQ(lexeme(TokenKind::RParen), ")");
}

// Test Token construction
TEST_F(LexTest, TokenConstruction) {
    Token token(TokenKind::Plus, 0, 1);
    EXPECT_EQ(token.kind, TokenKind::Plus);
    EXPECT_EQ(token.start, 0u);
    EXPECT_EQ(token.end, 1u);
}

// Test Lexer basic functionality
TEST_F(LexTest, LexerBasicTokens) {
    std::string_view source = "+";
    Lexer lexer(source);

    Token token = lexer.next();
    EXPECT_EQ(token.kind, TokenKind::Plus);
    EXPECT_EQ(token.start, 0u);
    EXPECT_EQ(token.end, 1u);
}

// Test Lexer with multiple tokens
TEST_F(LexTest, LexerMultipleTokens) {
    std::string_view source = "+ - * /";
    Lexer lexer(source);

    // Plus token
    Token token1 = lexer.next();
    EXPECT_EQ(token1.kind, TokenKind::Plus);

    // Minus token
    Token token2 = lexer.next();
    EXPECT_EQ(token2.kind, TokenKind::Minus);

    // Star token
    Token token3 = lexer.next();
    EXPECT_EQ(token3.kind, TokenKind::Star);

    // Slash token
    Token token4 = lexer.next();
    EXPECT_EQ(token4.kind, TokenKind::Slash);
}

// Test Lexer with identifiers
TEST_F(LexTest, LexerIdentifiers) {
    std::string_view source = "variable_name";
    Lexer lexer(source);

    Token token = lexer.next();
    EXPECT_EQ(token.kind, TokenKind::Id);
    EXPECT_EQ(token.start, 0u);
    EXPECT_EQ(token.end, 13u);
}

// Test Lexer with keywords
TEST_F(LexTest, LexerKeywords) {
    std::string_view source = "fn if else";
    Lexer lexer(source);

    // fn keyword
    Token token1 = lexer.next();
    EXPECT_EQ(token1.kind, TokenKind::Fn);

    // if keyword
    Token token2 = lexer.next();
    EXPECT_EQ(token2.kind, TokenKind::If);

    // else keyword
    Token token3 = lexer.next();
    EXPECT_EQ(token3.kind, TokenKind::Else);
}

// Test Lexer with numbers
TEST_F(LexTest, LexerNumbers) {
    std::string_view source = "123";
    Lexer lexer(source);

    Token token = lexer.next();
    EXPECT_EQ(token.kind, TokenKind::Int);
    EXPECT_EQ(token.start, 0u);
    EXPECT_EQ(token.end, 3u);
}

// Test Lexer with string literals
TEST_F(LexTest, LexerStringLiterals) {
    std::string_view source = R"("hello world")";
    Lexer lexer(source);

    Token token = lexer.next();
    EXPECT_EQ(token.kind, TokenKind::Str);
    EXPECT_EQ(token.start, 0u);
    EXPECT_EQ(token.end, 13u);
}

// Test Lexer end of file
TEST_F(LexTest, LexerEOF) {
    std::string_view source = "";
    Lexer lexer(source);

    Token token = lexer.next();
    EXPECT_EQ(token.kind, TokenKind::Eof);
}
