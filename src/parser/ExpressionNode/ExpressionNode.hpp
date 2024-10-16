#ifndef EXPRESSION_NODE_HPP
#define EXPRESSION_NODE_HPP

#include "../Node.hpp"

class ExpressionNode : public node::Node {
  /*
      An expression node is a node that represents an expression.
      In the context of the grammar, this is a binary expression.
  */
public:
  char OP;
  std::shared_ptr<Node> left;
  std::shared_ptr<Node> right;
  int line; /* Line number of the operator */

  ExpressionNode(char OP, std::shared_ptr<Node> left,
                 std::shared_ptr<Node> right);

  void print() const override;

  void toString() const override;
};

#endif // !EXPRESSION_NODE_HPP
