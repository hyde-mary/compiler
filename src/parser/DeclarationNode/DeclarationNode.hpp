#ifndef DECLARATION_NODE_HPP
#define DECLARATION_NODE_HPP

#include "../AssignmentNode/AssignmentNode.hpp"
#include "../IdentifierNode/IdentifierNode.hpp"
#include "../Node.hpp"

class DeclarationNode : public node::Node {
public:
  TokenType type;
  std::variant<std::vector<std::shared_ptr<IdentifierNode>>,
               std::shared_ptr<AssignmentNode>>
      product;

  // Constructor that accepts the type and identifier as parameters
  DeclarationNode(TokenType type,
                  std::variant<std::vector<std::shared_ptr<IdentifierNode>>,
                               std::shared_ptr<AssignmentNode>>
                      product);

  void print() const override;

  void toString() const override;
};

#endif // !DECLARATION_NODE_HPP
