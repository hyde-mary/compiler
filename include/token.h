#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
  ADDITION_OPERATOR,
  SUBTRACTION_OPERATOR,
  ASSIGNMENT_OPERATOR,
  COUT_KEYWORD,
  DELIMITER,
  PUNCTUATOR,
  IDENTIFIER,
  INT_KEYWORD,
  CONSTANT,
  LITERAL,
  COUT_OPERATOR,
  CIN_OPERATOR,
  CIN_KEYWORD,
  UNKNOWN
};

struct TOKEN {
  TokenType type;
  std::string lexeme;
  int line;
};

#endif
