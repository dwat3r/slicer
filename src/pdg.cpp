#include "pdg.h"
#include "llvm/Support/Casting.h"

Statement* Statement::create(const clang::Stmt* astref,clang::FullSourceLoc loc)
{
  // Assignment
  if (auto bs = llvm::dyn_cast<clang::BinaryOperator>(astref)) {
    if (bs->isAssignmentOp()) {
      return new AssignStatement(astref,loc);
    }
  }
  else if (llvm::isa<clang::DeclStmt>(astref)) {
    return new AssignStatement(astref,loc);
  }
  else if (llvm::isa<clang::ReturnStmt>(astref)) {
    return new AssignStatement(astref,loc);
  }
  // Branch
  else if (auto is = llvm::dyn_cast<clang::IfStmt>(astref)) {
      return new BranchStatement(astref,loc);
  }
  // Compound
  else if (llvm::isa<clang::CompoundStmt>(astref)) {
    return new CompoundStatement(astref, loc);
  }
  // Loop
  else if (llvm::isa<clang::WhileStmt>(astref)) {
    return new LoopStatement(astref, loc);
  }
  return new Statement(astref, loc);
}
std::string Statement::dump() {
  return dumpLevel(1);

}
std::string Statement::dumpLevel(int level) {
  std::string tab(level, ' ');
  std::string nodeId = ": id: " + std::to_string(id);
  std::string defs = ", defines: " + (define == nullptr ? "" : define->getNameAsString());
  std::string uses = ", uses: ";
  for (auto& var : use) {
    uses += var->getNameAsString() + ", ";
  }
  std::string dataDeps = ", data dependence edges: ";
  for (auto& stmt : dataEdges) {
    dataDeps += std::to_string(stmt->getId()) + ", ";
  }
  std::string locs = ", loc: ("
    + std::to_string(loc.getSpellingLineNumber()) + ","
    + std::to_string(loc.getSpellingColumnNumber()) + ")";
  std::string ret = nameAsString() + nodeId + defs + uses + locs + dataDeps + "\n";
  for (auto& child : controlChildren) {
    ret += tab + child.first->dumpLevel(level + 1);
  }
  return ret;
}

void Statement::setDataEdges(std::map <const clang::ValueDecl*, std::pair<Statement*,bool>> parent_def_map) {
  std::map <const clang::ValueDecl*, std::pair<Statement*,bool>> def_map;
  // create loop-carried dependences by 
  // visiting every child twice
  // TODO MEGA TODO: IN WHILE THERE'S A COMPOUND SO WE're SCREWED UP MAJOR DANGER:
  // solution is to bypass CompoundStatements and visit their childs. 
  int passes = 1;
  if (name() == Type::Loop) { passes = 2; }
  for (int i = 0; i < passes; ++i) {
    
    for (auto& stmt : controlChildren) {
      auto def = stmt.first->getDefine();
      if (def != nullptr) {
        // def-def edges
        if (def_map.find(def) != def_map.end()) {
          // if they're on the same branch
          if (def_map[def].second == stmt.second) {
            def_map[def].first->addDataEdge(stmt.first);
          }
        }
        // make this stmt the latest definition
        def_map[def] = stmt;

        // while specific stuff
        if (name() == Type::Loop) {
          // if def, make backedge to predicate
          if (use.find(def) != use.end()) {
            def_map[def].first->addDataEdge(this);
          }
        }
      }
      // def-use edges
      for (auto& uses : use) {
        uses->dumpColor();
        def_map[uses].first->addDataEdge(stmt.first);
      }

      if (!stmt.first->controlChildren.empty()) {
        // go down
        std::map <const clang::ValueDecl*, std::pair<Statement*, bool>> child_def_map;
        if (name() == Type::Branch) {
          // erase defs from the other branch
          for (auto& def : def_map) {
            if (def.second.second == stmt.second)
              child_def_map.insert(def);
          }
        }
        else {
          child_def_map.insert(def_map.begin(), def_map.end());
        }
        stmt.first->setDataEdges(child_def_map);
      }
    }
    // create loop-carried dependences by 
    // visiting every child once again after
    // deleting local definitions (declStmts)
    for (auto it = def_map.begin(); it != def_map.end();)
    {
      if (!it->second.first->isLocalDefinition()) def_map.erase(it++);
      else ++it;
    }
  }
}