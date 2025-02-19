#include <codegen.h>
#include <typeinfo>

#include <llvm-c/TargetMachine.h>

using namespace Codegen;
using namespace Parse;
using namespace std;
using namespace llvm;

CodeGen::CodeGen(string filename, vector<shared_ptr<ASTNode>> astTree):
    Builder(*TheContext, llvm::ConstantFolder()),
    astTree(std::move(astTree))
{
    TheModule = make_unique<llvm::Module>(filename, *TheContext);
    // TheModule->setTargetTriple(LLVMGetDefaultTargetTriple()); //TODO: necessary? check src code?
    
}



Value *genCommand(shared_ptr<ASTNode> cmd) {
    if (auto letCmd = dynamic_pointer_cast<LetCmd>(cmd)) {
        
    }


    return nullptr;
}

Value *genExpr(shared_ptr<ASTNode> expr) {
    if (auto iExpr = dynamic_pointer_cast<IntExpr>(expr)) {

    }

    return nullptr;
}

