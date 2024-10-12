#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

// USED BY THE LEXER
enum class TokenType
{
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

struct TOKEN
{
  TokenType type;
  std::string lexeme;
  int line;
};

std::string printTokenType(TokenType type); // helper function to print token type (Debugging purposes)

std::string fetchSourceCode(std::string filename);

class Lexer
{
public:
  /* Constructor */
  Lexer(const std::string SRC_CODE) : SRC_CODE(std::move(SRC_CODE)) {}

  std::vector<TOKEN> lex()
  {

    std::vector<TOKEN> tokens;
    std::string buffer;

    while (peek().has_value())
    {

      if (peek().value() == '\n' || peek().value() == '\r')
      {
        PROGRAM_LINE++;
        consume();
        continue;
      }

      if (std::iswspace(peek().value()))
      {
        consume();
        continue;
      }

      if (peek().value() == ';')
      {
        buffer.push_back(consume());
        tokens.push_back({TokenType::DELIMITER, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      if (peek().value() == ',')
      {
        buffer.push_back(consume());
        tokens.push_back({TokenType::PUNCTUATOR, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      /* check for literal */
      if (peek().value() == '"')
      {
        consume();
        while (peek().has_value() && peek().value() != '"')
        {
          buffer.push_back(consume());
        }
        if (peek().has_value() && peek().value() == '"')
        {
          consume(); // Consume closing quote
          tokens.push_back({TokenType::LITERAL, buffer, PROGRAM_LINE});
        }
        else
        {
          // Handle error: unmatched opening quote
        }
        buffer.clear();
        continue;
      }

      if (isOperator(peek().value()))
      {
        buffer.push_back(consume());
        tokens.push_back({getOpTokenType(buffer[0]), buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      /* for indentifiers */
      if (std::isalpha(peek().value()))
      {
        buffer.push_back(consume());
        while (std::isalnum(peek().value()) || peek().value() == '_')
        {
          buffer.push_back(consume());
        }
        tokens.push_back({getKeywordTokenType(buffer), buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      if (std::isdigit(peek().value()))
      {
        buffer.push_back(consume());
        while (std::isdigit(peek().value()))
        {
          buffer.push_back(consume());
        }
        tokens.push_back({TokenType::CONSTANT, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      /* Check for CIN and COUT operators */
      if (peek().value() == '<')
      {
        buffer.push_back(consume());
        if (peek().value() == '<')
        {
          buffer.push_back(consume());
          tokens.push_back({TokenType::COUT_OPERATOR, buffer, PROGRAM_LINE});
          buffer.clear();
          continue;
        }
        tokens.push_back({TokenType::UNKNOWN, buffer, PROGRAM_LINE});
        buffer.clear();
        continue;
      }

      if (peek().value() == '>')
      {

        buffer.push_back(consume());
        if (peek().value() == '>')
        {
          buffer.push_back(consume());
          tokens.push_back({TokenType::CIN_OPERATOR, buffer, PROGRAM_LINE});
          buffer.clear();
          continue;
        }
        tokens.push_back({TokenType::UNKNOWN, buffer, PROGRAM_LINE});
        buffer.clear();
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

  std::optional<char> peek()
  {
    /*
    It checks if we have reached the end of the source code.
    If we have reached the end, it returns std::nullopt
    If we have not reached the end, it returns character
    */
    if (LEX_COUNTER >= SRC_CODE.size())
    {
      return std::nullopt;
    }
    return SRC_CODE.at(LEX_COUNTER);
  }

  char consume()
  {
    /*
    Function to consume the current character in the source code.
    It checks if we have reached the end of the source code.
    If we have reached the end, it throws an exception
    If we have not reached the end, it returns the character an increments the
    LEX_COUNTER
    */
    if (LEX_COUNTER >= SRC_CODE.size())
    {
      throw std::out_of_range(
          "Attempted to consume past the end of the source code.");
    }
    return SRC_CODE.at(LEX_COUNTER++);
  }

  TokenType getOpTokenType(char ch)
  {
    switch (ch)
    {
    case '+':
      return TokenType::ADDITION_OPERATOR;
    case '-':
      return TokenType::SUBTRACTION_OPERATOR;
    case '=':
      return TokenType::ASSIGNMENT_OPERATOR;
    }
    return TokenType::IDENTIFIER;
  }

  TokenType getKeywordTokenType(std::string keyword)
  {
    if (keyword == "cin")
    {
      return TokenType::CIN_KEYWORD;
    }
    else if (keyword == "cout")
    {
      return TokenType::COUT_KEYWORD;
    }
    else if (keyword == "int")
    {
      return TokenType::INT_KEYWORD;
    }
    return TokenType::IDENTIFIER;
  }

  bool isOperator(char ch)
  {
    return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' ||
            ch == '^' || ch == '=' || ch == '!');
  }
};

/* SYNTAX ANALYZER NAMESPACE */
namespace parser
{
  class Node
  {
  public:
    ~Node() = default; /* Destructor which can be overridden by subclasses */
    virtual void print() const = 0;
    virtual void toString() const = 0;
  };

  class IdentifierNode : public Node
  {
  public:
    TOKEN identifier;
    int line; /* Line number of the identifier */
    IdentifierNode(TOKEN identifier, int line)
        : identifier(std::move(identifier)), line(line) {}

    void print() const override
    {
      std::cout << "\t - IdentifierNode :: IDENTIFIER - " << identifier.lexeme << std::endl;
    }

    void toString() const override
    {
      std::cout << "IdentifierNode" << std::endl;
    }
  };

  class ConstantNode : public Node
  {
    /*
    A constant node is a node that represents a constant or an Integer Literal.
    */
  public:
    TOKEN constant;
    int line; /* Line number of the constant */
    ConstantNode(TOKEN constant, int line) : constant(std::move(constant)), line(line) {}

    void print() const override
    {
      std::cout << "\t - ConstantNode " << constant.lexeme << std::endl;
    }
    void toString() const override
    {
      std::cout << "ConstantNode" << std::endl;
    }
  };

  class ExpressionNode : public Node
  {
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

    void print() const override
    {
      std::cout << "\t - ExpressionNode " << OP << std::endl;
      std::cout << "\t\t - left: ";
      left->print();
      std::cout << "\t\t - right: ";
      right->print();
    }

    void toString() const override
    {
      std::cout << "ExpressionNode" << std::endl;
    }
  };

  class AssignmentNode : public Node
  {
  public:
    std::shared_ptr<IdentifierNode> identifier;
    std::shared_ptr<Node> assignment;

    // Updated constructor to remove the line parameter
    AssignmentNode(std::shared_ptr<IdentifierNode> identifier,
                   std::shared_ptr<Node> assignment)
        : identifier(std::move(identifier)), assignment(std::move(assignment)) {}

    void print() const override
    {
      std::cout << "- AssignmentNode" << " " << identifier->identifier.lexeme << " "
                << std::endl;
      assignment->print();
    }
    void toString() const override
    {
      std::cout << "AssignmentNode" << std::endl;
    }
  };

  class DeclarationNode : public Node
  {
  public:
    TokenType type;
    std::variant<std::vector<std::shared_ptr<IdentifierNode>>, std::shared_ptr<AssignmentNode>> product;

    // Constructor that accepts the type and identifier as parameters
    DeclarationNode(TokenType type, std::variant<std::vector<std::shared_ptr<IdentifierNode>>, std::shared_ptr<AssignmentNode>> product)
        : type(type), product(std::move(product)) {}

    void print() const override
    {
      std::cout << "DeclarationNode" << std::endl;
      if (std::holds_alternative<std::shared_ptr<AssignmentNode>>(product))
      {
        auto assignment = std::get<std::shared_ptr<AssignmentNode>>(product);
        assignment->print();
      }
      else
      {
        auto identifiers = std::get<std::vector<std::shared_ptr<IdentifierNode>>>(product);
        for (const auto &identifier : identifiers)
        {
          identifier->print();
        }
      }
    }

    void toString() const override
    {
      std::cout << "DeclarationNode" << std::endl;
    }
  };

  class SequenceNode : public Node
  {
    /* This node represents a sequence of nodes. Can be sequence for assignment
     * declaration, etc.*/
  public:
    std::vector<std::shared_ptr<Node>> statements;

    SequenceNode() = default;

    // Add a statement to the sequence
    void addStatement(std::shared_ptr<Node> statement)
    {
      statements.push_back(std::move(statement));
    }

    void print() const override
    {
      std::cout << "SequenceNode" << std::endl;
      for (const auto &stmt : statements)
      {
        stmt->print();
      }
    }

    void toString() const override
    {
      std::cout << "SequenceNode" << std::endl;
    }
  };

  class StringLiteralNode : public Node
  {
  public:
    TOKEN literal;
    StringLiteralNode(TOKEN literal) : literal(std::move(literal)) {}

    void print() const override
    {
      std::cout << "\t - StringLiteralNode " << "'" << literal.lexeme << "'" << std::endl;
    }

    void toString() const override
    {
      std::cout << "StringLiteralNode" << std::endl;
    }
  };

  class CoutNode : public Node
  {
  public:
    std::vector<std::shared_ptr<Node>> operands;
    CoutNode(std::vector<std::shared_ptr<Node>> operands) : operands(std::move(operands)) {}

    void print() const override
    {
      std::cout << "CoutNode " << std::endl;
      for (const auto &operand : operands)
      {
        operand->print();
      }
    }
    void toString() const override
    {
      std::cout << "CoutNode" << std::endl;
    }
  };

  class CinNode : public Node
  {
  public:
    std::vector<std::shared_ptr<Node>> operands;
    CinNode(std::vector<std::shared_ptr<Node>> operands) : operands(std::move(operands)) {}

    void print() const override
    {
      std::cout << "CinNode " << std::endl;
      for (const auto &operand : operands)
      {
        operand->print();
      }
    }
    void toString() const override
    {
      std::cout << "CinNode" << std::endl;
    }
  };

  class Parser
  {
  public:
    Parser(std::vector<TOKEN> tokens) : TOKENS(std::move(tokens)), NODES{} {}

    std::vector<std::shared_ptr<Node>> parse()
    {
      // Parse all the statements from the tokens vector
      while (peek().has_value())
      {
        if (auto node = parseStatement(); node.has_value())
        {
          NODES.push_back(*node);
        }
      }
      return NODES;
    }

  private:
    std::vector<TOKEN> TOKENS;
    std::vector<std::shared_ptr<Node>> NODES;

    int P_COUNTER = 0;

    std::optional<TOKEN> peek()
    {
      if (P_COUNTER >= TOKENS.size())
      {
        return std::nullopt;
      }
      return TOKENS.at(P_COUNTER);
    }

    std::optional<TOKEN> consume()
    {
      if (P_COUNTER >= TOKENS.size())
      {
        return std::nullopt;
      }
      return TOKENS.at(P_COUNTER++);
    }

    std::shared_ptr<ConstantNode> parseConstant()
    {
      auto token = peek().value(); // peak the current token
      expect(TokenType::CONSTANT,
             token); // check if the current token is a constant
      consume();
      return std::make_shared<ConstantNode>(token, token.line);
    }

    std::shared_ptr<IdentifierNode> parseIdentifier()
    {
      auto token = peek().value();
      expect(TokenType::IDENTIFIER, token);
      consume();
      return std::make_shared<IdentifierNode>(token, token.line);
    }

    std::shared_ptr<Node> parseConstantOrIdentifier()
    {
      if (peek().value().type == TokenType::CONSTANT)
      {
        return parseConstant();
      }
      else if (peek().value().type == TokenType::IDENTIFIER)
      {
        return parseIdentifier();
      }
      else
      {
        throw std::runtime_error("Invalid token");
      }
    }

    std::shared_ptr<Node> parseExpression()
    {
      auto left = parseConstantOrIdentifier(); // this will parse a constant and consume the token

      // Check if the current token after consumption is an operator
      if (peek().value().type == TokenType::ADDITION_OPERATOR)
      {
        consume(); // Consume '+'
        auto right = parseConstantOrIdentifier();
        return std::make_shared<ExpressionNode>('+', left, right);
      }
      else if (peek().value().type == TokenType::SUBTRACTION_OPERATOR)
      {
        consume(); // Consume '-'
        auto right = parseConstantOrIdentifier();
        return std::make_shared<ExpressionNode>('-', left, right);
      }
      // Check if the next token is a delimiter (e.g., ';')
      // auto token = peek().value();
      // expect(TokenType::DELIMITER, token);

      return left;
    }

    std::shared_ptr<AssignmentNode> parseAssignment()
    {
      // Parse the identifier
      // automatically consumes
      auto identifier = parseIdentifier(); // This should return a shared_ptr<IdentifierNode>
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

    std::shared_ptr<Node> parseDeclaration()
    {
      std::vector<std::shared_ptr<IdentifierNode>> identifiers;

      const TokenType declarationType = peek().value().type;
      consume();

      // Parse identifiers until the next delimiter (e.g., ';')
      // Possible side effect here is when there is no delimiter
      // in the source code.
      while (peek().value().type == TokenType::IDENTIFIER || peek().value().type == TokenType::PUNCTUATOR)
      {
        if (peek().value().type == TokenType::IDENTIFIER)
        {
          auto identifier = parseIdentifier(); // simply parse the identifier

          // after consumption of the parseIdentifier, we check if the next token is an assignment operator
          if (peek().value().type == TokenType::ASSIGNMENT_OPERATOR)
          {
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
          }
          else
          {
            // Otherwise, it's just an identifier in a list of declarations
            identifiers.push_back(identifier);

            // checks if there is a missing punctuator
            if (peek().has_value() && peek().value().type == TokenType::IDENTIFIER)
            {
              throw std::runtime_error("Syntax Error: Missing punctuator between identifiers.");
            }
          }
        }
        else if (peek().value().type == TokenType::PUNCTUATOR)
        {
          consume();
          continue;
        }
        else
        {
          throw std::runtime_error("Syntax Error: Expected delimiter at the end of declaration.");
        }
      }
      auto token = peek().value();
      expect(TokenType::DELIMITER, token);
      consume();
      return std::make_shared<DeclarationNode>(declarationType, identifiers);
    }

    std::shared_ptr<Node> parseLiteral()
    {
      auto token = peek().value();
      expect(TokenType::LITERAL, token);
      consume();
      return std::make_shared<StringLiteralNode>(token);
    }

    std::shared_ptr<Node> parseCout()
    {

      std::vector<std::shared_ptr<Node>> outputOperands;

      auto token = peek().value();
      expect(TokenType::COUT_KEYWORD, token);
      consume();

      while (peek().value().type == TokenType::IDENTIFIER ||
             peek().value().type == TokenType::CONSTANT ||
             peek().value().type == TokenType::LITERAL ||
             peek().value().type == TokenType::COUT_OPERATOR)
      {

        if (peek().value().type == TokenType::COUT_OPERATOR)
        {
          consume();
          token = peek().value();
          if (token.type != TokenType::IDENTIFIER && token.type != TokenType::CONSTANT && token.type != TokenType::LITERAL)
          {
            throw std::runtime_error("Syntax Error: Expected identifier, constant, or literal after << operator.");
          }
          continue;
        }

        if (peek().value().type == TokenType::IDENTIFIER || peek().value().type == TokenType::CONSTANT)
        {
          auto expr = parseExpression();
          outputOperands.push_back(expr);
        }
        else if (peek().value().type == TokenType::LITERAL)
        {
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

    std::shared_ptr<Node> parseCin()
    {
      std::vector<std::shared_ptr<Node>> inputOperands;

      auto token = peek().value();
      expect(TokenType::CIN_KEYWORD, token);
      consume();

      while (peek().value().type == TokenType::IDENTIFIER || peek().value().type == TokenType::CIN_OPERATOR)
      {

        if (peek().value().type == TokenType::IDENTIFIER)
        {
          auto identifier = parseIdentifier();
          inputOperands.push_back(identifier);
        }
        else if (peek().value().type == TokenType::CIN_OPERATOR)
        {
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

    std::optional<std::shared_ptr<Node>> parseStatement()
    {
      TOKEN token = peek().value();

      switch (token.type)
      {
      case TokenType::IDENTIFIER:
      {
        auto assignmentNode = parseAssignment();

        if (assignmentNode)
        {
          // Create a new SequenceNode and add the parsed statement
          auto sequenceNode = std::make_shared<SequenceNode>();
          sequenceNode->addStatement(assignmentNode);
          return sequenceNode;
        }
        else
        {
          throw std::runtime_error("Failed to parse statement at line " +
                                   std::to_string(token.line));
        }
      }
      break;
      case TokenType::INT_KEYWORD:
      {

        auto declarationNode = parseDeclaration();

        if (declarationNode)
        {
          // auto sequenceNode = std::make_shared<SequenceNode>();
          // sequenceNode->addStatement(declarationNode);
          return declarationNode;
        }
        else
        {
          throw std::runtime_error("Failed to parse statement at line " +
                                   std::to_string(token.line));
        }
      }
      break;
      case TokenType::COUT_KEYWORD:
      {
        auto coutNode = parseCout();

        if (coutNode)
        {
          // Create a new SequenceNode and add the parsed statement
          return coutNode;
        }
        else
        {
          throw std::runtime_error("Failed to parse statement at line " +
                                   std::to_string(token.line));
        }
      }

      case TokenType::CIN_KEYWORD:
      {
        auto cinNode = parseCin();

        if (cinNode)
        {
          // Create a new SequenceNode and add the parsed statement
          return cinNode;
        }
        else
        {
          throw std::runtime_error("Failed to parse statement at line " +
                                   std::to_string(token.line));
        }
      }
      }
      return std::nullopt;
    }

    std::string printTokenType(TokenType type)
    {
      switch (type)
      {
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

    void expect(TokenType type, TOKEN &token)
    {
      // if we reach the end
      if (token.type != type)
      {
        throw std::runtime_error(
            "Expected " + printTokenType(type) + " but got " +
            printTokenType(token.type) + " at line " + std::to_string(token.line));
      }
    }
  };

}; // namespace parser

/* SEMANTIC ANALYZER NAMESPACE*/
namespace semantic
{
  class SymbolTable
  {
  public:
    void declareVariable(TOKEN &token)
    {
      if (variables.find(token.lexeme) != variables.end())
      {
        throw std::runtime_error("Semantic Error: Variable '" + token.lexeme + "' is already declared.");
      }
      variables[token.lexeme] = token.type;
    }

    TokenType lookupVariable(TOKEN &token)
    {
      if (variables.find(token.lexeme) == variables.end())
      {
        throw std::runtime_error("Semantic Error: Variable '" + token.lexeme + "' is not declared.");
      }
      return token.type;
    }

  private:
    std::unordered_map<std::string, TokenType> variables;
  };

  class SyntaxAnalyzer
  {
  public:
    SyntaxAnalyzer(const std::vector<std::shared_ptr<parser::Node>> &Nodes) : Nodes(std::move(Nodes)) {}

    void analyzeSemantics()
    {
      for (auto &node : Nodes)
      {
        std::cout << "Analyzing: ";
        node->toString();
        analyzeNode(node);
      }
      std::cout << "Semantics Analyzed: No errors." << std::endl;
    }

  private:
    SymbolTable symbolTable;
    std::vector<std::shared_ptr<parser::Node>> Nodes;

    void analyzeDeclaration(std::shared_ptr<parser::DeclarationNode> node)
    {
      // we add all declared identifiers in the symbol table
      if (std::holds_alternative<std::vector<std::shared_ptr<parser::IdentifierNode>>>(node->product))
      {
        auto identifiers = std::get<std::vector<std::shared_ptr<parser::IdentifierNode>>>(node->product);
        for (auto &id : identifiers)
        {
          // process identifier
          symbolTable.declareVariable(id->identifier);
        }
      }
      else if (std::holds_alternative<std::shared_ptr<parser::AssignmentNode>>(node->product))
      {
        auto assignment = std::get<std::shared_ptr<parser::AssignmentNode>>(node->product);
        // process assignment
        // LMAO AHSJDAKJDKAJSDLAWKDJLAKWDJLAKWD
        symbolTable.declareVariable(assignment->identifier->identifier);
        analyzeAssignment(assignment);
      }
    }

    void analyzeExpression(std::shared_ptr<parser::ExpressionNode> node)
    {
      std::shared_ptr<parser::Node> left = node->left;
      std::shared_ptr<parser::Node> right = node->right;

      // process left
      if (auto identifierNode = std::dynamic_pointer_cast<parser::IdentifierNode>(left))
      {
        symbolTable.lookupVariable(identifierNode->identifier);
      }

      // process right
      if (auto identifierNode = std::dynamic_pointer_cast<parser::IdentifierNode>(right))
      {
        symbolTable.lookupVariable(identifierNode->identifier);
      }
    }

    void analyzeIdentifier(std::shared_ptr<parser::IdentifierNode> node)
    {
      symbolTable.lookupVariable(node->identifier);
    }

    void analyzeAssignment(std::shared_ptr<parser::AssignmentNode> node)
    {
      /*
        An assingment in the Assignment node can be a
        Constant Node, Identifier Node or Expression Node.
        We nee to analyze each of these nodes individually.
      */
      if (auto constantNode = std::dynamic_pointer_cast<parser::ConstantNode>(node->assignment))
      {
        // maybe a bit redundant lmao
        // since we already know that it's a constant
        // Just incase
        if (constantNode->constant.type != TokenType::CONSTANT)
        {
          throw std::runtime_error("Semantic Error: Type mismatch in assignment at line " + std::to_string(constantNode->constant.line));
        }
      }
      else if (auto identifierNode = std::dynamic_pointer_cast<parser::IdentifierNode>(node->assignment))
      {
        TokenType type = symbolTable.lookupVariable(identifierNode->identifier);
        if (type != node->identifier->identifier.type)
        {
          throw std::runtime_error("Semantic Error: Type mismatch in assignment at line " + std::to_string(identifierNode->identifier.line));
        }
      }
      else if (auto expressionNode = std::dynamic_pointer_cast<parser::ExpressionNode>(node->assignment))
      {
        analyzeExpression(expressionNode);
      }
      else
      {
        throw std::runtime_error("Semantic Error: Unknown node type in assignment.");
      }
    }

    void analyzeNode(const std::shared_ptr<parser::Node> &node)
    {
      if (auto identifierNode = std::dynamic_pointer_cast<parser::IdentifierNode>(node))
      {
        analyzeIdentifier(identifierNode);
      }
      else if (auto declNode = std::dynamic_pointer_cast<parser::DeclarationNode>(node))
      {
        analyzeDeclaration(declNode);
      }
      else if (auto assignNode = std::dynamic_pointer_cast<parser::AssignmentNode>(node))
      {
        analyzeAssignment(assignNode);
      }
      else if (auto exprNode = std::dynamic_pointer_cast<parser::ExpressionNode>(node))
      {
        analyzeExpression(exprNode);
      }
      else if (auto sequenceNode = std::dynamic_pointer_cast<parser::SequenceNode>(node))
      {
        for (auto statement : sequenceNode->statements)
        {
          analyzeNode(statement);
        }
      }
      else if (auto cinNode = std::dynamic_pointer_cast<parser::CinNode>(node))
      {
        for (auto operand : cinNode->operands)
        {
          analyzeNode(operand);
        }
      }
      else if (auto coutNode = std::dynamic_pointer_cast<parser::CoutNode>(node))
      {
        for (auto operand : coutNode->operands)
        {
          analyzeNode(operand);
        }
      }
      else
      {
        throw std::runtime_error("Semantic Error: Unknown node type.");
      }
    }
  };
}

namespace generator
{
  
}

int main(int argc, char *argv[])
{

  if (argc < 2)
  {
    std::cout << "Usage: ./main <file>" << std::endl;
    return 1;
  }

  std::string CODE;
  std::string filename = argv[1];

  CODE = fetchSourceCode(filename);

  try
  {
    Lexer lexer(CODE);

    std::vector<TOKEN> TOKENS = lexer.lex();

    parser::Parser parser(TOKENS);
    std::vector<std::shared_ptr<parser::Node>> NODES = parser.parse();

    semantic::SyntaxAnalyzer analyzer(NODES);

    // Print tokens for debugging
    for (auto token : TOKENS)
    {
      std::cout << token.lexeme << " :: " << printTokenType(token.type)
                << " :: LINE: " << token.line << std::endl;
    }
    std::cout << "\n++++++++++++++++++++++++++++" << std::endl;
    std::cout << "++++++++++++++++++++++++++++\n"
              << std::endl;
    std::cout << "Parse Tree: " << std::endl;

    for (auto node : NODES)
    {
      node->print();
      std::cout << std::endl;
    }

    std::cout << "\n++++++++++++++++++++++++++++" << std::endl;
    std::cout << "++++++++++++++++++++++++++++\n"
              << std::endl;
    std::cout << "Semantic Analyzer Results: " << std::endl;

    analyzer.analyzeSemantics();
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }

  return 0;
}

std::string fetchSourceCode(std::string fileName)
{
  std::ifstream file(fileName);

  if (!file)
  {
    std::cout << "Could not open file: " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }

  std::ostringstream ss;
  ss << file.rdbuf(); // Reading Code
  return ss.str();    // Code as string
}

std::string printTokenType(TokenType type)
{
  switch (type)
  {
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
