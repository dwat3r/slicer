#ifndef ASTWALKER_H
#define ASTWALKER_H
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "pdg.h"
#include <iostream>
class ASTWalker : public clang::RecursiveASTVisitor<ASTWalker>
{
public:
  explicit ASTWalker(clang::ASTContext *Context)
    : Context(Context),pdg() {}
  ~ASTWalker()
  {
    std::cout << "in dtor" << std::endl;
    pdg.print();
  }

  // we process nodes in Visit*
  void assign(clang::Stmt *Stmt);
  void while_start(clang::WhileStmt *Stmt);
  void while_end(clang::WhileStmt *Stmt);
  void if_start(clang::IfStmt *Stmt);
  void if_b_start(clang::IfStmt *Stmt,Label label);
  void if_b_end(clang::IfStmt *Stmt);
  void if_end(clang::IfStmt *Stmt);
  void control_trans(clang::Stmt *Stmt);
  bool VisitGotoStmt(clang::GotoStmt *Stmt);
  bool VisitLabelStmt(clang::LabelStmt *Stmt);

  // we define order of processing in Traverse*
  bool TraverseWhileStmt(clang::WhileStmt *Stmt);
//  bool TraverseDoStmt(clang::DoStmt *Stmt);

  bool TraverseIfStmt(clang::IfStmt *Stmt);
private:
  clang::ASTContext *Context;
  PDG pdg;
};

class ASTWalkerConsumer : public clang::ASTConsumer {
public:
  explicit ASTWalkerConsumer(clang::ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  ASTWalker Visitor;
};

class ASTWalkerAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new ASTWalkerConsumer(&Compiler.getASTContext()));
  }
};
#endif // ASTWALKER_H
