#ifndef __TYPECHECK__
#define __TYPECHECK__

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <ranges>

namespace Parse {
    class ASTNode {
        public:
            ASTNode() = default;
            virtual std::string to_string() {return "";}
            virtual ~ASTNode() = default;
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

            virtual std::string to_string() {return "";}
            virtual bool equals (const std::shared_ptr<ResolvedType> other) {
                return false;
            }
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


    };


    class TypeChecker {
        private:
            std::vector<std::shared_ptr<Parse::ASTNode>> astTree;
            std::vector<std::vector<std::string>> operatorSplit;
            std::shared_ptr<ResolvedType> type_of(std::shared_ptr<Parse::ASTNode>);

            void type_cmd(std::shared_ptr<Parse::ASTNode>);

        public:
            TypeChecker(std::vector<std::shared_ptr<Parse::ASTNode>>);

            void prettyPrint();
            void doTypeCheck();

    };


    
} // namespace Typecheck

#endif