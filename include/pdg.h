#ifndef PDG_H
#define PDG_H

#include "clang/AST/AST.h"
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
  enum class Edge {
	  None,
	  False,
	  True,
  };
  std::string EdgeToStr(Edge e);

  struct StatementLocCmp {
    bool operator()(const std::pair<Statement*, Statement::Edge>& lhs, 
                    const std::pair<Statement*, Statement::Edge>& rhs) const {
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
              std::set<const clang::ValueDecl*> _define)
    : astRef(_astRef)
    , loc(_loc)
    , define(_define)
  {
    setId();
  }

  virtual ~Statement(){}

  // graph
  void addControlChild(std::pair<Statement*, Edge> child);
  void addDataEdge(Statement* s);

  // define/use
  void addUse(const clang::ValueDecl* _use) { use.insert(_use); }
  void addDefine(const clang::ValueDecl* _define) { define.insert(_define); }
  // location
  void setLocation(clang::FullSourceLoc _loc) { loc = _loc; }
  clang::FullSourceLoc getLocation() { return loc; }

  // getters
  std::set<std::pair<Statement*, Edge>, StatementLocCmp> getControlChildren() { return controlChildren; }
  std::set<std::pair<Statement*, Edge>, StatementLocCmp> getControlParents() { return controlParents; }
  std::set<Statement*> getDataEdges() { return dataEdges; }
  std::set<Statement*> getDataParents() { return dataParents; }
  std::set<const clang::ValueDecl*> getDefine() { return define; }
  std::set<const clang::ValueDecl*> getUses() { return use; }
  const clang::Stmt* getAstRef() { return astRef; }
  // returns if source has path to dest in graph.

  // name
  virtual Type name() { return Type::Base; }
  virtual std::string nameAsString() { return "Statement"; }
  virtual std::string sourceString(clang::SourceManager &sm);

  // factory method
  static Statement* create(const clang::Stmt* astref,clang::FullSourceLoc loc);
  
  // this draws in the data dependence edges to the graph
  // caller and initializer
  void setDataEdges();

  //s.l.i.c.e
  void slice(Statement* slicingStmt,bool backwards);
  void resetSlice();
  void markSliced() { inSlice = true; }
  void unmarkSliced() { inSlice = false; }
  bool isInSlice() { return inSlice; }

  int getId() { return id; }
  // print structure
  std::string dump();
  // graphviz output
  std::string dumpDot(clang::SourceManager &sm,bool markSliced);
protected:
  std::string dumpLevel(int level);
  std::string dumpDotRec(clang::SourceManager &sm,bool markSliced,
	                     std::map<int, std::vector<int>>& rank_map,int depth);
  // [(var,[(stmt,edge)]]
  typedef std::map<const clang::ValueDecl*, std::set<std::pair<Statement*, Statement::Edge>>> defsMap;
  defsMap
  setDataEdgesRec(defsMap parent_def_map,
                  std::vector<Statement*> loopRefs,
                  int inABranch);
  
  void setId() { static int _id = 0; id = _id++; }
  static std::string stmt2str(const clang::Stmt *s, clang::SourceManager &sm);
  static std::string firstOnly(const clang::Stmt *s, const clang::Stmt *s2, clang::SourceManager &sm);
  // graph
  std::set<std::pair<Statement*,Edge>, StatementLocCmp> controlChildren;
  std::set<Statement*> dataEdges;
  std::set < std::pair<Statement*, Edge>,StatementLocCmp> controlParents;
  std::set<Statement*> dataParents;

  // These store the variables that are defined / used in this statement
  std::set<const clang::ValueDecl*> define;
  std::set<const clang::ValueDecl*> use;
  
  clang::FullSourceLoc loc;
  int id;
  bool inSlice = false;
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
  virtual Type name() { return Type::Branch; }
  virtual std::string nameAsString() { return "Branch"; }
  virtual std::string sourceString(clang::SourceManager &sm);
};
// Loop and Branch almost the same, but they differ in the data dependence edge creations.
class LoopStatement : public Statement{
public:
  using Statement::Statement;
  virtual Type name() { return Type::Loop; }
  virtual std::string nameAsString() { return "Loop"; }
  virtual std::string sourceString(clang::SourceManager &sm);
};

class CompoundStatement : public Statement {
public:
  using Statement::Statement;
  virtual Type name() { return Type::Compound; }
  virtual std::string nameAsString() { return "Compound"; }
  virtual std::string sourceString(clang::SourceManager &sm);
};
#endif // PDG_H
