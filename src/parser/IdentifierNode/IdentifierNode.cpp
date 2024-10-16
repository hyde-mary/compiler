#include "IdentifierNode.hpp"

IdentifierNode::IdentifierNode(TOKEN identifier, int line)
    : identifier(std::move(identifier)), line(line) {}

void IdentifierNode::print() const {
  std::cout << "\t - IdentifierNode :: IDENTIFIER - " << identifier.lexeme
            << std::endl;
}

void IdentifierNode::toString() const {
  std::cout << "IdentifierNode" << std::endl;
}
