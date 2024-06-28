#include "../include/parser.h"
#include "../include/typechecker.h"
using namespace Parse;
using namespace Typecheck;
using namespace SymTbl;
using namespace std;

TypeCheckException::TypeCheckException(string msg) :
    message(msg) {};

string TypeCheckException::what() {
    return message;
}


TypeChecker::TypeChecker(std::vector<std::shared_ptr<Parse::ASTNode>> astTree) :
    astTree(std::move(astTree)) {
        operatorSplit =   { {"<=", "<", ">=", ">", "==", "!="}, // num comparisons
                            {"||", "&&", "==", "!="} }; // bool comparisons
        globalTbl = make_shared<SymbolTable>();
}

void Typecheck::TypeChecker::globalSetup() {
    globalTbl->add("argnum", make_shared<VariableInfo>(make_shared<IntResolvedType>()));
    globalTbl->add("args", make_shared<VariableInfo>(make_shared<ArrayResolvedType>(make_shared<IntResolvedType>(), 1)));
}

void TypeChecker::prettyPrint() {
    for (const shared_ptr<ASTNode> &node : astTree) {
        cout << node->to_string() << endl;
    }
    cout << "Compilation succeeded: parsing complete\n";
}


void TypeChecker::doTypeCheck() {
    for (const auto &ast: astTree) {
        type_cmd(ast, globalTbl);
    }

}

void TypeChecker::type_cmd(std::shared_ptr<Parse::ASTNode> cmd, std::shared_ptr<SymTbl::SymbolTable> tbl) {
    if (shared_ptr<ShowCmd> sCmd = dynamic_pointer_cast<ShowCmd>(cmd)) {
        shared_ptr<ResolvedType> retTy = type_of(sCmd->expr, tbl);
        sCmd->expr->setResolvedType(std::move(retTy));
        // cout << retTy->to_string() << endl;
    } else if (shared_ptr<ReadCmd> rCmd = dynamic_pointer_cast<ReadCmd>(cmd)){
        vector<shared_ptr<ResolvedType>> tupVec = {make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>()};
        shared_ptr<VariableInfo> pict = make_shared<VariableInfo>(make_shared<ArrayResolvedType>(make_shared<TupleResolvedType>(tupVec), 2));
        if (shared_ptr<VarArgument> vArg = dynamic_pointer_cast<VarArgument>(rCmd->varArg)) {
            tbl->add(vArg->variable->name, pict);
        } else if (shared_ptr<ArrayArgument> arrArg = dynamic_pointer_cast<ArrayArgument>(rCmd->varArg)) {
            if (arrArg->vars.size() != 2) {
                throw TypeCheckException("You cannot bind a " + to_string(arrArg->vars.size()) + " sized argument to an image (2D Array)");
            }
            for (const auto &v: arrArg->vars) {
                tbl->add(v->name, make_shared<VariableInfo>(make_shared<IntResolvedType>()));
            }
            tbl->add(arrArg->var->name, pict);
        } else {
            throw TypeCheckException("You cannot bind an image to a " + rCmd->varArg->to_string());
        }      
    } else if (shared_ptr<WriteCmd> wCmd = dynamic_pointer_cast<WriteCmd>(cmd)) {
        shared_ptr<ResolvedType> retTy = type_of(wCmd->exp, tbl);
        vector<shared_ptr<ResolvedType>> tupVec = {make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>()};
        ArrayResolvedType pict = ArrayResolvedType(make_shared<TupleResolvedType>(tupVec), 2);
        if (!pict.equals(retTy)) {
            throw TypeCheckException("You cannot write in image not of the form {float, float, float, float}[,]");
        }
    } else if (shared_ptr<LetCmd> lCmd = dynamic_pointer_cast<LetCmd>(cmd)) {
        shared_ptr<ResolvedType> expTy = type_of(lCmd->expr, tbl);
        tbl->addLVal(lCmd->lval, expTy);

    } else if (shared_ptr<AssertCmd> aCmd = dynamic_pointer_cast<AssertCmd>(cmd)) {
        shared_ptr<BoolResolvedType> aExpr = dynamic_pointer_cast<BoolResolvedType>(type_of(aCmd->expr, tbl));
        if(!aExpr) {
            throw TypeCheckException("You cannot assert on a non boolean expression: " + aCmd->expr->to_string());
        }
    } else if (shared_ptr<TimeCmd> tCmd = dynamic_pointer_cast<TimeCmd>(cmd)) {
        type_cmd(tCmd->cmd, tbl);
    }
}

std::shared_ptr<ResolvedType> TypeChecker::type_of(std::shared_ptr<Parse::ASTNode> expr, std::shared_ptr<SymTbl::SymbolTable> tbl) {
    // literal exprs
    if(shared_ptr<IntExpr> iExpr = dynamic_pointer_cast<IntExpr>(expr)) {
        iExpr->setResolvedType(make_shared<IntResolvedType>());
        return make_shared<IntResolvedType>();
    } else if(shared_ptr<FloatExpr> fExpr = dynamic_pointer_cast<FloatExpr>(expr)) {
        fExpr->setResolvedType(make_shared<FloatResolvedType>());
        return make_shared<FloatResolvedType>();
    } else if(shared_ptr<TrueExpr> tExpr = dynamic_pointer_cast<TrueExpr>(expr)) {
        tExpr->setResolvedType(make_shared<BoolResolvedType>());
        return make_shared<BoolResolvedType>();
    } else if (shared_ptr<FalseExpr> fExpr = dynamic_pointer_cast<FalseExpr>(expr)) {
        fExpr->setResolvedType(make_shared<BoolResolvedType>());
        return make_shared<BoolResolvedType>();
    } else if (shared_ptr<TupleLiteralExpr> tExpr = dynamic_pointer_cast<TupleLiteralExpr>(expr)) {
        vector<shared_ptr<ResolvedType>> tys; 
        for (const auto &e: tExpr->exprs) { //TODO: memsafe action? VV all VV ??
            shared_ptr<ResolvedType> subTy = type_of(e, tbl);
            // cout << subTy->to_string() << endl;
            tys.push_back(subTy);
            // e->setResolvedType(std::move(subTy));
        }
        shared_ptr<TupleResolvedType> tTy = make_shared<TupleResolvedType>(tys);
        tExpr->setResolvedType(tTy);
        return tTy;
    } else if (shared_ptr<ArrayLiteralExpr> aExpr = dynamic_pointer_cast<ArrayLiteralExpr>(expr)) {
        shared_ptr<ResolvedType> fstTy = type_of(aExpr->exprs[0], tbl); //TODO: calls default??
        shared_ptr<ResolvedType> nxtTy;
        for (int i = 1; i < aExpr->exprs.size(); i++) {
            nxtTy = type_of(aExpr->exprs[i], tbl);
            if (!fstTy->equals(nxtTy)) {
                throw TypeCheckException("You cannot have incongruent types in array literals: " + aExpr->exprs[i]->to_string());
            }
            aExpr->exprs[i]->setResolvedType(nxtTy);
        }
        shared_ptr<ArrayResolvedType> arrTy = make_shared<ArrayResolvedType>(fstTy, 1);
        aExpr->setResolvedType(arrTy);
        return arrTy;
    }
    
    // operation exprs
    else if(shared_ptr<BinopExpr> bExpr = dynamic_pointer_cast<BinopExpr>(expr)) {
        shared_ptr<ResolvedType> lTy = type_of(bExpr->lExpr, tbl);
        shared_ptr<ResolvedType> rTy = type_of(bExpr->rExpr, tbl);

        if (lTy->equals(rTy)) {
            bExpr->lExpr->setResolvedType(lTy);
            bExpr->rExpr->setResolvedType(rTy);

            if (FloatResolvedType().equals(lTy) || IntResolvedType().equals(lTy)) {
                if (bExpr->op == "||" || bExpr->op == "&&") {
                    throw TypeCheckException("You cannot do a boolean operation (&&, ||) on non booleans");
                } else if (find(operatorSplit[0].begin(), operatorSplit[0].end(), bExpr->op) != operatorSplit[0].end()) {
                    lTy = make_shared<BoolResolvedType>();
                }
            } else if (BoolResolvedType().equals(lTy)) {
                if (find(operatorSplit[1].begin(), operatorSplit[1].end(), bExpr->op) == operatorSplit[1].end()) {
                    throw TypeCheckException("You cannot do a non boolean operation " + bExpr->op + " on booleans");
                } 
            } else if (!FloatResolvedType().equals(lTy) && !IntResolvedType().equals(lTy) && !BoolResolvedType().equals(lTy)) {
                throw TypeCheckException("You cannot do binary operations on " + lTy->to_string());
            }
            bExpr->setResolvedType(lTy);
            return lTy;
        } else {
            throw TypeCheckException("You cannot have a binary operation on incompatible types: " + lTy->to_string() + " " + bExpr->op \
                + " " + rTy->to_string());
        }
    } else if (shared_ptr<UnopExpr> uExpr = dynamic_pointer_cast<UnopExpr>(expr)) {
        shared_ptr<ResolvedType> subTy = type_of(uExpr->expr, tbl);
        
        if (uExpr->op == "!" && !subTy->equals(make_unique<BoolResolvedType>())) {
            throw TypeCheckException("You cannot do a boolean negation (!) on a non boolean expr: " + subTy->to_string());
        } else if (uExpr->op == "-" && !(subTy->equals(make_unique<IntResolvedType>()) || subTy->equals(make_unique<FloatResolvedType>()))) {
            throw TypeCheckException("You cannot do a mathematical negation (-) on a non mathematical expr: " + subTy->to_string());
        }
        // uExpr->expr->setResolvedType(subTy);
        uExpr->setResolvedType(subTy);
        return subTy;
    }

    // index exprs
    else if (shared_ptr<ArrayIndexExpr> iExpr = dynamic_pointer_cast<ArrayIndexExpr>(expr)) {
        shared_ptr<ArrayResolvedType> subTy = dynamic_pointer_cast<ArrayResolvedType>(type_of(iExpr->expr, tbl));
        if (!subTy) {
            throw TypeCheckException("You cannot access a non array: " + iExpr->expr->to_string() + " with array indexing");
        } else if (subTy->rank != iExpr->indices.size()) {
            throw TypeCheckException("You cannot access an array of rank " + to_string(iExpr->indices.size()) + " with "\
            + to_string(subTy->rank) + " indices");
        }

        vector<shared_ptr<ResolvedType>> indTys;
        for (const auto &e: iExpr->indices) {
            shared_ptr<ResolvedType> indTy = type_of(e, tbl);
            if (!indTy->equals(make_shared<IntResolvedType>())) {
                throw TypeCheckException("You cannot access an array with non integer indeces");
            }
            e->setResolvedType(indTy);
        }
        iExpr->expr->setResolvedType(subTy);
        iExpr->setResolvedType(subTy->ty);
        return subTy->ty;
    } else if (shared_ptr<TupleIndexExpr> iExpr = dynamic_pointer_cast<TupleIndexExpr>(expr)) {
        shared_ptr<TupleResolvedType> tTy = dynamic_pointer_cast<TupleResolvedType>(type_of(iExpr->expr, tbl));
        long long indx = stoll(iExpr->index);
        if(!tTy) {
            throw TypeCheckException("You cannot access a non tuple: " + iExpr->expr->to_string() + " with a tuple index expression");
        } else if (tTy->tys.size() - 1 < indx || indx < 0) {
            throw TypeCheckException("You cannot access a tuple of size " + to_string(tTy->tys.size()) + " out of bounds");
        } 
        // iExpr->expr->setResolvedType(tTy);
        iExpr->setResolvedType(tTy->tys[indx]); 
        return tTy->tys[indx];        
    }

    // compound types
    else if (shared_ptr<IfExpr> ifExpr = dynamic_pointer_cast<IfExpr>(expr)) {
        shared_ptr<BoolResolvedType> condTy = dynamic_pointer_cast<BoolResolvedType>(type_of(ifExpr->condExpr, tbl));
        if(!condTy) {
            throw TypeCheckException("You cannot execute and conditional on a non boolean: " + ifExpr->condExpr->to_string());
        }
        shared_ptr<ResolvedType> thnTy = type_of(ifExpr->thenExpr, tbl);
        shared_ptr<ResolvedType> elsTy = type_of(ifExpr->elseExpr, tbl);

        if (!thnTy->equals(elsTy)) {
            throw TypeCheckException("You cannot have mismatched type returns on conditional: " + ifExpr->to_string());
        }
        ifExpr->setResolvedType(thnTy);
        return thnTy;
    } else if (shared_ptr<ArrayLoopExpr> arrExpr = dynamic_pointer_cast<ArrayLoopExpr>(expr)) {
        if (!arrExpr->vars.size()) {
            throw TypeCheckException("You cannot create a 0 dimension array");
        }
        shared_ptr<SymbolTable> childTbl = tbl->makeChild();
        for (int i = 0; i < arrExpr->exprs.size(); i++) {
            shared_ptr<IntResolvedType> limTy = dynamic_pointer_cast<IntResolvedType>(type_of(arrExpr->exprs[i], childTbl));
            if (!limTy) {
                throw TypeCheckException("You cannot base the limits of a loop on a non integer expression");
            }
        }
        for (int i = 0; i < arrExpr->vars.size(); i++) {
            childTbl->add(arrExpr->vars[i]->name, make_shared<VariableInfo>(make_shared<IntResolvedType>()));
        }
        shared_ptr<ResolvedType> bdTy = type_of(arrExpr->body, childTbl);
        arrExpr->body->setResolvedType(bdTy);
        shared_ptr<ArrayResolvedType> expTy = make_unique<ArrayResolvedType>(bdTy, arrExpr->exprs.size());
        arrExpr->setResolvedType(expTy);
        return expTy;
    } else if (shared_ptr<SumLoopExpr> sumExpr = dynamic_pointer_cast<SumLoopExpr>(expr)) {
        if (!sumExpr->vars.size()) {
            throw TypeCheckException("You cannot create a 0 dimension array");
        }
        shared_ptr<SymbolTable> childTbl = tbl->makeChild();
        for (int i = 0; i < sumExpr->exprs.size(); i++) {
            shared_ptr<IntResolvedType> limTy = dynamic_pointer_cast<IntResolvedType>(type_of(sumExpr->exprs[i], childTbl));
            if (!limTy) {
                throw TypeCheckException("You cannot base the limits of a loop on a non integer expression");
            }
        }
        for (int i = 0; i < sumExpr->vars.size(); i++) {
            childTbl->add(sumExpr->vars[i]->name, make_shared<VariableInfo>(make_shared<IntResolvedType>()));
        }
        shared_ptr<ResolvedType> bdTy = type_of(sumExpr->body, childTbl);
        if (!dynamic_pointer_cast<IntResolvedType>(bdTy) && !dynamic_pointer_cast<FloatResolvedType>(bdTy)) {
            throw TypeCheckException("Sum loop bodies must always produce an integer or float, not " + bdTy->to_string());
        }
        sumExpr->setResolvedType(bdTy);
        return bdTy;
    }

    // variable handling
    else if (shared_ptr<VarExpr> vExpr = dynamic_pointer_cast<VarExpr>(expr)) {
        if (vExpr->var->name == "pict.") {
            vector<shared_ptr<ResolvedType>> tupVec = {make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>(), make_shared<FloatResolvedType>()};
            shared_ptr<ArrayResolvedType> pict = make_shared<ArrayResolvedType>(make_shared<TupleResolvedType>(tupVec), 2);
            vExpr->setResolvedType(pict);
            return pict;
        }
        if (tbl->has(vExpr->var->name)) {
            shared_ptr<VariableInfo> vInfo = dynamic_pointer_cast<VariableInfo>(tbl->get(vExpr->var->name));
            if (!vInfo) {
                throw TypeCheckException(vExpr->var->name + " is not a variable");
            }
            vExpr->setResolvedType(vInfo->ty);
            return vInfo->ty;
        } else {
            throw TypeCheckException(vExpr->var->name + " is an unbound variable in this scope");
        }
    }
    throw TypeCheckException("You've made an impossible situation, an object with no parse type");
}
