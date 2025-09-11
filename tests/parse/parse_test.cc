#include <gtest/gtest.h>
#include "parse/parse.hh"
#include "source_map/source_map.hh"

class ParseTest : public ::testing::Test {
  protected:
    SourceMap source_map_;
    std::vector<Token> tokens_;
    DiagCtxt diag_ctx_;

    void SetUp() override {
        // Setup code for each test
    }

    void TearDown() override {
        // Cleanup code for each test
    }
};

// 测试 ParseErrorKind 功能
TEST_F(ParseTest, ParseErrorKindTest) {
    Span test_span{5, 10};

    // 测试不同的错误种类
    ParseError unexpected_token_error(
        test_span, "Unexpected token", ParseErrorKind::UnexpectedToken);
    EXPECT_EQ(unexpected_token_error.kind(), ParseErrorKind::UnexpectedToken);

    ParseError missing_paren_error(
        test_span, "Missing parenthesis", ParseErrorKind::MissingParenthesis);
    EXPECT_EQ(missing_paren_error.kind(), ParseErrorKind::MissingParenthesis);

    ParseError eof_error(test_span, "Unexpected end of file", ParseErrorKind::UnexpectedEof);
    EXPECT_EQ(eof_error.kind(), ParseErrorKind::UnexpectedEof);

    // 测试默认构造函数使用 InternalError
    ParseError default_error(test_span, "Generic error");
    EXPECT_EQ(default_error.kind(), ParseErrorKind::InternalError);

    // 测试错误分类
    EXPECT_EQ(static_cast<int>(ParseErrorKind::UnexpectedToken), 0);
    EXPECT_EQ(static_cast<int>(ParseErrorKind::ExpectedToken), 1);
    EXPECT_EQ(static_cast<int>(ParseErrorKind::InvalidToken), 2);
    EXPECT_EQ(static_cast<int>(ParseErrorKind::MissingSemicolon), 3);
    EXPECT_EQ(static_cast<int>(ParseErrorKind::MissingParenthesis), 4);
    EXPECT_EQ(static_cast<int>(ParseErrorKind::MissingBrace), 5);
    EXPECT_EQ(static_cast<int>(ParseErrorKind::UnexpectedEof), 6);
    EXPECT_EQ(static_cast<int>(ParseErrorKind::InternalError), 7);
}

// 测试 ScopedGuard 的基本功能
TEST_F(ParseTest, ScopedGuardBasic) {
    // 创建一些测试 tokens
    tokens_ = {Token{TokenKind::Id, 0, 3},
               Token{TokenKind::Plus, 4, 5},
               Token{TokenKind::Int, 6, 8},
               Token{TokenKind::Eof, 8, 8}};

    Parser parser(&source_map_, tokens_, 0);

    // 初始状态：构造函数调用了 enter()，所以深度为 1
    EXPECT_EQ(parser.current_degree(), 1);

    // 测试 ScopedGuard 的 RAII 行为
    {
        auto guard = parser.scoped_guard();
        // 进入作用域后，深度为 2
        EXPECT_EQ(parser.current_degree(), 2);

        // 在作用域内再创建一个守卫
        {
            auto inner_guard = parser.scoped_guard();
            // 内部守卫，深度为 3
            EXPECT_EQ(parser.current_degree(), 3);
        }
        // inner_guard 退出后，应该回到深度 2
        EXPECT_EQ(parser.current_degree(), 2);
    }
    // guard 退出后，应该回到初始深度 1
    EXPECT_EQ(parser.current_degree(), 1);
}

// 测试解析器的基本功能
TEST_F(ParseTest, ParserBasic) {
    tokens_ = {Token{TokenKind::Eof, 0, 0}};

    Parser parser(&source_map_, tokens_, 0);

    // 测试基本方法
    EXPECT_EQ(parser.current_token().kind, TokenKind::Sof);
    EXPECT_EQ(parser.peek_next_token().kind, TokenKind::Eof);

    // 测试游标移动
    auto token = parser.next_token();
    EXPECT_EQ(token.kind, TokenKind::Eof);
    EXPECT_EQ(parser.current_token().kind, TokenKind::Eof);
}

// 测试 ScopedGuard 在实际解析中的使用
TEST_F(ParseTest, ScopedGuardUsageExample) {
    tokens_ = {Token{TokenKind::Id, 0, 3},       // function name
               Token{TokenKind::LParen, 3, 4},   // (
               Token{TokenKind::Id, 4, 5},       // param
               Token{TokenKind::RParen, 5, 6},   // )
               Token{TokenKind::LBrace, 7, 8},   // {
               Token{TokenKind::Id, 9, 12},      // return value
               Token{TokenKind::RBrace, 13, 14}, // }
               Token{TokenKind::Eof, 14, 14}};

    Parser parser(&source_map_, tokens_, 0);

    // 模拟解析函数定义
    auto result = [&]() -> ParseResult {
        auto guard = parser.scoped_guard();

        // 解析函数名
        if (parser.peek_next_token().kind != TokenKind::Id) {
            return std::unexpected(ParseError{
                parser.current_span(), "Expected function name", ParseErrorKind::ExpectedToken});
        }
        parser.next_token();

        // 解析参数列表
        if (!parser.eat_token(TokenKind::LParen)) {
            return std::unexpected(ParseError{
                parser.current_span(), "Expected '('", ParseErrorKind::MissingParenthesis});
        }

        {
            auto param_guard = parser.scoped_guard();
            // 解析参数
            if (parser.peek_next_token().kind == TokenKind::Id) {
                parser.next_token();
            }
        }

        if (!parser.eat_token(TokenKind::RParen)) {
            return std::unexpected(ParseError{
                parser.current_span(), "Expected ')'", ParseErrorKind::MissingParenthesis});
        }

        // 解析函数体
        if (!parser.eat_token(TokenKind::LBrace)) {
            return std::unexpected(
                ParseError{parser.current_span(), "Expected '{'", ParseErrorKind::MissingBrace});
        }

        {
            auto body_guard = parser.scoped_guard();
            // 解析函数体内容
            if (parser.peek_next_token().kind == TokenKind::Id) {
                parser.next_token();
            }
        }

        if (!parser.eat_token(TokenKind::RBrace)) {
            return std::unexpected(
                ParseError{parser.current_span(), "Expected '}'", ParseErrorKind::MissingBrace});
        }

        // 创建函数定义节点
        NodeBuilder builder(NodeKind::FunctionDef, parser.current_span());
        NodeIndex node_idx = 0; // 临时节点索引
        // 注意：这里只是演示 ScopedGuard 的使用，实际的 AST 操作需要通过其他方式
        return node_idx;
    }();

    // 验证解析成功（简化验证）
    EXPECT_TRUE(result.has_value());
}

// 测试 ParseError 继承 Issue 的功能
TEST_F(ParseTest, ParseErrorInheritance) {
    Span test_span{10, 20};
    String test_message = "Test parse error";
    DiagLevel test_level = DiagLevel::Error;

    ParseError error(test_span, test_message, ParseErrorKind::InternalError, test_level);

    // 测试继承的属性访问
    EXPECT_EQ(error.span().start, 10);
    EXPECT_EQ(error.span().end, 20);
    EXPECT_EQ(error.message(), test_message);
    EXPECT_EQ(error.level(), test_level);

    // 测试多态性
    Issue* issue_ptr = &error;
    EXPECT_EQ(issue_ptr->span().start, 10);
    EXPECT_EQ(issue_ptr->message(), test_message);
    EXPECT_EQ(issue_ptr->level(), test_level);
}
