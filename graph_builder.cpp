#include "graph_builder.h"
#include <iostream>

bool GraphBuilder::VisitGotoStmt(clang::GotoStmt *Stmt){
  return true;
}

bool GraphBuilder::VisitLabelStmt(clang::LabelStmt *Stmt){
  return true;
}

bool GraphBuilder::TraverseWhileStmt(clang::WhileStmt *Stmt){
  clang::RecursiveASTVisitor<GraphBuilder>::TraverseStmt(Stmt->getBody());
  return true;
}

bool GraphBuilder::TraverseIfStmt(clang::IfStmt *Stmt){
  clang::RecursiveASTVisitor<GraphBuilder>::TraverseStmt(Stmt->getThen());
  clang::RecursiveASTVisitor<GraphBuilder>::TraverseStmt(Stmt->getElse());
  return true;
}

