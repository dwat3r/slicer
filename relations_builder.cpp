#include "relations_builder.h"

#include <llvm/Support/Casting.h>

#include <memory>


/*!
 * The algorithm for building the relations:
 * We specialize Statement with dyn_cast
 * Then add general children.
 */

bool RelationsBuilder::VisitGotoStmt(clang::GotoStmt *Stmt){
  return true;
}

bool RelationsBuilder::VisitLabelStmt(clang::LabelStmt *Stmt){
  return true;
}

bool RelationsBuilder::VisitCompoundStmt(clang::CompoundStmt *Stmt){

  statements[Stmt] = std::dynamic_pointer_cast<CompoundStatement>(statements[Stmt]);

  for (auto& s : Stmt->body()){
      auto act = std::make_shared<Statement>(new Statement(s));
      statements[Stmt]->addChild(act);
      statements[s] = act;
    }
  return base::VisitCompoundStmt(Stmt);
}

bool RelationsBuilder::VisitWhileStmt(clang::WhileStmt *Stmt){
  statements[Stmt] = std::dynamic_pointer_cast<LoopStatement>(statements[Stmt]);



  return base::VisitWhileStmt(Stmt);
}

bool RelationsBuilder::VisitIfStmt(clang::IfStmt *Stmt){
  return true;
}
/*!
 * \brief Gets variables in relation expression recursively
 *        by going down to the DeclRefExpr
 * \param Stmt
 * \return Set of variables (DeclRefExprs)
 */
std::set<clang::DeclRefExpr*>
RelationsBuilder::vars(clang::Stmt* Stmt){
  // If we're at it, return
  if(clang::DeclRefExpr* var = llvm::dyn_cast<clang::DeclRefExpr>(Stmt)){
      return {var};
    }
  // else go deeper
  std::set<clang::DeclRefExpr*> ret;
  if (clang::WhileStmt *ws = llvm::dyn_cast<clang::WhileStmt>(Stmt)){
      auto vs(vars(ws->getCond()));
      ret.insert(vs.begin(),vs.end());
    }
  else if(clang::IfStmt *is = llvm::dyn_cast<clang::IfStmt>(Stmt)){
      auto vs(vars(is->getCond()));
      ret.insert(vs.begin(),vs.end());
    }
  else if(clang::BinaryOperator *bo = llvm::dyn_cast<clang::BinaryOperator>(Stmt)){
      auto vs(vars(bo->getLHS()));
      ret.insert(vs.begin(),vs.end());
      vs = vars(bo->getRHS());
      ret.insert(vs.begin(),vs.end());
    }
  else if(clang::ImplicitCastExpr *ic = llvm::dyn_cast<clang::ImplicitCastExpr>(Stmt)){
      auto vs(vars(ic->getSubExpr()));
      ret.insert(vs.begin(),vs.end());
    }

  return ret;
}

std::set<clang::DeclRefExpr*>
RelationsBuilder::defs(clang::Stmt* Stmt){
  std::set<clang::DeclRefExpr*> ret;
}
