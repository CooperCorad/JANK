#ifndef __TYPECHECK__
#define __TYPECHECK__

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>

namespace Parse {
    class ASTNode {
        public:
            ASTNode() = default;
            virtual std::string to_string() {return "";}
            virtual ~ASTNode() = default;
    };


    class Variable : public ASTNode {
        public:
            std::string name;
            Variable(std::string name) : name(name) {}
            ~Variable() override = default;
            std::string to_string() override { return name; }
    };


    class Argument : public ASTNode {};

    class VarArgument : public Argument {
        public:
            std::shared_ptr<Variable> variable;
            VarArgument(std::unique_ptr<Variable> variable) :
                variable(std::move(variable)) {}
            ~VarArgument() override = default;
            std::string to_string() override {
                return "(VarArgument " + variable->to_string() + ")"; 
            }

    };
    class ArgLValue : public Argument {
        public:
            std::shared_ptr<Argument> varArg;
            ArgLValue(std::unique_ptr<Argument> varArg) :
                varArg(std::move(varArg)) {}
            ~ArgLValue() = default;
            std::string to_string() override {
                return "(ArgLValue " + varArg->to_string() + ")";
            }
    }; // TODO: move to LValue class?
    class TupleLValue : public Argument {
        public:
            std::vector<std::shared_ptr<Argument>> args;

            TupleLValue(std::vector<std::unique_ptr<Argument>> inArgs) {
                args.insert(args.end(),
                    std::make_move_iterator(inArgs.begin()),
                    std::make_move_iterator(inArgs.end()));
            }
            ~TupleLValue() = default;

            std::string to_string() override {
                std::string str = "(TupleLValue";
                for(const auto &a: args) {
                    str += " " + a->to_string();
                }
                return str + ")";
            }
    };
    class ArrayArgument : public Argument {
        public:
            std::shared_ptr<Variable> var;
            std::vector<std::shared_ptr<Variable>> vars;

            ArrayArgument(std::unique_ptr<Variable> var, std::vector<std::unique_ptr<Variable>> inVars) :
                var(std::move(var)) {
                    vars.insert(vars.end(),
                    std::make_move_iterator(inVars.begin()),
                    std::make_move_iterator(inVars.end()));
                }
            ~ArrayArgument() = default;

            std::string to_string() override {
                std::string str = "(ArrayArgument " + var->to_string();
                for (const auto &v: vars) {
                    str += " " + v->to_string();
                }
                return str + ")";
            }
    };
}

namespace Typecheck
{
    class TypeCheckException : public std::exception {
        using std::exception::what;
        private:
            std::string message;
        public:
            TypeCheckException(std::string);
            std::string what();
    };

    class ResolvedType {
        public:
            ResolvedType() = default;
            virtual ~ResolvedType() = default;

            virtual std::string to_string() { return ""; }
            virtual bool equals (const std::shared_ptr<ResolvedType> other) {
                return false;
            }
            virtual std::shared_ptr<ResolvedType> clone() { return NULL; }
    };

    class IntResolvedType : public ResolvedType {
        public:
            IntResolvedType() = default;

            std::string to_string() override {
                return "(IntType)";
            }

            bool equals (const std::shared_ptr<ResolvedType> other) override {
                return std::dynamic_pointer_cast<IntResolvedType>(other) != NULL;
            }

            std::shared_ptr<ResolvedType> clone() override {
                return std::make_unique<IntResolvedType>();
            }
    };

    class FloatResolvedType : public ResolvedType {
        public:
            FloatResolvedType() = default;

            std::string to_string() override {
                return "(FloatType)";
            }

            bool equals (const std::shared_ptr<ResolvedType> other) override {
                return std::dynamic_pointer_cast<FloatResolvedType>(other) != NULL;
            }
            std::shared_ptr<ResolvedType> clone() override {
                return std::make_unique<FloatResolvedType>();
            }

    };

    class BoolResolvedType : public ResolvedType {
        public:
            BoolResolvedType() = default;

            std::string to_string() override {
                return "(BoolType)";
            }

            bool equals (const std::shared_ptr<ResolvedType> other) override {
                return std::dynamic_pointer_cast<BoolResolvedType>(other) != NULL;
            }
            std::shared_ptr<ResolvedType> clone() override {
                return std::make_unique<BoolResolvedType>();
            }

    };

    class TupleResolvedType : public ResolvedType {
        public:
            std::vector<std::shared_ptr<ResolvedType>> tys;

            TupleResolvedType(std::vector<std::shared_ptr<ResolvedType>> tys) :
                tys(std::move(tys)) {}

            std::string to_string() override {
                std::string str = "(TupleType";
                for (const auto &e: tys) {
                    str += " " + e->to_string();
                }
                return str + ")";
            }

            bool equals (const std::shared_ptr<ResolvedType> other) override {
                std::shared_ptr<TupleResolvedType> otherTy = std::dynamic_pointer_cast<TupleResolvedType>(other);
                if (!(otherTy) || tys.size() != otherTy->tys.size()) {
                    return false;
                }
                for (int i = 0; i < tys.size(); i++) {
                    if (!tys[i]->equals(otherTy->tys[i])) {
                        return false;
                    }
                }
                return true;   
            }
            std::shared_ptr<ResolvedType> clone() override {
                std::vector<std::shared_ptr<ResolvedType>> cTys;
                for (const auto &t: tys) {
                    cTys.push_back(t->clone());
                }
                return std::make_unique<TupleResolvedType>(cTys);
            }

    };

    class ArrayResolvedType : public ResolvedType {
        public:
            std::shared_ptr<ResolvedType> ty;
            long long rank;
            
            ArrayResolvedType(std::shared_ptr<ResolvedType> ty, long long rank) :
                ty(std::move(ty)), rank(rank) {}

            std::string to_string() override {
                return "(ArrayType " + ty->to_string() + " " + std::to_string(rank) + ")";
            }

            bool equals (const std::shared_ptr<ResolvedType> other) override {
                std::shared_ptr<ArrayResolvedType> otherTy = std::dynamic_pointer_cast<ArrayResolvedType>(other);
                if (!(otherTy)) {
                    return false;
                } 
                return ty->equals(otherTy->ty);
            }
            std::shared_ptr<ResolvedType> clone() override {
                return std::make_unique<ArrayResolvedType>(ty->clone(), rank);
            }
    };

} // namespace Typecheck


namespace SymTbl
{
    class NameInfo {
        public:
            NameInfo() = default;
            virtual ~NameInfo() = default;
    };

    class VariableInfo : public NameInfo { 
        public:
            std::shared_ptr<Typecheck::ResolvedType> ty;

            VariableInfo(std::shared_ptr<Typecheck::ResolvedType> ty) :
                ty(ty) {}
            ~VariableInfo() = default;
    };

    class SymbolTable {
        private:
            std::unordered_map<std::string, std::shared_ptr<NameInfo>> tbl;
            std::shared_ptr<SymbolTable> parent;
        public:
            SymbolTable() {
                parent = NULL;
            }
            ~SymbolTable() = default;

            std::shared_ptr<SymbolTable> makeChild() {
                std::shared_ptr<SymbolTable> child = std::make_shared<SymbolTable>();
                child->parent = std::make_shared<SymbolTable>(*this); //TODO ???? works?
                return child;
            }

            bool has(std::string name) {
                if (tbl.contains(name)) {
                    return true;
                } else if (parent == NULL) {
                    return false;
                } else {
                    return parent->has(name);
                }
            }

            void add(std::string name, std::shared_ptr<NameInfo> info) {
                if (has(name)) {
                    throw Typecheck::TypeCheckException("You cannot redefine " + name + " in this context");
                }
                tbl.insert({name, info});
            }

            std::shared_ptr<NameInfo> get(std::string name) {
                if (has(name)) {
                    return tbl.at(name);
                } else if (parent == NULL) {
                    return NULL;
                } else {
                    return parent->get(name);
                }
            }

            void addArg(std::shared_ptr<Parse::Argument> arg, std::shared_ptr<Typecheck::ResolvedType> ty) {
                if (std::shared_ptr<Parse::VarArgument> vArg = std::dynamic_pointer_cast<Parse::VarArgument>(arg)) {
                    add(vArg->variable->name, std::make_unique<VariableInfo>(ty));
                } else if (std::shared_ptr<Parse::ArrayArgument> arrArg = std::dynamic_pointer_cast<Parse::ArrayArgument>(arg)) {
                    if (std::shared_ptr<Typecheck::ArrayResolvedType> arrTy = std::dynamic_pointer_cast<Typecheck::ArrayResolvedType>(ty)) {
                        if (arrTy->rank != arrArg->vars.size()) {
                            throw Typecheck::TypeCheckException("You cannot bind an array of rank " + std::to_string(arrArg->vars.size()) + \
                            " to an binding of rank " + std::to_string(arrTy->rank));
                        }
                        add(arrArg->var->name, std::make_shared<VariableInfo>(ty));
                    } 
                    for (const auto &a: arrArg->vars) {
                        add(a->name, std::make_shared<VariableInfo>(std::make_shared<Typecheck::IntResolvedType>()));
                    }
                }
            }

            void addLVal(std::shared_ptr<Parse::Argument> arg, std::shared_ptr<Typecheck::ResolvedType> ty) {
                if (std::shared_ptr<Parse::TupleLValue> tArg = std::dynamic_pointer_cast<Parse::TupleLValue>(arg)) {
                    std::shared_ptr<Typecheck::TupleResolvedType> tTy = std::dynamic_pointer_cast<Typecheck::TupleResolvedType>(ty);
                    if (!tTy) {
                        throw Typecheck::TypeCheckException("You cannot bind a non tuple type " + ty->to_string() + " to a tuple!");
                    } else if (tTy->tys.size() != tArg->args.size()) {
                        throw Typecheck::TypeCheckException("You cannot bind a tuple of unequal rank!");
                    }
                    for (int i = 0; i < tTy->tys.size(); i++) {
                        addLVal(tArg->args[i], tTy->tys[i]);
                    }
                 } else if (std::shared_ptr<Parse::ArgLValue> lArg = std::dynamic_pointer_cast<Parse::ArgLValue>(arg)) {
                    addArg(lArg->varArg, ty);
                 } else {
                    throw Typecheck::TypeCheckException("You cannot add a non lVal");
                 }
            }

    };
    
} // namespace SymbolTable


namespace Typecheck
{

    class TypeChecker {
        private:
            std::vector<std::shared_ptr<Parse::ASTNode>> astTree;
            std::vector<std::vector<std::string>> operatorSplit;
            std::shared_ptr<SymTbl::SymbolTable> globalTbl;

            std::shared_ptr<ResolvedType> type_of(std::shared_ptr<Parse::ASTNode>, std::shared_ptr<SymTbl::SymbolTable>);
            void type_cmd(std::shared_ptr<Parse::ASTNode>, std::shared_ptr<SymTbl::SymbolTable>);

        public:
            TypeChecker(std::vector<std::shared_ptr<Parse::ASTNode>>);

            void prettyPrint();
            void doTypeCheck();

    };


    
} // namespace Typecheck

#endif