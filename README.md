JANK (Just A Nother Kompiler(Compiler))
==========================

C++ based compiler utilizing LLVMIR to compile JPL (JohnPavelLang - University of Utah) 
 - currently focused on converting to SSA style LLVMIR
 - planning on {register allocation, CSE, peephole, more(?)} optimizations

Notes: 
 - cutting a mild amount of tuple functionality (namely tuple fn params) until i better understand LLVM IR better
 - similarly things like `let {a, b[c]} = {1, [2, 3, 4]}` are suspended for now 
 I hope to return to these features eventually but they're secondary to the main goals of the project