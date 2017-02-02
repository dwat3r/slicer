
#include "relations.h"

// common parts of the Statements
template<SType S>
std::vector<cVar>
Statement<S>::getDefs() const{
  std::vector<cVar> ret(defs);
  for (Statement s : children){
      ret.push_back(s.getDefs());
    }
  return ret;
}

// Statement-specific parts
// Assignment
template<>
std::vector<cVar>
Statement<SType::Assign>::getDefs() const{
  return {var};
}

template<>
std::vector<std::pair<cVar,cStmt> >
Statement<SType::Assign>::lambda(){
  return cart_prod(evars,expr);
}

template<>
std::vector<std::pair<cStmt,cVar> >
Statement<SType::Assign>::u(){
  return {{expr,var}};
}

template<>
std::vector<std::pair<cVar,cVar> >
Statement<SType::Assign>::p(){

  return cart_prod(evars,var);
}

// Compound
template<>
class Statement<SType::Compound>
{
public:
};
// Branch
template<>
class Statement<SType::Branch>
{
public:
};
// Loop
template<>
class Statement<SType::Loop>
{
public:
};
