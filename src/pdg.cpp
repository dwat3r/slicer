#include "pdg.h"
#include "llvm/Support/Casting.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/LangOptions.h"

#include <queue>

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

void Statement::setDataEdges() {
	// initialize initial def_map from this node's defines
  std::map <const clang::ValueDecl*, std::pair<Statement*, Statement::Edge>> def_map;
  for (auto& def : define) {
    def_map.insert({ def,{this,Statement::Edge::None} });
  }
  setDataEdgesRec(def_map);
}

void Statement::setDataEdgesRec(std::map <const clang::ValueDecl*, 
	                                   std::pair<Statement*,Statement::Edge>> parent_def_map) {
  std::map <const clang::ValueDecl*, std::pair<Statement*,Statement::Edge>> def_map;
  // make every parent definition edge true.
  for (auto& d : parent_def_map) {
	  def_map.insert({ d.first, { d.second.first,Statement::Edge::None } });
  }
  // we have to bypass CompoundStatements and visit their childs. 
  std::set<std::pair<Statement*, Statement::Edge>, StatementLocCmp> visitingChildren;
  for (auto c : controlChildren) {
	  if (c.first->name() == Type::Compound) {
		  // inherit edge labels
		  for (auto& cc : c.first->getControlChildren()) {
			  visitingChildren.insert({ cc.first,c.second });
		  }
	  }
	  else {
		  visitingChildren.insert(c);
	  }
  }
  // create loop-carried dependences by 
  // visiting every child twice
  for (int i = 0; i < 2; ++i) {
    for (auto& stmt : visitingChildren) {
		  // def-def edges
		  for (auto& def : stmt.first->getDefine()) {
			  if (def_map.find(def) != def_map.end()) {
				  // if they're on the same branch
				  if (def_map[def].second == Statement::Edge::None ||
					  def_map[def].second == stmt.second) {
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
      for (auto& uses : stmt.first->getUses()) {
		  if (def_map.find(uses) == def_map.end()) {
			  llvm::errs() << "use " << uses->getNameAsString() 
				           << " not found in def_map, parent: " << std::to_string(id) 
				           << ", this: " << stmt.first->getId() << "\n";
			  assert(false);
		  }
        def_map[uses].first->addDataEdge(stmt.first);
      }

      // go down
      if (!stmt.first->controlChildren.empty()) {
        std::map <const clang::ValueDecl*, std::pair<Statement*, Statement::Edge>> child_def_map;
        if (name() == Type::Branch) {
          // erase defs from the other branch
          for (auto& def : def_map) {
			  if (def.second.second == Statement::Edge::None ||
				  def.second.second == stmt.second) {
				  child_def_map.insert(def);
			  }
          }
        }
        else {
          child_def_map.insert(def_map.begin(), def_map.end());
        }
        stmt.first->setDataEdgesRec(child_def_map);
      }
    }
	// create loop-carried dependences by 
	// visiting every child once again after
	// deleting local definitions (declStmts)
	if (name() == Type::Loop && i < 2) {
		for (auto it = def_map.begin(); it != def_map.end();)
		{
			if (llvm::isa<clang::DeclStmt>(it->second.first->getAstRef())
				&& it->second.second != Statement::Edge::None) def_map.erase(it++);
			else ++it;
		}
	}
	else break; // if we're not in a loop, don't visit twice
  }
}
// print out graph structure
std::string Statement::dump() {
  return dumpLevel(1);

}
std::string Statement::dumpLevel(int level) {
  std::string tab(level, ' ');
  std::string nodeId = ": id: " + std::to_string(id);
  std::string defs = ", def: ";
  for (auto& var : define) {
    defs += var->getNameAsString() + ", ";
  }
  std::string uses = ", use: ";
  for (auto& var : use) {
    uses += var->getNameAsString() + ", ";
  }
  std::string dataDeps = ", data deps: ";
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

std::string Statement::EdgeToStr(Statement::Edge e) {
  switch (e) {
  case Statement::Edge::None:  return "";
  case Statement::Edge::False: return "F";
  case Statement::Edge::True:  return "T";
  default: return "";
  }
}
std::string Statement::stmt2str(const clang::Stmt *s, clang::SourceManager &sm) {
  // (T, U) => "T,,"
  std::string text = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(s->getSourceRange()), 
    sm, clang::LangOptions(), 0);
  if (text.at(text.size() - 1) == ',')
    return clang::Lexer::getSourceText(clang::CharSourceRange::getCharRange(s->getSourceRange()), 
      sm, clang::LangOptions(), 0);
  return text;
}
std::string Statement::firstOnly(const clang::Stmt *s, const clang::Stmt *s2, clang::SourceManager &sm) {
  std::string first = stmt2str(s, sm);
  std::string second = stmt2str(s2, sm);
  assert(first.size() > second.size());
  assert(first.find(second) != std::string::npos);
    first = first.substr(0, first.find(second));
    return first;
}
std::string Statement::sourceString(clang::SourceManager &sm) {
  return stmt2str(astRef, sm);
}
std::string BranchStatement::sourceString(clang::SourceManager &sm) {
  return firstOnly(astRef, (*controlChildren.begin()).first->getAstRef(),sm);
}
std::string LoopStatement::sourceString(clang::SourceManager &sm) {
  return firstOnly(astRef, (*controlChildren.begin()).first->getAstRef(), sm);
}
std::string CompoundStatement::sourceString(clang::SourceManager &sm) {
  return "{}";
}

std::string Statement::dumpDot(clang::SourceManager &sm) {
  std::string ret = "digraph {\nrankdir=TD;\n";
  ret += dumpDotRec(sm) + "\n}";
  return ret;
}
std::string Statement::dumpDotRec(clang::SourceManager &sm) {
  std::string ranklist = "{ rank=same";
  std::string ret = std::to_string(id) + "[label=\"" + sourceString(sm) + "\"];\n";
  for (auto& c : controlChildren) {
    ret += std::to_string(id) + " -> " + std::to_string(c.first->getId())
      + "[label=\"" + EdgeToStr(c.second) + "\",style=bold];\n";
    if (!c.first->getControlChildren().empty()) {
      ret += c.first->dumpDotRec(sm);
    }
    else {
      ret += std::to_string(c.first->getId())
        + "[label=\"" + c.first->sourceString(sm) + "\"];\n";
      for (auto& e : c.first->getDataEdges()) {
        ret += std::to_string(c.first->getId()) + " -> " + std::to_string(e->getId()) + ";\n";
      }
    }
    ranklist += " " + std::to_string(c.first->getId());
  }
  ranklist += " }\n";
  ret += ranklist;
  for (auto& e : dataEdges) {
    ret += std::to_string(id) + " -> " + std::to_string(e->getId()) + ";\n";
  }
  return ret;
}

// s.l.i.c.e
void Statement::BFS(Statement* slicingStmt) {
/*
Breadth-First-Search(Graph, root):
    
  create empty set S
  create empty queue Q      

  root.parent = NIL
  add root to S
  Q.enqueue(root)                      

  while Q is not empty:
      current = Q.dequeue()
      if current is the goal:
          return current
      for each node n that is adjacent to current:
          if n is not in S:
              add n to S
              n.parent = current
              Q.enqueue(n)
*/
  std::set<Statement*> S;
  std::queue<Statement*> Q;
  S.insert(this);
  Statement* current = nullptr;
  Q.emplace(this);
  while (!Q.empty()) {
    current = Q.front(); Q.pop();
    if (current == slicingStmt) {
      // do the magic
    }
    else {
      for (auto& c : current->getControlChildren()) {
        if (S.find(c.first) == S.end()) {
          S.insert(c.first);
          c.first->parent = current;
          Q.emplace(c.first);
        }
      }
      for (auto& c : current->getDataEdges()) {
        if (S.find(c) == S.end()) {
          S.insert(c);
          c->parent = current;
          Q.emplace(c);
        }
      }
    }
  }

}