#ifndef PDG_H
#define PDG_H
#include <map>
#include <vector>
#include <utility>
#include <stack>
#include <memory>
#include "clang/AST/AST.h"

enum NodeType {Statement, Region, Entry, Exit};
enum Label {No_Label,False,True};
class Edge;
class Node;
typedef std::pair<Node*,Edge*> neighbor;
class Node
{
public:
  Node(NodeType type,clang::Stmt* astref = NULL)
    : adlist()
    , type(type)
    , astref(astref){}

  void AddChild(Node *child,Edge *edge)
  {adlist.push_back({child,edge});}
friend std::ostream& operator << (std::ostream& os,Node& node);

private:
  /* Nodes contain their adjancencies
   */
  std::vector<neighbor> adlist;
  NodeType type;
  clang::Stmt* astref;
};

class Edge
{
public:
  Edge() : from(NULL),to(NULL),label(No_Label){}
  Edge(Node *from,Node *to,Label label) :
    from(from),to(to),label(label) {}
  friend std::ostream& operator << (std::ostream& os,Edge& edge);

private:
  Node *from;
  Node *to;
  Label label;
};

class PDG
{
public:
  PDG() : nodes(),stack()
  {
    /* Initializing
     */
    Node *entry = new Node(Entry);
    nodes.push_back(entry);
    nodes.push_back(new Node(Exit));
    stack.push(entry);
  }
  void addNode(Node *parent,Node *child,Label label);
  Node* getActive(){return stack.top();}
  void popStack(){stack.pop();}
  void pushStack(Node* node){stack.push(node);}
  void print();
private:

  std::vector<Node*> nodes;
  std::stack <Node*> stack;
};


#endif // PDG_H
