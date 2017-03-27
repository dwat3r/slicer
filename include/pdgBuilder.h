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
namespace slicer {
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
  explicit PDGBuilder(std::string _funcName,
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
  inline bool hasStmt(const clang::Stmt* value) {
    return stmt_map.find(value) != stmt_map.end();
  }
  static clang::FullSourceLoc 
    getLoc(const ast_matchers::MatchFinder::MatchResult &result, const clang::Stmt* astRef) {
    return result.Context->getFullLoc(astRef->getLocStart());
  }
  const std::string funcName;
  int lineNo = 0;
  int colNo = 0;
  const clang::ValueDecl* slicingVar = nullptr;
  clang::SourceManager* sm = nullptr;
  // the function
  clang::Stmt* root = nullptr;
  // We store the Statements in a map.
  std::map<const clang::Stmt*, Statement*> stmt_map;
};
}
} // namespace clang
#endif // PDG_BUILDER_H
