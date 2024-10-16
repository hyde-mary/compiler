#ifndef SEQUENCE_NODE_HPP
#define SEQUENCE_NODE_HPP

#include "../Node.hpp"

class SequenceNode : public node::Node {
  /* This node represents a sequence of nodes. Can be sequence for assignment
   * declaration, etc.*/
public:
  std::vector<std::shared_ptr<Node>> statements;

  SequenceNode() = default;

  // Add a statement to the sequence
  void addStatement(std::shared_ptr<Node> statement);

  void print() const override;

  void toString() const override;
};

#endif // !SEQUENCE_NODE_HPP
