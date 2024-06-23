#ifndef _TYPECHECK_
#define _TYPECHECK_

#include <string>
#include <vector>
#include <algorithm>
#include "parser.h" 

// class ASTNode;

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

            virtual std::string to_string();
            virtual bool equals (const std::shared_ptr<ResolvedType> other);
    };

    class IntResolvedType : public ResolvedType {
        public:
            IntResolvedType() = default;

            std::string to_string() override {
                return "(IntType)";
            }

            bool equals (const std::shared_ptr<ResolvedType> other) override {
                return dynamic_cast<IntResolvedType*>(other.get()) != NULL;
            }
    };

    class FloatResolvedType : public ResolvedType {
        public:
            FloatResolvedType() = default;

            std::string to_string() override {
                return "(FloatType)";
            }
    };

    class BoolResolvedType : public ResolvedType {
        public:
            BoolResolvedType() = default;

            std::string to_string() override {
                return "(BoolType)";
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
    };

    class ArrayResolvedType : public ResolvedType {
        public:
            std::shared_ptr<ResolvedType> ty;
            
            ArrayResolvedType(std::shared_ptr<ResolvedType> ty) :
                ty(std::move(ty)) {}

            std::string to_string() override {
                return "(ArrayResolvedType " + ty->to_string() + ")";
            }
    };


    class TypeChecker {
        private:
            std::vector<std::shared_ptr<Parse::ASTNode>> astTree;
            
        public:
            TypeChecker(std::vector<std::shared_ptr<Parse::ASTNode>>);

            void prettyPrint();
            void doTypeCheck();

    };


    
} // namespace Typecheck

#endif