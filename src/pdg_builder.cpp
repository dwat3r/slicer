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
  auto decls = varDecl().bind("decl");
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
                                hasOperatorName("|=")).bind("binop");
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
  if (auto decl = result.Nodes.getNodeAs<VarDecl>("decl")) {
        stmt_map[decl] = std::make_shared<AssignStatement>(decl,
                                                           decl,
                                                           { decl });
      
  }
  else if (auto bo = result.Nodes.getNodeAs<BinaryOperator>("binop")) {
    stmt_map[bo] = Statement::create(bo);

  }
  else if (auto is = result.Nodes.getNodeAs<IfStmt>("if")) {
    stmt_map[is] = Statement::create(is);
  }
  else if (auto ws = result.Nodes.getNodeAs<WhileStmt>("while")){
    stmt_map[ws] = Statement::create(ws);
  }
  else if (auto cs = result.Nodes.getNodeAs<CompoundStmt>("compound")) {
    stmt_map[cs] = Statement::create(cs);
  }
  else if (auto f = result.Nodes.getNodeAs<FunctionDecl>("f")) {
    stmt_map[f->getBody()] = Statement::create(f->getBody());
  }
}

void PDGBuilder::onEndOfTranslationUnit() {
  // slice here
}

} // namespace clang