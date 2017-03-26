#include "pdgBuilderAction.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/STLExtras.h"

namespace clang {
namespace slicer {
PDGBuilderAction::PDGBuilderAction(
  std::string _funcName,
  int _lineNo,
  int _colNo)
  : funcName(_funcName)
  , lineNo(_lineNo)
  , colNo(_colNo)
  , Matcher(funcName, lineNo, colNo) {
  Matcher.registerMatchers(&MatchFinder);
}

std::unique_ptr<ASTConsumer>
PDGBuilderAction::CreateASTConsumer(CompilerInstance &Compiler,
                                    StringRef InFile) {

  return MatchFinder.newASTConsumer();
}
} // namespace slicer
} // namespace clang
