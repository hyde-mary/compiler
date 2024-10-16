#ifndef ASSIGNMENT_NODE_HPP
#define ASSIGNMENT_NODE_HPP

#include "../IdentifierNode/IdentifierNode.hpp"
#include "../Node.hpp"

class AssignmentNode : public node::Node {
public:
  std::shared_ptr<IdentifierNode> identifier;
  std::shared_ptr<Node> assignment;

  // Updated constructor to remove the line parameter
  AssignmentNode(std::shared_ptr<IdentifierNode> identifier,
                 std::shared_ptr<Node> assignment);

  void print() const override;

  void toString() const override;
};

#endif // !ASSIGNMENT_NODE_HPP
