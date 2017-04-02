#include "pdg.h"
#include "llvm/Support/Casting.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/LangOptions.h"
#include <iostream>
#include <queue>

// make graph bidirectional
void Statement::addControlChild(std::pair<Statement*, Edge> child) {
  controlChildren.insert(child);
  child.first->controlParents.insert({this, child.second});
}

void Statement::addDataEdge(Statement* s){
  dataEdges.insert(s);
  s->dataParents.insert(this);
}

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

// making the PDG
// todo refactor it to iterative
// use DFS with a (searchable) stack, and handle Loop, Branch specific operations differently
// maybe use the class design
// handle the nested whiles, ifs others
// for stack, use a: vector<Statement*,def_map> for registering level defs
// or just hack another variable into it
void Statement::setDataEdges() {
	// initialize initial def_map from this node's defines
  std::map <const clang::ValueDecl*, std::pair<Statement*, Statement::Edge>> def_map;
  for (auto& def : define) {
    def_map.insert({ def,{this,Statement::Edge::None} });
  }
  // we're using DFS for visiting every statement in execution order.
  setDataEdgesRec(def_map,nullptr);
}

std::map<const clang::ValueDecl*, std::pair<Statement*, Statement::Edge>>
Statement::setDataEdgesRec(std::map <const clang::ValueDecl*, 
	                                   std::pair<Statement*,Statement::Edge>> parent_def_map,
                           Statement* loopRef) {
  std::map <const clang::ValueDecl*, std::pair<Statement*, Statement::Edge>> def_map;
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
				  if ((def_map[def].second == Statement::Edge::None ||
					  def_map[def].second == stmt.second) && 
            // and they're not the same
            def_map[def].first != stmt.first) {
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
        // when in nested child
        else if (loopRef != nullptr) {
          auto uses = loopRef->getUses();
          if (uses.find(def) != uses.end()) {
            def_map[def].first->addDataEdge(loopRef);
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

        auto child_new_defs(stmt.first->setDataEdgesRec(child_def_map, 
                                                        name() == Type::Loop ? this : loopRef));
        // merge new definitions from child to our def_map
		for (auto& kv : child_new_defs) {
			def_map[kv.first] = kv.second;
		}
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
  // return our defs to caller parent, erasing local defs.
  for (auto it = def_map.begin(); it != def_map.end();)
  {
    if (llvm::isa<clang::DeclStmt>(it->second.first->getAstRef())
      && it->second.second != Statement::Edge::None) def_map.erase(it++);
    else ++it;
  }
  return def_map;
}
// print out graph structure
std::string Statement::dump() {
  return dumpLevel(1);

}
// todo make it nicer 
std::string Statement::dumpLevel(int level) {
  std::string tab(level, ' ');
  std::string nodeId = ": id: " + std::to_string(id);
  std::string defs = ", def: ";
  for (auto& var : define) {
    defs += ", " + var->getNameAsString();
  }
  std::string uses = ", use: ";
  for (auto& var : use) {
    uses += ", " + var->getNameAsString();
  }
  std::string dataDeps = ", data deps: ";
  for (auto& stmt : dataEdges) {
    dataDeps += ", " + std::to_string(stmt->getId());
  }
  std::string cparents = ", control parents: ";
  for (auto& p : controlParents) {
    cparents += ", " + std::to_string(p.first->getId());
  }
  std::string dparents = ", data parents: ";
  for (auto& p : dataParents) {
    dparents += ", " + std::to_string(p->getId());
  }
  std::string locs = ", loc: ("
    + std::to_string(loc.getSpellingLineNumber()) + ","
    + std::to_string(loc.getSpellingColumnNumber()) + ")";
  std::string ret = nameAsString() + nodeId + defs + uses + locs + dataDeps + cparents + dparents + "\n";
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

std::string Statement::dumpDot(clang::SourceManager &sm,bool markSliced) {
  std::string ret = "digraph {\nrankdir=TD;\n";
  std::map<int, std::vector<int>> rank_map;
  ret += dumpDotRec(sm, markSliced,rank_map,0);
  // insert ranks
  bool first = true;
  for (auto& kv : rank_map) {
    if (first) {
      ret += "{ rank=same ";
      first = false;
    }else
      ret += " -> { rank=same ";
    for (auto& i : kv.second) {
		  ret += std::to_string(i) + " ";
	  }
	  ret += "}";
  }

  ret += "\n}";
  return ret;
}
// todo make it nicer + make left-to-right ordering of nodes correct.
std::string Statement::dumpDotRec(clang::SourceManager &sm,bool markSliced,
	                              std::map<int, std::vector<int>>& rank_map,int depth) {
  std::string ret = std::to_string(id) + "[label=\"" + sourceString(sm) + "\"";
  if (markSliced && isInSlice()) { ret += ",color=red]; \n"; }
  else { ret += "]; \n"; }

  for (auto& c : controlChildren) {
    ret += std::to_string(id) + " -> " + std::to_string(c.first->getId())
      + "[label=\"" + EdgeToStr(c.second) + "\",style=bold";
    if (markSliced && isInSlice() && c.first->isInSlice()) { ret += ",color=red];\n"; }
    else { ret += "];\n"; }

    if (!c.first->getControlChildren().empty()) {
      ret += c.first->dumpDotRec(sm, markSliced,rank_map,depth+1);
    }
    else {
      // edge case of recursion
      ret += std::to_string(c.first->getId()) + "[label=\"" + c.first->sourceString(sm) + "\"";
      if (markSliced && c.first->isInSlice()) { ret += ",color=red];\n"; }
      else { ret += "];\n"; }

      for (auto& e : c.first->getDataEdges()) {
		  ret += std::to_string(c.first->getId()) + " -> " + std::to_string(e->getId());
        if (markSliced && c.first->isInSlice() && e->isInSlice()) { ret += "[color=red];\n"; }
        else { ret += ";\n"; }
      }
    }
    rank_map[depth].push_back(c.first->getId());
  }
  for (auto& e : dataEdges) {
    ret += std::to_string(id) + " -> " + std::to_string(e->getId());
	if (markSliced && isInSlice() && e->isInSlice()) { ret += "[color=red];\n"; }
	else { ret += ";\n"; }
  }
  return ret;
}

// s.l.i.c.e
// slicing sets a flag on the affected nodes so they can be visualized.
void Statement::slice(Statement* slicingStmt,bool backwards) {
  std::map<Statement*, Statement*> child;
  std::queue<Statement*> Q;
  std::set<Statement*> S;
  Q.emplace(slicingStmt);
  child[slicingStmt] = nullptr;
  Statement* current = slicingStmt;
  while (!Q.empty()) {
    current = Q.front(); Q.pop();
    std::set<Statement*> toVisit;
    if (backwards) {
      for (auto& e : current->getControlParents()){ toVisit.insert(e.first); }
      for (auto& e : current->getDataParents()) { toVisit.insert(e); }
    }
    else {
      for (auto& e : current->getControlChildren()) { toVisit.insert(e.first); }
      for (auto& e : current->getDataEdges()) { toVisit.insert(e); }
    }
    for (auto& node : toVisit) {
      if (S.find(node) == S.end()) {
        S.insert(node);
        child[node] = current;
        Q.emplace(node);
      }
    }
  }
  // mark edges
  for (auto& kv : child) {
    kv.first->markSliced();
    if (kv.second != nullptr)
      kv.second->markSliced();
  }
  // debug
  for (auto& kv : child) {
    if(kv.second != nullptr) std::cout << kv.first->getId() << " -> " << kv.second->getId() << "\n";
    else std::cout << kv.first->getId() << "\n";
  }
}

void Statement::resetSlice() {
  unmarkSliced();
  for (auto& c : controlChildren) {
    c.first->resetSlice();
  }
}