#include "pdgBuilder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include <fstream>

using namespace clang::ast_matchers;

namespace clang {
namespace slicer {
namespace {
AST_MATCHER(BinaryOperator, isAssignmentOp) {
  return Node.isAssignmentOp();
}
AST_MATCHER(UnaryOperator, isIncrementDecrementOp) {
  return Node.isIncrementDecrementOp();
}
std::string getFile(const clang::Stmt* stmt, clang::SourceManager* sm) {
  assert(stmt == nullptr || sm == nullptr);
  clang::FileID fileID = sm->getFileID(stmt->getLocStart());
  const clang::FileEntry* fileEntry = sm->getFileEntryForID(fileID);
  if (!fileEntry) {
    return "";
  }
  return fileEntry->getName();  
}
}
bool PDGBuilder::slicingStmtPos::refined(unsigned int sl,unsigned int sc,unsigned int el,unsigned int ec) {
  bool ret = false;
  if (sl > sline) { sline = sl;ret = true;}
  if (sc > scol)  { scol  = sc;ret = true;}
  if (el < eline) { eline = el;ret = true;}
  if (ec < ecol)  { ecol  = ec;ret = true;}
  return ret;
}
void PDGBuilder::setSlicingStmt(const ast_matchers::MatchFinder::MatchResult &result, const clang::Stmt* astRef) {
  clang::FullSourceLoc start = result.Context->getFullLoc(astRef->getLocStart());
  clang::FullSourceLoc end = result.Context->getFullLoc(astRef->getLocEnd());
  
  if (start.getSpellingLineNumber()   <= lineNo &&
      start.getSpellingColumnNumber() <= colNo  &&
      lineNo <= end.getSpellingLineNumber()     &&
      colNo  <= end.getSpellingColumnNumber()   &&
      slicePos.refined(start.getSpellingLineNumber(), start.getSpellingColumnNumber(),
                         end.getSpellingLineNumber(),   end.getSpellingColumnNumber())) {
    slicingStmt = astRef;
  }
}
void PDGBuilder::registerMatchers(MatchFinder *MatchFinder) {
  // first define matchers
  // assign
  auto decls = declStmt(hasDescendant(varDecl().bind("decl"))).bind("declStmt");
  auto declWithInit = declStmt(hasDescendant(
                                  varDecl(forEachDescendant(
                                    declRefExpr(to(varDecl().bind("declInit"))))).bind("declWithInit")
                              )).bind("declStmtWithInit");

  auto bop = binaryOperator(isAssignmentOp(),
                                hasLHS(ignoringImpCasts(declRefExpr(to(varDecl().bind("lval"))))),
                                hasRHS(forEachDescendant(
                                  expr(declRefExpr(to(varDecl().bind("rval"))))))).bind("binop");
  
  auto bopLit = binaryOperator(isAssignmentOp(),
	  hasLHS(ignoringImpCasts(declRefExpr(to(varDecl().bind("lval")))))).bind("binopLit");
  //fixme do something with unary ops which are embedded in statements like: int x = a++;
  auto uop = unaryOperator(isIncrementDecrementOp(),
                           hasDescendant(declRefExpr(to(varDecl().bind("uval"))))).bind("unop");

  auto ret = returnStmt(forEachDescendant(declRefExpr(to(varDecl().bind("retVar"))))).bind("ret");
  // branch
  //fixme it won't match for "if(0)"
  auto ifs = ifStmt(hasCondition(
    forEachDescendant(declRefExpr(to(varDecl().bind("ifCondVar")))))).bind("if");
  // loop
  auto whiles = whileStmt(hasCondition(
    forEachDescendant(declRefExpr(to(varDecl().bind("whileCondVar")))))).bind("while");
  // compound
  auto compounds = compoundStmt().bind("comp");

  // then add them to MatchFinder
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(decls)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(declWithInit)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(bop)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(bopLit)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(uop)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(ret)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(ifs)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(whiles)).bind("f"), this);
  MatchFinder->addMatcher(functionDecl(hasName(funcName), forEachDescendant(compounds)).bind("f"), this);
}
void PDGBuilder::run(const ast_matchers::MatchFinder::MatchResult &result) {
  // process the match results
  // ensure every entry added once
  // branch
  // todo add parent references
  if (auto is = result.Nodes.getNodeAs<IfStmt>("if")) {
    if (!hasStmt(is)) {
      stmt_map[is] = new BranchStatement(is, getLoc(result, is));
    }
    stmt_map[is->getThen()] = Statement::create(is->getThen(), getLoc(result, is->getThen()));
    stmt_map[is]->addControlChild({ stmt_map[is->getThen()],Statement::Edge::True });
	  if (is->getElse() != nullptr) {
		  stmt_map[is->getElse()] = Statement::create(is->getElse(), getLoc(result, is->getElse()));
      stmt_map[is]->addControlChild({ stmt_map[is->getElse()],Statement::Edge::False });
	  }
    stmt_map[is]->addUse(result.Nodes.getNodeAs<VarDecl>("ifCondVar"));

  }
  // loop
  if (auto ws = result.Nodes.getNodeAs<WhileStmt>("while")) {
    if (!hasStmt(ws)) {
      stmt_map[ws] = new LoopStatement(ws, getLoc(result,ws));
    }
    stmt_map[ws->getBody()] = Statement::create(ws->getBody(),getLoc(result,ws->getBody()));
    stmt_map[ws]->addControlChild({ stmt_map[ws->getBody()],Statement::Edge::True });
    
    stmt_map[ws]->addUse(result.Nodes.getNodeAs<VarDecl>("whileCondVar"));
  }
  // compound
  if (auto cs = result.Nodes.getNodeAs<CompoundStmt>("comp")) {
    if (!hasStmt(cs)) {
      stmt_map[cs] = new CompoundStatement(cs,getLoc(result,cs));
    }
    if (stmt_map[cs]->getControlChildren().empty()) {
      for (auto c : cs->children()) {
        if (stmt_map.find(c) == stmt_map.end()) {
          stmt_map[c] = Statement::create(c,getLoc(result,c));
        }
        stmt_map[cs]->addControlChild({ stmt_map[c],Statement::Edge::True });
      }
    }
  }
  if (auto f = result.Nodes.getNodeAs<FunctionDecl>("f")) {
    //root
    root = f->getBody();
    if (!hasStmt(root)) {
      stmt_map[root] = new CompoundStatement(root,getLoc(result,root));
      for (auto& var : f->parameters()) {
        stmt_map[root]->addDefine(var);
      }
    }
    if (stmt_map[root]->getControlChildren().empty()) {
      for (auto c : root->children()) {
        if (stmt_map.find(c) == stmt_map.end()) {
          stmt_map[c] = Statement::create(c, getLoc(result, c));
        }
        stmt_map[root]->addControlChild({ stmt_map[c],Statement::Edge::True });
      }
    }
  }
  // assign
  if (auto ds = result.Nodes.getNodeAs<DeclStmt>("declStmt")) {
    //todo multiple decl...
    auto d = result.Nodes.getNodeAs<VarDecl>("decl");
    if (!hasStmt(ds)) {
      stmt_map[ds] = new AssignStatement(ds, getLoc(result, ds), { d });
    }
    else {
      stmt_map[ds]->addDefine(d);
    }
    setSlicingStmt(result, ds);
  }
  if (auto ds = result.Nodes.getNodeAs<DeclStmt>("declStmtWithInit")) {
    //todo multiple decl...
    auto d = result.Nodes.getNodeAs<VarDecl>("declWithInit");
    if (!hasStmt(ds)) {
      stmt_map[ds] = new AssignStatement(ds, getLoc(result, ds), { d });
    }
    else {
      stmt_map[ds]->addDefine(d);
    }
    stmt_map[ds]->addUse(result.Nodes.getNodeAs<VarDecl>("declInit"));
    setSlicingStmt(result, ds);
  }
  if (auto bo = result.Nodes.getNodeAs<BinaryOperator>("binop")) {
    auto lhs = result.Nodes.getNodeAs<VarDecl>("lval");
    if (!hasStmt(bo)) {
      stmt_map[bo] = new AssignStatement(bo, getLoc(result, bo), { lhs });
    }
    else {
      stmt_map[bo]->addDefine(lhs);
    }
    stmt_map[bo]->addUse(result.Nodes.getNodeAs<VarDecl>("rval"));
    setSlicingStmt(result, bo);
  }
  if (auto boL = result.Nodes.getNodeAs<BinaryOperator>("binopLit")) {
    auto lhs = result.Nodes.getNodeAs<VarDecl>("lval");
    if (!hasStmt(boL)) {
      stmt_map[boL] = new AssignStatement(boL, getLoc(result, boL), { lhs });
    }
    else {
      stmt_map[boL]->addDefine(lhs);
    }
    setSlicingStmt(result, boL);
  }
  if (auto uo = result.Nodes.getNodeAs<UnaryOperator>("unop")) {
    auto var = result.Nodes.getNodeAs<VarDecl>("uval");
    if (!hasStmt(uo)) {
      stmt_map[uo] = new AssignStatement(uo, getLoc(result, uo), { var });
    }
    else {
      stmt_map[uo]->addDefine(var);
    }
    stmt_map[uo]->addUse(var);
    setSlicingStmt(result, uo);
  }
  if (auto ret = result.Nodes.getNodeAs<ReturnStmt>("ret")) {
    auto var = result.Nodes.getNodeAs<VarDecl>("retVar");
    if (!hasStmt(ret)) {
      stmt_map[ret] = new AssignStatement(ret, getLoc(result, ret));
    }
    stmt_map[ret]->addUse(var);
    setSlicingStmt(result, ret);
  }
  // save sourcemanager, we'll need it later for dot creation.
  sm = result.SourceManager;
}

void PDGBuilder::onEndOfTranslationUnit() {
  // dump useful info
  if (slicingStmt) { llvm::errs() << "slicing statement is:\n";slicingStmt->dumpColor(); }
  else { llvm::errs() << "You've given invalid location for slicing var, since I've found no variable there.\n"; }
  llvm::errs() << "With control edges, but no data dependence edges:\n";
  llvm::errs() << stmt_map[root]->dump();
  stmt_map[root]->setDataEdges();
  llvm::errs() << "With data dependence edges too:\n";
  llvm::errs() << stmt_map[root]->dump();
  if (dumpDot) {
    std::string out = getFile(root, sm) + "_" + funcName + ".dot";
    std::ofstream file(out);
    file << stmt_map[root]->dumpDot(*sm);
    file.close();
  }
  // slice here
  stmt_map[root]->DFS(stmt_map[slicingStmt]);
}
} // namespace slicer
} // namespace clang