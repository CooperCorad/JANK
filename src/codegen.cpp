#include <codegen.h>
#include <typechecker.h>
#include <utils.h>

#include <typeinfo>
#include <iostream>
#include <string>


using namespace Codegen;
using namespace Parse;
using namespace Typecheck;

using namespace std;

using namespace llvm;

CodeGenerator::CodeGenerator(string filename, vector<shared_ptr<ASTNode>> astTree):
    Builder(TheContext, llvm::ConstantFolder()),
    astTree(std::move(astTree))
{
    TheModule = make_unique<llvm::Module>("<translation_unit>", TheContext);
    TheModule->setSourceFileName(filename);
    // TheModule->setTargetTriple(LLVMGetDefaultTargetTriple()); //TODO : necessary? check src code?
}

void CodeGenerator::prettyprint() {
    cout << "IM PRETTY!" << endl;
}



Value *CodeGenerator::genCommand(shared_ptr<ASTNode> cmd) {
    if (auto lCmd = dynamic_pointer_cast<LetCmd>(cmd)) {
        Value *expr = genExpr(lCmd->expr);
        // expr->setName(lCmd->lval->)
    }


    return nullptr;
}

Value *CodeGenerator::genExpr(shared_ptr<ASTNode> expr) {
    // literal expers
    if (auto iExpr = dynamic_pointer_cast<IntExpr>(expr)) {
        return ConstantInt::getSigned(Builder.getInt64Ty(), iExpr->value);
    } else if (auto fExpr = dynamic_pointer_cast<FloatExpr>(expr)) {
        return ConstantFP::get(Builder.getDoubleTy(), fExpr->value);
    } else if (auto tExpr = dynamic_pointer_cast<TrueExpr>(expr)) {
        return ConstantInt::get(Builder.getInt8Ty(), 1);
    } else if (auto fExpr = dynamic_pointer_cast<FalseExpr>(expr)) {
        return ConstantInt::get(Builder.getInt8Ty(), 0);
    } else if (auto tExpr = dynamic_pointer_cast<TupleLiteralExpr>(expr)) {
        //TODO: Tuple Construction
        vector<llvm::Type *> tys;
        vector<Constant *> exps;
        for (const auto &e: tExpr->exprs) {
            Value *genE = genExpr(e);
            tys.push_back(genE->getType());
            exps.push_back(static_cast<Constant *>(genE)); //TODO: UHHHHH???
        }
        return ConstantStruct::get(StructType::create(tys), exps);
    } 

    // binary operation exprs
    else if (auto bExpr = dynamic_pointer_cast<BinopExpr>(expr)) {
        return genBinopExpr(bExpr);
    }
    //unary operation exprs
    else if (auto uExpr = dynamic_pointer_cast<UnopExpr>(expr)) {
        return genUnopExpr(uExpr);
    }

    return nullptr;
}

Value *CodeGenerator::genBinopExpr(shared_ptr<BinopExpr> bExpr) {
    Value *lExpr = genExpr(bExpr->lExpr);
    Value *rExpr = genExpr(bExpr->rExpr);

    const char* op = bExpr->op.c_str();
    int opTy = utils::BOOL;
    if (auto _ = dynamic_pointer_cast<FloatType>(bExpr->ty)) {
        opTy = utils::FLOAT;
    } else if (auto _ = dynamic_pointer_cast<IntType>(bExpr->ty)) {
        opTy = utils::INTEGER;
    }

    if (!strcmp(op, "+")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateAdd(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFAdd(lExpr, rExpr);
        }
    } else if (!strcmp(op, "-")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateSub(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFSub(lExpr, rExpr);
        }
    } else if (!strcmp(op, "*")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateMul(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFMul(lExpr, rExpr);
        }
    } else if (!strcmp(op, "/")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateSDiv(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFDiv(lExpr, rExpr);
        }
    } else if (!strcmp(op, "%")) { //TODO: Long term expand to floats?? -> PL Q first
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateSRem(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFRem(lExpr, rExpr);
        }
    } else if (!strcmp(op, ">")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateICmpSGT(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFCmpOGT(lExpr, rExpr);
        }
    } else if (!strcmp(op, ">=")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateICmpSGE(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFCmpOGE(lExpr, rExpr);
        }
    } else if (!strcmp(op, "<")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateICmpSLT(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFCmpOLT(lExpr, rExpr);
        }
    } else if (!strcmp(op, "<=")) {
        switch (opTy) {
            case utils::INTEGER:
                return Builder.CreateICmpSLE(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFCmpOLE(lExpr, rExpr);
        }
    } else if (!strcmp(op, "==")) {
        switch (opTy) {
            case utils::INTEGER: case utils::BOOL: //TODO: works??
                return Builder.CreateICmpEQ(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFCmpOEQ(lExpr, rExpr);
        }
    } else if (!strcmp(op, "!=")) {
        switch (opTy) {
            case utils::INTEGER: case utils::BOOL:
                return Builder.CreateICmpNE(lExpr, rExpr);
            case utils::FLOAT:
                return Builder.CreateFCmpONE(lExpr, rExpr);
        }
    } else if (!strcmp(op, "&&")) { //TODO:PHI node fuckery
        
    } else if (!strcmp(op, "||")) {
        
    }

    return nullptr;
}

Value *CodeGenerator::genUnopExpr(shared_ptr<UnopExpr> uExpr) {
    Value *subExpr = genExpr(uExpr->expr);

    if (uExpr->ty->equals(make_unique<FloatResolvedType>())) { // (float)
        // return Builder.creat
    } else { // (bool or int)
        return Builder.CreateXor(subExpr, 1); //TODO: ?????
    }
    return nullptr;
}


Function *genFn(shared_ptr<FnCmd> fnCmd) {
    
    return nullptr;
}