#include "../include/fetch_source_code.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string fetchSourceCode(const std::string &fileName) {
  std::ifstream file(fileName);

  if (!file) {
    std::cout << "Could not open file: " << fileName << std::endl;
    exit(EXIT_FAILURE);
  }

  std::ostringstream ss;
  ss << file.rdbuf(); // Reading Code
  return ss.str();    // Code as string
}
