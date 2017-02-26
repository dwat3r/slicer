#ifndef ASTWALKER_H
#define ASTWALKER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"

#include <map>
#include <iostream>
#include <string>

#include "relations.h"

/*! main class for processing the AST
 */
class RelationsBuilder : public clang::RecursiveASTVisitor<RelationsBuilder>
{
public:
  explicit RelationsBuilder(clang::ASTContext *Context,
                            std::vector<std::string> params)
    : Context(Context)
    , funcName(params[0])
    , row(std::stoi(params[1]))
    , column(std::stoi(params[2]))
    , statements()
  {}
  //bool shouldTraversePostOrder() const {return true;}
  /* Query function, does not recurse into
   * Compound, Loop or Branch, just gets the vars at that level.
   */
  std::set<clang::ValueDecl*> vars(clang::Stmt *Stmt);

  // we process nodes in Visit*

  // Assign
  bool VisitDeclStmt(clang::DeclStmt *Stmt);
  bool VisitBinaryOperator(clang::BinaryOperator *Stmt);
  // Compound
  bool VisitCompoundStmt(clang::CompoundStmt *Stmt);
  // Branch
  bool VisitIfStmt(clang::IfStmt *Stmt);
  // Loop
  bool VisitWhileStmt(clang::WhileStmt *Stmt);

  // determine pointed variable by the location parameters
  bool VisitDeclRefExpr(clang::DeclRefExpr *Stmt);
  
  // restrict traversal to function only
  bool TraverseFunctionDecl(clang::FunctionDecl *Decl);
  // Compute slice
  std::set<clang::Stmt*> computeSlice();

private:
  typedef clang::RecursiveASTVisitor<RelationsBuilder> base;
  clang::ASTContext *Context;
  //! Slicing informations: function name
  std::string funcName;
  //! The position and the reference to the statement asked for slicing
  int column;
  int row;
  clang::DeclRefExpr* var;
  // We store the Statements in a map.
  std::map<clang::Stmt*,std::shared_ptr<Statement>> statements;
};

// necessary class to hook
class RelationsBuilderConsumer : public clang::ASTConsumer {
public:
  explicit RelationsBuilderConsumer(clang::ASTContext *Context,
                                    std::vector<std::string> params);


  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  RelationsBuilder Visitor;
};

class RelationsBuilderAction : public clang::ASTFrontendAction {
public:
  RelationsBuilderAction(std::vector<std::string> params);

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new RelationsBuilderConsumer(&Compiler.getASTContext(),params));
  }
private:
  std::vector<std::string> params;
};
#endif // ASTWALKER_H
