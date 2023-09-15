add_llvm_library( LLVMOurDeadCodeElimination MODULE
  OurCFG.cpp
  OurDeadCodeEliminationPass.cpp
  OurDeadInstructionEliminationPass.cpp

  PLUGIN_TOOL
  opt
  )


