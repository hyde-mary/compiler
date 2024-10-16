#include "ExpressionNode.hpp"

ExpressionNode::ExpressionNode(char OP, std::shared_ptr<Node> left,
                               std::shared_ptr<Node> right)
    : OP(OP), left(std::move(left)), right(std::move(right)) {}

void ExpressionNode::print() const {
  std::cout << "\t - ExpressionNode " << OP << std::endl;
  std::cout << "\t\t - left: ";
  left->print();
  std::cout << "\t\t - right: ";
  right->print();
}

void ExpressionNode::toString() const {
  std::cout << "ExpressionNode" << std::endl;
}
