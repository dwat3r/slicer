#include "pdgBuilder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
/*!
 * The algorithm for building the relations:
 * We specialize Statement with dyn_cast
 * Then add general children.
 */
using namespace clang::ast_matchers;

namespace clang {
namespace slicer {
namespace {
AST_MATCHER(BinaryOperator, isAssignmentOp) {
  return Node.isAssignmentOp();
}
AST_MATCHER(UnaryOperator, isIncrementDecrementOp) {
  return Node.isIncrementDecrementOp();
}

}
void PDGBuilder::registerMatchers(MatchFinder *MatchFinder) {
  // first define matchers
  // assign
  auto decls = declStmt(hasDescendant(varDecl().bind("decl"))).bind("declStmt");
  auto declWithInit = declStmt(hasDescendant(
                                  varDecl(forEachDescendant(
                                    declRefExpr(to(varDecl().bind("declInit"))))).bind("declWithInit")
                              )).bind("declStmtWithInit");

  auto bop = binaryOperator(isAssignmentOp(),
                                hasLHS(ignoringImpCasts(declRefExpr(to(varDecl().bind("lval"))))),
                                hasRHS(forEachDescendant(
                                  expr(declRefExpr(to(varDecl().bind("rval"))))))).bind("binop");

  auto uop = unaryOperator(isIncrementDecrementOp(),
                           hasDescendant(declRefExpr(to(varDecl().bind("uval"))))).bind("unop");
  // branch
  //fixme it won't match for "if(0)"
  auto ifs = ifStmt(hasCondition(
    forEachDescendant(declRefExpr(to(varDecl().bind("ifCondVar")))))).bind("if");
  // loop
  auto whiles = whileStmt(hasCondition(
    forEachDescendant(declRefExpr(to(varDecl().bind("whileCondVar")))))).bind("while");
  // compound
  auto compounds = compoundStmt().bind("comp");

  // then add them to MatchFinder
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(decls)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(declWithInit)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(bop)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(uop)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(ifs)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(whiles)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(compounds)).bind("f"), this);
}
void PDGBuilder::run(const ast_matchers::MatchFinder::MatchResult &result) {
  // process the match results
  // ensure every entry added once
  if (auto ds = result.Nodes.getNodeAs<DeclStmt>("declStmt")) {
    //todo multiple decl...
    auto d = result.Nodes.getNodeAs<VarDecl>("decl");
    if (!hasStmt(ds)) {
      stmt_map[ds] = new AssignStatement(ds, d);
    }
    else {
      stmt_map[ds]->setDefine(d);
    }
  }
  if (auto ds = result.Nodes.getNodeAs<DeclStmt>("declStmtWithInit")) {
    //todo multiple decl...
    auto d = result.Nodes.getNodeAs<VarDecl>("declWithInit");
    if (!hasStmt(ds)) {
      stmt_map[ds] = new AssignStatement(ds, d);
    }
    else {
      stmt_map[ds]->setDefine(d);
    }
    stmt_map[ds]->addUse(result.Nodes.getNodeAs<VarDecl>("declInit"));
  }
  if (auto bo = result.Nodes.getNodeAs<BinaryOperator>("binop")) {
    auto lhs = result.Nodes.getNodeAs<VarDecl>("lval");
    if (!hasStmt(bo)) {
      stmt_map[bo] = new AssignStatement(bo, lhs);
    }
    else {
      stmt_map[bo]->setDefine(lhs);
    }
      stmt_map[bo]->addUse(result.Nodes.getNodeAs<VarDecl>("rval"));
  }
  if (auto uo = result.Nodes.getNodeAs<UnaryOperator>("unop")) {
    auto var = result.Nodes.getNodeAs<VarDecl>("uval");
    if (!hasStmt(uo)) {
      stmt_map[uo] = new AssignStatement(uo, var);
    }
    else {
      stmt_map[uo]->setDefine(var);
    }
  }
  if (auto is = result.Nodes.getNodeAs<IfStmt>("if")) {
    if (!hasStmt(is)) {
      stmt_map[is->getThen()] = Statement::create(is->getThen());
      stmt_map[is->getElse()] = Statement::create(is->getElse());
      stmt_map[is] = new BranchStatement(is, { {stmt_map[is->getThen()],true},
                                               {stmt_map[is->getElse()],false} });
    }
      stmt_map[is]->addUse(result.Nodes.getNodeAs<VarDecl>("ifCondVar"));

  }
  if (auto ws = result.Nodes.getNodeAs<WhileStmt>("while")) {
    if (!hasStmt(ws)) {
      stmt_map[ws->getBody()] = Statement::create(ws->getBody());
      stmt_map[ws] = new LoopStatement(ws, { {stmt_map[ws->getBody()],true} });
    }
      stmt_map[ws]->addUse(result.Nodes.getNodeAs<VarDecl>("whileCondVar"));
  }
  if (auto cs = result.Nodes.getNodeAs<CompoundStmt>("comp")) {
    if (!hasStmt(cs)) {
      stmt_map[cs] = new CompoundStatement(cs);
    }
    if (stmt_map[cs]->getControlChildren().empty()) {
      for (auto c : cs->children()) {
        if (stmt_map.find(c) == stmt_map.end()) {
          stmt_map[c] = Statement::create(c);
        }
        stmt_map[cs]->addControlChild({ stmt_map[c],true });
      }
    }
  }
  if (auto f = result.Nodes.getNodeAs<FunctionDecl>("f")) {
    //root
    root = f->getBody();
    if (!hasStmt(root)) {
      stmt_map[root] = new CompoundStatement(root);
    }
  }
}

void PDGBuilder::onEndOfTranslationUnit() {
  // slice here
  // first todo: dump out created Statement tree, to see what we've done.
  llvm::errs() << stmt_map[root]->dump();

}
} // namespace slicer
} // namespace clang