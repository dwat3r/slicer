#include <numeric>
#include <memory>
#include <algorithm>
#include <llvm/Support/Casting.h>
#include "relations.h"
#include "relation_ops.h"
#include <iostream>

// common parts of the Statements
Statement::Statement()
  : var()
  , expr()
  , evars()
  , defs()
  , children()
{}

std::set<std::pair<cVar,cStmt>> Statement::lambda() const {return {};}
std::set<std::pair<cStmt,cVar>> Statement::u() const      {return {};}
std::set<std::pair<cVar,cVar>>  Statement::p() const      {return {};}

std::set<cVar>
Statement::getDefs() const{
  std::set<cVar> ret(defs);
  for (auto& s : children){
      auto tmp = s->getDefs();
      ret.insert(tmp.begin(),tmp.end());
    }
  return ret;
}

std::set<std::pair<cVar,cVar>>
Statement::id() const{
  std::set<std::pair<cVar,cVar>> ret;
  ret.insert({var,var});
  for (auto& v : evars)
    ret.insert({v,v});
  for (auto& s : children){
      auto tmp = s->id();
      ret.insert(tmp.begin(),tmp.end());
    }
  return ret;
}

std::set<cStmt>
Statement::slice(cVar var){
  auto evs(u());
  std::set<cStmt> ret;
  std::cout << "we're slicing:\n";
  std::cout << "want to check if this:\n";
  var->dump();
  for (auto ev : evs){
    std::cout << "matches this:\n";
    ev.second->dumpColor();
    
      if(ev.second == var)
        ret.insert(ev.first);
    }
  return ret;
}

void Statement::addChild(std::shared_ptr<Statement> c){
  children.push_back(c);
}

std::shared_ptr<Statement> Statement::create(cStmt astref)
{
  // Assignment
  if (auto bs = llvm::dyn_cast<clang::BinaryOperator>(astref)) {
    if (bs->isAssignmentOp()) {
      return std::make_shared<AssignStatement>(astref);
    }
  }
  else if (llvm::isa<clang::DeclStmt>(astref)) {
    return std::make_shared<AssignStatement>(astref);
  }
  else if (llvm::isa<clang::ReturnStmt>(astref)) {
    return std::make_shared <AssignStatement>(astref);
  }
  // Branch
  else if (auto is = llvm::dyn_cast<clang::IfStmt>(astref)) {
    if (is->getElse() == nullptr) {
      return std::make_shared<BranchStatement>(astref);
    }
    else {
      return std::make_shared<Branch_elseStatement>(astref);
    }
  }
  // Compound
  else if (llvm::isa<clang::CompoundStmt>(astref)) {
    return std::make_shared<CompoundStatement>(astref);
  }
  // Loop
  else if (llvm::isa<clang::WhileStmt>(astref)) {
    return std::make_shared<LoopStatement>(astref);
  }
  return std::make_shared<Statement>(astref);
}

// Statement-specific parts
// Assignment

std::set<cVar>
AssignStatement::getDefs() const{
  return {var};
}

std::set<std::pair<cVar,cVar>>
AssignStatement::p() const{
  auto tmp(id());
  tmp.erase(tmp.find({var,var}));
  return cart_prod(evars,var) + tmp;
}


std::set<std::pair<cVar,cStmt>>
AssignStatement::lambda() const{
  return cart_prod(evars,expr);
}


std::set<std::pair<cStmt,cVar>>
AssignStatement::u() const{
  return {{expr,var}};
}

// Compound

std::set<std::pair<cVar,cVar>>
CompoundStatement::p() const{
  std::set<std::pair<cVar,cVar>> ret;
  for (auto& s : children)
    ret += s->p();
  return ret;
}
std::set<std::pair<cVar,cStmt>>
CompoundStatement::lambda() const{
  std::set<std::pair<cVar, cStmt>> ret;
  auto tmp = children[0]->p();
  for (size_t i = 1;i < children.size();){
      tmp = rel_comp(tmp,children[i]->p());
      ret += children[i]->lambda();
      ++i;
      ret += rel_comp(tmp,children[i]->lambda());
    }
  return ret;
}


std::set<std::pair<cStmt,cVar>>
CompoundStatement::u() const{
  std::set<std::pair<cStmt,cVar>> ret;
  ret = children[0]->u();
  for (size_t i = 1;i<children.size();++i){
      rel_comp(ret,children[i]->p());
      ret += children[i]->u();
    }
  return ret;
}
// Branch
std::set<std::pair<cVar,cVar>>
BranchStatement::p() const {
  return cart_prod(evars,children[0]->getDefs())
      + children[0]->p()
      + id();
}

std::set<std::pair<cVar,cStmt>>
BranchStatement::lambda() const {
  return cart_prod(evars,expr)
      + children[0]->lambda();
}

std::set<std::pair<cStmt,cVar>>
BranchStatement::u() const {
  return cart_prod(expr,children[0]->getDefs())
      + children[0]->u();
}
// Branch_else
std::set<std::pair<cVar,cVar>>
Branch_elseStatement::p() const {
  return cart_prod(evars,
                   (children[0]->getDefs() +
                   children[1]->getDefs()))
      + children[0]->p()
      + children[1]->p();
}

std::set<std::pair<cVar,cStmt>>
Branch_elseStatement::lambda() const {
  return cart_prod(evars,expr)
      + children[0]->lambda()
      + children[1]->lambda();
}

std::set<std::pair<cStmt,cVar>>
Branch_elseStatement::u() const {
  return cart_prod(expr,
                   (children[0]->getDefs()
                   +children[1]->getDefs()))
      + children[0]->u()
      + children[1]->u();
}
// Loop
std::set<std::pair<cVar,cVar>>
LoopStatement::p() const {
  return rel_comp(trans_clos(children[0]->p()),
      ( cart_prod(evars,children[0]->getDefs())
      + id()));
}

std::set<std::pair<cVar,cStmt>>
LoopStatement::lambda() const {
  return rel_comp(trans_clos(children[0]->p()),
      ( cart_prod(evars,expr)
        + children[0]->lambda()));
}

std::set<std::pair<cStmt,cVar>>
LoopStatement::u() const {
  auto a = *(children[0]);
  return cart_prod(expr,a.getDefs())
      + rel_comp(
        rel_comp(a.u(),trans_clos(a.p()))
        , (cart_prod(evars,a.getDefs()) + id()));
}
