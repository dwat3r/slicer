#ifndef ASTWALKER_H
#define ASTWALKER_H

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <string>

#include <map>
#include <iostream>
#include <string>

#include "pdg.h"
namespace clang {

/*! main class for processing the AST
  */
struct data {
  std::string funcName;
  int line;
  int col;
};

class PDGBuilder : public ast_matchers::MatchFinder::MatchCallback {
public:
  explicit PDGBuilder(data _params)
    : params(_params) 
  {}

  void registerMatchers(ast_matchers::MatchFinder *MatchFinder);

  void run(const ast_matchers::MatchFinder::MatchResult &result) override;
  
  void onEndOfTranslationUnit() override;

private:
  data params;
  clang::ValueDecl* var;
  // We store the Statements in a map.
  std::map<const clang::Stmt*, Statement*> stmt_map;
};

} // namespace clang
#endif // ASTWALKER_H
