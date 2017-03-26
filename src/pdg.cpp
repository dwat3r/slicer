#include "pdg.h"
#include "llvm/Support/Casting.h"

bool Statement::isControlReachableUpwards(Statement* source, Statement* dest)
{
	bool edgetype = source->getControlParent().second;
	// search in siblings
	for (auto& sibling : source->getControlParent().first->getControlChildren()) {
		if (edgetype == sibling.second &&
			sibling.first == dest) return true;
	}
	// search in parent's siblings, recursively
	if (source->getControlParent().first != nullptr)
		return isControlReachableUpwards(source->getControlParent().first, dest);
	else
		return false;
}

Statement* Statement::create(const clang::Stmt* astref)
{
  // Assignment
  if (auto bs = llvm::dyn_cast<clang::BinaryOperator>(astref)) {
    if (bs->isAssignmentOp()) {
      return new AssignStatement(astref);
    }
  }
  else if (llvm::isa<clang::DeclStmt>(astref)) {
    return new AssignStatement(astref);
  }
  else if (llvm::isa<clang::ReturnStmt>(astref)) {
    return new AssignStatement(astref);
  }
  // Branch
  else if (auto is = llvm::dyn_cast<clang::IfStmt>(astref)) {
      return new BranchStatement(astref);
  }
  // Compound
  else if (llvm::isa<clang::CompoundStmt>(astref)) {
    return new CompoundStatement(astref);
  }
  // Loop
  else if (llvm::isa<clang::WhileStmt>(astref)) {
    return new LoopStatement(astref);
  }
  return new Statement(astref);
}
std::string Statement::dump() {
  return dumpLevel(1);
}
std::string Statement::dumpLevel(int level) {
  std::string tab(level, ' ');
  std::string defs = define == nullptr ? "" : define->getNameAsString();
  std::string uses;
  for (auto& var : use) {
    uses += var->getNameAsString() + ", ";
  }
  std::string ret = name() + ": defines: " + defs + ", uses: " + uses + "\n";
  for (auto& child : controlChildren) {
    ret += tab + child.first->dumpLevel(level + 1);
  }
  return ret;
}