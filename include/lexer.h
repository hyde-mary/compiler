#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <cwctype>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

class Lexer {
public:
  Lexer(const std::string &src_code);
  std::vector<TOKEN> lex();

private:
  std::string SRC_CODE;
  int LEX_COUNTER = 0;
  int PROGRAM_LINE = 1;

  std::optional<char> peek();
  char consume();
  TokenType getOpTokenType(char ch);
  TokenType getKeywordTokenType(std::string keyword);
  bool isOperator(char ch);
};

#endif
