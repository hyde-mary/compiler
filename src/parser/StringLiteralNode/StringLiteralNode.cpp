#include "StringLiteralNode.hpp"

StringLiteralNode::StringLiteralNode(TOKEN literal)
    : literal(std::move(literal)) {}

void StringLiteralNode::print() const {
  std::cout << "\t - StringLiteralNode " << "'" << literal.lexeme << "'"
            << std::endl;
}

void StringLiteralNode::toString() const {
  std::cout << "StringLiteralNode" << std::endl;
}
