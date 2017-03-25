#ifndef PDG_BUILDER_H
#define PDG_BUILDER_H

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <string>

#include <map>
#include <iostream>
#include <string>
#include <set>

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
  explicit PDGBuilder() {}
  explicit PDGBuilder(llvm::StringRef _funcName,
                      int _lineNo,
                      int _colNo)
    : funcName(_funcName)
    , lineNo(_lineNo)
    , colNo(_colNo)
  {}

  void registerMatchers(ast_matchers::MatchFinder *MatchFinder);

  void run(const ast_matchers::MatchFinder::MatchResult &result) override;
  
  void onEndOfTranslationUnit() override;

private:
  llvm::StringRef funcName;
  int lineNo;
  int colNo;
  clang::ValueDecl* var;
  // We store the Statements in a map.
  std::map<const clang::Stmt*, Statement*> stmt_map;
  // We store the def variables in another map
  std::map<const clang::ValueDecl*, std::set<Statement*>> def_map;
};

} // namespace clang
#endif // PDG_BUILDER_H
