#include "SequenceNode.hpp"

// Add a statement to the sequence
void SequenceNode::addStatement(std::shared_ptr<Node> statement) {
  statements.push_back(std::move(statement));
}

void SequenceNode::print() const {
  std::cout << "SequenceNode" << std::endl;
  for (const auto &stmt : statements) {
    stmt->print();
  }
}

void SequenceNode::toString() const {
  std::cout << "SequenceNode" << std::endl;
}
