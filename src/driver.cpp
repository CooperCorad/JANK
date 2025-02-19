#include <iostream>

#include <lexer.h>
#include <parser.h>
#include <typechecker.h>
#include <codegen.h>

using namespace std;
using namespace Lex;
using namespace Parse;
using namespace Typecheck;
using namespace Codegen;


unique_ptr<Lexer> throughLexer(string filename) {
    unique_ptr<Lexer> lex = make_unique<Lexer>(filename);
    lex->doLex();
    return std::move(lex);
}

unique_ptr<Parser> throughParser(string filename) {
    unique_ptr<Lexer> lex = throughLexer(filename);
    unique_ptr<Parser> par = make_unique<Parser>(lex->getTokens());
    try{
        par->doParse();
    } catch (ParseException &e) {
        cout << "Compilation failed! " << e.what() << endl;
        exit(-1);
    }
    return std::move(par);
}

unique_ptr<TypeChecker> throughTypeChecker(string filename) {
    unique_ptr<Parser> par = throughParser(filename);
    unique_ptr<TypeChecker> tyc = make_unique<TypeChecker>(par->getAst());
    tyc->globalSetup();
    try {
        tyc->doTypeCheck();
    } catch (TypeCheckException &e) {
        cout << "Compilation failed! " << e.what() << endl;
        exit(-1);
    }
    return std::move(tyc);
}

unique_ptr<CodeGen> throughCodeGen(string filename) {
    unique_ptr<TypeChecker> tyc = throughTypeChecker(filename);
    unique_ptr<CodeGen> cog = make_unique<CodeGen>(filename, tyc->getAst());
    //TODO: ...
    return nullptr;
}


int main(int argc, char **argv) {
    if (!strcmp(argv[1], "-l")){
        // Lexer lexMachine = Lexer(argv[2]);
        // lexMachine.doLex();
        unique_ptr<Lexer> lex = throughLexer(argv[2]);
        lex->prettyPrint();
    } else if (!strcmp(argv[1], "-h")) {
        string help_msg = 
        "./jank [stage flag] [optimizer level] [in filename] [out filename]\
        \n\tstage flags:\n\t\t -h -> help \n\t\t -l -> lex \n\t\t -p -> parse \n\t\t -c -> compile\
        \n\toptimizer level: -O[1-3]";
        cout << help_msg << endl;
    } else if (!strcmp(argv[1], "-p")) {
        // Lexer lexMachine = Lexer(argv[2]);
        // lexMachine.doLex();
        // Parser parseMachine = Parser(lexMachine.getTokens());
        // try{
        //     parseMachine.doParse();
        // } catch (ParseException &e) {
        //     cout << "Compilation failed! " << e.what() << endl;
        //     exit(-1);
        // }
        unique_ptr<Parser> par = throughParser(argv[2]);
        par->prettyPrint();
    } else if (!strcmp(argv[1], "-t")) {
        // Lexer lexMachine = Lexer(argv[2]);
        // lexMachine.doLex();
        // Parser parseMachine = Parser(lexMachine.getTokens());
        // try {
        //     parseMachine.doParse();
        // } catch (ParseException &e) {
        //     cout << "Compilation failed! " << e.what() << endl;
        //     exit(-1);
        // }
        // TypeChecker tcMachine = TypeChecker(parseMachine.getAst());
        // tcMachine.globalSetup();
        // try {
        //     tcMachine.doTypeCheck();
        // } catch (TypeCheckException &e) {
        //     cout << "Compilation failed! " << e.what() << endl;
        //     exit(-1);
        // }
        unique_ptr<TypeChecker> tyc = throughTypeChecker(argv[2]);
        tyc->prettyPrint();
    }


    return 0;
}