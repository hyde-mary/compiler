#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../include/fetch_source_code.h"
#include "../include/lexer.h"

namespace parser {
class Node {
public:
  ~Node() = default; /* Destructor which can be overridden by subclasses */
  virtual void print() const = 0;
};

class IdentifierNode : public Node {
public:
  std::string name;
  int line; /* Line number of the identifier */
  IdentifierNode(std::string name, int line)
      : name(std::move(name)), line(line) {}

  void print() const override {
    std::cout << "\t - IdentifierNode " << name << std::endl;
  }
};

class ConstantNode : public Node {
  /*
  A constant node is a node that represents a constant or an Integer Literal.
  */
public:
  int value;
  int line; /* Line number of the constant */
  ConstantNode(int value, int line) : value(std::move(value)), line(line) {}

  void print() const override {
    std::cout << "\t - ConstantNode " << value << std::endl;
  }
};

class ExpressionNode : public Node {
  /*
      An expression node is a node that represents an expression.
      In the context of the grammar, this is a binary expression.
  */
public:
  char OP;
  std::shared_ptr<Node> left;
  std::shared_ptr<Node> right;
  int line; /* Line number of the operator */

  ExpressionNode(char OP, std::shared_ptr<Node> left,
                 std::shared_ptr<Node> right)
      : OP(OP), left(std::move(left)), right(std::move(right)) {}

  void print() const override {
    std::cout << "\t - ExpressionNode " << OP << std::endl;
    std::cout << "\t\t - left: ";
    left->print();
    std::cout << "\t\t - right: ";
    right->print();
  }
};

class AssignmentNode : public Node {
public:
  std::shared_ptr<IdentifierNode> identifier;
  std::shared_ptr<Node> assignment;

  // Updated constructor to remove the line parameter
  AssignmentNode(std::shared_ptr<IdentifierNode> identifier,
                 std::shared_ptr<Node> assignment)
      : identifier(std::move(identifier)), assignment(std::move(assignment)) {}

  void print() const override {
    std::cout << "- AssignmentNode" << " " << identifier->name << " "
              << std::endl;
    assignment->print();
  }
};

class DeclarationNode : public Node {
public:
  TokenType type;
  std::variant<std::vector<std::shared_ptr<IdentifierNode>>,
               std::shared_ptr<AssignmentNode>>
      product;

  // Constructor that accepts the type and identifier as parameters
  DeclarationNode(TokenType type,
                  std::variant<std::vector<std::shared_ptr<IdentifierNode>>,
                               std::shared_ptr<AssignmentNode>>
                      product)
      : type(type), product(std::move(product)) {}

  void print() const override {
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
};

class SequenceNode : public Node {
  /* This node represents a sequence of nodes. Can be sequence for assignment
   * declaration, etc.*/
public:
  std::vector<std::shared_ptr<Node>> statements;

  SequenceNode() = default;

  // Add a statement to the sequence
  void addStatement(std::shared_ptr<Node> statement) {
    statements.push_back(std::move(statement));
  }

  void print() const override {
    std::cout << "SequenceNode" << std::endl;
    for (const auto &stmt : statements) {
      stmt->print();
    }
  }
};

class StringLiteralNode : public Node {
public:
  std::string literal;
  StringLiteralNode(std::string literal) : literal(std::move(literal)) {}

  void print() const override {
    std::cout << "\t - StringLiteralNode " << "'" << literal << "'"
              << std::endl;
  }
};

class CoutNode : public Node {
public:
  std::vector<std::shared_ptr<Node>> operands;
  CoutNode(std::vector<std::shared_ptr<Node>> operands)
      : operands(std::move(operands)) {}

  void print() const override {
    std::cout << "CoutNode " << std::endl;
    for (const auto &operand : operands) {
      operand->print();
    }
  }
};
class Parser {
public:
  Parser(std::vector<TOKEN> tokens) : TOKENS(std::move(tokens)), NODES{} {}

  std::vector<std::shared_ptr<Node>> parse() {
    // Parse all the statements from the tokens vector
    while (peek().has_value()) {
      if (auto node = parseStatement(); node.has_value()) {
        NODES.push_back(*node);
      }
    }
    return NODES;
  }

private:
  std::vector<TOKEN> TOKENS;
  std::vector<std::shared_ptr<Node>> NODES;

  int P_COUNTER = 0;

  std::optional<TOKEN> peek() {
    if (P_COUNTER >= TOKENS.size()) {
      return std::nullopt;
    }
    return TOKENS.at(P_COUNTER);
  }

  std::optional<TOKEN> consume() {
    if (P_COUNTER >= TOKENS.size()) {
      return std::nullopt;
    }
    return TOKENS.at(P_COUNTER++);
  }

  std::shared_ptr<ConstantNode> parseConstant() {
    auto token = peek().value(); // peak the current token
    expect(TokenType::CONSTANT,
           token); // check if the current token is a constant
    consume();
    return std::make_shared<ConstantNode>(std::stoi(token.lexeme), token.line);
  }

  std::shared_ptr<IdentifierNode> parseIdentifier() {
    auto token = peek().value();
    expect(TokenType::IDENTIFIER, token);
    consume();
    return std::make_shared<IdentifierNode>(token.lexeme, token.line);
  }

  std::shared_ptr<Node> parseConstantOrIdentifier() {
    if (peek().value().type == TokenType::CONSTANT) {
      return parseConstant();
    } else if (peek().value().type == TokenType::IDENTIFIER) {
      return parseIdentifier();
    } else {
      throw std::runtime_error("Invalid token");
    }
  }

  std::shared_ptr<Node> parseExpression() {
    auto left = parseConstantOrIdentifier(); // this will parse a constant and
                                             // consume the token

    // Check if the current token after consumption is an operator
    if (peek().value().type == TokenType::ADDITION_OPERATOR) {
      consume(); // Consume '+'
      auto right = parseConstantOrIdentifier();
      return std::make_shared<ExpressionNode>('+', left, right);
    } else if (peek().value().type == TokenType::SUBTRACTION_OPERATOR) {
      consume(); // Consume '-'
      auto right = parseConstantOrIdentifier();
      return std::make_shared<ExpressionNode>('-', left, right);
    }
    // Check if the next token is a delimiter (e.g., ';')
    auto token = peek().value();
    expect(TokenType::DELIMITER, token);

    // If the token is not an operator, return the constant only
    // this indicates a simple assignment operation
    return left;
  }

  std::shared_ptr<AssignmentNode> parseAssignment() {
    // Parse the identifier
    auto identifier =
        parseIdentifier(); // This should return a shared_ptr<IdentifierNode>
    // Check if the next token is the assignment operator
    TOKEN token = peek().value();

    expect(TokenType::ASSIGNMENT_OPERATOR,
           token); // Throws error if not an assignment operator
    consume();     // Consume the assignment operator

    // Parse the expression or constant
    auto expression =
        parseExpression(); // Ensure this returns the correct variant type

    // Check if the next token is a delimiter (e.g., ';')
    token = peek().value();

    expect(TokenType::DELIMITER, token); // Throws error if not a delimiter
    consume();

    // Create and return the AssignmentNode
    return std::make_shared<AssignmentNode>(identifier, expression);
  }

  std::shared_ptr<Node> parseDeclaration() {
    std::vector<std::shared_ptr<IdentifierNode>> identifiers;

    const TokenType declarationType = peek().value().type;
    consume();

    // Parse identifiers until the next delimiter (e.g., ';')
    // Possible side effect here is when there is no delimiter
    // in the source code.
    while (peek().value().type == TokenType::IDENTIFIER ||
           peek().value().type == TokenType::PUNCTUATOR) {
      if (peek().value().type == TokenType::IDENTIFIER) {
        auto identifier = parseIdentifier(); // simply parse the identifier

        // after consumption of the parseIdentifier, we check if the next token
        // is an assignment operator
        if (peek().value().type == TokenType::ASSIGNMENT_OPERATOR) {
          consume(); // we consume the token
          // we parse the expression and create an assignment node
          auto expr = parseExpression();
          auto assignment = std::make_shared<AssignmentNode>(identifier, expr);
          consume();
          // We encapsulate the assignment node in a declaration node
          // and return it
          return std::make_shared<DeclarationNode>(declarationType, assignment);
        } else {
          // Otherwise, it's just an identifier in a list of declarations
          identifiers.push_back(identifier);

          // checks if there is a missing punctuator
          if (peek().has_value() &&
              peek().value().type == TokenType::IDENTIFIER) {
            throw std::runtime_error(
                "Syntax Error: Missing punctuator between identifiers.");
          }
        }
      } else if (peek().value().type == TokenType::PUNCTUATOR) {
        consume();
        continue;
      } else {
        throw std::runtime_error(
            "Syntax Error: Expected delimiter at the end of declaration.");
      }
    }
    auto token = peek().value();
    expect(TokenType::DELIMITER, token);
    consume();
    return std::make_shared<DeclarationNode>(declarationType, identifiers);
  }

  std::shared_ptr<Node> parseLiteral() {
    auto token = peek().value();
    expect(TokenType::LITERAL, token);
    consume();
    return std::make_shared<StringLiteralNode>(token.lexeme);
  }

  std::shared_ptr<Node> parseCout() {

    std::vector<std::shared_ptr<Node>> outputOperands;

    auto token = peek().value();
    expect(TokenType::COUT_KEYWORD, token);
    consume();

    while (peek().value().type == TokenType::IDENTIFIER ||
           peek().value().type == TokenType::CONSTANT ||
           peek().value().type == TokenType::LITERAL ||
           peek().value().type == TokenType::COUT_OPERATOR) {

      if (peek().value().type == TokenType::IDENTIFIER ||
          peek().value().type == TokenType::CONSTANT) {
        auto expr = parseExpression();
        outputOperands.push_back(expr);
      } else if (peek().value().type == TokenType::LITERAL) {
        auto literal = parseLiteral();
        outputOperands.push_back(literal);
      } else if (peek().value().type == TokenType::COUT_OPERATOR) {
        consume();
        continue;
      }
    }

    // expect token at the end
    token = peek().value();
    expect(TokenType::DELIMITER, token);
    consume();

    return std::make_shared<CoutNode>(outputOperands);
  }

  std::optional<std::shared_ptr<Node>> parseStatement() {
    TOKEN token = peek().value();

    switch (token.type) {
    case TokenType::IDENTIFIER: {
      auto assignmentNode = parseAssignment();

      if (assignmentNode) {
        // Create a new SequenceNode and add the parsed statement
        auto sequenceNode = std::make_shared<SequenceNode>();
        sequenceNode->addStatement(assignmentNode);
        return sequenceNode;
      } else {
        throw std::runtime_error("Failed to parse statement at line " +
                                 std::to_string(token.line));
      }
    } break;
    case TokenType::INT_KEYWORD: {

      auto declarationNode = parseDeclaration();

      if (declarationNode) {
        // auto sequenceNode = std::make_shared<SequenceNode>();
        // sequenceNode->addStatement(declarationNode);
        return declarationNode;
      } else {
        throw std::runtime_error("Failed to parse statement at line " +
                                 std::to_string(token.line));
      }

    } break;
    case TokenType::COUT_KEYWORD: {
      auto coutNode = parseCout();

      if (coutNode) {
        // Create a new SequenceNode and add the parsed statement
        return coutNode;
      } else {
        throw std::runtime_error("Failed to parse statement at line " +
                                 std::to_string(token.line));
      }
    }
    }
    return std::nullopt;
  }

  std::string printTokenType(TokenType type) {
    switch (type) {
    case TokenType::IDENTIFIER:
      return "IDENTIFIER";
    case TokenType::CONSTANT:
      return "CONSTANT";
    case TokenType::CIN_KEYWORD:
      return "CIN_KEYWORD";
    case TokenType::COUT_KEYWORD:
      return "COUT_KEYWORD";
    case TokenType::PUNCTUATOR:
      return "PUNCTUATOR";
    case TokenType::LITERAL:
      return "LITERAL";
    case TokenType::ADDITION_OPERATOR:
      return "ADDITION_OPERATOR";
    case TokenType::SUBTRACTION_OPERATOR:
      return "SUBTRACTION_OPERATOR";
    case TokenType::DELIMITER:
      return "DELIMITER";
    case TokenType::ASSIGNMENT_OPERATOR:
      return "ASSIGNMENT_OPERATOR";
    case TokenType::CIN_OPERATOR:
      return "CIN_OPERATOR";
    case TokenType::COUT_OPERATOR:
      return "COUT_OPERATOR";
    case TokenType::UNKNOWN:
      return "UNKNOWN";
    }
    return "UNKNOWN";
  }

  void expect(TokenType type, TOKEN &token) {
    // if we reach the end
    if (token.type != type) {
      throw std::runtime_error("Expected " + printTokenType(type) +
                               " but got " + printTokenType(token.type) +
                               " at line " + std::to_string(token.line));
    }
  }
};

}; // namespace parser

std::string printTokenType(
    TokenType type); // helper function to print token type (Debugging purposes)

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "Usage: ./main <file>" << std::endl;
    return 1;
  }

  std::string CODE;
  std::string filename = argv[1];

  CODE = fetchSourceCode(filename); // get the source code

  try {
    Lexer lexer(CODE); // instantiate a lexer object under Lexer class

    std::vector<TOKEN> TOKENS = lexer.lex();

    parser::Parser parser(TOKENS);

    std::vector<std::shared_ptr<parser::Node>> NODES = parser.parse();

    // Print tokens for debugging
    for (auto token : TOKENS) {
      std::cout << token.lexeme << " :: " << printTokenType(token.type)
                << " :: LINE: " << token.line << std::endl;
    }
    std::cout << "\n++++++++++++++++++++++++++++" << std::endl;
    std::cout << "++++++++++++++++++++++++++++\n" << std::endl;
    std::cout << "Parse Tree: " << std::endl;

    for (auto node : NODES) {
      node->print();
      std::cout << std::endl;
    }

  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  return 0;
}

std::string printTokenType(TokenType type) {
  switch (type) {
  case TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::CONSTANT:
    return "CONSTANT";
  case TokenType::CIN_KEYWORD:
    return "CIN_KEYWORD";
  case TokenType::COUT_KEYWORD:
    return "COUT_KEYWORD";
  case TokenType::PUNCTUATOR:
    return "PUNCTUATOR";
  case TokenType::LITERAL:
    return "LITERAL";
  case TokenType::ADDITION_OPERATOR:
    return "ADDITION_OPERATOR";
  case TokenType::SUBTRACTION_OPERATOR:
    return "SUBTRACTION_OPERATOR";
    return "DIVISION_OPERATOR";
  case TokenType::DELIMITER:
    return "DELIMITER";
  case TokenType::ASSIGNMENT_OPERATOR:
    return "ASSIGNMENT_OPERATOR";
  case TokenType::INT_KEYWORD:
    return "INT_KEYWORD";
  case TokenType::CIN_OPERATOR:
    return "CIN_OPERATOR";
  case TokenType::COUT_OPERATOR:
    return "COUT_OPERATOR";
  case TokenType::UNKNOWN:
    return "UNKNOWN";
  }
  return "UNKNOWN";
}
