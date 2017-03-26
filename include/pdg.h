#ifndef PDG_H
#define PDG_H

#include "clang/AST/AST.h"
#include <set>

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
            std::set<std::pair<Statement*,bool>> _cChildren)
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
  void addControlChild(std::pair<Statement*,bool> edge) { controlChildren.insert(edge); }
  void addDataEdge(Statement* s) { dataEdges.insert(s); }
  void setControlParent(std::pair<Statement*, bool> parent) { controlParent = parent; }

  // define/use
  void addUse(const clang::ValueDecl* _use) { use.insert(_use); }
  void setDefine(const clang::ValueDecl* _define) { define = _define; }
  // getters
  std::pair<Statement*,bool> getControlParent() { return controlParent; }
  std::set<std::pair<Statement*, bool>> getControlChildren() { return controlChildren; }
  std::set<Statement*> getDataEdges() { return dataEdges; }
  const clang::ValueDecl* getDefine() { return define; }
  std::set<const clang::ValueDecl*> getUses() { return use; }
  // returns if source has path to dest in graph.
  bool isControlReachableUpwards(Statement* source, Statement* dest);

  // name
  virtual std::string name() { return "Statement"; }
  // returns true if branch
  virtual bool isBranchStatement() { return false; }

  // factory method
  static Statement* create(const clang::Stmt* astref);
  // print structure
  std::string dump();
protected:
  std::string dumpLevel(int level);
  std::set<std::pair<Statement*,bool>> controlChildren;
  std::set<Statement*> dataEdges;
  std::pair<Statement*,bool> controlParent;
  // These store the variables that are defined / used in this statement
  const clang::ValueDecl* define = nullptr;
  std::set<const clang::ValueDecl*> use;
  
  // Store a reference to the AST
  const clang::Stmt* astRef = nullptr;
};

// Specializations
class AssignStatement : public Statement{
public:
  using Statement::Statement;
  virtual std::string name() { return "Assign"; }
};

class BranchStatement : public Statement{
public:
  using Statement::Statement;
  virtual bool isBranchStatement() override { return true; } 
  virtual std::string name() { return "Branch"; }
};
// Loop and Branch almost the same, but they differ in the data dependence edge creations.
class LoopStatement : public Statement{
public:
  using Statement::Statement;
  virtual std::string name() { return "Loop"; }
};

class CompoundStatement : public Statement {
public:
  using Statement::Statement;
  virtual std::string name() { return "Compound"; }
};
#endif // PDG_H
