#include "relations_builder.h"
#include <iostream>

bool RelationsBuilder::VisitGotoStmt(clang::GotoStmt *Stmt){  
  return true;
}

bool RelationsBuilder::VisitLabelStmt(clang::LabelStmt *Stmt){
  return true;
}

bool RelationsBuilder::VisitCompoundStmt(clang::CompoundStmt *Stmt){
  for (auto& s : Stmt->body()){
      addChild(Stmt,s);
    }
  return true;
}

bool RelationsBuilder::VisitWhileStmt(clang::WhileStmt *Stmt){

  return true;
}

bool RelationsBuilder::VisitIfStmt(clang::IfStmt *Stmt){
  return true;
}

void RelationsBuilder::addChild(clang::Stmt *parent, clang::Stmt *child){
  //implement switch here
  switch
}
