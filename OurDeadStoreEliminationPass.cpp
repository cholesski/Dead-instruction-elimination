#include "llvm/ADT/Statistic.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Constants.h"
#include "OurCFG.h"

#include <codecvt>
#include <iostream>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <iterator>

using namespace llvm;

namespace {
struct OurDeadStoreEliminationPass : public FunctionPass {
  bool InstructionRemoved;

  static char ID;
  OurDeadStoreEliminationPass() : FunctionPass(ID) {}

  std::unordered_map<BasicBlock*, std::set<Value*>> defVar;
  std::unordered_map<BasicBlock*, std::set<Value*>> usedVar;
  std::unordered_map<BasicBlock*, std::set<Value*>> top; //skup promenljivih koje su zive na izlazu iz basic bloka
  std::unordered_map<BasicBlock*, std::set<Value*>> bottom; //skup promenljivih koje su zive na ulazu u basic blok 

  void printMap(std::unordered_map<BasicBlock*, std::set<Value*>> myMap){
    for(auto pair : myMap){
      auto bb = pair.first;
      auto var = pair.second;
      for (auto v : var){
        errs() << v->getName() << "\n";
      }
   }
  }


  void InitializeVariableSets(BasicBlock* Current){
    for (Instruction &Instr : *Current) {
      // Instr.printAsOperand(errs());
      Instr.print(errs());
        if (isa<LoadInst>(&Instr)) {
          defVar[Current].insert(&Instr);
          usedVar[Current].insert(Instr.getOperand(0));
          // errs() << "[LOAD INSTR OPERAND] : \n" << Instr.getOperand(0)->getName() << "\n";
          // errs() << "_______________________\n";
        }
        else if (isa<StoreInst>(&Instr)) {
            usedVar[Current].insert(Instr.getOperand(0));
            defVar[Current].insert(Instr.getOperand(1));
            // errs() << "[STORE INSTR OPERANDS] : \n" << Instr.getOperand(0)->getName() << "\n";
            // errs() << Instr.getOperand(1)->getName() << "\n";
            // errs() << "_______________________\n";
        }
        else {
          int NumOfOperands = (int)Instr.getNumOperands();
          for (int i = 0; i < NumOfOperands; i++) {
            defVar[Current].insert(&Instr);
            usedVar[Current].insert(Instr.getOperand(i));
            // errs() << "[OTHER INSTR OPERANDS] : \n" << Instr.getOperand(i)->getName() << "\n";
            // errs() << "_______________________\n";
            }
        }
    }
}

/*ovo prolazi kompilaciju ali to sto nije zavrseno nije testirano i 99.99% ne radi dobro to boze moj idem da spavam*/
  void GlobalLivenessAnalysis(Function &F) {
    OurCFG *CFG = new OurCFG(F);
    CFG->CreateTransposeCFG();
    for (BasicBlock &BB : F){
      InitializeVariableSets(&BB);
    }
    bool hasChanges = false;
    do {
      hasChanges = false;
      for (BasicBlock& BB : F){
        //temporary variables 
        std::set<Value*> top_ = *(new std::set<Value*>(top[&BB]));
        std::set<Value*> bottom_ = *(new std::set<Value*>(bottom[&BB]));
        /*sad tek vidim i mislim da se ovaj graf ne pravi dobro jer on ide redom kroz BB i samo dodaje linearno mesto 
        da uvezuje successore i predecessore*/
        /*ovo je odvratno i treba bolje da se uradi*/
        for (auto it = succ_begin(&BB), et = succ_end(&BB); it != et; ++it){
          BasicBlock* successor = *it;
          for (Value* v : top[successor]){
            bottom[&BB].insert(v);
          }
        }
        //ovde ide unija skupova pa razlika skupova pa test jesu li jednaki ne mogu to sada vise
        //Pomocni set je potreban zbog nacina kako funckionisu set_union itd.
        std::set<Value*> pomocni1;
        std::set_difference(bottom[&BB].begin(),bottom[&BB].end(),defVar[&BB].begin(),defVar[&BB].end(),pomocni1.begin());
        std::set<Value*> pomocni2;
        std::set_union(usedVar[&BB].begin(),usedVar[&BB].end(),pomocni1.begin(),pomocni1.end(),pomocni2.begin());
        top[&BB]= pomocni2;

        //Demorgan !(stariB == noviB && stariT == noviT)
        if(top_ != top[&BB] || bottom_ != bottom[&BB])
        {
            hasChanges = true;
        }
      }
    } while (hasChanges);
  };

  void EliminateUnusedVariables(Function &F) {};

  
  bool runOnFunction(Function &F)  {
    for (BasicBlock &BB : F) {
      // BB.print(errs());
      InitializeVariableSets(&BB);
    }
    // printMap();
    return false;
  }

};
}


char OurDeadStoreEliminationPass::ID = 1;
static RegisterPass<OurDeadStoreEliminationPass> X("dead-store-elimination", "Our dead store elimination pass");
