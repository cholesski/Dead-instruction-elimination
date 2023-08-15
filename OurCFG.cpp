//
// Created by strahinja on 4/20/23.
//

#include "OurCFG.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

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
    EndBlock = &BB;
  }
}

/* pravi obrnut graf ali mislim da je mnogo ruzna slozenost 
 mada mislim da se uvecava samo za const faktor ++ slozenost
 samog algoritma je i dalje ruznija ja mislim treba proveriti 
 ovo mozda uraditi na efikasniji nacin ali mrzi me sad da 
radim bolje 
ovo napravi normalan graf i onda uzima iz vektora i usmerava ih nazad ka clanu mape
i btw napravice skroz novi graf u memoriji odvojen*/
void OurCFG::CreateTransposeCFG(llvm::Function &F)
{
  for(BasicBlock &BB : F) {
    TransposeAdjacencyList[&BB] = {};
    for (BasicBlock *Predecessor : predecessors(&BB)) {
      TransposeAdjacencyList[&BB].push_back(Predecessor);
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

std::vector<BasicBlock*> OurCFG::GetTraverseOrder()
{
  std::queue<BasicBlock*> q;
  std::unordered_set<BasicBlock*> returnVal;

  q.push(EndBlock);
  returnVal.insert(EndBlock);
  while(!q.empty()){
    BasicBlock* Current = q.front();
    // errs() << "[CURRENT] : \n";
    // Current->print(errs());
    q.pop();
    for (BasicBlock* BB : predecessors(Current)){
      // errs() << "[tekuci predecesor] :\n";
      // BB->print(errs());
      if (returnVal.find(BB) == returnVal.end()){
        returnVal.insert(BB);
        q.push(BB);
        // errs() << "[dodao sam predecesora] : \n";
        // BB->print(errs());
      }
    }
  }
  std::vector<BasicBlock*> vc(returnVal.begin(), returnVal.end());
  reverse(vc.begin(), vc.end());
  return vc;
}

bool OurCFG::IsReachable(llvm::BasicBlock *BB)
{
  return Visited.find(BB) != Visited.end();
}
