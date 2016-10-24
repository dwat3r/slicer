#include "astwalker.h"
#include <iostream>


void ASTWalker::while_start(clang::WhileStmt *Stmt)
{
  //prepare while head
  std::cout << "while_start\n";
  Node *pred = new Node(Statement,Stmt->getCond());
  Node *header = new Node(Region,Stmt);
  pdg.addNode(pdg.getActive(),header,No_Label);
  pdg.addNode(header,pred,No_Label);
  pdg.addNode(pred,new Node(Region,Stmt->getBody()),True);
}

void ASTWalker::while_end(clang::WhileStmt *Stmt)
{
  std::cout << "while_end\n";
  pdg.popStack();
  //resolve unresolved nodes
  pdg.popStack();
  //replace stack with while follow region
  //set adjustflag
}

void ASTWalker::if_start(clang::IfStmt *Stmt)
{
  std::cout << "if_start\n";
  Node* pred = new Node(Statement,Stmt->getCond());
  pdg.addNode(pdg.getActive(),pred,No_Label);
  pdg.pushStack(pred);
}

void ASTWalker::if_b_start(clang::IfStmt *Stmt,Label label)
{
  std::cout << "if_b_start\n";
  clang::Stmt *block;
  if(label == True)
    block = Stmt->getThen();
  else
    block = Stmt->getElse();
  Node *region = new Node(Region,block);
  pdg.addNode(pdg.getActive(),region,label);
  pdg.pushStack(region);
}

void ASTWalker::if_b_end(clang::IfStmt *Stmt)
{
  //list unresolved nodes
  std::cout << "if_b_end\n";
  pdg.popStack();

}

void ASTWalker::if_end(clang::IfStmt *Stmt)
{
  std::cout << "if_end\n";
  pdg.popStack();
  //replace top of CDStack with if-else follow region
}

bool ASTWalker::VisitGotoStmt(clang::GotoStmt *Stmt)
{
  std::cout << "in goto\n";
  pdg.addNode(pdg.getActive(),new Node(Statement,Stmt),No_Label);
  //update labeltable
  //set adjustflag

  return true;
}
bool ASTWalker::VisitLabelStmt(clang::LabelStmt *Stmt)
{
  std::cout << "in label\n";
  Node* label = new Node(Region,Stmt);
  pdg.addNode(pdg.getActive(),label,No_Label);
  pdg.pushStack(label);
  //update labeltable
  return true;
}

bool ASTWalker::TraverseWhileStmt(clang::WhileStmt *Stmt)
{
  std::cout << "in while\n";
  while_start(Stmt);
  clang::RecursiveASTVisitor<ASTWalker>::TraverseStmt(Stmt->getBody());
  while_end(Stmt);
  std::cout << "returned from while\n";
  pdg.print();
  return true;
}
bool ASTWalker::TraverseIfStmt(clang::IfStmt *Stmt)
{
  std::cout << "in if\n";
  if_start(Stmt);
  if_b_start(Stmt,True);
  clang::RecursiveASTVisitor<ASTWalker>::TraverseStmt(Stmt->getThen());
  if_b_end(Stmt);
  if_b_start(Stmt,False);
  clang::RecursiveASTVisitor<ASTWalker>::TraverseStmt(Stmt->getElse());
  if_b_end(Stmt);
  if_end(Stmt);
  std::cout << "returned from if\n";
  return true;
}
