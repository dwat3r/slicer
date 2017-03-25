#ifndef PDG_H
#define PDG_H

#include "clang/AST/AST.h"
#include <vector>

/*!
 * Program Dependence Graph
 * Node information stored in Statements
 * specifics are in the specializations
 */

class Statement
{
public:

  explicit
    Statement(const clang::Stmt* _astRef)
    : astRef(_astRef)
  {}

  explicit
  Statement(const clang::Stmt* _astRef,
            std::vector<std::pair<Statement*,bool>> _cChildren)
    : astRef(_astRef)
    , controlChildren(_cChildren)
  {}

  explicit
    Statement(const clang::Stmt* _astRef,
              const clang::ValueDecl* _define)
    : astRef(_astRef)
    , define(_define)
  {}

  virtual ~Statement(){}

  // Edge adders
  void addControlChild(std::pair<Statement*,bool> edge) { controlChildren.push_back(edge); }
  void addDataEdge(Statement* s) { dataEdges.push_back(s); }
  void setControlParent(std::pair<Statement*, bool> parent) { controlParent = parent; }

  // define/use
  void addUse(const clang::ValueDecl* _use) { use.push_back(_use); }
  void setDefine(const clang::ValueDecl* _define) { define = _define; }
  // getters
  std::pair<Statement*,bool> getControlParent() { return controlParent; }
  std::vector<std::pair<Statement*, bool>> getControlChildren() { return controlChildren; }
  std::vector<Statement*> getDataEdges() { return dataEdges; }
  // returns if source has path to dest in graph.
  bool isControlReachableUpwards(Statement* source, Statement* dest);

  // returns true if branch
  virtual bool isBranchStatement() { return false; }

  // factory method
  static Statement* create(const clang::Stmt* astref);
protected:
  std::vector<std::pair<Statement*,bool>> controlChildren;
  std::vector<Statement*> dataEdges;
  std::pair<Statement*,bool> controlParent;
  // These store the variables that are defined / used in this statement
  const clang::ValueDecl* define;
  std::vector<const clang::ValueDecl*> use;
  
  // Store a reference to the AST
  const clang::Stmt* astRef;
};

// Specializations
class AssignStatement : public Statement{
public:
  using Statement::Statement;
};

class BranchStatement : public Statement{
public:
  using Statement::Statement;
  virtual bool isBranchStatement() override { return true; } 
};
// Loop and Branch almost the same, but they differ in the data dependence edge creations.
class LoopStatement : public Statement{
public:
  using Statement::Statement;
};

class CompoundStatement : public Statement {
public:
  using Statement::Statement;

};
#endif // PDG_H
