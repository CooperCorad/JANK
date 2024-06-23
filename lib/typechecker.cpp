#include "../include/typechecker.h"
#include "../include/parser.h"
using namespace Parse;
using namespace Typecheck;
using namespace std;

TypeCheckException::TypeCheckException(string msg) :
    message(msg) {};

string TypeCheckException::what() {
    return message;
}


TypeChecker::TypeChecker(std::vector<std::shared_ptr<ASTNode>> astTree) :
    astTree(std::move(astTree)) {}

void TypeChecker::prettyPrint() {
    for (const shared_ptr<ASTNode> &node : astTree) {
        cout << node->to_string() << endl;
    }
    cout << "Compilation succeeded: parsing complete\n";
}

void TypeChecker::doTypeCheck() {}