#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/AssignmentTrackingAnalysis.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorOr.h"
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
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <bits/stdc++.h>

using namespace llvm;

namespace {
struct OurDeadInstructionEliminationPass : public FunctionPass {
  bool InstructionRemoved;

  static char ID;
  OurDeadInstructionEliminationPass() : FunctionPass(ID) {}

  std::unordered_map<BasicBlock*, std::set<Value*>> defVar;
  std::unordered_map<BasicBlock*, std::set<Value*>> usedVar;
  std::unordered_map<BasicBlock*, std::set<Value*>> top; //skup promenljivih koje su zive na izlazu iz basic bloka
  std::unordered_map<BasicBlock*, std::set<Value*>> bottom; //skup promenljivih koje su zive na ulazu u basic blok 

  //inicijalizuje defVar i usedVar skupove za BasicBlock
  void InitializeVariableSets(BasicBlock* Current){
    for (Instruction &Instr : *Current) {
        if (isa<LoadInst>(&Instr)) {
          defVar[Current].insert(&Instr);
          if (defVar[Current].find(Instr.getOperand(0)) == defVar[Current].end()){
            usedVar[Current].insert(Instr.getOperand(0));
          }
        }
        else if (isa<StoreInst>(&Instr)) {
          if (defVar[Current].find(Instr.getOperand(0)) == defVar[Current].end()){
            usedVar[Current].insert(Instr.getOperand(0));
          }
          defVar[Current].insert(Instr.getOperand(1));
        }
        else {
          int NumOfOperands = (int)Instr.getNumOperands();
          defVar[Current].insert(&Instr);
          for (int i = 0; i < NumOfOperands; i++) {
            if (defVar[Current].find(Instr.getOperand(i)) == defVar[Current].end()){
              usedVar[Current].insert(Instr.getOperand(i));
            }
          }
        }
    }
  }

  void GlobalLivenessAnalysis(Function &F) {
    OurCFG *CFG = new OurCFG(F);
    CFG->CreateTransposeCFG(F);
    for (BasicBlock &BB : F){
      InitializeVariableSets(&BB);
    }
    bool hasChanges = false;
    std::vector<BasicBlock*> reverseBB = CFG->GetTraverseOrder();
    do {
      hasChanges = false;
      for(BasicBlock *BB : reverseBB){
        //temporary variables 
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
    } while (hasChanges);
  };

  /*ovaj tvoj kod sam samo promenila ime funkcije nista nisam dirala*/
  void EliminateUnusedVariables(Function &F)
  {
    std::set<Instruction*> brisanje;
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
          Value *result = Operacija;
          if(bottom_.find(result) != bottom_.end())//Provera da li se nalazi u dosadasnjem skupu promenljivih koje se koriste
          {
            bottom_.erase(result);//Ako se nalazi sklanjamo iz skupa a njegove operande dodajemo u skup koriscenjih(zivih) a = b + c (a sklanjamo , b i c dodajemo)
            // uz to da smo sigurni da b ili c nisu konstante jer konstante ne treba da ubacujemo u kod
            for(auto operand = Instr->op_begin();operand != Instr->op_end();++operand)
              {
                Value* var = *operand;
                if(!isa<Constant>(var))
                {
                  bottom_.insert(var);//Dodavanje u skup zivih
                }
              }
          }else
            {
              brisanje.insert(Instr);//Dodavanje u skup za brisanje
              continue;
            }
        }
        //Isto to za load instrukciju
        else if(auto load = dyn_cast<LoadInst>(Instr))
        {
          Value *result = load;
          if(bottom_.find(result) != bottom_.end())//Provera da li se nalazi u dosadasnjem skupu promenljivih koje se koriste
          {
            bottom_.erase(result);
            Value *operand = load->getPointerOperand();
            if(!isa<Constant>(operand))//Provera da li je operand variabla a ne nesto poput konstante
            {
              bottom_.insert(operand);//Dodavanje u skup zivih
            }
          }else
            {
              brisanje.insert(Instr);//Dodavanje u skup za brisanje
              continue;
            }
        }
        //Isto za store inst
        else if(auto store = dyn_cast<StoreInst>(Instr))
        {
          Value *result = store->getPointerOperand();
          if(bottom_.find(result) != bottom_.end())//Provera da li se nalazi u dosadasnjem skupu promenljivih koje se koriste
          {
            bottom_.erase(result);
            Value *operand = store->getValueOperand();
            if(!isa<Constant>(operand))//Provera da li je operand variabla a ne nesto poput konstante
            {
              bottom_.insert(operand);//Dodavanje u skup zivih
            }
          }else
            {
              brisanje.insert(Instr);//Dodavanje u skup za brisanje
              continue;
            }
        }
        //Posto call instrukcija ne moze da se brise zato sto ne znamo da li funckija ima bocne efekte onda samo dodajemo operande u skup
        else if(auto call = dyn_cast<CallInst>(Instr))
        {

          for(auto arg=call->arg_begin(),argE = call->arg_end();arg!=argE;++arg){
            Value* operand = *arg;
            if(!isa<Constant>(operand))//Provera da li je operand variabla a ne nesto poput konstante
            {
              bottom_.insert(operand);//Dodavanje u skup zivih
            }
          }
        }else //OSTALE INSTRUKCIJE Posto su to specifcne instrukcije ne znam da li smemo da ih brisemo tako da samo dodajemo operande u skup
        {
          for(auto it = Instr->op_begin(),e = Instr->op_end(); it!=e;++it)
          {
            Value *operand = *it;
            bottom_.insert(operand);
          }
        }

      }
    }
    for(auto I : brisanje)//Brisanje iz skupa za brisanje
    {
      I->eraseFromParent();
    }
  }

  //brise nedostizne basic blokove
  void EliminateUnreachableInstructions(Function &F){
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

  bool runOnFunction(Function &F)  {
    GlobalLivenessAnalysis(F);
    EliminateUnusedVariables(F);
    EliminateUnreachableInstructions(F);
    return true;
  }

};
}


char OurDeadInstructionEliminationPass::ID = 1;
static RegisterPass<OurDeadInstructionEliminationPass> X("dead-instruction-elimination", "Our dead instruction elimination pass");

