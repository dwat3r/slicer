#ifndef GRAPH_H
#define GRAPH_H

#include "clang/AST/AST.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

enum NodeType {Statement, Region, Entry, Exit};

struct SNode
{
  NodeType type;
  clang::Stmt* astref;
};

struct SEdge
{
  bool label;
};

typedef boost::adjacency_list<
    boost::listS, boost::vecS, boost::bidirectionalS,
    SNode, SEdge> Graph;

typedef boost::graph_traits<Graph>::vertex_descriptor Node;
typedef boost::graph_traits<Graph>::edge_descriptor Edge;

#endif // GRAPH_H
