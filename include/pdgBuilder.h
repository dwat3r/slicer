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
    unsigned int sline;
    unsigned int scol;
    unsigned int eline;
    unsigned int ecol;
    bool refined(unsigned int sl,unsigned int sc,unsigned int el,unsigned int ec);
  };

  explicit PDGBuilder() {}
  explicit PDGBuilder(std::string _funcName,
                      int _lineNo,
                      int _colNo,
                      bool _dumpDot)
    : funcName(_funcName)
    , lineNo(_lineNo)
    , colNo(_colNo)
    , dumpDot(_dumpDot)
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
  void dumpDots();
  // tool params
  const std::string funcName;
  unsigned int lineNo = 0;
  unsigned int colNo = 0;
  bool dumpDot = false;
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
