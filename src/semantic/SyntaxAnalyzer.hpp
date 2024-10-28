#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

#include "../parser/Parser.hpp"
#include "SymbolTable.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

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

#endif // !SYNTAX_ANALYZER_HPP
