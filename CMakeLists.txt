add_llvm_library( LLVMOurDeadCodeElimination MODULE
  OurCFG.cpp
  OurDeadCodeEliminationPass.cpp

  PLUGIN_TOOL
  opt
  )
