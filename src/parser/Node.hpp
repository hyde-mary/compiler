#ifndef NODE_HPP
#define NODE_HPP

#include <iostream>
#include <memory>
#include <variant>
#include <vector>

#include "../common/Token.hpp"

namespace node {
class Node {
public:
  ~Node() = default; /* Destructor which can be overridden by subclasses */
  virtual void print() const = 0;
  virtual void toString() const = 0;
};
} // namespace node

#endif // !NODE_HPP
