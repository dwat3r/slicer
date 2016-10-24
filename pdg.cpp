#include "pdg.h"
#include <iostream>

std::ostream& operator << (std::ostream& os,Node& node)
{
  os << node.type   << " "
     << node.astref << ",";
  for (neighbor n : node.adlist)
    os << std::get<0>(n) << " "
       << std::get<1>(n) << ",";
  os << std::endl;
  return os;
}

std::ostream& operator << (std::ostream& os,Edge& edge)
{
  os << edge.from  << " "
     << edge.to    << " "
     <<  edge.label   << " "
     << std::endl;
  return os;
}


void PDG::addNode(Node *parent, Node *child, Label label)
{
  Edge *edge = new Edge(parent,child,label);
  parent->AddChild(child,edge);
}

void PDG::print()
{
  for (Node* node : nodes)
      std::cout << *node << std::endl;
}

