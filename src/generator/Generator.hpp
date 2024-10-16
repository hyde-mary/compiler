#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "../parser/Node.hpp"
#include "../parser/Parser.hpp"

#include <optional>
#include <sstream>
#include <unordered_map>

namespace generator {
class Generator {
public:
  Generator(std::vector<std::shared_ptr<node::Node>> nodes);

  std::string generate();

private:
  std::vector<std::shared_ptr<node::Node>> NODES;
  size_t generator_count = 0; // for incrementing the nodes / parse tree

  std::unordered_map<std::string, bool> initialized_variables;
  std::unordered_map<std::string, bool> uninitialized_variables;

  std::ostringstream bss_segment;
  std::ostringstream data_segment;
  std::ostringstream text_segment;

  std::optional<std::shared_ptr<node::Node>> peek();

  std::optional<std::shared_ptr<node::Node>> consume();

  std::ostringstream
  processExpression(std::shared_ptr<ExpressionNode> expressionNode,
                    bool isIn = false);

  std::ostringstream
  processAssignment(std::shared_ptr<AssignmentNode> assignmentNode);

  bool isInitialized(std::string key);

  bool isUninitialized(std::string key);

  void processDeclaration(std::shared_ptr<DeclarationNode> declarationNode);

  std::ostringstream processCout(std::shared_ptr<CoutNode> coutNode);

  std::ostringstream processCin(std::shared_ptr<CinNode> cinNode);

  void nodeGenerator(std::shared_ptr<node::Node> node);
};
} // namespace generator

#endif // !GENERATOR_HPP
