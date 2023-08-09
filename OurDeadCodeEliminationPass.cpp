#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Constants.h"
#include "OurCFG.h"

#include <unordered_set>

using namespace llvm;

namespace {
struct OurDeadCodeEliminationPass : public FunctionPass {
  bool InstructionRemoved;

  static char ID;
  OurDeadCodeEliminationPass() : FunctionPass(ID) {}

  void EliminateUnusedVariables(Function &F)
  {
    //promenljiva -> ziva je ili nije ziva
    std::unordered_map<Value *, bool> Variables;
    //promenljiva desno -> promenljiva levo 
    std::unordered_map<Value *, Value *>VariablesMap;
    std::unordered_set<Instruction *> InstructionsRemove;

    //x = y;        
    //bice y1 = load y, store y1 x 
    for (BasicBlock &BB : F) {
      for (Instruction &Instr : BB) {
        if (Instr.getType() != Type::getVoidTy(Instr.getContext())) {
          if (!isa<CallInst>(&Instr)) {
            Variables[&Instr] = false;
          }
        }
        if (isa<LoadInst>(&Instr)) {
          //y1 = load y
          //operand 0 je y 
          //load uzima vrednost promenljive y 
          //i kopira je i vraca pokazivac na novu promenljivu y1
          //y1 -> y
          VariablesMap[&Instr] = Instr.getOperand(0);
        }

        if (isa<StoreInst>(&Instr)) {
          //ako y cuva neku vrednost (ako je na y prethodno primenjena load instrukcija)
          if (Variables.find(Instr.getOperand(0)) != Variables.end()) {
            //store prepisuje vrednost postojece promenljive
            //x = y;
            //store y1 x 
            //promenljiva y1 -> true
            Variables[Instr.getOperand(0)] = true;
            //sve promenljive kojima je dodeljena vrednost y1 -> true
            Variables[VariablesMap[Instr.getOperand(0)]] = true;
          }
        }
        //ako nije u pitanju ni load ni store 
        else {
          int NumOfOperands = (int)Instr.getNumOperands();
          //prolazimo kroz sve promenljive u izrazu 
          for (int i = 0; i < NumOfOperands; i++) {
            //ako promenljiva nije ziva
            if (Variables.find(Instr.getOperand(i)) != Variables.end()) {
              //proglasi je zivom 
              Variables[Instr.getOperand(i)] = true;
              //proglasi zivom sve promenljive ciju vrednost ona cuva
              Variables[VariablesMap[Instr.getOperand(i)]] = true;
            }
          }
        }
      }
    }

    for (BasicBlock &BB : F) {
      for (Instruction &Instr : BB) {
        if (isa<StoreInst>(&Instr)) {
          //store y1 x, ako x nije ziva onda nam je ova store instrukcija beskorisna i sklanjamo je
          if (!Variables[Instr.getOperand(1)]) {
            InstructionsRemove.insert(&Instr);
          }
        }
        //ako je to load instrukcija koja je mrtva skloni je
        else if (Variables.find(&Instr) != Variables.end() && !Variables[&Instr]) {
          InstructionsRemove.insert(&Instr);
        }
      }
    }

    //brisemo...
    if (InstructionsRemove.size() > 0) {
      InstructionRemoved = true;
    }

    for (Instruction *Instr : InstructionsRemove) {
      Instr->eraseFromParent();
    }
  }

//samo se obidje dfs graf i pobrisu se nedostizne instrukcije
  void EliminateUnreachableInstructions(Function &F)
  {
    OurCFG *CFG = new OurCFG(F);

    CFG->TraverseGraph();

    std::vector<BasicBlock *> BasicBlocksToRemove;

    for(BasicBlock &BB : F) {
      if (!CFG->IsReachable(&BB)) {
        BasicBlocksToRemove.push_back(&BB);
      }
    }

    if (BasicBlocksToRemove.size() > 0) {
      InstructionRemoved = true;
    }

    for (BasicBlock *BasicBlockToRemove : BasicBlocksToRemove) {
      BasicBlockToRemove->eraseFromParent();
    }
  }

  bool runOnFunction(Function &F) override {
    do {
      InstructionRemoved = false;
      EliminateUnusedVariables(F);
      EliminateUnreachableInstructions(F);
    } while (InstructionRemoved);

    return true;
  }
};
}

char OurDeadCodeEliminationPass::ID = 0;
static RegisterPass<OurDeadCodeEliminationPass> X("dead-code-elimination", "Our dead code elimination pass");