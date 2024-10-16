#include "CoutNode.hpp"

CoutNode::CoutNode(std::vector<std::shared_ptr<Node>> operands)
    : operands(std::move(operands)) {}

void CoutNode::print() const {
  std::cout << "CoutNode " << std::endl;
  for (const auto &operand : operands) {
    operand->print();
  }
}

void CoutNode::toString() const { std::cout << "CoutNode" << std::endl; }
