#ifndef LLVM_PROJECT_OURCFG_H
#define LLVM_PROJECT_OURCFG_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CFG.h"

#include <unordered_set>
#include <queue>

using namespace llvm;

class OurCFG {
private:
  std::unordered_map<BasicBlock *, std::vector<BasicBlock *>> AdjacencyList;
  std::unordered_map<BasicBlock *, std::vector<BasicBlock *>> TransposeAdjacencyList;
  std::unordered_set<BasicBlock *> Visited;
  BasicBlock *StartBlock;
  BasicBlock *EndBlock;

  void CreateCFG(Function &F);
  void DFS(BasicBlock *Current);
public:
  OurCFG(Function &F);
  std::vector<BasicBlock*> GetTraverseOrder();
  void CreateTransposeCFG(Function &F);
  void TraverseGraph();
  bool IsReachable(BasicBlock *);
};

#endif // LLVM_PROJECT_OURCFG_H

