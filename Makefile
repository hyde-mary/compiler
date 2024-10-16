CXX = g++
CXXFLAGS = -Wall -g
SOURCES = src/main.cpp src/lexer/Lexer.cpp src/parser/Parser.cpp \
          src/parser/AssignmentNode/AssignmentNode.cpp \
          src/parser/CinNode/CinNode.cpp \
          src/parser/ConstantNode/ConstantNode.cpp \
          src/parser/CoutNode/CoutNode.cpp \
          src/parser/DeclarationNode/DeclarationNode.cpp \
          src/parser/ExpressionNode/ExpressionNode.cpp \
          src/parser/IdentifierNode/IdentifierNode.cpp \
          src/parser/SequenceNode/SequenceNode.cpp \
          src/parser/StringLiteralNode/StringLiteralNode.cpp \
          src/semantic/SyntaxAnalyzer.cpp \
          src/generator/Generator.cpp
TARGET = src/main.exe

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
