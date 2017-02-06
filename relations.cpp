#include <numeric>
#include "relations.h"
#include "relation_ops.h"

// common parts of the Statements
template<SType S>
std::set<cVar>
Statement<S>::getDefs() const{
  std::set<cVar> ret(defs);
  for (Statement s : children){
      auto tmp = s.getDefs();
      ret.insert(tmp.begin(),tmp.end());
    }
  return ret;
}

template<SType S>
std::set<std::pair<cVar,cVar> >
Statement<S>::id() const{
  std::set<std::pair<cVar,cVar> > ret;
  ret.insert({var,var});
  for (auto& v : evars)
    ret.insert({v,v});
  for (auto& s : children){
      auto tmp = s.id();
      ret.insert(tmp.begin(),tmp.end());
    }
  return ret;
}

// Statement-specific parts
// Assignment
template<>
std::set<cVar>
Statement<SType::Assign>::getDefs() const{
  return {var};
}

template<>
std::set<std::pair<cVar,cVar> >
Statement<SType::Assign>::p() const{
  auto tmp(id());
  tmp.erase(tmp.find({var,var}));
  return cart_prod(evars,var) + tmp;
}

template<>
std::set<std::pair<cVar,cStmt> >
Statement<SType::Assign>::lambda() const{
  return cart_prod(evars,expr);
}

template<>
std::set<std::pair<cStmt,cVar> >
Statement<SType::Assign>::u() const{
  return {{expr,var}};
}

// Compound
template<>
std::set<std::pair<cVar,cVar> >
Statement<SType::Compound>::p() const{
  std::set<std::pair<cVar,cVar> > ret;
  for (auto s : children)
    ret += s.p();
  return ret;
}
template<>
std::set<std::pair<cVar,cStmt> >
Statement<SType::Compound>::lambda() const{
  std::set<std::pair<cVar, cStmt> > ret;
  auto tmp = children[0].p();
  for (size_t i = 1;i < children.size();){
      tmp = rel_comp(tmp,children[i].p());
      ret += children[i].lambda();
      ++i;
      ret += rel_comp(tmp,children[i].lambda());
    }
  return ret;
}

template<>
std::set<std::pair<cStmt,cVar> >
Statement<SType::Compound>::u() const{
  std::set<std::pair<cStmt,cVar> > ret;
  ret = children[0].u();
  for (size_t i = 1;i<children.size();++i){
      rel_comp(ret,children[i].p());
      ret += children[i].u();
    }
  return ret;
}

// Branch
template<>
std::set<std::pair<cVar,cVar> >
Statement<SType::Branch>::p() const {
  return cart_prod(evars,children[0].getDefs())
         + children[0].p()
         + id();
}
template<>
std::set<std::pair<cVar,cStmt> >
Statement<SType::Branch>::lambda() const {
  return cart_prod(evars,expr)
         + children[0].lambda();
}
template<>
std::set<std::pair<cStmt,cVar> >
Statement<SType::Branch>::u() const {
  return cart_prod(expr,children[0].getDefs())
         + children[0].u();
}

// Branch_else
template<>
std::set<std::pair<cVar,cVar> >
Statement<SType::Branch_else>::p() const {
  return cart_prod(evars,
                   (children[0].getDefs() +
                   children[1].getDefs()))
         + children[0].p()
         + children[1].p();
}
template<>
std::set<std::pair<cVar,cStmt> >
Statement<SType::Branch_else>::lambda() const {
  return cart_prod(evars,expr)
         + children[0].lambda()
         + children[1].lambda();
}
template<>
std::set<std::pair<cStmt,cVar> >
Statement<SType::Branch_else>::u() const {
  return cart_prod(expr,
                   (children[0].getDefs()
                   +children[1].getDefs()))
         + children[0].u()
         + children[1].u();
}

// Loop
template<>
std::set<std::pair<cVar,cVar> >
Statement<SType::Loop>::p() const {
  return rel_comp(trans_clos(children[0].p()),
      ( cart_prod(evars,children[0].getDefs())
      + id()));
}
template<>
std::set<std::pair<cVar,cStmt> >
Statement<SType::Loop>::lambda() const {
  return rel_comp(trans_clos(children[0].p()),
      ( cart_prod(evars,expr)
      + children[0].lambda()));
}
template<>
std::set<std::pair<cStmt,cVar> >
Statement<SType::Loop>::u() const {
  auto a = children[0];
  return cart_prod(expr,a.getDefs())
       + rel_comp(
            rel_comp(a.u(),trans_clos(a.p()))
          , (cart_prod(evars,a.getDefs()) + id()));
}

