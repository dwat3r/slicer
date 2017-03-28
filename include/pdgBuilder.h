#ifndef PDG_BUILDER_H
#define PDG_BUILDER_H

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <string>

#include <map>
#include <iostream>
#include <string>
#include <set>

#include "pdg.h"
/*! main class for processing the AST
  */
namespace clang {
namespace slicer {

class PDGBuilder : public ast_matchers::MatchFinder::MatchCallback {
public:
  struct slicingStmtPos {
    slicingStmtPos()
      : sline(0), scol(0), eline(INT_MAX), ecol(INT_MAX) {}
    slicingStmtPos(int _sline, int _scol, int _eline, int _ecol)
      : sline(_sline), scol(_scol), eline(_eline), ecol(_ecol) {}
    int sline;
    int scol;
    int eline;
    int ecol;
    bool refined(int sl, int sc, int el, int ec);
  };

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
  void setSlicingStmt(const ast_matchers::MatchFinder::MatchResult &result, const clang::Stmt* astRef);
  const std::string funcName;
  int lineNo = 0;
  int colNo = 0;
  const clang::Stmt* slicingStmt = nullptr;
  slicingStmtPos slicePos;
  clang::SourceManager* sm = nullptr;
  // the function
  clang::Stmt* root = nullptr;
  // We store the Statements in a map.
  std::map<const clang::Stmt*, Statement*> stmt_map;
};
}
} // namespace clang
#endif // PDG_BUILDER_H
