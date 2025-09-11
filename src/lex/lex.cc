#include "lex/lex.hh"

auto lexeme(TokenKind kind) -> std::string_view {

#define CASE(kind, lex)                                                        \
    case TokenKind::kind:                                                      \
        return lex;

    switch (kind) {
        CASE(Plus, "+")
        CASE(PlusEq, "+=")
        CASE(PlusPlus, "++")
        CASE(Lt, "<")
        CASE(LtEq, "<=")
        CASE(Gt, ">")
        CASE(GtEq, ">=")
        CASE(Bang, "!")
        CASE(BangEq, "!=")
        CASE(Minus, "-")
        CASE(Arrow, "->")
        CASE(MinusEq, "-=")
        CASE(Dot, ".")
        CASE(Colon, ":")
        CASE(Star, "*")
        CASE(StarEq, "*=")
        CASE(Slash, "/")
        CASE(SlashEq, "/=")
        CASE(Percent, "%")
        CASE(PercentEq, "%=")
        CASE(Eq, "=")
        CASE(FatArrow, "=>")
        CASE(EqEq, "==")
        CASE(Tilde, "~")
        CASE(Pipe, "|")
        CASE(Hash, "#")
        CASE(Question, "?")
        CASE(Backslash, R"(\)")
        CASE(Ampersand, "&")
        CASE(LBracket, "[")
        CASE(RBracket, "]")
        CASE(LParen, "(")
        CASE(RParen, ")")
        CASE(LBrace, "{")
        CASE(RBrace, "}")
        CASE(Comma, ",")
        CASE(Quote, "'")
        CASE(Semi, ";")
        CASE(Caret, "^")
        CASE(Dollar, "$")
        CASE(At, "@")
        CASE(Underscore, "_")
        CASE(And, "and")
        CASE(As, "as")
        CASE(Bool, "bool")
        CASE(Break, "break")
        CASE(Catch, "catch")
        CASE(Char, "<character_literal>")
        CASE(Comment, "<comment>")
        CASE(Const, "const")
        CASE(Continue, "continue")
        CASE(Else, "else")
        CASE(Enum, "enum")
        CASE(Eof, "<end_of_file>")
        CASE(Error, "error")
        CASE(Extern, "extern")
        CASE(False, "false")
        CASE(Fn, "fn")
        CASE(For, "for")
        CASE(Id, "<identifier>")
        CASE(If, "if")
        CASE(In, "in")
        CASE(Inline, "inline")
        CASE(Int, "<integer_literal>")
        CASE(IntBin, "<binary_integer_literal>")
        CASE(IntHex, "<hexadecimal_integer_literal>")
        CASE(IntOct, "<octal_integer_literal>")
        CASE(Invalid, "<invalid_token>")
        CASE(Is, "is")
        CASE(Let, "let")
        CASE(Match, "match")
        CASE(Mod, "mod")
        CASE(Newtype, "newtype")
        CASE(Not, "not")
        CASE(Null, "null")
        CASE(Or, "or")
        CASE(Private, "private")
        CASE(Real, "<real_literal>")
        CASE(RealSci, "<scientific_real_literal>")
        CASE(Ref, "ref")
        CASE(Return, "return")
        CASE(SelfCap, "Self")
        CASE(SelfLower, "self")
        CASE(Sof, "<start_of_file>")
        CASE(Static, "static")
        CASE(Str, "<string_literal>")
        CASE(Struct, "struct")
        CASE(Test, "test")
        CASE(True, "true")
        CASE(Typealias, "typealias")
        CASE(Union, "union")
        CASE(Use, "use")
        CASE(When, "when")
        CASE(While, "while")
#undef CASE
    }

    // Should never reach here for valid TokenKind values
    return "<unknown>";
}

auto Lexer::next() -> Token {
    // Skip whitespace
    while (cursor < src.size() && std::isspace(src[cursor])) {
        cursor++;
    }

    std::optional<char> ch = current_char();

    if (!ch) {
        return Token(TokenKind::Eof, cursor, cursor);
    }

    Token token = Token(TokenKind::Invalid, cursor, cursor);
    switch (*ch) {
#define SINGLE_TOKEN(kind, char)                                               \
    case char:                                                                 \
        token = Token(TokenKind::kind, cursor, cursor + 1);                    \
        break;

        SINGLE_TOKEN(Dot, '.')
        SINGLE_TOKEN(Colon, ':')
        SINGLE_TOKEN(Semi, ';')
        SINGLE_TOKEN(Comma, ',')
        SINGLE_TOKEN(LParen, '(')
        SINGLE_TOKEN(RParen, ')')
        SINGLE_TOKEN(LBrace, '{')
        SINGLE_TOKEN(RBrace, '}')
        SINGLE_TOKEN(LBracket, '[')
        SINGLE_TOKEN(RBracket, ']')
        SINGLE_TOKEN(Tilde, '~')
        SINGLE_TOKEN(Pipe, '|')
        SINGLE_TOKEN(Hash, '#')
        SINGLE_TOKEN(Question, '?')
        SINGLE_TOKEN(Backslash, '\\')
        SINGLE_TOKEN(Ampersand, '&')
        SINGLE_TOKEN(Caret, '^')
        SINGLE_TOKEN(Dollar, '$')
        SINGLE_TOKEN(At, '@')
#undef SINGLE_TOKEN

    case '+':
        if (peek("+=")) {
            token = Token(TokenKind::PlusEq, cursor, cursor + 2);
        } else if (peek("++")) {
            token = Token(TokenKind::PlusPlus, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Plus, cursor, cursor + 1);
        }
        break;

    case '-':
        if (peek("->")) {
            token = Token(TokenKind::Arrow, cursor, cursor + 2);
        } else if (peek("-=")) {
            token = Token(TokenKind::MinusEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Minus, cursor, cursor + 1);
        }
        break;

    case '*':
        if (peek("*=")) {
            token = Token(TokenKind::StarEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Star, cursor, cursor + 1);
        }
        break;

    case '/':
        if (peek("/=")) {
            token = Token(TokenKind::SlashEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Slash, cursor, cursor + 1);
        }
        break;

    case '%':
        if (peek("%=")) {
            token = Token(TokenKind::PercentEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Percent, cursor, cursor + 1);
        }
        break;

    case '<':
        if (peek("<=")) {
            token = Token(TokenKind::LtEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Lt, cursor, cursor + 1);
        }
        break;

    case '>':
        if (peek(">=")) {
            token = Token(TokenKind::GtEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Gt, cursor, cursor + 1);
        }
        break;

    case '=':
        if (peek("=>")) {
            token = Token(TokenKind::FatArrow, cursor, cursor + 2);
        } else if (peek("==")) {
            token = Token(TokenKind::EqEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Eq, cursor, cursor + 1);
        }
        break;

    case '!':
        if (peek("!=")) {
            token = Token(TokenKind::BangEq, cursor, cursor + 2);
        } else {
            token = Token(TokenKind::Bang, cursor, cursor + 1);
        }
        break;

    case '"':
        return recognize_string_literal();
    case '\'':
        return recognize_char();

    default:
        if (std::isalpha(*ch) || *ch == '_') {
            return recognize_identifier();
        } else if (std::isdigit(*ch)) {
            return recognize_number();
        }

        token = Token(TokenKind::Invalid, cursor, cursor);
        break;
    }

    // Advance cursor by the token length
    cursor = token.end;
    return token;
}

auto Lexer::peek(std::string_view str) -> bool {
    if (cursor + str.size() > src.size()) {
        return false;
    }
    return src.substr(cursor, str.size()) == str;
}

auto Lexer::recognize_identifier() -> Token {
    u32 start = cursor;
    while (cursor < src.size()
           && (std::isalnum(src[cursor]) || src[cursor] == '_')) {
        cursor++;
    }
    u32 end                = cursor;

    std::string_view ident = src.substr(start, end - start);
    auto keyword           = is_keyword(ident);
    if (keyword.has_value()) {
        return Token(keyword.value(), start, end);
    }
    return Token(TokenKind::Id, start, end);
}

auto Lexer::recognize_string_literal() -> Token {
    u32 start = cursor;
    cursor++; // Skip opening quote

    while (cursor < src.size() && src[cursor] != '"') {
        if (src[cursor] == '\\' && cursor + 1 < src.size()) {
            cursor += 2; // Skip escape sequence
        } else {
            cursor++;
        }
    }

    if (cursor < src.size()) {
        cursor++; // Skip closing quote
    }

    return Token(TokenKind::Str, start, cursor);
}

auto Lexer::recognize_number() -> Token {
    u32 start = cursor;

    // Handle binary numbers (0b...)
    if (src[cursor] == '0' && cursor + 1 < src.size()
        && (src[cursor + 1] == 'b' || src[cursor + 1] == 'B')) {
        cursor += 2; // Skip "0b"
        while (cursor < src.size()
               && (src[cursor] == '0' || src[cursor] == '1')) {
            cursor++;
        }
        return Token(TokenKind::IntBin, start, cursor);
    }

    // Handle octal numbers (0o...)
    if (src[cursor] == '0' && cursor + 1 < src.size()
        && (src[cursor + 1] == 'o' || src[cursor + 1] == 'O')) {
        cursor += 2; // Skip "0o"
        while (cursor < src.size() && src[cursor] >= '0'
               && src[cursor] <= '7') {
            cursor++;
        }
        return Token(TokenKind::IntOct, start, cursor);
    }

    // Handle hexadecimal numbers (0x...)
    if (src[cursor] == '0' && cursor + 1 < src.size()
        && (src[cursor + 1] == 'x' || src[cursor + 1] == 'X')) {
        cursor += 2; // Skip "0x"
        while (cursor < src.size() && std::isxdigit(src[cursor])) {
            cursor++;
        }
        return Token(TokenKind::IntHex, start, cursor);
    }

    // Handle decimal numbers
    while (cursor < src.size() && std::isdigit(src[cursor])) {
        cursor++;
    }

    // Check for decimal point
    if (cursor < src.size() && src[cursor] == '.') {
        cursor++; // Skip decimal point
        while (cursor < src.size() && std::isdigit(src[cursor])) {
            cursor++;
        }

        // Check for scientific notation
        if (cursor < src.size() && (src[cursor] == 'e' || src[cursor] == 'E')) {
            cursor++; // Skip 'e' or 'E'
            if (cursor < src.size()
                && (src[cursor] == '+' || src[cursor] == '-')) {
                cursor++; // Skip sign
            }
            while (cursor < src.size() && std::isdigit(src[cursor])) {
                cursor++;
            }
            return Token(TokenKind::RealSci, start, cursor);
        }
        return Token(TokenKind::Real, start, cursor);
    }

    return Token(TokenKind::Int, start, cursor);
}

auto Lexer::recognize_char() -> Token {
    u32 start = cursor;
    cursor++; // Skip opening quote

    if (cursor < src.size() && src[cursor] == '\\') {
        cursor += 2; // Skip escape sequence
    } else {
        cursor++; // Skip character
    }

    if (cursor < src.size() && src[cursor] == '\'') {
        cursor++; // Skip closing quote
    }

    return Token(TokenKind::Char, start, cursor);
}

/**
 * @brief Checks if a given identifier is a language
 * keyword.
 *
 * This function takes a string view and determines if it
 * matches any of the reserved keywords in the language.
 *
 * @note The current implementation uses a linear series of
 * string comparisons, which has a time complexity
 * proportional to the number of keywords. For better
 * performance, this could be optimized by using a more
 * efficient lookup structure, such as a static
 * `std::unordered_map` or a perfect hash function, to
 * achieve constant-time average complexity.
 *
 * @param ident The identifier string to check.
 * @return An optional containing the corresponding
 * TokenKind if the identifier is a keyword, otherwise
 * std::nullopt.
 */
auto Lexer::is_keyword(std::string_view ident) -> std::optional<TokenKind> {
#define KEYWORD(keyword, kind)                                                 \
    if (ident == #keyword)                                                     \
        return TokenKind::kind;

    KEYWORD(and, And)
    KEYWORD(as, As)
    KEYWORD(bool, Bool)
    KEYWORD(break, Break)
    KEYWORD(catch, Catch)
    KEYWORD(const, Const)
    KEYWORD(continue, Continue)
    KEYWORD(else, Else)
    KEYWORD(enum, Enum)
    KEYWORD(error, Error)
    KEYWORD(extern, Extern)
    KEYWORD(false, False)
    KEYWORD(fn, Fn)
    KEYWORD(for, For)
    KEYWORD(if, If)
    KEYWORD(in, In)
    KEYWORD(inline, Inline)
    KEYWORD(is, Is)
    KEYWORD(let, Let)
    KEYWORD(match, Match)
    KEYWORD(mod, Mod)
    KEYWORD(newtype, Newtype)
    KEYWORD(not, Not)
    KEYWORD(null, Null)
    KEYWORD(or, Or)
    KEYWORD(private, Private)
    KEYWORD(ref, Ref)
    KEYWORD(return, Return)
    KEYWORD(self, SelfLower)
    KEYWORD(Self, SelfCap)
    KEYWORD(static, Static)
    KEYWORD(struct, Struct)
    KEYWORD(test, Test)
    KEYWORD(true, True)
    KEYWORD(typealias, Typealias)
    KEYWORD(union, Union)
    KEYWORD(use, Use)
    KEYWORD(when, When)
    KEYWORD(while, While)

#undef KEYWORD
    return std::nullopt;
}
