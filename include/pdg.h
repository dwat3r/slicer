#ifndef RELATIONS_H
#define RELATIONS_H

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
    , controlDeps()
    , dataDeps()
  {}
  
  virtual ~Statement(){}

  // Edge adders
  void addControlDep(std::shared_ptr<Statement> s, bool edge) { controlDeps.push_back({ s,edge }); }
  void addDataDep(std::shared_ptr<Statement> s) { dataDeps.push_back(s); }
protected:

  // dependences
  std::vector<std::pair<std::shared_ptr<Statement>,bool>> controlDeps;
  std::vector<std::shared_ptr<Statement>> dataDeps;

  //! Store a reference to the AST
  const clang::Stmt* astRef;
};

//! Specializations
class AssignStatement : public Statement{
public:
  AssignStatement(const clang::Stmt* astref,
                  const clang::ValueDecl* _define,
                  std::vector<const clang::ValueDecl*> _use)
    : Statement(astref)
    , define(_define)
    , use(_use)
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
#endif // RELATIONS_H
