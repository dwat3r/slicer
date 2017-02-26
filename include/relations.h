#ifndef RELATIONS_H
#define RELATIONS_H

#include "clang/AST/AST.h"
#include <set>

/*!
 * Information-flow relations:
 * There are Statements
 *
 * Every function solves it's task recursively.
 */
namespace {
	typedef clang::ValueDecl* cVar;
	typedef clang::Stmt* cStmt;
}
class Statement
{
public:

  Statement();

  explicit
  Statement(cStmt astRef)
    : astRef(astRef)
	, children()
	, var()
	, expr()
	, evars()
	, defs()
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
  // Factory method
  static std::shared_ptr<Statement> create(cStmt astref);
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
  using Statement::Statement;
  std::set<cVar> getDefs() const override;
  std::set<std::pair<cVar,cStmt>> lambda() const override;
  std::set<std::pair<cStmt,cVar>> u() const override;
  std::set<std::pair<cVar,cVar>>  p() const override;
};
class CompoundStatement : public Statement{
public:
  using Statement::Statement;
  std::set<std::pair<cVar,cStmt>> lambda() const override;
  std::set<std::pair<cStmt,cVar>> u() const override;
  std::set<std::pair<cVar,cVar>>  p() const override;
};
class BranchStatement : public Statement{
public:
  using Statement::Statement;
  std::set<std::pair<cVar,cStmt>> lambda() const override;
  std::set<std::pair<cStmt,cVar>> u() const override;
  std::set<std::pair<cVar,cVar>>  p() const override;
};
class Branch_elseStatement : public Statement{
public:
  using Statement::Statement;
  std::set<std::pair<cVar,cStmt>> lambda() const override;
  std::set<std::pair<cStmt,cVar>> u() const override;
  std::set<std::pair<cVar,cVar>>  p() const override;
};
class LoopStatement : public Statement{
public:
  using Statement::Statement;
  std::set<std::pair<cVar,cStmt>> lambda() const override;
  std::set<std::pair<cStmt,cVar>> u() const override;
  std::set<std::pair<cVar,cVar>>  p() const override;
};

#endif // RELATIONS_H
