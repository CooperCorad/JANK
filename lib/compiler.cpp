#include <iostream>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/typechecker.h"


using namespace std;
using namespace Lex;
using namespace Parse;
using namespace Typecheck;

int main(int argc, char **argv) {
    if (!strcmp(argv[1], "-l")){
        Lexer lexMachine = Lexer(argv[2]);
        lexMachine.doLex();
        lexMachine.prettyPrint();
    } else if (!strcmp(argv[1], "-h")) {
        string help_msg = 
        "./jank [stage flag] [optimizer level] [in filename] [out filename]\
        \n\tstage flags:\n\t\t -h -> help \n\t\t -l -> lex \n\t\t -p -> parse \n\t\t -c -> compile\
        \n\toptimizer level: -O[1-3]";
        cout << help_msg << endl;
    } else if (!strcmp(argv[1], "-p")) {
        Lexer lexMachine = Lexer(argv[2]);
        lexMachine.doLex();
        Parser parseMachine = Parser(lexMachine.getTokens());
        try{
            parseMachine.doParse();
        } catch (ParseException &e) {
            cout << "Compilation failed! " << e.what() << endl;
            exit(-1);
        }
        parseMachine.prettyPrint();
    } else if (!strcmp(argv[1], "-t")) {
        Lexer lexMachine = Lexer(argv[2]);
        lexMachine.doLex();
        Parser parseMachine = Parser(lexMachine.getTokens());
        try {
            parseMachine.doParse();
        } catch (ParseException &e) {
            cout << "Compilation failed! " << e.what() << endl;
            exit(-1);
        }
        TypeChecker tcMachine = TypeChecker(parseMachine.getAst());
        try {
            tcMachine.doTypeCheck();
        } catch (TypeCheckException &e) {
            cout << "Compilation failed! " << e.what() << endl;
            exit(-1);
        }
        tcMachine.prettyPrint();
    }


    return 0;
}