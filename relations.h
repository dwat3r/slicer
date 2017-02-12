#ifndef RELATIONS_H
#define RELATIONS_H

#include "clang/AST/AST.h"
#include <set>

/*!
 * Information-flow relations:
 * There are Statements
 *
 * Every function solves it's task recursively.
 * TODO: put into a namespace
 */

typedef clang::ValueDecl* cVar;
typedef clang::Stmt* cStmt;

class Statement
{
public:

  Statement();

  explicit
  Statement(cStmt astRef)
    : astRef(astRef)
  {}

  virtual ~Statement(){}
  /*! Returns definitions used in the Statement,
   *  recursively.
   */
  virtual std::set<cVar> getDefs() const;
  //! Returns variables used in expr.
  std::set<cVar> getEVars() const {return evars;}
  //
  std::set<std::pair<cVar,cVar>> id() const;
  virtual std::set<std::pair<cVar,cStmt>> lambda() const;
  virtual std::set<std::pair<cStmt,cVar>> u() const;
  virtual std::set<std::pair<cVar,cVar>>  p() const;

  /*! Returns the expressions which may affect
   * the value of cVar in parameter
   * recursively
   * {e `elem` E|e |u| v}
   */
  std::set<cStmt> slice(cVar var);

  //! Setter functions
  void addChild(std::shared_ptr<Statement> c);
  //! Bulk setter
  void fill(cVar varp,
            cStmt exprp,
            std::set<cVar> evarsp,
            std::set<cVar> defsp){
    var = varp;
    expr = exprp;
    evars = evarsp;
    defs = defsp;
  }

protected:

  // sub-statements
  std::vector<std::shared_ptr<Statement>> children;
  //! Variable and expression exclusive to this Statement.
  cVar  var;
  cStmt expr;
  //! evars contain the variables occuring in expr.
  std::set<cVar> evars;
  std::set<cVar> defs;
  //! Store a reference to the AST
  cStmt astRef;
};

//! Specializations
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
