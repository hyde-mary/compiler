#ifndef STRING_LITERAL_NODE_HPP
#define STRING_LITERAL_NODE_HPP

#include "../Node.hpp"

class StringLiteralNode : public node::Node {
public:
  TOKEN literal;
  StringLiteralNode(TOKEN literal);

  void print() const override;

  void toString() const override;
};

#endif // !STRING_LITERAL_NODE_HPP
