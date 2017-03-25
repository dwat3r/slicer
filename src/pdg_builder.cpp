#include "pdg_builder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include <memory>
/*!
 * The algorithm for building the relations:
 * We specialize Statement with dyn_cast
 * Then add general children.
 */
using namespace clang::ast_matchers;

namespace clang {

AST_MATCHER(BinaryOperator, isAssignmentOp) {
  return Node.isAssignmentOp();
}

void PDGBuilder::registerMatchers(MatchFinder *MatchFinder) {
  // first define matchers
  // assign
  auto decls = declStmt(hasDescendant(varDecl().bind("decl"))).bind("declStmt");
  auto assigns = binaryOperator(isAssignmentOp(),
	  hasLHS(ignoringImpCasts(declRefExpr(to(varDecl().bind("lval"))))),
	  hasRHS(ignoringImpCasts(
		  forEachDescendant(expr(declRefExpr().bind("rval")))))).bind("binop");

  // branch
  auto ifs = ifStmt(hasCondition(
    forEachDescendant(declRefExpr(to(varDecl().bind("ifCondVar")))))).bind("if");
  // loop
  auto whiles = whileStmt().bind("while");
  // compound
  auto compounds = compoundStmt().bind("comp");
  // the function
  auto function = functionDecl(hasName(funcName),
                               forEachDescendant(expr(anyOf(whiles, 
                                                            decls, 
                                                            ifs, 
                                                            assigns,
                                                            compounds)))).bind("f");

  // then add them to MatchFinder
  MatchFinder->addMatcher(function, this);
}
void PDGBuilder::run(const ast_matchers::MatchFinder::MatchResult &result) {
  // process the match results
  // todo find out how matching works, then ensure every entry added once
  if (auto ds = result.Nodes.getNodeAs<DeclStmt>("declStmt")) {
    //todo multiple decl...
    auto d = result.Nodes.getNodeAs<VarDecl>("decl");
    if (stmt_map.find(ds) == stmt_map.end()) {
      stmt_map[ds] = new AssignStatement(ds, d);
      def_map[d].insert(stmt_map[ds]);
    }
    else {
      stmt_map[ds]->setDefine(d);
    }
  }
  else if (auto bo = result.Nodes.getNodeAs<BinaryOperator>("binop")) {
    auto lhs = result.Nodes.getNodeAs<VarDecl>("lval");
    if (stmt_map.find(bo) == stmt_map.end()) {
      stmt_map[bo] = new AssignStatement(bo, lhs);
      def_map[lhs].insert(stmt_map[bo]);
    }
    else {
      stmt_map[bo]->setDefine(lhs);
    }
      stmt_map[bo]->addUse(result.Nodes.getNodeAs<VarDecl>("rval"));
  }
  else if (auto is = result.Nodes.getNodeAs<IfStmt>("if")) {
    if (stmt_map.find(is) == stmt_map.end()) {
      stmt_map[is] = new BranchStatement(is, { {Statement::create(is->getThen()),true},
                                               {Statement::create(is->getElse()),false} });
    }
    else {
      stmt_map[is]->addUse(result.Nodes.getNodeAs<VarDecl>("ifCondVar"));
    }
  }
  else if (auto ws = result.Nodes.getNodeAs<WhileStmt>("while")){
    if (stmt_map.find(ws) == stmt_map.end()) {
      stmt_map[ws] = new LoopStatement(ws,{ {Statement::create(ws->getBody()),true} });
    }
  }
  else if (auto cs = result.Nodes.getNodeAs<CompoundStmt>("compound")) {
    if (stmt_map.find(cs) == stmt_map.end()) {
      stmt_map[cs] = new CompoundStatement(cs);
      for (auto c : cs->children()) {
        stmt_map[cs]->addControlChild({ Statement::create(c),true });
      }
    }
  }
  else if (auto f = result.Nodes.getNodeAs<FunctionDecl>("f")) {
    //root
    if (stmt_map.find(f->getBody()) == stmt_map.end()) {
      stmt_map[f->getBody()] = new CompoundStatement(f->getBody());
    }
  }
}

void PDGBuilder::onEndOfTranslationUnit() {
  // slice here
  int i = 0;
}

} // namespace clang