#ifndef IDENTIFIER_NODE_HPP
#define IDENTIFIER_NODE_HPP

#include "../Node.hpp"

class IdentifierNode : public node::Node {
public:
  TOKEN identifier;
  int line; /* Line number of the identifier */
  IdentifierNode(TOKEN identifier, int line);

  void print() const override;

  void toString() const override;
};

#endif // !IDENTIFIER_NODE_HPP
