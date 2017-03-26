#ifndef PDG_BUILDER_ACTION_H
#define PDG_BUILDER_ACTION_H

#include "pdgBuilder.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include <memory>

namespace clang {
namespace slicer {

class PDGBuilderAction : public clang::ASTFrontendAction {
public:
  explicit PDGBuilderAction(std::string _funcName,
                            int _lineNo,
                            int _colNo);
  std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &Compiler,
                      StringRef InFile) override;

private:
  std::string funcName;
  int lineNo;
  int colNo;
  clang::ast_matchers::MatchFinder MatchFinder;
  PDGBuilder Matcher;
};

class PDGBuilderActionFactory : public tooling::FrontendActionFactory {
public:
  PDGBuilderActionFactory(std::string _funcName,
                          int _lineNo,
                          int _colNo)
    : funcName(_funcName)
    , lineNo(_lineNo)
    , colNo(_colNo)
  {}

  clang::FrontendAction *create() override {
    return new PDGBuilderAction(funcName, lineNo, colNo);
  }

private:
  std::string funcName;
  int lineNo;
  int colNo;
};

} // namespace slicer
} // namespace clang

#endif // PDG_BUILDER_ACTION_H
