#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/AssignmentTrackingAnalysis.h"
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
#include <unordered_set>
#include <bits/stdc++.h>

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
      //Instr.print(errs());
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


  void GlobalLivenessAnalysis(Function &F) {
    OurCFG *CFG = new OurCFG(F);
    CFG->CreateTransposeCFG(F);
    for (BasicBlock &BB : F){
      InitializeVariableSets(&BB);
      // BB.print(errs());
    }
    bool hasChanges = false;
    std::vector<BasicBlock*> reverseBB = CFG->GetTraverseOrder();
    // for (BasicBlock* BB : reverseBB){
    //   BB->print(errs());
    // }
    //ovo za sajt primer radi u 3 iteracije PROVERITI JEL TO OKEJ 
    do {
      hasChanges = false;
      for(BasicBlock *BB : reverseBB){
        //temporary variables 
        // BB->print(errs());
        std::set<Value*> top_ = *(new std::set<Value*>(top[BB]));
        std::set<Value*> bottom_ = *(new std::set<Value*>(bottom[BB]));

        for (auto it = succ_begin(BB), et = succ_end(BB); it != et; ++it){
          BasicBlock* successor = *it;
          for (Value* v : top[successor]){
            bottom[BB].insert(v);
          }
        }

        //Pomocni set je potreban zbog nacina kako funckionisu set_union itd.
        std::set<Value*> pomocni1;
        std::set_difference(bottom[BB].begin(),bottom[BB].end(),defVar[BB].begin(),defVar[BB].end(),std::inserter(pomocni1, pomocni1.begin()));
        std::set<Value*> pomocni2;
        std::set_union(usedVar[BB].begin(),usedVar[BB].end(),pomocni1.begin(),pomocni1.end(),std::inserter(pomocni2, pomocni2.begin()));
        top[BB]= pomocni2;

        //Demorgan !(stariB == noviB && stariT == noviT)
        if(top_ != top[BB] || bottom_ != bottom[BB])
        {
            hasChanges = true;
        }
      }
    } while (!hasChanges);
    // for(auto it : top)
    // {
    //   for (Value* v : it.second){
    //     errs() << v->getValueID() << " " << v->getValueName() << " " << v->getName();
    //     errs() << "\n";
    //   }
    //   errs() <<'\n';
    // }
  };

  //TODO SLEDECE :
  void EliminateUnusedVariables(Function &F)
  {
    for(BasicBlock &BB : F)
    {
      std::set<Value*> bottom_ = *(new std::set<Value*>(bottom[&BB]));// Onaj skup sto pise da je specificno za prog jezik cemo za svaki BB pojedinacno 
      //staviti na njegov out skup zato sto su to promenljive koje moraju biti zive posle njega
      for(BasicBlock::reverse_iterator In = (&BB)->rbegin(),InEnd = (&BB)->rend();In != InEnd; ++In)
      {
        Instruction *Instr = &*In;
        //Provera da li je instukcija binarni operator(+-*/...)
        if(auto Operacija = dyn_cast<BinaryOperator>(Instr))
        {
          Value *result = Operacija->getOperand(0);
          if(bottom_.find(result) != bottom_.end())//Provera da li se nalazi u dosadasnjem skupu promenljivih koje se koriste
          {
            bottom_.erase(result);//Ako se nalazi sklanjamo iz skupa a njegove operande dodajemo u skup koriscenjih(zivih) a = b + c (a sklanjamo , b i c dodajemo)
            // uz to da smo sigurni da b ili c nisu konstante jer konstante ne treba da ubacujemo u kod
            for(auto operand = Instr->op_begin();operand != Instr->op_end();++operand)
              {
                Value* var = *operand;
                if(!isa<Constant>(var))
                {
                  bottom_.insert(var);
                }
              }
          }else
            {
              Instr->eraseFromParent();
              continue;
            }
        }
        //Isto to za load instrukciju
        //Isto za store inst
        //Isto za call instrukcije

      }
    }
  }
  bool runOnFunction(Function &F)  {
    for (BasicBlock &BB : F) {
      // BB.print(errs());
      InitializeVariableSets(&BB);

    }
    GlobalLivenessAnalysis(F);
    // printMap();
    return false;
  }

};
}


char OurDeadStoreEliminationPass::ID = 1;
static RegisterPass<OurDeadStoreEliminationPass> X("dead-store-elimination", "Our dead store elimination pass");
