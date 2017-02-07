#include "relations_builder.h"
#include <iostream>

bool RelationsBuilder::VisitGotoStmt(clang::GotoStmt *Stmt){
  return true;
}

bool RelationsBuilder::VisitLabelStmt(clang::LabelStmt *Stmt){
  return true;
}

bool RelationsBuilder::VisitCompoundStmt(clang::CompoundStmt *Stmt){
  return true;
}

bool RelationsBuilder::TraverseWhileStmt(clang::WhileStmt *Stmt){
  clang::RecursiveASTVisitor<RelationsBuilder>::TraverseStmt(Stmt->getBody());
  return true;
}

bool RelationsBuilder::TraverseIfStmt(clang::IfStmt *Stmt){
  clang::RecursiveASTVisitor<RelationsBuilder>::TraverseStmt(Stmt->getThen());
  clang::RecursiveASTVisitor<RelationsBuilder>::TraverseStmt(Stmt->getElse());
  return true;
}

