#ifndef __CODEGEN__
#define __CODEGEN__

#include <parser.h>

#include <string>
#include <vector>
#include <memory>

#include <llvm/IR/Module.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/ConstantFolder.h>
#include <llvm/TargetParser/Triple.h>


#include <map>

namespace Codegen
{
    class CodeGenerator {
        
        std::vector<std::shared_ptr<Parse::ASTNode>> astTree;

        llvm::LLVMContext TheContext;
        llvm::IRBuilder<> Builder;
        std::unique_ptr<llvm::Module> TheModule;
        std::map<std::string, llvm::Value *> NamedVals;

        llvm::Value *genCommand(std::shared_ptr<Parse::ASTNode>);

        llvm::Value *genExpr(std::shared_ptr<Parse::ASTNode>);
        llvm::Value *genBinopExpr(std::shared_ptr<Parse::BinopExpr>);
        llvm::Value *genUnopExpr(std::shared_ptr<Parse::UnopExpr>);

        llvm::Function *genFn(std::shared_ptr<Parse::FnCmd>);


        public:
            CodeGenerator(std::string filename, std::vector<std::shared_ptr<Parse::ASTNode>> astTree); //TODO: fleshout?
            void generateLLVM();
            void prettyprint();
    };
    

} // namespace Codegen


#endif 