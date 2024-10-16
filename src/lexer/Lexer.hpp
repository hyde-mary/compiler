#ifndef LEXER_HPP
#define LEXER_HPP

#include "../common/Token.hpp"
#include <optional>
#include <string>
#include <vector>

class Lexer {
public:
  // constructor
  Lexer(const std::string SRC_CODE);
  // method
  std::vector<TOKEN> lex();

private:
  // class variables
  size_t LEX_COUNTER = 0;
  int PROGRAM_LINE = 1;
  std::string SRC_CODE;

  // methods
  std::optional<char> peek();

  char consume();

  TokenType getOpTokenType(char ch);

  TokenType getKeywordTokenType(std::string keyword);

  bool isOperator(char ch);
};

#endif // !LEXER_HPP
