#include "relations_builder.h"

#include <llvm/Support/Casting.h>

/*!
 * The algorithm for building the relations:
 * We specialize Statement with dyn_cast
 * Then add general children.
 */
bool RelationsBuilder::VisitBinaryOperator(clang::BinaryOperator *Stmt){
  if(Stmt->isAssignmentOp()){
    statements[Stmt] = std::dynamic_pointer_cast<AssignStatement>(statements[Stmt]);
    clang::ValueDecl* var = nullptr;
    if(auto *varRef = llvm::dyn_cast<clang::DeclRefExpr>(Stmt->getLHS())){
        var = varRef->getDecl();
      }

    statements[Stmt]->fill(var,
                          Stmt->getRHS(),
                          vars(Stmt->getRHS()),
                          {var});
  }
  return true;
}

bool RelationsBuilder::VisitCompoundStmt(clang::CompoundStmt *Stmt){
  statements[Stmt] = std::dynamic_pointer_cast<CompoundStatement>(statements[Stmt]);

  for (clang::Stmt* s : Stmt->body()){
      auto act = std::make_shared<Statement>(s);
      statements[Stmt]->addChild(act);
      statements[s] = act;
    }

  return base::VisitCompoundStmt(Stmt);
}

bool RelationsBuilder::VisitIfStmt(clang::IfStmt *Stmt){
  if(Stmt->getElse() != nullptr){
      statements[Stmt] = std::dynamic_pointer_cast<BranchStatement>(statements[Stmt]);
    }
  else
    statements[Stmt] = std::dynamic_pointer_cast<Branch_elseStatement>(statements[Stmt]);

  statements[Stmt]->fill(nullptr,
                        Stmt->getCond(),
                        vars(Stmt->getCond()),
                        {});

  auto thenExpr = std::make_shared<Statement>(Stmt->getThen());
  statements[Stmt]->addChild(thenExpr);
  statements[Stmt->getThen()] = thenExpr;

  if(Stmt->getElse() != nullptr){
    auto elseExpr = std::make_shared<Statement>(Stmt->getElse());
    statements[Stmt]->addChild(elseExpr);
    statements[Stmt->getElse()] = elseExpr;
    }

  return true;
}

bool RelationsBuilder::VisitWhileStmt(clang::WhileStmt *Stmt){
  statements[Stmt] = std::dynamic_pointer_cast<LoopStatement>(statements[Stmt]);
  statements[Stmt]->fill(nullptr,
                        Stmt->getCond(),
                        vars(Stmt->getCond()),
                        {});
  auto bodyExpr = std::make_shared<Statement>(Stmt->getBody());
  statements[Stmt]->addChild(bodyExpr);
  statements[Stmt->getBody()] = bodyExpr;

  return true;
}

/*!
 * \brief Gets variables in relation expression recursively
 *        by going down to the DeclRefExpr
 * \param Stmt
 * \return Set of variables (DeclRefExprs)
 */
std::set<clang::ValueDecl*>
RelationsBuilder::vars(clang::Stmt* Stmt){
  // If we're at it, return
  if(auto *varRef = llvm::dyn_cast<clang::DeclRefExpr>(Stmt)){
      return {varRef->getDecl()};
    }
  // else go deeper
  std::set<clang::ValueDecl*> ret;
  if(auto *bo = llvm::dyn_cast<clang::BinaryOperator>(Stmt)){
      auto vs(vars(bo->getLHS()));
      ret.insert(vs.begin(),vs.end());
      vs = vars(bo->getRHS());
      ret.insert(vs.begin(),vs.end());
    }
  else if(auto *ic = llvm::dyn_cast<clang::ImplicitCastExpr>(Stmt)){
      auto vs(vars(ic->getSubExpr()));
      ret.insert(vs.begin(),vs.end());
    }

  return ret;
}
// We're controlling slicing from here
bool RelationsBuilder::TraverseFunctionDecl(clang::FunctionDecl *Decl){
  // Check if we're in the function
  if (Decl->getNameAsString() == funcName){
      base::TraverseStmt(Decl->getBody());
    }
  return true;
}

//! Slicing
std::set<clang::Stmt*> RelationsBuilder::computeSlice(){
  std::set<clang::Stmt*> ret;

  return ret;
}

//! misc

RelationsBuilderAction::RelationsBuilderAction(std::vector<std::string> params)
  : params(params)
{}

RelationsBuilderConsumer::RelationsBuilderConsumer(clang::ASTContext *Context,
                                  std::vector<std::string> params)
  : Visitor(Context,params)
{}
