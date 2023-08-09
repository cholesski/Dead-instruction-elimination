//
// Created by strahinja on 4/20/23.
//

#include "OurCFG.h"
#include "llvm/IR/BasicBlock.h"

OurCFG::OurCFG(llvm::Function &F)
{
  CreateCFG(F);
}

void OurCFG::CreateCFG(llvm::Function &F)
{
  StartBlock = &F.front();

  for(BasicBlock &BB : F) {
    AdjacencyList[&BB] = {};
    for (BasicBlock *Successor : successors(&BB)) {
      AdjacencyList[&BB].push_back(Successor);
    }
  }
}

void OurCFG::TraverseGraph()
{
  DFS(StartBlock);
}

void OurCFG::DFS(llvm::BasicBlock *Current)
{
  Visited.insert(Current);

  for(BasicBlock *Successor : AdjacencyList[Current]) {
    if (Visited.find(Successor) == Visited.end()) {
      DFS(Successor);
    }
  }
}


bool OurCFG::IsReachable(llvm::BasicBlock *BB)
{
  return Visited.find(BB) != Visited.end();
}