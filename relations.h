#ifndef RELATIONS_H
#define RELATIONS_H

#include "clang/AST/AST.h"
#include <vector>
#include <algorithm>

/*
 * Information-flow relations:
 * There are Statements
 * parametrized on type SType
 *
 * Every function solves it's task recursively.
 */

enum class SType {Assign, Compound, Branch, Loop};

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
            std::vector<cVar> evars,std::vector<cVar> defs)
    : var(var)
    , expr(expr)
    , evars(evars)
    , defs(defs)
  {}
  /* Returns definitions used in the Statement,
   * recursively.
   */
  std::vector<cVar> getDefs() const;
  // Returns variables used in expr.
  std::vector<cVar> getEVars() const {return evars;}
  //
  std::vector<std::pair<cVar,cStmt> > lambda();
  std::vector<std::pair<cStmt,cVar> > u();
  std::vector<std::pair<cVar,cVar> >  p();

  /* Returns the statements which may affect
   * the value of cVar in parameter
   * recursively
   */
  std::vector<Statement> slice(cVar);
  // Auxilliary functions
  void addChild();

private:
  // sub-statements
  std::vector<Statement> children;
  // Variable and expression exclusive to this Statement.
  cVar  var;
  cStmt expr;
  // evars contain the variables occuring in expr.
  std::vector<cVar> evars;
  std::vector<cVar> defs;
  // Store a reference to the AST
  cStmt astRef;
};

// Binary operations
// Cartesian product for
// N x N
template<typename LHS,typename RHS>
inline
std::vector<std::pair <LHS,RHS> >
cart_prod(const std::vector<LHS> lhs,
          const std::vector<RHS> rhs){
  std::vector<std::pair <LHS,RHS> > ret;
  for (auto l : lhs){
      for (auto r : rhs){
          ret.push_back({l,r});
        }
    }
  return ret;
}
// 1 x N
template<typename LHS,typename RHS>
inline
std::vector<std::pair<LHS,RHS> >
cart_prod(const LHS lhs,
          const std::vector<RHS> rhs){
  std::vector<std::pair <LHS,RHS> > ret;
  for (auto r : rhs){
      ret.push_back({lhs,r});
    }
  return ret;
}
// N x 1
template<typename LHS,typename RHS>
inline std::vector<std::pair<LHS,RHS> >
cart_prod(const std::vector<LHS> lhs,
          const RHS rhs){
  std::vector<std::pair <LHS,RHS> > ret;
  for (auto l : lhs){
      ret.push_back({l,rhs});
    }
  return ret;
}

// relation composition
template<typename LHS,typename MID,typename RHS>
inline std::vector<std::pair <LHS,RHS> >
rel_comp(const std::vector<std::pair<LHS,MID> > lhs,
         const std::vector<std::pair<MID,RHS> > rhs){
  std::vector<std::pair <LHS,RHS> > ret;
  for (auto l : lhs){
      for (auto r : rhs){
          if(l.second == r.first)
            ret.push_back({l.first,r.second});
        }
    }
  return ret;
}

// union
// took from:
// http://stackoverflow.com/questions/22383474/c-lazy-way-to-find-union-of-stdsets
// TODO: rewrite vectors to sets.
template <typename T>
inline std::set<T>& operator+=(std::set<T>& lhs, const std::set<T>& rhs){
    lhs.insert(begin(rhs), end(rhs));
    return lhs;
}

template <typename T>
inline std::set<T> operator+(std::set<T> lhs, const std::set<T>& rhs){
    lhs += rhs;
    return lhs;
}

#endif // RELATIONS_H
