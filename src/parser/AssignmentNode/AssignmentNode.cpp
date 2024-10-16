#include "AssignmentNode.hpp"

// Updated constructor to remove the line parameter
AssignmentNode::AssignmentNode(std::shared_ptr<IdentifierNode> identifier,
                               std::shared_ptr<Node> assignment)
    : identifier(std::move(identifier)), assignment(std::move(assignment)) {}

void AssignmentNode::print() const {
  std::cout << "- AssignmentNode" << " " << identifier->identifier.lexeme << " "
            << std::endl;
  assignment->print();
}
void AssignmentNode::toString() const {
  std::cout << "AssignmentNode" << std::endl;
}
