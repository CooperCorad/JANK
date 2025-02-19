#ifndef __CODEGEN__
#define __CODEGEN__

#include <parser.h>

#include <string>
#include <vector>
#include <memory>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/ConstantFolder.h>
#include <llvm/TargetParser/Triple.h>


#include <map>

namespace Codegen
{
    class CodeGen {
        
        std::vector<std::shared_ptr<Parse::ASTNode>> astTree;

        std::unique_ptr<llvm::LLVMContext> TheContext;
        llvm::IRBuilder<> Builder;
        std::unique_ptr<llvm::Module> TheModule;
        std::map<std::string, llvm::Value *> NamedVals;

        llvm::Value *genCommand(std::shared_ptr<Parse::ASTNode>);

        llvm::Value *genExpr(std::shared_ptr<Parse::ASTNode>);
        

        public:
            CodeGen(std::string filename, std::vector<std::shared_ptr<Parse::ASTNode>> astTree); //TODO: fleshout?
            void generateLLVM();
    };
    

} // namespace Codegen


#endif 