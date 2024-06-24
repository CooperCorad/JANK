#include "../include/parser.h"
#include "../include/typechecker.h"
using namespace Parse;
using namespace Typecheck;
using namespace std;

TypeCheckException::TypeCheckException(string msg) :
    message(msg) {};

string TypeCheckException::what() {
    return message;
}


TypeChecker::TypeChecker(std::vector<std::shared_ptr<Parse::ASTNode>> astTree) :
    astTree(std::move(astTree)) {
        operatorSplit =   { {"+", "-", "/", "%"},
                            {"<=", "<", ">=", ">"} };
    }

void TypeChecker::prettyPrint() {
    for (const shared_ptr<ASTNode> &node : astTree) {
        cout << node->to_string() << endl;
    }
    cout << "Compilation succeeded: parsing complete\n";
}

void TypeChecker::doTypeCheck() {
    for (const auto &ast: astTree) {
        type_cmd(ast);
    }

}

void TypeChecker::type_cmd(std::shared_ptr<Parse::ASTNode> cmd) {
    if (shared_ptr<ShowCmd> showCmd = dynamic_pointer_cast<ShowCmd>(cmd)) {
        shared_ptr<ResolvedType> retTy = type_of(showCmd->expr);
        showCmd->expr->setResolvedType(std::move(retTy));
        // cout << retTy->to_string() << endl;
    }


}


std::shared_ptr<ResolvedType> TypeChecker::type_of(std::shared_ptr<Parse::ASTNode> expr) {
    // literal and type exprs
    if(dynamic_pointer_cast<IntExpr>(expr) || dynamic_pointer_cast<IntType>(expr)) {
        return make_shared<IntResolvedType>();
    } else if(dynamic_pointer_cast<FloatExpr>(expr) || dynamic_pointer_cast<FloatType>(expr)) {
        return make_shared<FloatResolvedType>();
    } else if(dynamic_pointer_cast<TrueExpr>(expr) || dynamic_pointer_cast<FalseExpr>(expr) || 
                dynamic_pointer_cast<BoolType>(expr)) {
        return make_shared<BoolResolvedType>();
    } else if (shared_ptr<TupleLiteralExpr> tExpr = dynamic_pointer_cast<TupleLiteralExpr>(expr)) {
        vector<shared_ptr<ResolvedType>> tys; 
        for (const auto &e: tExpr->exprs) { //TODO: memsafe action? VV all VV ??
            shared_ptr<ResolvedType> subTy = type_of(e);
            // cout << subTy->to_string() << endl;
            tys.push_back(subTy);
            e->setResolvedType(std::move(subTy));
        }
        return make_shared<TupleResolvedType>(std::move(tys));
    } else if (shared_ptr<ArrayLiteralExpr> aExpr = dynamic_pointer_cast<ArrayLiteralExpr>(expr)) {
        shared_ptr<ResolvedType> fstTy = type_of(aExpr->exprs[0]); //TODO: calls default??
        shared_ptr<ResolvedType> nxtTy;
        for (int i = 1; i < aExpr->exprs.size(); i++) {
            nxtTy = type_of(aExpr->exprs[i]);
            if (!fstTy->equals(nxtTy)) {
                throw TypeCheckException("You cannot have incongruent types in array literals: " + aExpr->exprs[i]->to_string());
            }
            aExpr->exprs[i]->setResolvedType(nxtTy);
        }
        aExpr->exprs[0]->setResolvedType(fstTy);
        return make_shared<ArrayResolvedType>(fstTy, 1);
    }
    
    // operation exprs
    else if(shared_ptr<BinopExpr> bExpr = dynamic_pointer_cast<BinopExpr>(expr)) {
        shared_ptr<ResolvedType> lTy = type_of(bExpr->lExpr);
        shared_ptr<ResolvedType> rTy = type_of(bExpr->rExpr);

        if (lTy->equals(rTy)) {
            bExpr->lExpr->setResolvedType(lTy);
            bExpr->rExpr->setResolvedType(rTy);

            if (find(operatorSplit[0].begin(), operatorSplit[0].end(), bExpr->op) != operatorSplit[0].end()) {
                // does +, -, /, % | VV TOOD: may cause issues later with array access additions? VV
                if (!lTy->equals(make_unique<IntResolvedType>()) || lTy->equals(make_unique<FloatResolvedType>())) {
                    throw TypeCheckException("You cannot use the " + bExpr->op + " on type: " + lTy->to_string());
                }

                return lTy;
            } else if (lTy->equals(make_unique<BoolResolvedType>()) 
                        && find(operatorSplit[1].begin(), operatorSplit[1].end(), bExpr->op) != operatorSplit[1].end()) {
                //does <=, >=, <, >
                throw TypeCheckException("You cannot mathematically compare booleans!");
            } else {
                return make_unique<BoolResolvedType>();
            }
        } else {
            throw TypeCheckException("You cannot have a binary operation on incompatible types: " + lTy->to_string() + " " + bExpr->op \
                + " " + rTy->to_string());
        }
    } else if (shared_ptr<UnopExpr> uExpr = dynamic_pointer_cast<UnopExpr>(expr)) {
        shared_ptr<ResolvedType> subTy = type_of(uExpr->expr);
        
        if (uExpr->op == "!" && !subTy->equals(make_unique<BoolResolvedType>())) {
            throw TypeCheckException("You cannot do a boolean negation (!) on a non boolean expr: " + subTy->to_string());
        } else if (uExpr->op == "-" && !(subTy->equals(make_unique<IntResolvedType>()) || subTy->equals(make_unique<FloatResolvedType>()))) {
            throw TypeCheckException("You cannot do a mathematical negation (-) on a non mathematical expr: " + subTy->to_string());
        }
        uExpr->expr->setResolvedType(subTy);
        uExpr->setResolvedType(subTy);
        return subTy;
    }
}
