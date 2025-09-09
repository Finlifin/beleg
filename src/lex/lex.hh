#ifndef LEX_HH
#define LEX_HH

enum class TokenKind {
  Invalid = 0,
  Eof,

  Integer,
  Float,
  Identifier,
  String,
  Char,

};

#endif