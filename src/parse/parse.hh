#ifndef PARSE_HH
#define PARSE_HH

#include "common.hh"
#include "ast/ast.hh"
#include "lex/lex.hh"
#include "source_map/source_map.hh"
#include "diag/diag.hh"
#include <vector>
#include <expected>

// 解析错误种类枚举
enum class ParseErrorKind {
    UnexpectedToken,
    ExpectedToken,
    InvalidToken,

    MissingSemicolon,
    MissingParenthesis,
    MissingBrace,
    UnexpectedEof,

    InternalError,
};

// 解析错误类型
class ParseError : public Issue {
  private:
    ParseErrorKind kind_;

  public:
    ParseError(Span span, String message, DiagLevel level = DiagLevel::Error)
        : Issue(span, std::move(message), level), kind_(ParseErrorKind::InternalError) {
    }

    ParseError(Span span, String message, ParseErrorKind kind, DiagLevel level = DiagLevel::Error)
        : Issue(span, std::move(message), level), kind_(kind) {
    }

    auto kind() const -> ParseErrorKind {
        return kind_;
    }

    void emit(DiagCtxt& diag_ctx) const override;
}; // 解析结果类型
using ParseResult = std::expected<NodeIndex, ParseError>;

// 前向声明
class Parser;

// RAII 游标栈守卫
class ScopedGuard {
  private:
    Parser* parser_;

  public:
    explicit ScopedGuard(Parser* parser);
    ~ScopedGuard();

    // 禁用拷贝
    ScopedGuard(const ScopedGuard&) = delete;
    ScopedGuard& operator=(const ScopedGuard&) = delete;

    // 允许移动
    ScopedGuard(ScopedGuard&&) = default;
    ScopedGuard& operator=(ScopedGuard&&) = default;
};

// 手写 PEG 解析器
class Parser {
  private:
    const SourceMap* source_map_;
    std::vector<Token> tokens_;
    Ast ast_;
    usize cursor_;
    std::vector<usize> cursor_stack_;
    u32 start_pos_;
    std::vector<ParseError> errors_;

  public:
    Parser(const SourceMap* source_map, std::vector<Token> tokens, u32 start_pos);

    // 主解析方法
    auto parse(DiagCtxt& diag_ctx) -> void;

    // 错误处理
    auto parse_error(const ParseError& error) -> void;

    // 游标栈管理
    auto enter() -> void;
    auto exit() -> void;

    // 创建作用域守卫
    auto scoped_guard() -> ScopedGuard {
        return ScopedGuard(this);
    }

    // 完成解析，返回 AST
    auto finalize() -> Ast;

    // Token 检查和消费
    auto peek(std::span<const TokenKind> expected) const -> bool;
    auto eat_token(TokenKind expected) -> bool;
    auto eat_tokens(usize amount) -> void;

    // Token 获取
    auto next_token() -> Token;
    auto peek_next_token() const -> Token;
    auto current_token() const -> Token;
    auto get_token(usize index) const -> Token;
    auto previous_token() const -> Token;

    // 跨度计算
    auto current_span() const -> Span;
    auto next_token_span() const -> Span;
    auto current_degree() const -> usize;

  private:
    // 解析方法（待实现）
    auto try_file_scope() -> ParseResult;
};

#endif