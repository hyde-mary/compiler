#include "Generator.hpp"
#include "../parser/Node.hpp"

namespace generator {
Generator::Generator(std::vector<std::shared_ptr<node::Node>> nodes)
    : NODES(std::move(nodes)) {}

std::string Generator::generate() {

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

std::optional<std::shared_ptr<node::Node>> Generator::peek() {
  if (generator_count > NODES.size()) {
    return std::nullopt;
  }
  return NODES.at(generator_count);
}

std::optional<std::shared_ptr<node::Node>> Generator::consume() {
  if (generator_count > NODES.size()) {
    return std::nullopt;
  }
  return NODES.at(generator_count++);
}

std::ostringstream
Generator::processExpression(std::shared_ptr<ExpressionNode> expressionNode,
                             bool) {
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
    if (auto rightNode =
            std::dynamic_pointer_cast<IdentifierNode>(expressionNode->right)) {
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
    if (auto rightNode =
            std::dynamic_pointer_cast<IdentifierNode>(expressionNode->right)) {
      output << "\t\t" << "sub rax, " << rightNode->identifier.lexeme << "\n";
    } else if (auto rightNode = std::dynamic_pointer_cast<ConstantNode>(
                   expressionNode->right)) {
      output << "\t\t" << "sub rax, " << rightNode->constant.lexeme << "\n";
    }
  }

  return output;
}

std::ostringstream
Generator::processAssignment(std::shared_ptr<AssignmentNode> assignmentNode) {

  std::ostringstream output;

  auto assignment = assignmentNode->assignment;

  if (auto constantNode = std::dynamic_pointer_cast<ConstantNode>(assignment)) {
    output << constantNode->constant.lexeme;
  } else if (auto expressionNode =
                 std::dynamic_pointer_cast<ExpressionNode>(assignment)) {
    output << processExpression(expressionNode).str();
  }

  return output;
}

bool Generator::isInitialized(std::string key) {
  if (initialized_variables.find(key) != initialized_variables.end()) {
    return initialized_variables[key];
  }
  return false;
}

bool Generator::isUninitialized(std::string key) {
  if (uninitialized_variables.find(key) != uninitialized_variables.end()) {
    return uninitialized_variables[key];
  }
  return false;
}

void Generator::processDeclaration(
    std::shared_ptr<DeclarationNode> declarationNode) {
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
    } else if (auto expressionNode = std::dynamic_pointer_cast<ExpressionNode>(
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
                   << assignmentNode->identifier->identifier.lexeme << "], rax"
                   << "\n";
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

std::string remove_literal_whitespace(std::string str) {
  std::string result;
  std::string::iterator it = str.begin();
  while (it != str.end()) {
    if (*it == ' ') {
      result += '_';
      ++it;
      continue;
    }
    result += *it;
    ++it;
  }
  std::cout<<result<<std::endl;
  return result;
}

std::ostringstream Generator::processCout(std::shared_ptr<CoutNode> coutNode) {

  std::ostringstream output;

  for (auto &expr : coutNode->operands) {
    if (auto expressionNode = std::dynamic_pointer_cast<ExpressionNode>(expr)) {
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
      data_segment << "    " << remove_literal_whitespace(literalNode->literal.lexeme) << " db \""
                   << literalNode->literal.lexeme << "\", 0" << "\n";
      output << "\t\t" << "mov rcx, fmt_literal" << "\n";
      output << "\t\t" << "lea rdx, [" << remove_literal_whitespace(literalNode->literal.lexeme) << "]"
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

std::ostringstream Generator::processCin(std::shared_ptr<CinNode> cinNode) {

  std::ostringstream output;

  for (auto &expr : cinNode->operands) {
    auto identifierNode = std::dynamic_pointer_cast<IdentifierNode>(expr);
    output << "\t\t" << "lea rcx, [input_int]" << "\n";
    output << "\t\t" << "lea rdx, [" << identifierNode->identifier.lexeme << "]"
           << "\n";
    output << "\t\t" << "call scanf" << "\n";
  }

  return output;
}

void Generator::nodeGenerator(std::shared_ptr<node::Node> node) {

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
    } else if (auto expressionNode = std::dynamic_pointer_cast<ExpressionNode>(
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
}; // namespace generator
