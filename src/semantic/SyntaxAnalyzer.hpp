#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

#include "../parser/Parser.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace semantic {
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

class SyntaxAnalyzer {
public:
  SyntaxAnalyzer(const std::vector<std::shared_ptr<node::Node>> &Nodes);

  void analyzeSemantics();

private:
  SymbolTable symbolTable;
  std::vector<std::shared_ptr<node::Node>> Nodes;

  void analyzeDeclaration(std::shared_ptr<DeclarationNode> node);

  void analyzeExpression(std::shared_ptr<ExpressionNode> node);

  void analyzeIdentifier(std::shared_ptr<IdentifierNode> node);

  void analyzeAssignment(std::shared_ptr<AssignmentNode> node);

  void analyzeNode(const std::shared_ptr<node::Node> &node,
                   bool isInCin = false);
};

} // namespace semantic

#endif // !SYNTAX_ANALYZER_HPP
