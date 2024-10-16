#ifndef CIN_NODE_HPP
#define CIN_NODE_HPP

#include "../Node.hpp"

class CinNode : public node::Node {
public:
  std::vector<std::shared_ptr<Node>> operands;
  CinNode(std::vector<std::shared_ptr<Node>> operands);

  void print() const override;

  void toString() const override;
};

#endif //! CIN_NODE_HPP
