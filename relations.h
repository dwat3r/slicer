#ifndef RELATIONS_H
#define RELATIONS_H

#include "clang/AST/AST.h"
#include <set>

/*
 * Information-flow relations:
 * There are Statements
 * parametrized on type SType
 *
 * Every function solves it's task recursively.
 */

enum class SType {Assign, Compound, Branch, Branch_else, Loop};

typedef clang::DeclRefExpr* cVar;
typedef clang::Stmt* cStmt;

template <SType>
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
  /* Returns definitions used in the Statement,
   * recursively.
   */
  std::set<cVar> getDefs() const;
  // Returns variables used in expr.
  std::set<cVar> getEVars() const {return evars;}
  //
  std::set<std::pair<cVar,cVar> > id() const;
  std::set<std::pair<cVar,cStmt> > lambda() const;
  std::set<std::pair<cStmt,cVar> > u() const;
  std::set<std::pair<cVar,cVar> >  p() const;

  /* Returns the statements which may affect
   * the value of cVar in parameter
   * recursively
   */
  std::set<Statement> slice(cVar);
  // Auxilliary functions
  void addChild();

private:
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

#endif // RELATIONS_H
