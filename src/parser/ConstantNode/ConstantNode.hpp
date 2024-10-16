#ifndef CONSTANT_NODE_HPP
#define CONSTANT_NODE_HPP

#include "../Node.hpp"

class ConstantNode : public node::Node {
  /*
  A constant node is a node that represents a constant or an Integer Literal.
  */
public:
  TOKEN constant;
  int line; /* Line number of the constant */
  ConstantNode(TOKEN constant, int line);

  void print() const override;

  void toString() const override;
};

#endif // !CONSTANT_NODE_HPP
