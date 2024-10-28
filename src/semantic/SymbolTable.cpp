#include "SymbolTable.hpp"

void SymbolTable::declareVariable(TOKEN &token) {
  if (declared_variables.find(token.lexeme) != declared_variables.end()) {
    throw std::runtime_error("Semantic Error: Variable '" + token.lexeme +
                             "' is already declared.");
  }
  declared_variables[token.lexeme] = token.type;
  initialized_variables[token.lexeme] = false;
}

TokenType SymbolTable::lookupVariable(TOKEN &token) {
  if (declared_variables.find(token.lexeme) == declared_variables.end()) {
    throw std::runtime_error("Semantic Error: Variable '" + token.lexeme +
                             "' is not declared.");
  }
  return token.type;
}

void SymbolTable::setInitialized(TOKEN &token) {
  initialized_variables[token.lexeme] = true;
}

void SymbolTable::isInitialized(TOKEN &token) {
  auto it = initialized_variables.find(token.lexeme);
  if (it == initialized_variables.end() || it->second != true) {
    throw std::runtime_error("Semantic Error: Variable '" + token.lexeme +
                             "' is not initialized.");
  }
}
// for debugging
void SymbolTable::printInitialized() {
  for (auto var : initialized_variables) {
    std::cout << var.first << " :: " << var.second << std::endl;
  }
}
