#include "parse/parse.hh"

// ScopedGuard 实现
ScopedGuard::ScopedGuard(Parser* parser) : parser_(parser) {
    parser_->enter();
}

ScopedGuard::~ScopedGuard() {
    parser_->exit();
}

// ParseError 实现
void ParseError::emit(DiagCtxt& diag_ctx) const {
    // 使用诊断上下文创建诊断信息
    auto builder = diag_ctx.diag_builder(level(), message(), span())
                       .label(span(), message());

    builder.emit();
}

// Parser 实现
Parser::Parser(const SourceMap* source_map,
               std::vector<Token> tokens,
               u32 start_pos)
    : source_map_(source_map), tokens_(std::move(tokens)), cursor_(0),
      start_pos_(start_pos) {
    enter(); // 初始化游标栈
}

auto Parser::parse(DiagCtxt& diag_ctx) -> void {
    auto result = try_file_scope();
    if (result) {
        ast_.set_root(*result);
    } else {
        // 处理解析错误
        result.error().emit(diag_ctx);
    }
}

auto Parser::parse_error(const ParseError& error) -> void {
    // 这里可以处理错误，比如添加到错误列表或直接发射
    // 暂时直接发射错误
    DiagCtxt temp_diag_ctx;
    const_cast<ParseError&>(error).emit(temp_diag_ctx);
}

auto Parser::enter() -> void {
    cursor_stack_.push_back(cursor_);
}

auto Parser::exit() -> void {
    if (!cursor_stack_.empty()) {
        cursor_stack_.pop_back();
    }
}

auto Parser::finalize() -> Ast {
    return std::move(ast_);
}

auto Parser::peek(std::span<const TokenKind> expected) const -> bool {
    if (cursor_ + expected.size() >= tokens_.size()) {
        return false;
    }

    for (usize i = 0; i < expected.size(); ++i) {
        if (tokens_[cursor_ + i].kind != expected[i]) {
            return false;
        }
    }
    return true;
}

auto Parser::eat_token(TokenKind expected) -> bool {
    if (cursor_ >= tokens_.size()) {
        return false;
    }

    if (tokens_[cursor_].kind == expected) {
        ++cursor_;
        return true;
    }
    return false;
}

auto Parser::eat_tokens(usize amount) -> void {
    cursor_ += amount;
    if (cursor_ > tokens_.size()) {
        cursor_ = tokens_.size();
    }
}

auto Parser::next_token() -> Token {
    if (cursor_ >= tokens_.size()) {
        return Token(TokenKind::Eof, 0, 0);
    }
    return tokens_[cursor_++];
}

auto Parser::peek_next_token() const -> Token {
    if (cursor_ >= tokens_.size()) {
        return Token(TokenKind::Eof, 0, 0);
    }
    return tokens_[cursor_];
}

auto Parser::current_token() const -> Token {
    if (cursor_ == 0 || cursor_ > tokens_.size()) {
        return Token(TokenKind::Sof, 0, 0);
    }
    return tokens_[cursor_ - 1];
}

auto Parser::get_token(usize index) const -> Token {
    if (index >= tokens_.size()) {
        return Token(TokenKind::Eof, 0, 0);
    }
    return tokens_[index];
}

auto Parser::previous_token() const -> Token {
    if (cursor_ == 0) {
        return Token(TokenKind::Sof, 0, 0);
    }
    return tokens_[cursor_ - 1];
}

auto Parser::current_span() const -> Span {
    if (cursor_stack_.empty()) {
        return Span(0,
                    0); // 返回空跨度
    }

    usize start   = cursor_stack_.back();
    usize end     = cursor_;

    u32 start_pos = (start < tokens_.size()) ? tokens_[start].start : 0;
    u32 end_pos
        = (end < tokens_.size() && end > 0) ? tokens_[end - 1].end : start_pos;

    return Span(start_pos, end_pos).with_offset(start_pos_);
}

auto Parser::next_token_span() const -> Span {
    if (cursor_ >= tokens_.size()) {
        return Span(0, 0);
    }

    const auto& token = tokens_[cursor_];
    return Span(token.start, token.end).with_offset(start_pos_);
}

auto Parser::current_degree() const -> usize {
    return cursor_stack_.size();
}

// 解析方法实现（基础框架）
auto Parser::try_file_scope() -> ParseResult {
    // 使用 RAII
    // 守卫自动管理游标栈
    auto guard = scoped_guard();

    // 创建文件作用域节点
    NodeBuilder builder(NodeKind::FileScope, current_span());
    NodeIndex file_scope = ast_.add_node(builder);

    return file_scope;
}
