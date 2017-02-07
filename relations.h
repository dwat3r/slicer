#ifndef RELATIONS_H
#define RELATIONS_H

#include <cmath>
#include "clang/AST/AST.h"
#include <set>

/*
 * Information-flow relations:
 * There are Statements
 *
 * Every function solves it's task recursively.
 */


typedef clang::DeclRefExpr* cVar;
typedef clang::Stmt* cStmt;

class Statement
{
public:
  /* Give it statement-local vars/exps
   */
  Statement();
  Statement(cVar var,cStmt expr,
            std::set<cVar> evars,std::set<cVar> defs)
    : var(var)
    , expr(expr)
    , evars(evars)
    , defs(defs)
  {}
  virtual ~Statement(){}
  /* Returns definitions used in the Statement,
   * recursively.
   */
  virtual std::set<cVar> getDefs() const;
  // Returns variables used in expr.
  std::set<cVar> getEVars() const {return evars;}
  //
  std::set<std::pair<cVar,cVar>> id() const;
  virtual std::set<std::pair<cVar,cStmt>> lambda() const;
  virtual std::set<std::pair<cStmt,cVar>> u() const;
  virtual std::set<std::pair<cVar,cVar>>  p() const;

  /* Returns the expressions which may affect
   * the value of cVar in parameter
   * recursively
   * {e `elem` E|e |u| v}
   */
  std::set<cStmt> slice(cVar var);
  // Auxilliary functions
  void addChild();

protected:
  // sub-statements
  std::vector<Statement> children;
  // Variable and expression exclusive to this Statement.
  cVar  var;
  cStmt expr;
  // evars contain the variables occuring in expr.
  std::set<cVar> evars;
  std::set<cVar> defs;
  // Store a reference to the AST
  cStmt astRef;
};

// Specializations
class AssignStatement : public Statement{
public:
  std::set<cVar> getDefs() const;
  std::set<std::pair<cVar,cStmt>> lambda() const;
  std::set<std::pair<cStmt,cVar>> u() const;
  std::set<std::pair<cVar,cVar>>  p() const;
};
class CompoundStatement : public Statement{
public:
  std::set<std::pair<cVar,cStmt>> lambda() const;
  std::set<std::pair<cStmt,cVar>> u() const;
  std::set<std::pair<cVar,cVar>>  p() const;
};
class BranchStatement : public Statement{
public:
  std::set<std::pair<cVar,cStmt>> lambda() const;
  std::set<std::pair<cStmt,cVar>> u() const;
  std::set<std::pair<cVar,cVar>>  p() const;
};
class Branch_elseStatement : public Statement{
public:
  std::set<std::pair<cVar,cStmt>> lambda() const;
  std::set<std::pair<cStmt,cVar>> u() const;
  std::set<std::pair<cVar,cVar>>  p() const;
};
class LoopStatement : public Statement{
public:
  std::set<std::pair<cVar,cStmt>> lambda() const;
  std::set<std::pair<cStmt,cVar>> u() const;
  std::set<std::pair<cVar,cVar>>  p() const;
};

#endif // RELATIONS_H
