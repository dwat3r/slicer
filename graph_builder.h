#ifndef ASTWALKER_H
#define ASTWALKER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"

#include <stack>
#include <iostream>

#include "graph.h"

// main class for processing the AST
class GraphBuilder : public clang::RecursiveASTVisitor<GraphBuilder>
{
public:
  explicit GraphBuilder(clang::ASTContext *Context)
    : Context(Context),pdg() {}

  // we process nodes in Visit*
  void assign(clang::Stmt *Stmt);
  bool VisitGotoStmt(clang::GotoStmt *Stmt);
  bool VisitLabelStmt(clang::LabelStmt *Stmt);

  // we define order of processing in Traverse*
  bool TraverseWhileStmt(clang::WhileStmt *Stmt);
//  bool TraverseDoStmt(clang::DoStmt *Stmt);

  bool TraverseIfStmt(clang::IfStmt *Stmt);
private:
  clang::ASTContext *Context;
  Graph graph;
  std::stack<Node> CDStack;
};

// necessary class to hook
class GraphBuilderConsumer : public clang::ASTConsumer {
public:
  explicit GraphBuilderConsumer(clang::ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  GraphBuilder Visitor;
};

class GraphBuilderAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new GraphBuilderConsumer(&Compiler.getASTContext()));
  }
};
#endif // ASTWALKER_H
