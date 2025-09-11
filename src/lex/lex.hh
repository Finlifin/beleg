#ifndef LEX_HH
#define LEX_HH

#include "common.hh"
#include <format>
#include <string_view>

enum class TokenKind {
    // operators
    Plus,       // +
    PlusEq,     // +=
    PlusPlus,   // ++
    Lt,         // <
    LtEq,       // <=
    Gt,         // >
    GtEq,       // >=
    Bang,       // !
    BangEq,     // !=
    Minus,      // -
    Arrow,      // ->
    MinusEq,    // -=
    Dot,        // .
    Colon,      // :
    Star,       // *
    StarEq,     // *=
    Slash,      // /
    SlashEq,    // /=
    Percent,    // %
    PercentEq,  // %=
    Eq,         // =
    FatArrow,   // =>
    EqEq,       // ==
    Tilde,      // ~
    Pipe,       // |
    Hash,       // #
    Question,   // ?
    Backslash,  // {\}
    Ampersand,  // &
    LBracket,   // [
    RBracket,   // ]
    LParen,     // (
    RParen,     // )
    LBrace,     // {
    RBrace,     // }
    Comma,      // ,
    Quote,      // '
    Semi,       // ;
    Caret,      // ^
    Dollar,     // $
    At,         // @
    Underscore, // _

    // primitive literals
    Str,     // "..."
    Int,     // 123
    IntBin,  // 0b1010
    IntOct,  // 0o777
    IntHex,  // 0xFF
    Real,    // 123.45
    RealSci, // 1.23e-4 (scientific notation)
    Char,    // 'a' or '\n' or '\x{1F600}'

    // keywords
    And,       // and
    As,        // as
    Bool,      // bool
    Break,     // break
    Catch,     // catch
    Const,     // const
    Continue,  // continue
    Else,      // else
    Enum,      // enum
    Error,     // error
    Extern,    // extern
    False,     // false
    Fn,        // fn
    For,       // for
    If,        // if
    In,        // in
    Inline,    // inline
    Is,        // is
    Let,       // let
    Match,     // match
    Mod,       // mod
    Newtype,   // newtype
    Not,       // not
    Null,      // null
    Or,        // or
    Private,   // private
    Ref,       // ref
    Return,    // return
    SelfLower, // self
    SelfCap,   // Self
    Static,    // static
    Struct,    // struct
    Test,      // test
    True,      // true
    Typealias, // typealias
    Union,     // union
    Use,       // use
    When,      // when
    While,     // while

    // others
    Id,      // identifier
    Comment, // -- comment or {- comment -}
    Invalid, // invalid token
    Sof,     // start of file
    Eof,     // end of file

};

auto lexeme(TokenKind kind) -> std::string_view;

struct Token {
    TokenKind kind;
    u32 start;
    u32 end;

    Token(TokenKind kind, u32 start, u32 end)
        : kind(kind), start(start), end(end) {
    }
};

template <>
struct std::formatter<TokenKind> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(TokenKind kind, std::format_context& ctx) const {
        std::string_view str = lexeme(kind);
        return std::format_to(ctx.out(), "{}", str);
    }
};

template <>
struct std::formatter<Token> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(Token token, std::format_context& ctx) const {
        return std::format_to(ctx.out(),
                              "Token({}, {}, {})",
                              token.kind,
                              token.start,
                              token.end);
    }
};

struct Lexer {
    std::string_view src;
    usize cursor;

    Lexer(std::string_view src) : src(src), cursor(0) {
    }

    auto next() -> Token;

    auto current_char() -> std::optional<char> {
        if (cursor >= src.size()) {
            return std::nullopt;
        }
        return src[cursor];
    }

    auto peek(std::string_view str) -> bool;

    auto recognize_identifier() -> Token;
    auto recognize_string_literal() -> Token;
    auto recognize_number() -> Token;
    auto recognize_char() -> Token;
    auto is_keyword(std::string_view ident) -> std::optional<TokenKind>;
};
#endif