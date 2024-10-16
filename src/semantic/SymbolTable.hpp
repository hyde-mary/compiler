#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include "../parser/Parser.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class SymbolTable {
public:
  void declareVariable(TOKEN &token);

  TokenType lookupVariable(TOKEN &token);

  void setInitialized(TOKEN &token);

  void isInitialized(TOKEN &token);
  // for debugging
  void printInitialized();

private:
  std::unordered_map<std::string, TokenType> declared_variables;
  std::unordered_map<std::string, bool> initialized_variables;
};

#endif //! SYMBOL_TABLE_HPP
