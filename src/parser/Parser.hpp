#ifndef PARSER_HPP
#define PARSER_HPP

#include "../common/Token.hpp"
#include "AssignmentNode/AssignmentNode.hpp"
#include "CinNode/CinNode.hpp"
#include "ConstantNode/ConstantNode.hpp"
#include "CoutNode/CoutNode.hpp"
#include "DeclarationNode/DeclarationNode.hpp"
#include "ExpressionNode/ExpressionNode.hpp"
#include "Node.hpp"
#include "SequenceNode/SequenceNode.hpp"
#include "StringLiteralNode/StringLiteralNode.hpp"
#include <optional>
#include <vector>

namespace parser {
class Parser {
public:
  Parser(std::vector<TOKEN> tokens);
  std::vector<std::shared_ptr<node::Node>> parse();

private:
  std::vector<TOKEN> TOKENS;
  std::vector<std::shared_ptr<node::Node>> NODES;

  size_t P_COUNTER = 0;

  std::optional<TOKEN> peek();

  std::optional<TOKEN> consume();

  std::shared_ptr<ConstantNode> parseConstant();

  std::shared_ptr<IdentifierNode> parseIdentifier();

  std::shared_ptr<node::Node> parseConstantOrIdentifier();

  std::shared_ptr<node::Node> parseExpression();

  std::shared_ptr<AssignmentNode> parseAssignment();

  std::shared_ptr<node::Node> parseDeclaration();

  std::shared_ptr<node::Node> parseLiteral();

  std::shared_ptr<node::Node> parseCout();

  std::shared_ptr<node::Node> parseCin();

  std::optional<std::shared_ptr<node::Node>> parseStatement();

  std::string printTokenType(TokenType type);

  void expect(TokenType type, TOKEN &token);
};
} // namespace parser
#endif //! PARSER_HPP
