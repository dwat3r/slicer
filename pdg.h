#ifndef PDG_H
#define PDG_H

#include "clang/AST/AST.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

enum NodeType {Statement, Region, Entry, Exit};

struct PDGNode
{
  NodeType type;
  clang::Stmt* astref;
};

struct PDGEdge
{
  bool label;
};

typedef boost::adjacency_list<
    boost::listS, boost::vecS, boost::bidirectionalS,
    PDGNode, PDGEdge> PDG;

typedef boost::graph_traits<PDG>::vertex_descriptor Node;
typedef boost::graph_traits<PDG>::edge_descriptor Edge;

#endif // PDG_H
