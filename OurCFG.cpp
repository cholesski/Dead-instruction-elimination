//
// Created by strahinja on 4/20/23.
//

#include "OurCFG.h"
#include "llvm/IR/BasicBlock.h"
#include <cassert>
#include <vector>

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

/* pravi obrnut graf ali mislim da je mnogo ruzna slozenost 
 mada mislim da se uvecava samo za const faktor ++ slozenost
 samog algoritma je i dalje ruznija ja mislim treba proveriti 
 ovo mozda uraditi na efikasniji nacin ali mrzi me sad da 
radim bolje 
ovo napravi normalan graf i onda uzima iz vektora i usmerava ih nazad ka clanu mape
i btw napravice skroz novi graf u memoriji odvojen*/
void OurCFG::CreateTransposeCFG()
{
  assert(!AdjacencyList.empty());
  for (auto pair : AdjacencyList){
    BasicBlock* BB = pair.first;
    std::vector<BasicBlock*> succ = pair.second;
    for (BasicBlock* s : succ){
      TransposeAdjacencyList[s].push_back(BB);
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
