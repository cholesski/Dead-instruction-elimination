add_llvm_library( LLVMOurDeadCodeElimination MODULE
  OurCFG.cpp
  OurDeadCodeEliminationPass.cpp
  OurDeadStoreEliminationPass.cpp

  PLUGIN_TOOL
  opt
  )

