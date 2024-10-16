#include "Lexer.hpp"
#include <cwctype>
#include <stdexcept>

Lexer::Lexer(const std::string SRC_CODE) : SRC_CODE(std::move(SRC_CODE)) {}

std::vector<TOKEN> Lexer::lex() {
  std::vector<TOKEN> tokens;
  std::string buffer;

  while (peek().has_value()) {

    if (peek().value() == '\n' || peek().value() == '\r') {
      PROGRAM_LINE++;
      consume();
      continue;
    }

    if (std::iswspace(peek().value())) {
      consume();
      continue;
    }

    if (peek().value() == ';') {
      buffer.push_back(consume());
      tokens.push_back({TokenType::DELIMITER, buffer, PROGRAM_LINE});
      buffer.clear();
      continue;
    }

    if (peek().value() == ',') {
      buffer.push_back(consume());
      tokens.push_back({TokenType::PUNCTUATOR, buffer, PROGRAM_LINE});
      buffer.clear();
      continue;
    }

    /* check for literal */
    if (peek().value() == '"') {
      consume();
      while (peek().has_value() && peek().value() != '"') {
        buffer.push_back(consume());
      }
      if (peek().has_value() && peek().value() == '"') {
        consume(); // Consume closing quote
        tokens.push_back({TokenType::LITERAL, buffer, PROGRAM_LINE});
      } else {
        // Handle error: unmatched opening quote
      }
      buffer.clear();
      continue;
    }

    if (isOperator(peek().value())) {
      buffer.push_back(consume());
      tokens.push_back({getOpTokenType(buffer[0]), buffer, PROGRAM_LINE});
      buffer.clear();
      continue;
    }

    /* for indentifiers */
    if (std::isalpha(peek().value())) {
      buffer.push_back(consume());
      while (std::isalnum(peek().value()) || peek().value() == '_') {
        buffer.push_back(consume());
      }
      tokens.push_back({getKeywordTokenType(buffer), buffer, PROGRAM_LINE});
      buffer.clear();
      continue;
    }

    if (std::isdigit(peek().value())) {
      buffer.push_back(consume());
      while (std::isdigit(peek().value())) {
        buffer.push_back(consume());
      }
      tokens.push_back({TokenType::CONSTANT, buffer, PROGRAM_LINE});
      buffer.clear();
      continue;
    }

    /* Check for CIN and COUT operators */
    if (peek().value() == '<') {
      buffer.push_back(consume());
      if (peek().value() == '<') {
        buffer.push_back(consume());
        tokens.push_back({TokenType::COUT_OPERATOR, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }
      tokens.push_back({TokenType::UNKNOWN, buffer, PROGRAM_LINE});
      buffer.clear();
      continue;
    }

    if (peek().value() == '>') {

      buffer.push_back(consume());
      if (peek().value() == '>') {
        buffer.push_back(consume());
        tokens.push_back({TokenType::CIN_OPERATOR, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }
      tokens.push_back({TokenType::UNKNOWN, buffer, PROGRAM_LINE});
      buffer.clear();
      continue;
    }

    // we add a token with unknown type
    tokens.push_back(
        {TokenType::UNKNOWN, std::string(1, consume()), PROGRAM_LINE});
  }

  return tokens;
}

std::optional<char> Lexer::peek() {
  /*
  It checks if we have reached the end of the source code.
  If we have reached the end, it returns std::nullopt
  If we have not reached the end, it returns character
  */
  if (LEX_COUNTER >= SRC_CODE.size()) {
    return std::nullopt;
  }
  return SRC_CODE.at(LEX_COUNTER);
}

char Lexer::consume() {
  /*
  Function to consume the current character in the source code.
  It checks if we have reached the end of the source code.
  If we have reached the end, it throws an exception
  If we have not reached the end, it returns the character an increments the
  LEX_COUNTER
  */
  if (LEX_COUNTER >= SRC_CODE.size()) {
    throw std::out_of_range(
        "Attempted to consume past the end of the source code.");
  }
  return SRC_CODE.at(LEX_COUNTER++);
}

TokenType Lexer::getOpTokenType(char ch) {
  switch (ch) {
  case '+':
    return TokenType::ADDITION_OPERATOR;
  case '-':
    return TokenType::SUBTRACTION_OPERATOR;
  case '=':
    return TokenType::ASSIGNMENT_OPERATOR;
  }
  return TokenType::IDENTIFIER;
}

TokenType Lexer::getKeywordTokenType(std::string keyword) {
  if (keyword == "cin") {
    return TokenType::CIN_KEYWORD;
  } else if (keyword == "cout") {
    return TokenType::COUT_KEYWORD;
  } else if (keyword == "int") {
    return TokenType::INT_KEYWORD;
  }
  return TokenType::IDENTIFIER;
}

bool Lexer::isOperator(char ch) {
  return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
          ch == '^' || ch == '=' || ch == '!');
}
