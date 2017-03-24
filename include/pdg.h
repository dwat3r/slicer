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
  Statement(const clang::Stmt* astRef)
    : astRef(astRef)
  {}
  
  virtual ~Statement(){}

  // Edge adders
  void addControlChild(Statement* s, bool edge) { controlChildren.push_back({ s,edge }); }
  void addDataEdge(Statement* s) { dataEdges.push_back(s); }
  void setControlParent(std::pair<Statement*, bool> parent) { controlParent = parent; }

  // getters
  std::pair<Statement*,bool> getControlParent() { return controlParent; }
  std::vector<std::pair<Statement*, bool>> getControlChildren() { return controlChildren; }
  std::vector<Statement*> getDataEdges() { return dataEdges; }
  // returns if source has path to dest in graph.
  bool isControlReachableUpwards(Statement* source, Statement* dest);

  // returns true if branch
  virtual bool isBranchStatement() { return false; }
protected:

  // dependences
  std::vector<std::pair<Statement*,bool>> controlChildren;
  std::vector<Statement*> dataEdges;
  std::pair<Statement*,bool> controlParent;

  // Store a reference to the AST
  const clang::Stmt* astRef;
};

// Specializations
class AssignStatement : public Statement{
public:
  AssignStatement(const clang::Stmt* astref,
                  const clang::ValueDecl* _define)
    : Statement(astref)
    , define(_define)
  {}
  void setDefine(clang::ValueDecl* _define) { define = _define; }
  void addUse(clang::ValueDecl* _use) { use.push_back(_use); }
private:
  // These store the variables that are defined / used in this assignment
  const clang::ValueDecl* define;
  std::vector<const clang::ValueDecl*> use;
};

class BranchStatement : public Statement{
public:
  using Statement::Statement;
  void setCondRef(clang::Stmt* _condRef) { condRef = _condRef; }
  void addCondVar(clang::ValueDecl* _condVar) { condVars.push_back(_condVar); }
  virtual bool isBranchStatement() override { return true; } 
private:
  // We store the condition reference and the variables used in this condition
  clang::Stmt* condRef;
  std::vector<clang::ValueDecl*> condVars;
};
// Loop and Branch almost the same, but they differ in the data dependence edge creations.
class LoopStatement : public Statement{
public:
  using Statement::Statement;
  void setCondRef(clang::Stmt* _condRef) { condRef = _condRef; }
  void addCondVar(clang::ValueDecl* _condVar) { condVars.push_back(_condVar); }
private:
  // We store the condition reference and the variables used in this condition
  clang::Stmt* condRef;
  std::vector<clang::ValueDecl*> condVars;
};

class CompoundStatement : public Statement {
public:
  using Statement::Statement;

};
#endif // PDG_H
