#include "CinNode.hpp"

CinNode::CinNode(std::vector<std::shared_ptr<Node>> operands)
    : operands(std::move(operands)) {}

void CinNode::print() const {
  std::cout << "CinNode " << std::endl;
  for (const auto &operand : operands) {
    operand->print();
  }
}

void CinNode::toString() const { std::cout << "CinNode" << std::endl; }
