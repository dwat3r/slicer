#include "relations_builder.h"
#include <exception>
#include <llvm/Support/Casting.h>

/*!
 * The algorithm for building the relations:
 * We specialize Statement with dyn_cast
 * Then add general children.
 */

bool RelationsBuilder::VisitReturnStmt(clang::ReturnStmt *Stmt) {
  if (auto ic = llvm::dyn_cast<clang::ImplicitCastExpr>(Stmt->getRetValue())) {
    if (auto de = llvm::dyn_cast<clang::DeclRefExpr>(ic)) {
      statements[Stmt]->fill(de->getDecl(),
        de,
        vars(de),
        { de->getDecl() });
    }
  }
  return true;
}
bool RelationsBuilder::VisitDeclStmt(clang::DeclStmt *Stmt) {
  if (Stmt->isSingleDecl()) {
    if (auto vd = llvm::dyn_cast<clang::VarDecl>(Stmt->getSingleDecl())) {
      statements[Stmt]->fill(vd,
        vd->getInit(),
        vars(vd->getInit()),
        { vd });
    }
  }
  return true;
}
bool RelationsBuilder::VisitBinaryOperator(clang::BinaryOperator *Stmt) {
  if (Stmt->isAssignmentOp()) {
    if (auto varRef = llvm::dyn_cast<clang::DeclRefExpr>(Stmt->getLHS())) {
      auto var = varRef->getDecl();
      statements[Stmt]->fill(var,
        Stmt->getRHS(),
        vars(Stmt->getRHS()),
        { var });
    }
  }
  return true;
}

bool RelationsBuilder::VisitCompoundStmt(clang::CompoundStmt *Stmt) {
  for (clang::Stmt* s : Stmt->body()) {
    auto act = Statement::create(s);
    statements[Stmt]->addChild(act);
    statements[s] = act;
  }

  return true;
}

bool RelationsBuilder::VisitIfStmt(clang::IfStmt *Stmt) {
  statements[Stmt]->fill(nullptr,
    Stmt->getCond(),
    vars(Stmt->getCond()),
    {});

  auto thenExpr = Statement::create(Stmt->getThen());
  statements[Stmt]->addChild(thenExpr);
  statements[Stmt->getThen()] = thenExpr;

  if (Stmt->getElse() != nullptr) {
    auto elseExpr = Statement::create(Stmt->getElse());
    statements[Stmt]->addChild(elseExpr);
    statements[Stmt->getElse()] = elseExpr;
  }

  return true;
}

bool RelationsBuilder::VisitWhileStmt(clang::WhileStmt *Stmt) {
  statements[Stmt]->fill(nullptr,
    Stmt->getCond(),
    vars(Stmt->getCond()),
    {});

  auto bodyExpr = Statement::create(Stmt->getBody());
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
RelationsBuilder::vars(clang::Stmt* Stmt) {
  // If we're at it, return
  if (auto *varRef = llvm::dyn_cast<clang::DeclRefExpr>(Stmt)) {
    return { varRef->getDecl() };
  }
  else if (auto ds = llvm::dyn_cast<clang::DeclStmt>(Stmt)) {
    if (auto vd = llvm::dyn_cast<clang::VarDecl>(ds->getSingleDecl())) {
      return{ vd };
    }
  }
  // else go deeper
  std::set<clang::ValueDecl*> ret;
  if (auto *bo = llvm::dyn_cast<clang::BinaryOperator>(Stmt)) {
    auto vs(vars(bo->getLHS()));
    ret.insert(vs.begin(), vs.end());
    vs = vars(bo->getRHS());
    ret.insert(vs.begin(), vs.end());
  }
  else if (auto *ic = llvm::dyn_cast<clang::ImplicitCastExpr>(Stmt)) {
    auto vs(vars(ic->getSubExpr()));
    ret.insert(vs.begin(), vs.end());
  }

  return ret;
}
bool RelationsBuilder::VisitDeclRefExpr(clang::DeclRefExpr *Stmt) {
  clang::FullSourceLoc loc = Context->getFullLoc(Stmt->getLocStart());
  //std::cout << Stmt->getDecl()->getNameAsString() << ": " << loc.getSpellingLineNumber() << ", " << loc.getSpellingColumnNumber() << std::endl;
  if (loc.getSpellingLineNumber() == row &&
    loc.getSpellingColumnNumber() == column) {
    var = Stmt->getDecl();
  }
  return true;
}

// We're controlling slicing from here
bool RelationsBuilder::TraverseFunctionDecl(clang::FunctionDecl *Decl) {
  // Check if we're in the function
  if (Decl->getNameAsString() == funcName) {
    auto root = Decl->getBody();
    root->dumpColor();
    statements[root] = Statement::create(root);
    
    base::TraverseStmt(root);

    auto stmts = statements[root]->slice(var);
    
    std::cout << "Slice:\n";
    for (auto stmt : stmts) {
      stmt->dumpColor();
    }
  }
  return true;
}

//! misc

RelationsBuilderAction::RelationsBuilderAction(std::vector<std::string> params)
  : params(params)
{}

RelationsBuilderConsumer::RelationsBuilderConsumer(clang::ASTContext *Context,
  std::vector<std::string> params)
  : Visitor(Context, params)
{}
