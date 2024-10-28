#include "SyntaxAnalyzer.hpp"

SyntaxAnalyzer::SyntaxAnalyzer(
    const std::vector<std::shared_ptr<node::Node>> &Nodes)
    : Nodes(std::move(Nodes)) {}

void SyntaxAnalyzer::analyzeSemantics() {
  for (auto &node : Nodes) {
    std::cout << "Analyzing: ";
    node->toString();
    analyzeNode(node);
    // symbolTable.printInitialized();
  }
  std::cout << "Semantics Analyzed: No errors." << std::endl;
}

void SyntaxAnalyzer::analyzeDeclaration(std::shared_ptr<DeclarationNode> node) {
  // we add all declared identifiers in the symbol table
  if (std::holds_alternative<std::vector<std::shared_ptr<IdentifierNode>>>(
          node->product)) {
    auto identifiers =
        std::get<std::vector<std::shared_ptr<IdentifierNode>>>(node->product);
    for (auto &id : identifiers) {
      // process identifier
      symbolTable.declareVariable(id->identifier);
    }
  } else if (std::holds_alternative<std::shared_ptr<AssignmentNode>>(
                 node->product)) {
    auto assignment = std::get<std::shared_ptr<AssignmentNode>>(node->product);
    // process assignment
    // LMAO AHSJDAKJDKAJSDLAWKDJLAKWDJLAKWD
    symbolTable.declareVariable(assignment->identifier->identifier);
    analyzeAssignment(assignment);
  }
}

void SyntaxAnalyzer::analyzeExpression(std::shared_ptr<ExpressionNode> node) {
  std::shared_ptr<node::Node> left = node->left;
  std::shared_ptr<node::Node> right = node->right;

  // process left
  if (auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(left)) {
    symbolTable.isInitialized(identifierNode->identifier);
    symbolTable.lookupVariable(identifierNode->identifier);
  }

  // process right
  if (auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(right)) {
    symbolTable.isInitialized(identifierNode->identifier);
    symbolTable.lookupVariable(identifierNode->identifier);
  }
}

void SyntaxAnalyzer::analyzeIdentifier(std::shared_ptr<IdentifierNode> node) {
  symbolTable.lookupVariable(node->identifier);
}

void SyntaxAnalyzer::analyzeAssignment(std::shared_ptr<AssignmentNode> node) {
  /*
    An assingment in the Assignment node can be a
    Constant Node, Identifier Node or Expression Node.
    We nee to analyze each of these nodes individually.
  */
  symbolTable.setInitialized(
      node->identifier->identifier); // we set the initialized flag as true

  if (auto constantNode =
          std::dynamic_pointer_cast<ConstantNode>(node->assignment)) {
    // maybe a bit redundant lmao
    // since we already know that it's a constant
    // Just incase
    if (constantNode->constant.type != TokenType::CONSTANT) {
      throw std::runtime_error(
          "Semantic Error: Type mismatch in assignment at line " +
          std::to_string(constantNode->constant.line));
    }
  } else if (auto identifierNode =
                 std::dynamic_pointer_cast<IdentifierNode>(node->assignment)) {
    TokenType type = symbolTable.lookupVariable(identifierNode->identifier);
    if (type != node->identifier->identifier.type) {
      throw std::runtime_error(
          "Semantic Error: Type mismatch in assignment at line " +
          std::to_string(identifierNode->identifier.line));
    }
  } else if (auto expressionNode =
                 std::dynamic_pointer_cast<ExpressionNode>(node->assignment)) {
    analyzeExpression(expressionNode);
  } else {
    throw std::runtime_error(
        "Semantic Error: Unknown node type in assignment.");
  }
}

void SyntaxAnalyzer::analyzeNode(const std::shared_ptr<node::Node> &node,
                                 bool isInCin) {
  if (auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(node)) {
    analyzeIdentifier(identifierNode);
    // this may be cheating lol
    // basically since we are using cin, we do not need the varibales to be
    // initialized
    if (isInCin) {
      symbolTable.setInitialized(
          identifierNode->identifier); // we cheat by making it seem that the
                                       // variable is initialized already
    }
    symbolTable.isInitialized(identifierNode->identifier);
  } else if (auto stringLiteralNode =
                 std::dynamic_pointer_cast<StringLiteralNode>(node)) {
    // hatdog
  } else if (auto constantNode =
                 std::dynamic_pointer_cast<ConstantNode>(node)) {
    // hatdog
  } else if (auto declNode = std::dynamic_pointer_cast<DeclarationNode>(node)) {
    analyzeDeclaration(declNode);
  } else if (auto assignNode =
                 std::dynamic_pointer_cast<AssignmentNode>(node)) {
    analyzeAssignment(assignNode);
  } else if (auto exprNode = std::dynamic_pointer_cast<ExpressionNode>(node)) {
    analyzeExpression(exprNode);
  } else if (auto sequenceNode =
                 std::dynamic_pointer_cast<SequenceNode>(node)) {
    for (auto statement : sequenceNode->statements) {
      analyzeNode(statement);
    }
  } else if (auto cinNode = std::dynamic_pointer_cast<CinNode>(node)) {
    for (auto operand : cinNode->operands) {
      analyzeNode(operand, true);
    }
  } else if (auto coutNode = std::dynamic_pointer_cast<CoutNode>(node)) {
    for (auto operand : coutNode->operands) {
      analyzeNode(operand);
    }
  } else {
    throw std::runtime_error("Semantic Error: Unknown node type.");
  }
}
