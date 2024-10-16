#ifndef COUT_NODE_HPP
#define COUT_NODE_HPP

#include "../Node.hpp"

class CoutNode : public node::Node {
public:
  std::vector<std::shared_ptr<Node>> operands;
  CoutNode(std::vector<std::shared_ptr<Node>> operands);

  void print() const override;

  void toString() const override;
};

#endif // !COUT_NODE_HPP
