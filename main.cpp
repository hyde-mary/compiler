#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

// USED BY THE LEXER
enum class TokenType {
  ADDITION_OPERATOR,
  SUBTRACTION_OPERATOR,
  ASSIGNMENT_OPERATOR,
  COUT_KEYWORD,
  DELIMITER,
  PUNCTUATOR,
  IDENTIFIER,
  INT_KEYWORD,
  CONSTANT,
  LITERAL,
  COUT_OPERATOR,
  CIN_OPERATOR,
  CIN_KEYWORD,
  UNKNOWN
};

struct TOKEN {
  TokenType type;
  std::string lexeme;
  int line;
};

class Lexer {
public:
  /* Constructor */
  Lexer(const std::string SRC_CODE) : SRC_CODE(std::move(SRC_CODE)) {}

  std::vector<TOKEN> lex() {

    std::vector<TOKEN> tokens;
    std::string buffer;

    while (peek().has_value()) {

      if (peek().value() == '\n' || peek().value() == '\r') {
        PROGRAM_LINE++;
        consume();
        continue;
      }

      if (std::iswspace(peek().value())) {
        consume();
        continue;
      }

      if (peek().value() == ';') {
        buffer.push_back(consume());
        tokens.push_back({TokenType::DELIMITER, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      if (peek().value() == ',') {
        buffer.push_back(consume());
        tokens.push_back({TokenType::PUNCTUATOR, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      /* check for literal */
      if (peek().value() == '"') {
        consume();
        while (peek().has_value() && peek().value() != '"') {
          buffer.push_back(consume());
        }
        if (peek().has_value() && peek().value() == '"') {
          consume(); // Consume closing quote
          tokens.push_back({TokenType::LITERAL, buffer, PROGRAM_LINE});
        } else {
          // Handle error: unmatched opening quote
        }
        buffer.clear();
        continue;
      }

      if (isOperator(peek().value())) {
        buffer.push_back(consume());
        tokens.push_back({getOpTokenType(buffer[0]), buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      /* for indentifiers */
      if (std::isalpha(peek().value())) {
        buffer.push_back(consume());
        while (std::isalnum(peek().value()) || peek().value() == '_') {
          buffer.push_back(consume());
        }
        tokens.push_back({getKeywordTokenType(buffer), buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      if (std::isdigit(peek().value())) {
        buffer.push_back(consume());
        while (std::isdigit(peek().value())) {
          buffer.push_back(consume());
        }
        tokens.push_back({TokenType::CONSTANT, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      /* Check for CIN and COUT operators */
      if (peek().value() == '<') {
        buffer.push_back(consume());
        if (peek().value() == '<') {
          buffer.push_back(consume());
          tokens.push_back({TokenType::COUT_OPERATOR, buffer, PROGRAM_LINE});
          buffer.clear();
          continue;
        }
        buffer.clear(); /* clear buffer if not << */
        continue;
      }

      if (peek().value() == '>') {

        buffer.push_back(consume());
        if (peek().value() == '>') {
          buffer.push_back(consume());
          tokens.push_back({TokenType::CIN_OPERATOR, buffer, PROGRAM_LINE});
          buffer.clear();
          continue;
        }
        buffer.clear(); /* clear buffer if not >> */
        continue;
      }

      // we add a token with unknown type
      tokens.push_back(
          {TokenType::UNKNOWN, std::string(1, consume()), PROGRAM_LINE});
    }

    return tokens;
  }

private:
  int LEX_COUNTER = 0;
  int PROGRAM_LINE = 1;
  std::string SRC_CODE;

  std::optional<char> peek() {
    /*
    It checks if we have reached the end of the source code.
    If we have reached the end, it returns std::nullopt
    If we have not reached the end, it returns character
    */
    if (LEX_COUNTER >= SRC_CODE.size()) {
      return std::nullopt;
    }
    return SRC_CODE.at(LEX_COUNTER);
  }

  char consume() {
    /*
    Function to consume the current character in the source code.
    It checks if we have reached the end of the source code.
    If we have reached the end, it throws an exception
    If we have not reached the end, it returns the character an increments the
    LEX_COUNTER
    */
    if (LEX_COUNTER >= SRC_CODE.size()) {
      throw std::out_of_range(
          "Attempted to consume past the end of the source code.");
    }
    return SRC_CODE.at(LEX_COUNTER++);
  }

  TokenType getOpTokenType(char ch) {
    switch (ch) {
    case '+':
      return TokenType::ADDITION_OPERATOR;
    case '-':
      return TokenType::SUBTRACTION_OPERATOR;
    case '=':
      return TokenType::ASSIGNMENT_OPERATOR;
    }
    return TokenType::IDENTIFIER;
  }

  TokenType getKeywordTokenType(std::string keyword) {
    if (keyword == "cin") {
      return TokenType::CIN_KEYWORD;
    } else if (keyword == "cout") {
      return TokenType::COUT_KEYWORD;
    } else if (keyword == "int") {
      return TokenType::INT_KEYWORD;
    }
    return TokenType::IDENTIFIER;
  }

  bool isOperator(char ch) {
    return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
            ch == '^' || ch == '=' || ch == '!');
  }
};

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
  std::variant<std::shared_ptr<ConstantNode>, std::shared_ptr<ExpressionNode>>
      assignment;

  // Updated constructor to remove the line parameter
  AssignmentNode(std::shared_ptr<IdentifierNode> identifier,
                 std::variant<std::shared_ptr<ConstantNode>,
                              std::shared_ptr<ExpressionNode>>
                     assignment)
      : identifier(std::move(identifier)), assignment(std::move(assignment)) {}

  void print() const override {
    std::cout << "- AssignmentNode" << " " << identifier->name << " "
              << std::endl;
    if (std::holds_alternative<std::shared_ptr<ConstantNode>>(assignment)) {
      auto constant = std::get<std::shared_ptr<ConstantNode>>(assignment);
      constant->print();
    } else {
      auto expression = std::get<std::shared_ptr<ExpressionNode>>(assignment);
      expression->print();
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

  std::variant<std::shared_ptr<ConstantNode>, std::shared_ptr<ExpressionNode>>
  parseExpression() {
    auto left =
        parseConstant(); // this will parse a constant and consume the token

    // Check if the current token after consumption is an operator
    if (peek().value().type == TokenType::ADDITION_OPERATOR) {
      consume(); // Consume '+'
      auto right = parseConstant();
      return std::make_shared<ExpressionNode>('+', left, right);
    } else if (peek().value().type == TokenType::SUBTRACTION_OPERATOR) {
      consume(); // Consume '-'
      auto right = parseConstant();
      return std::make_shared<ExpressionNode>('-', left, right);
    }
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
      throw std::runtime_error("Syntax error at line " +
                               std::to_string(token.line));
    }
  }
};
}; // namespace parser

std::string printTokenType(
    TokenType type); // helper function to print token type (Debugging purposes)
std::string fetchSourceCode(std::string filename);
int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "Usage: ./main <file>" << std::endl;
    return 1;
  }

  std::string CODE;
  std::string filename = argv[1];

  CODE = fetchSourceCode(filename);

  try {
    Lexer lexer(CODE);

    std::vector<TOKEN> TOKENS = lexer.lex();

    parser::Parser parser(TOKENS);

    std::vector<std::shared_ptr<parser::Node>> NODES = parser.parse();

    // Print tokens for debugging
    for (auto token : TOKENS) {
      std::cout << token.lexeme << " :: " << printTokenType(token.type)
                << " :: LINE: " << token.line << std::endl;
    }

    std::cout << "Parse Tree: " << std::endl;

    for (auto node : NODES) {
      node->print();
    }

  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  return 0;
}

std::string fetchSourceCode(std::string fileName) {
  std::ifstream file(fileName);

  if (!file) {
    std::cout << "Could not open file: " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }

  std::ostringstream ss;
  ss << file.rdbuf(); // Reading Code
  return ss.str();    // Code as string
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
