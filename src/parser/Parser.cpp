#include "parser.hpp"

namespace parser {
Parser::Parser(std::vector<TOKEN> tokens)
    : TOKENS(std::move(tokens)), NODES{} {}

std::vector<std::shared_ptr<node::Node>> Parser::parse() {
  // Parse all the statements from the tokens vector
  while (peek().has_value()) {
    if (auto node = parseStatement(); node.has_value()) {
      NODES.push_back(*node);
    }
  }
  return NODES;
}

std::optional<TOKEN> Parser::peek() {
  if (P_COUNTER >= TOKENS.size()) {
    return std::nullopt;
  }
  return TOKENS.at(P_COUNTER);
}

std::optional<TOKEN> Parser::consume() {
  if (P_COUNTER >= TOKENS.size()) {
    return std::nullopt;
  }
  return TOKENS.at(P_COUNTER++);
}

std::shared_ptr<ConstantNode> Parser::parseConstant() {
  auto token = peek().value(); // peak the current token
  expect(TokenType::CONSTANT,
         token); // check if the current token is a constant
  consume();
  return std::make_shared<ConstantNode>(token, token.line);
}

std::shared_ptr<IdentifierNode> Parser::parseIdentifier() {
  auto token = peek().value();
  expect(TokenType::IDENTIFIER, token);
  consume();
  return std::make_shared<IdentifierNode>(token, token.line);
}

std::shared_ptr<node::Node> Parser::parseConstantOrIdentifier() {
  if (peek().value().type == TokenType::CONSTANT) {
    return parseConstant();
  } else if (peek().value().type == TokenType::IDENTIFIER) {
    return parseIdentifier();
  } else {
    throw std::runtime_error("Invalid token");
  }
}

std::shared_ptr<node::Node> Parser::parseExpression() {
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
  // auto token = peek().value();
  // expect(TokenType::DELIMITER, token);

  return left;
}

std::shared_ptr<AssignmentNode> Parser::parseAssignment() {
  // Parse the identifier
  // automatically consumes
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

std::shared_ptr<node::Node> Parser::parseDeclaration() {
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
        auto token = peek().value();
        expect(TokenType::DELIMITER, token);
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

std::shared_ptr<node::Node> Parser::parseLiteral() {
  auto token = peek().value();
  expect(TokenType::LITERAL, token);
  consume();
  return std::make_shared<StringLiteralNode>(token);
}

std::shared_ptr<node::Node> Parser::parseCout() {

  std::vector<std::shared_ptr<node::Node>> outputOperands;

  auto token = peek().value();
  expect(TokenType::COUT_KEYWORD, token);
  consume();

  while (peek().value().type == TokenType::IDENTIFIER ||
         peek().value().type == TokenType::CONSTANT ||
         peek().value().type == TokenType::LITERAL ||
         peek().value().type == TokenType::COUT_OPERATOR) {

    if (peek().value().type == TokenType::COUT_OPERATOR) {
      consume();
      token = peek().value();
      if (token.type != TokenType::IDENTIFIER &&
          token.type != TokenType::CONSTANT &&
          token.type != TokenType::LITERAL) {
        throw std::runtime_error("Syntax Error: Expected identifier, "
                                 "constant, or literal after << operator.");
      }
      continue;
    }

    if (peek().value().type == TokenType::IDENTIFIER ||
        peek().value().type == TokenType::CONSTANT) {
      auto expr = parseExpression();
      outputOperands.push_back(expr);
    } else if (peek().value().type == TokenType::LITERAL) {
      auto literal = parseLiteral();
      outputOperands.push_back(literal);
    }
  }

  // expect token at the end
  token = peek().value();
  expect(TokenType::DELIMITER, token);
  consume();

  return std::make_shared<CoutNode>(outputOperands);
}

std::shared_ptr<node::Node> Parser::parseCin() {
  std::vector<std::shared_ptr<node::Node>> inputOperands;

  auto token = peek().value();
  expect(TokenType::CIN_KEYWORD, token);
  consume();

  while (peek().value().type == TokenType::IDENTIFIER ||
         peek().value().type == TokenType::CIN_OPERATOR) {

    if (peek().value().type == TokenType::IDENTIFIER) {
      auto identifier = parseIdentifier();
      inputOperands.push_back(identifier);
    } else if (peek().value().type == TokenType::CIN_OPERATOR) {
      consume();
      // we always expect an identifier after the CIN operator
      // if there is no identifier, the expect() will throw an error
      token = peek().value();
      expect(TokenType::IDENTIFIER, token);
      continue;
    }
  }

  // expect token at the end
  token = peek().value();
  expect(TokenType::DELIMITER, token);
  consume();

  return std::make_shared<CinNode>(inputOperands);
}

std::optional<std::shared_ptr<node::Node>> Parser::parseStatement() {
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
  case TokenType::CIN_KEYWORD: {
    auto cinNode = parseCin();

    if (cinNode) {
      // Create a new SequenceNode and add the parsed statement
      return cinNode;
    } else {
      throw std::runtime_error("Failed to parse statement at line " +
                               std::to_string(token.line));
    }
  }
  default: {
  }
  }
  return std::nullopt;
}

std::string Parser::printTokenType(TokenType type) {
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
  default:
    return "UNKNOWN";
  }
  return "UNKNOWN";
}

void Parser::expect(TokenType type, TOKEN &token) {
  // if we reach the end
  if (token.type != type) {
    throw std::runtime_error("Expected " + printTokenType(type) + " but got " +
                             printTokenType(token.type) + " at line " +
                             std::to_string(token.line));
  }
}

} // namespace parser
