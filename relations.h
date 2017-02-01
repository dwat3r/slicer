#ifndef RELATIONS_H
#define RELATIONS_H

#include "clang/AST/AST.h"
#include <vector>

/*
 * Information-flow relations:
 * There are Statements
 * parametrized on type SType
 *
 * Every function solves it's task recursively.
 */

enum class SType;
typedef clang::DeclRefExpr* cVar;
typedef clang::Stmt* cStmt;

template <SType>
class Statement
{
public:
  // Functions described in paper
  std::vector<cVar> vars();
  std::vector<cVar> defs();
  //
  std::vector<std::pair<cVar,cStmt> > lambda();
  std::vector<std::pair<cVar,cStmt> > u();
  std::vector<std::pair<cVar,cStmt> > p();

  /* Returns the statements which may affect
   * the value of cVar in parameter
   */
  std::vector<Statement> slice(cVar);
  // Auxilliary functions
  void addChild();

private:
  std::vector<Statement> children;
  cStmt astRef;
};


#endif // RELATIONS_H
