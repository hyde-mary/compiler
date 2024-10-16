#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "generator/Generator.hpp"
#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"
#include "semantic/SyntaxAnalyzer.hpp"

std::string printTokenType(
    TokenType type); // helper function to print token type (Debugging purposes)
std::string fetchSourceCode(std::string filename);
void writeAssembly(std::string filename, std::string src);
std::string changeExtension(const std::string &filename,
                            const std::string &newExtension);
void assembleCode(std::string &filename, std::string &exename);

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
    // Representation
    std::cout << std::endl << "Tokens: " << std::endl;
    std::cout << std::left << std::setw(30) << "Token Type" << std::setw(20)
              << "Lexeme" << std::setw(10) << "Line" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const auto &token : TOKENS) {
      std::cout << std::left << std::setw(30) << printTokenType(token.type)
                << std::setw(20) << token.lexeme << std::setw(10) << token.line
                << std::endl;
    }

    std::cout << std::endl << std::string(60, '+') << std::endl;
    std::cout << std::endl << "Parse Tree: " << std::endl;

    for (auto node : NODES) {
      node->print();
      std::cout << std::endl;
    }

    std::cout << std::endl << std::string(60, '+') << std::endl;
    std::cout << std::endl << "Semantic Analyzer Results: " << std::endl;

    analyzer.analyzeSemantics();

    std::cout << std::endl << std::string(60, '+') << std::endl;
    std::cout << std::endl
              << "Code Generator Results: " << std::endl
              << std::endl;
    std::string asm_generated = generator.generate();

    std::string newFile = changeExtension(filename, ".asm");
    writeAssembly(newFile, asm_generated);

    assembleCode(filename, exename); // assemble the ASM file

  } catch (std::exception &e) {
    if (std::string(e.what()) == "bad optional access") {
      std::cerr << "Error: Missing Delimiter at the end of File" << std::endl;
    } else {
      std::cout << "Error: " << e.what() << std::endl;
    }

    return -1;
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
