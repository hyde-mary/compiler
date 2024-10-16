#include "ConstantNode.hpp"

ConstantNode::ConstantNode(TOKEN constant, int line)
    : constant(std::move(constant)), line(line) {}

void ConstantNode::print() const {
  std::cout << "\t - ConstantNode " << constant.lexeme << std::endl;
}
void ConstantNode::toString() const {
  std::cout << "ConstantNode" << std::endl;
}
