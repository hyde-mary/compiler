#include "DeclarationNode.hpp"

// Constructor that accepts the type and identifier as parameters
DeclarationNode::DeclarationNode(
    TokenType type, std::variant<std::vector<std::shared_ptr<IdentifierNode>>,
                                 std::shared_ptr<AssignmentNode>>
                        product)
    : type(type), product(std::move(product)) {}

void DeclarationNode::print() const {
  std::cout << "DeclarationNode" << std::endl;
  if (std::holds_alternative<std::shared_ptr<AssignmentNode>>(product)) {
    auto assignment = std::get<std::shared_ptr<AssignmentNode>>(product);
    assignment->print();
  } else {
    auto identifiers =
        std::get<std::vector<std::shared_ptr<IdentifierNode>>>(product);
    for (const auto &identifier : identifiers) {
      identifier->print();
    }
  }
}

void DeclarationNode::toString() const {
  std::cout << "DeclarationNode" << std::endl;
}
