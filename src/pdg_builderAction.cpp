#include "pdg_builderAction.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/STLExtras.h"

namespace clang {

PDGBuilderAction::PDGBuilderAction(
  llvm::StringRef _funcName,
  int _lineNo,
  int _colNo)
    : funcName(_funcName)
    , lineNo(_lineNo)
    , colNo(_colNo)
    ,  Matcher(funcName,lineNo,colNo) {
  Matcher.registerMatchers(&MatchFinder);
}

std::unique_ptr<ASTConsumer>
PDGBuilderAction::CreateASTConsumer(CompilerInstance &Compiler,
                                        StringRef InFile) {

  return MatchFinder.newASTConsumer();
}

} // namespace clang
