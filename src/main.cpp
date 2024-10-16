#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"

std::string printTokenType(
    TokenType type); // helper function to print token type (Debugging purposes)
std::string fetchSourceCode(std::string filename);
void writeAssembly(std::string filename, std::string src);
std::string changeExtension(const std::string &filename,
                            const std::string &newExtension);
void assembleCode(std::string &filename, std::string &exename);

/* SEMANTIC ANALYZER NAMESPACE*/
namespace semantic {
class SymbolTable {
public:
  void declareVariable(TOKEN &token) {
    if (declared_variables.find(token.lexeme) != declared_variables.end()) {
      throw std::runtime_error("Semantic Error: Variable '" + token.lexeme +
                               "' is already declared.");
    }
    declared_variables[token.lexeme] = token.type;
    initialized_variables[token.lexeme] = false;
  }

  TokenType lookupVariable(TOKEN &token) {
    if (declared_variables.find(token.lexeme) == declared_variables.end()) {
      throw std::runtime_error("Semantic Error: Variable '" + token.lexeme +
                               "' is not declared.");
    }
    return token.type;
  }

  void setInitialized(TOKEN &token) {
    initialized_variables[token.lexeme] = true;
  }

  void isInitialized(TOKEN &token) {
    auto it = initialized_variables.find(token.lexeme);
    if (it == initialized_variables.end() || it->second != true) {
      throw std::runtime_error("Semantic Error: Variable '" + token.lexeme +
                               "' is not initialized.");
    }
  }
  // for debugging
  void printInitialized() {
    for (auto var : initialized_variables) {
      std::cout << var.first << " :: " << var.second << std::endl;
    }
  }

private:
  std::unordered_map<std::string, TokenType> declared_variables;
  std::unordered_map<std::string, bool> initialized_variables;
};

class SyntaxAnalyzer {
public:
  SyntaxAnalyzer(const std::vector<std::shared_ptr<node::Node>> &Nodes)
      : Nodes(std::move(Nodes)) {}

  void analyzeSemantics() {
    for (auto &node : Nodes) {
      std::cout << "Analyzing: ";
      node->toString();
      analyzeNode(node);
      // symbolTable.printInitialized();
    }
    std::cout << "Semantics Analyzed: No errors." << std::endl;
  }

private:
  SymbolTable symbolTable;
  std::vector<std::shared_ptr<node::Node>> Nodes;

  void analyzeDeclaration(std::shared_ptr<DeclarationNode> node) {
    // we add all declared identifiers in the symbol table
    if (std::holds_alternative<std::vector<std::shared_ptr<IdentifierNode>>>(
            node->product)) {
      auto identifiers =
          std::get<std::vector<std::shared_ptr<IdentifierNode>>>(node->product);
      for (auto &id : identifiers) {
        // process identifier
        symbolTable.declareVariable(id->identifier);
      }
    } else if (std::holds_alternative<std::shared_ptr<AssignmentNode>>(
                   node->product)) {
      auto assignment =
          std::get<std::shared_ptr<AssignmentNode>>(node->product);
      // process assignment
      // LMAO AHSJDAKJDKAJSDLAWKDJLAKWDJLAKWD
      symbolTable.declareVariable(assignment->identifier->identifier);
      analyzeAssignment(assignment);
    }
  }

  void analyzeExpression(std::shared_ptr<ExpressionNode> node) {
    std::shared_ptr<node::Node> left = node->left;
    std::shared_ptr<node::Node> right = node->right;

    // process left
    if (auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(left)) {
      symbolTable.isInitialized(identifierNode->identifier);
      symbolTable.lookupVariable(identifierNode->identifier);
    }

    // process right
    if (auto identifierNode =
            std::dynamic_pointer_cast<IdentifierNode>(right)) {
      symbolTable.isInitialized(identifierNode->identifier);
      symbolTable.lookupVariable(identifierNode->identifier);
    }
  }

  void analyzeIdentifier(std::shared_ptr<IdentifierNode> node) {
    symbolTable.lookupVariable(node->identifier);
  }

  void analyzeAssignment(std::shared_ptr<AssignmentNode> node) {
    /*
      An assingment in the Assignment node can be a
      Constant Node, Identifier Node or Expression Node.
      We nee to analyze each of these nodes individually.
    */
    symbolTable.setInitialized(
        node->identifier->identifier); // we set the initialized flag as true

    if (auto constantNode =
            std::dynamic_pointer_cast<ConstantNode>(node->assignment)) {
      // maybe a bit redundant lmao
      // since we already know that it's a constant
      // Just incase
      if (constantNode->constant.type != TokenType::CONSTANT) {
        throw std::runtime_error(
            "Semantic Error: Type mismatch in assignment at line " +
            std::to_string(constantNode->constant.line));
      }
    } else if (auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(
                   node->assignment)) {
      TokenType type = symbolTable.lookupVariable(identifierNode->identifier);
      if (type != node->identifier->identifier.type) {
        throw std::runtime_error(
            "Semantic Error: Type mismatch in assignment at line " +
            std::to_string(identifierNode->identifier.line));
      }
    } else if (auto expressionNode = std::dynamic_pointer_cast<ExpressionNode>(
                   node->assignment)) {
      analyzeExpression(expressionNode);
    } else {
      throw std::runtime_error(
          "Semantic Error: Unknown node type in assignment.");
    }
  }

  void analyzeNode(const std::shared_ptr<node::Node> &node,
                   bool isInCin = false) {
    if (auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(node)) {
      analyzeIdentifier(identifierNode);
      // this may be cheating lol
      // basically since we are using cin, we do not need the varibales to be
      // initialized
      if (isInCin) {
        symbolTable.setInitialized(
            identifierNode->identifier); // we cheat by making it seem that the
                                         // variable is initialized already
      }
      symbolTable.isInitialized(identifierNode->identifier);
    } else if (auto stringLiteralNode =
                   std::dynamic_pointer_cast<StringLiteralNode>(node)) {
      // hatdog
    } else if (auto constantNode =
                   std::dynamic_pointer_cast<ConstantNode>(node)) {
      // hatdog
    } else if (auto declNode =
                   std::dynamic_pointer_cast<DeclarationNode>(node)) {
      analyzeDeclaration(declNode);
    } else if (auto assignNode =
                   std::dynamic_pointer_cast<AssignmentNode>(node)) {
      analyzeAssignment(assignNode);
    } else if (auto exprNode =
                   std::dynamic_pointer_cast<ExpressionNode>(node)) {
      analyzeExpression(exprNode);
    } else if (auto sequenceNode =
                   std::dynamic_pointer_cast<SequenceNode>(node)) {
      for (auto statement : sequenceNode->statements) {
        analyzeNode(statement);
      }
    } else if (auto cinNode = std::dynamic_pointer_cast<CinNode>(node)) {
      for (auto operand : cinNode->operands) {
        analyzeNode(operand, true);
      }
    } else if (auto coutNode = std::dynamic_pointer_cast<CoutNode>(node)) {
      for (auto operand : coutNode->operands) {
        analyzeNode(operand);
      }
    } else {
      throw std::runtime_error("Semantic Error: Unknown node type.");
    }
  }
};
} // namespace semantic

namespace generator {
class Generator {
public:
  Generator(std::vector<std::shared_ptr<node::Node>> nodes)
      : NODES(std::move(nodes)) {}

  std::string generate() {

    std::ostringstream headerCode;
    std::ostringstream dynamicCode;

    headerCode << "default rel\n";

    headerCode << "extern ExitProcess\n";
    headerCode << "extern printf\n";
    headerCode << "extern scanf\n";

    // declared variables un initialized
    bss_segment << "section .bss \n";

    // declared variables initialized
    data_segment << "section .data \n";
    data_segment << "    input_int db \"%d\", 0\n";
    data_segment << "    fmt_int db \"%d\", 10, 0\n";
    data_segment << "    fmt_literal db \"%s\", 10, 0\n";
    data_segment << "    fmt_char db \"%c\", 10, 0\n";

    // text segment
    text_segment << "segment .text \n";
    text_segment << "    " << "global main\n";
    text_segment << "    " << "main:\n";
    text_segment << "\t\t" << "push rbp\n";
    text_segment << "\t\t" << "mov rbp, rsp\n";

    for (auto node : NODES) {
      nodeGenerator(node);
    }

    // end of text segment
    text_segment << std::endl;
    text_segment << "\t\t" << "mov rcx, 1 \n";
    text_segment << "\t\t" << "xor rcx, rcx\n";
    text_segment << "\t\t" << "call ExitProcess";

    std::cout << headerCode.str() << std::endl;
    std::cout << bss_segment.str() << std::endl;
    std::cout << data_segment.str() << std::endl;
    std::cout << text_segment.str() << std::endl;

    return headerCode.str() + bss_segment.str() + data_segment.str() +
           text_segment.str();
  }

private:
  std::vector<std::shared_ptr<node::Node>> NODES;
  size_t generator_count = 0; // for incrementing the nodes / parse tree

  std::unordered_map<std::string, bool> initialized_variables;
  std::unordered_map<std::string, bool> uninitialized_variables;

  std::ostringstream bss_segment;
  std::ostringstream data_segment;
  std::ostringstream text_segment;

  std::optional<std::shared_ptr<node::Node>> peek() {
    if (generator_count > NODES.size()) {
      return std::nullopt;
    }
    return NODES.at(generator_count);
  }

  std::optional<std::shared_ptr<node::Node>> consume() {
    if (generator_count > NODES.size()) {
      return std::nullopt;
    }
    return NODES.at(generator_count++);
  }

  std::ostringstream
  processExpression(std::shared_ptr<ExpressionNode> expressionNode,
                    bool isIn = false) {
    // return output;
    std::ostringstream output;

    if (auto leftNode =
            std::dynamic_pointer_cast<IdentifierNode>(expressionNode->left)) {
      // if (!isInitialized(leftNode->identifier.lexeme)) {
      //   data_segment << "    "  << leftNode->identifier.lexeme << " dq 0" <<
      //   "\n";
      // }
      output << "\t\t" << "mov rax, [" << leftNode->identifier.lexeme << "]"
             << "\n";
    } else if (auto leftNode = std::dynamic_pointer_cast<ConstantNode>(
                   expressionNode->left)) {
      output << "\t\t" << "mov rax, " << leftNode->constant.lexeme << "\n";
    }

    if (expressionNode->OP == '+') {
      if (auto rightNode = std::dynamic_pointer_cast<IdentifierNode>(
              expressionNode->right)) {
        // if (!isInitialized(rightNode->identifier.lexeme)) {
        //   data_segment << "    "  << rightNode->identifier.lexeme << " dq 0"
        //   << "\n";
        // }
        output << "\t\t" << "add rax, [" << rightNode->identifier.lexeme << "]"
               << "\n";
      } else if (auto rightNode = std::dynamic_pointer_cast<ConstantNode>(
                     expressionNode->right)) {
        output << "\t\t" << "add rax, " << rightNode->constant.lexeme << "\n";
      }
    }

    if (expressionNode->OP == '-') {
      if (auto rightNode = std::dynamic_pointer_cast<IdentifierNode>(
              expressionNode->right)) {
        output << "\t\t" << "sub rax, " << rightNode->identifier.lexeme << "\n";
      } else if (auto rightNode = std::dynamic_pointer_cast<ConstantNode>(
                     expressionNode->right)) {
        output << "\t\t" << "sub rax, " << rightNode->constant.lexeme << "\n";
      }
    }

    return output;
  }

  std::ostringstream
  processAssignment(std::shared_ptr<AssignmentNode> assignmentNode) {

    std::ostringstream output;

    auto assignment = assignmentNode->assignment;

    if (auto constantNode =
            std::dynamic_pointer_cast<ConstantNode>(assignment)) {
      output << constantNode->constant.lexeme;
    } else if (auto expressionNode =
                   std::dynamic_pointer_cast<ExpressionNode>(assignment)) {
      output << processExpression(expressionNode).str();
    }

    return output;
  }

  bool isInitialized(std::string key) {
    if (initialized_variables.find(key) != initialized_variables.end()) {
      return initialized_variables[key];
    }
    return false;
  }

  bool isUninitialized(std::string key) {
    if (uninitialized_variables.find(key) != uninitialized_variables.end()) {
      return uninitialized_variables[key];
    }
    return false;
  }

  void processDeclaration(std::shared_ptr<DeclarationNode> declarationNode) {
    /*
      The process declaration function is used to generate code for variable
      declarations. If the product of the declaration node is an assignment
      node, it will generate code for that assignment. (.data section) Else if
      the product of the declaration node is an identifier node, it will
      generate code for that identifier. (.bss section)
    */
    if (std::holds_alternative<std::shared_ptr<AssignmentNode>>(
            declarationNode->product)) {
      auto assignmentNode =
          std::get<std::shared_ptr<AssignmentNode>>(declarationNode->product);
      if (auto constNode = std::dynamic_pointer_cast<ConstantNode>(
              assignmentNode->assignment)) {
        data_segment << "    " << assignmentNode->identifier->identifier.lexeme
                     << " dd " << constNode->constant.lexeme << "\n";
        initialized_variables[assignmentNode->identifier->identifier.lexeme] =
            true;
      } else if (auto expressionNode =
                     std::dynamic_pointer_cast<ExpressionNode>(
                         assignmentNode->assignment)) {
        /*
          if the declaration is an expression node, it will generate code for
          that expression. first it will declare the variable with a value of 0
          then it will assign the value of the expression to the variable after
          perform the expression
        */
        data_segment << "    " << assignmentNode->identifier->identifier.lexeme
                     << " dd 0" << "\n";
        text_segment << processExpression(expressionNode).str() << "\n";
        text_segment << "\t\t" << "mov ["
                     << assignmentNode->identifier->identifier.lexeme
                     << "], rax" << "\n";
      }

    } else if (std::holds_alternative<
                   std::vector<std::shared_ptr<IdentifierNode>>>(
                   declarationNode->product)) {
      auto identNodes = std::get<std::vector<std::shared_ptr<IdentifierNode>>>(
          declarationNode->product);
      for (auto &id : identNodes) {
        // indentifier nodes under declaration node indicates uninitialized
        // variables these variables are declared in the bss segment
        bss_segment << "    " << id->identifier.lexeme
                    << " resd 1 ;  Reserve 4 bytes for int"
                    << id->identifier.lexeme << std::endl;
        uninitialized_variables[id->identifier.lexeme] = true;
      }
    }
  }

  std::ostringstream processCout(std::shared_ptr<CoutNode> coutNode) {

    std::ostringstream output;

    for (auto &expr : coutNode->operands) {
      if (auto expressionNode =
              std::dynamic_pointer_cast<ExpressionNode>(expr)) {
        output << processExpression(expressionNode).str() << "\n";
        output << "\t\t" << "mov rcx, fmt_int" << "\n";
        output << "\t\t" << "mov rdx, rax" << "\n";
        output << "\t\t" << "call printf" << "\n";
      } else if (auto identifierNode =
                     std::dynamic_pointer_cast<IdentifierNode>(expr)) {
        output << "\t\t" << "mov rcx, fmt_int" << "\n";
        output << "\t\t" << "mov rdx, [" << identifierNode->identifier.lexeme
               << "]" << "\n";
        output << "\t\t" << "call printf" << "\n";
      } else if (auto literalNode =
                     std::dynamic_pointer_cast<StringLiteralNode>(expr)) {

        // we add a string in the datasegment first
        // then we refernce it when invoking lea rdx
        data_segment << "    " << literalNode->literal.lexeme << " db \""
                     << literalNode->literal.lexeme << "\", 0" << "\n";
        output << "\t\t" << "mov rcx, fmt_literal" << "\n";
        output << "\t\t" << "lea rdx, [" << literalNode->literal.lexeme << "]"
               << "\n";
        output << "\t\t" << "call printf" << "\n";
      } else if (auto constantLiteral =
                     std::dynamic_pointer_cast<ConstantNode>(expr)) {
        output << "\t\t" << "mov rcx, fmt_int" << "\n";
        output << "\t\t" << "mov rdx, " << constantLiteral->constant.lexeme
               << "\n";
        output << "\t\t" << "call printf" << "\n";
      }
    }

    return output;
  }

  std::ostringstream processCin(std::shared_ptr<CinNode> cinNode) {

    std::ostringstream output;

    for (auto &expr : cinNode->operands) {
      auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(expr);
      output << "\t\t" << "lea rcx, [input_int]" << "\n";
      output << "\t\t" << "lea rdx, [" << identifierNode->identifier.lexeme
             << "]" << "\n";
      output << "\t\t" << "call scanf" << "\n";
    }

    return output;
  }

  void nodeGenerator(std::shared_ptr<node::Node> node) {

    if (auto declNode = std::dynamic_pointer_cast<DeclarationNode>(node)) {
      // TODO: process declaration
      // this is a void function, as it only assigns/checks for variable
      // declarations the declared variables are added to a map of key value
      // pairs. this "optimizes" the code lmao. cheating in a sense ba to,
      // hatdog
      processDeclaration(declNode);

    } else if (auto assignNode =
                   std::dynamic_pointer_cast<AssignmentNode>(node)) {
      // TODO: process assignment
      if (auto constNode =
              std::dynamic_pointer_cast<ConstantNode>(assignNode->assignment)) {
        text_segment << "\t" << "mov dword ["
                     << assignNode->identifier->identifier.lexeme << "], "
                     << constNode->constant.lexeme << "\n";
      } else if (auto expressionNode =
                     std::dynamic_pointer_cast<ExpressionNode>(
                         assignNode->assignment)) {
        text_segment << processAssignment(assignNode).str() << "\n";
        text_segment << "\t\t" << "mov ["
                     << assignNode->identifier->identifier.lexeme << "], rax\n";
      }

    } else if (auto cinNode = std::dynamic_pointer_cast<CinNode>(node)) {
      // TODO: process cin
      /**
       *
       * ; Call scanf to read an integer
        lea rdi, [fmt]        ; Load the address of the format string
        lea rsi, [x]         ; Load the address of the variable to store the
       integer call scanf
       */
      text_segment << processCin(cinNode).str() << "\n";
    } else if (auto coutNode = std::dynamic_pointer_cast<CoutNode>(node)) {
      text_segment << processCout(coutNode).str() << "\n";
    } else if (auto sequenceNode =
                   std::dynamic_pointer_cast<SequenceNode>(node)) {
      for (auto statement : sequenceNode->statements) {
        nodeGenerator(statement);
      }
    }
  }
};
} // namespace generator

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "Usage: ./main <file>" << std::endl;
    return 1;
  }

  std::string CODE;
  std::string filename = argv[1];
  std::string exename = argv[2];

  CODE = fetchSourceCode(filename);

  try {
    Lexer lexer(CODE);

    std::vector<TOKEN> TOKENS = lexer.lex();

    parser::Parser parser(TOKENS);
    std::vector<std::shared_ptr<node::Node>> NODES = parser.parse();
    semantic::SyntaxAnalyzer analyzer(NODES);
    generator::Generator generator(NODES);

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

    std::cout << "\n++++++++++++++++++++++++++++" << std::endl;
    std::cout << "++++++++++++++++++++++++++++\n" << std::endl;
    std::cout << "Semantic Analyzer Results: " << std::endl;

    analyzer.analyzeSemantics();

    std::cout << "\n++++++++++++++++++++++++++++" << std::endl;
    std::cout << "++++++++++++++++++++++++++++\n" << std::endl;
    std::cout << "Code Generator Results: " << std::endl;
    std::string asm_generated = generator.generate();

    std::string newFile = changeExtension(filename, ".asm");
    writeAssembly(newFile, asm_generated);

    assembleCode(filename, exename); // assemble the ASM file

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

void writeAssembly(std::string filename, std::string src) {
  std::ofstream file;
  file.open(filename);
  file << src;
  file.close();
}

std::string changeExtension(const std::string &filename,
                            const std::string &newExtension) {
  // Find the last dot in the filename
  size_t dotPos = filename.rfind('.');

  // If there's no dot or it's the first character, return the filename
  // unchanged
  if (dotPos == std::string::npos || dotPos == 0) {
    return filename;
  }

  // Return the filename up to the dot, then add the new extension
  return filename.substr(0, dotPos) + newExtension;
}

void assembleCode(std::string &filename, std::string &exename) {
  std::string file_asm = changeExtension(filename, ".asm");
  std::string file_o = changeExtension(filename, ".o");
  std::string file_exe = changeExtension(exename, ".exe");

  std::string nasm = "nasm -f win64 -o " + file_o + " " + file_asm;
  std::cout << nasm << std::endl;
  std::system(nasm.c_str());
  std::string gcc = "gcc -o " + file_exe + " " + file_o;
  std::cout << gcc << std::endl;
  std::system(gcc.c_str());
}
