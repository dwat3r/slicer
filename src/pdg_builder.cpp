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

void PDGBuilder::registerMatchers(MatchFinder *MatchFinder) {
  // first define matchers
  // assign
  auto decls = declStmt(hasDescendant(varDecl().bind("decl"))).bind("declStmt");
  auto assigns = binaryOperator(hasOperatorName("="),
                                hasOperatorName("+="),
                                hasOperatorName("-="),
                                hasOperatorName("*="),
                                hasOperatorName("/="),
                                hasOperatorName("%="),
                                hasOperatorName("<<="), 
                                hasOperatorName(">>="), 
                                hasOperatorName("&="),
                                hasOperatorName("^="),
                                hasOperatorName("|="),
	  hasLHS(ignoringImpCasts(declRefExpr(to(varDecl().bind("lval"))))),
	  hasRHS(ignoringImpCasts(
		  forEachDescendant(expr(declRefExpr().bind("rval")))))).bind("binop");

  // branch
  auto ifs = ifStmt().bind("if");
  // loop
  auto whiles = whileStmt().bind("while");
  // compound
  auto compounds = compoundStmt().bind("comp");
  // the function
  auto function = functionDecl(hasName(params.funcName),
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
    if (auto d = result.Nodes.getNodeAs<VarDecl>("decl")) {
      stmt_map[ds] = new AssignStatement(ds,d);
	  def_map[d].push_back(stmt_map[ds]);
    }
  }
  else if (auto bo = result.Nodes.getNodeAs<BinaryOperator>("binop")) {
	  auto lhs = result.Nodes.getNodeAs<VarDecl>("lval");
	  auto rhs = result.Nodes.getNodeAs<DeclRefExpr>("rval");
	  stmt_map[bo] = new AssignStatement(bo, lhs);
	  def_map[lhs].push_back(stmt_map[bo]);
  }
  else if (auto is = result.Nodes.getNodeAs<IfStmt>("if")) {
    stmt_map[is] = new BranchStatement()
  }
  else if (auto ws = result.Nodes.getNodeAs<WhileStmt>("while")){
    stmt_map[ws] = new LoopStatement()
  }
  else if (auto cs = result.Nodes.getNodeAs<CompoundStmt>("compound")) {
    stmt_map[cs] = new CompoundStatement()
  }
  else if (auto f = result.Nodes.getNodeAs<FunctionDecl>("f")) {
    //root
	stmt_map[f->getBody()] = new CompoundStatement()
  }
}

void PDGBuilder::onEndOfTranslationUnit() {
  // slice here
}

} // namespace clang