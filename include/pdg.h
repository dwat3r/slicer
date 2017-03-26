#ifndef PDG_H
#define PDG_H

#include "clang/AST/AST.h"
#include "llvm/Support/Casting.h"
#include <set>

/*!
 * Program Dependence Graph
 * Node information stored in Statements
 * specifics are in the specializations
 */

class Statement
{
public:
  enum class Type {
    Base,
    Assign,
    Branch,
    Loop,
    Compound
  };

  struct StatementLocCmp {
    bool operator()(const std::pair<Statement*, bool>& lhs, 
                    const std::pair<Statement*, bool>& rhs) const {
      return lhs.first->loc.isBeforeInTranslationUnitThan(rhs.first->loc);
    }
  };
  explicit
    Statement(const clang::Stmt* _astRef, 
              clang::FullSourceLoc _loc)
    : astRef(_astRef)
    , loc(_loc)
  {
    setId();
  }

  explicit
  Statement(const clang::Stmt* _astRef,
            clang::FullSourceLoc _loc,
            std::set<std::pair<Statement*,bool>, StatementLocCmp> _cChildren)
    : astRef(_astRef)
    , loc(_loc)
    , controlChildren(_cChildren)
  {
    setId();
  }

  explicit
    Statement(const clang::Stmt* _astRef,
              clang::FullSourceLoc _loc,
              const clang::ValueDecl* _define)
    : astRef(_astRef)
    , loc(_loc)
    , define(_define)
  {
    setId();
  }

  virtual ~Statement(){}

  // Edge adders
  void addControlChild(std::pair<Statement*,bool> edge) { controlChildren.insert(edge); }
  void addDataEdge(Statement* s) { dataEdges.insert(s); }

  // define/use
  void addUse(const clang::ValueDecl* _use) { use.insert(_use); }
  void setDefine(const clang::ValueDecl* _define) { define = _define; }
  // location
  void setLocation(clang::FullSourceLoc _loc) { loc = _loc; }
  clang::FullSourceLoc getLocation() { return loc; }

  // getters
  std::set<std::pair<Statement*, bool>, StatementLocCmp> getControlChildren() { return controlChildren; }
  std::set<Statement*> getDataEdges() { return dataEdges; }
  const clang::ValueDecl* getDefine() { return define; }
  std::set<const clang::ValueDecl*> getUses() { return use; }
  // returns if source has path to dest in graph.

  // name
  virtual Type name() { return Type::Base; }
  virtual std::string nameAsString() { return "Statement"; }
  // returns true if branch
  virtual bool isBranchStatement() { return false; }
  
  bool isLocalDefinition() const { return llvm::isa<clang::DeclStmt>(astRef); }
  // factory method
  static Statement* create(const clang::Stmt* astref,clang::FullSourceLoc loc);
  // print structure
  std::string dump();
  // this draws in the data dependence edges to the graph
  void setDataEdges(std::map <const clang::ValueDecl*, std::pair<Statement*,bool>> parent_def_map);

  int getId() { return id; }
protected:
  std::string dumpLevel(int level);
  void setId() { static int _id = 0; id = _id++; }
  
  std::set<std::pair<Statement*,bool>, StatementLocCmp> controlChildren;
  std::set<Statement*> dataEdges;
  // These store the variables that are defined / used in this statement
  const clang::ValueDecl* define = nullptr;
  std::set<const clang::ValueDecl*> use;
  
  clang::FullSourceLoc loc;
  int id;

  // Store a reference to the AST
  const clang::Stmt* astRef = nullptr;
};

// Specializations
class AssignStatement : public Statement{
public:
  using Statement::Statement;
  virtual Type name() { return Type::Assign; }
  virtual std::string nameAsString() { return "Assign"; }
};

class BranchStatement : public Statement{
public:
  using Statement::Statement;
  virtual bool isBranchStatement() override { return true; } 
  virtual Type name() { return Type::Branch; }
  virtual std::string nameAsString() { return "Branch"; }
};
// Loop and Branch almost the same, but they differ in the data dependence edge creations.
class LoopStatement : public Statement{
public:
  using Statement::Statement;
  virtual Type name() { return Type::Loop; }
  virtual std::string nameAsString() { return "Loop"; }
};

class CompoundStatement : public Statement {
public:
  using Statement::Statement;
  virtual Type name() { return Type::Compound; }
  virtual std::string nameAsString() { return "Compound"; }
};
#endif // PDG_H
