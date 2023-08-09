#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Constants.h"
#include "OurCFG.h"

#include <set>
#include <unordered_map>

using namespace llvm;

namespace {
struct OurDeadStoreEliminationPass : public FunctionPass {
  bool InstructionRemoved;

  static char ID;
  OurDeadStoreEliminationPass() : FunctionPass(ID) {}

  std::unordered_map<Value*, std::set<BasicBlock*>> defVar;
  std::unordered_map<Value*, std::set<BasicBlock*>> usedVar;

  void initializeVariableSets();

  void GlobalLivenessAnalysis(OurCFG &CFG) {};

  void EliminateUnusedVariables(Function &F) {};

  
  bool runOnFunction(Function &F) override {
    return true;
  }
};
}

char OurDeadStoreEliminationPass::ID = 0;
static RegisterPass<OurDeadStoreEliminationPass> X("dead-store-elimination", "Our dead store elimination pass");