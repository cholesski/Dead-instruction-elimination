add_llvm_library( LLVMOurDeadInstructionElimination MODULE
  OurCFG.cpp
  OurDeadInstructionEliminationPass.cpp

  PLUGIN_TOOL
  opt
  )
