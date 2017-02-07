#ifndef ASTWALKER_H
#define ASTWALKER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"

#include <stack>
#include <iostream>

#include "relations.h"

// main class for processing the AST
class RelationsBuilder : public clang::RecursiveASTVisitor<RelationsBuilder>
{
public:
  explicit RelationsBuilder(clang::ASTContext *Context)
    : Context(Context)
    , graph()
  {}

  // Query functions
  void vars(clang::Stmt *Stmt);
  void defs(clang::Stmt *Stmt);

  // we process nodes in Visit*
  bool VisitGotoStmt(clang::GotoStmt *Stmt);
  bool VisitLabelStmt(clang::LabelStmt *Stmt);
  //bool VisitCompoundStmt(clang::CompoundStmt *Stmt);

  // we define order of processing in Traverse*
  bool TraverseWhileStmt(clang::WhileStmt *Stmt);
//  bool TraverseDoStmt(clang::DoStmt *Stmt);
  bool TraverseIfStmt(clang::IfStmt *Stmt);

private:
  clang::ASTContext *Context;
  CompoundStatement graph;
};

// necessary class to hook
class RelationsBuilderConsumer : public clang::ASTConsumer {
public:
  explicit RelationsBuilderConsumer(clang::ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  RelationsBuilder Visitor;
};

class RelationsBuilderAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new RelationsBuilderConsumer(&Compiler.getASTContext()));
  }
};
#endif // ASTWALKER_H
