#include <typeinfo>
#include "../include/parser.h"
// #include "../include/ast.h"
using namespace Parse;
using namespace Lex;
using namespace std;

ParseException::ParseException(string msg) :
    message(msg) {};

string ParseException::what() {
    return message;
}


Parser::Parser(std::vector<std::unique_ptr<Lex::Token>> tokens) :
    tokens(std::move(tokens)) {
        precedence = {  {"array", "sum", "if"},
                        {"&&", "||"},
                        {"<", "<=", "==", "!=", ">=", ">"},
                        {"+", "-"},
                        {"*", "/", "%"},
                        {"!", "-"},
                        {"{", "["}
                     };
    }

string Parser::expectToken(int *pos, Tokty tok) {
    if (tokens[*pos]->tokty != tok) {
        throw ParseException("incorrect token, needed: " + revTbl.at(tok) + " got: " + revTbl.at(tokens[*pos]->tokty) + " at: " + to_string(tokens[*pos]->loc).c_str());
    } 
    return tokens[(*pos)++]->text; 
}

Tokty Parser::peekToken(int pos) {
    return tokens[pos]->tokty;
}

void Parser::prettyPrint() {
    for (const shared_ptr<ASTNode> &node : astTree) {
        cout << node->to_string() << endl;
    }
    cout << "Compilation succeeded: parsing complete\n";
}

std::vector<std::shared_ptr<Parse::ASTNode>> Parser::getAst() {
    return (std::move(astTree));
}

void Parser::doParse() {
    int pos = 0;
    if (peekToken(pos) == NEWLINE) {
        pos++;
    }
    while (1) {
        if (peekToken(pos) == END_OF_FILE){
            return;
        }
        unique_ptr<ASTNode> command;
        tie(command, pos) = parseCmd(pos);
        astTree.push_back(std::move(command));
        expectToken(&pos, NEWLINE);
    }
}

pair<unique_ptr<Cmd>, int> Parser::parseCmd(int pos) {
    Tokty cmdTok = peekToken(pos);
    unique_ptr<Cmd> cmd;

    switch (cmdTok) {
        case READ:
            return parseReadCmd(pos);
        case WRITE:
            return parseWriteCmd(pos);
        case TYPE:
            return parseTypeCmd(pos);
        case LET:
            return parseLetCmd(pos);
        case ASSERT:
            return parseAssertCmd(pos);
        case PRINT:
            return parsePrintCmd(pos);
        case SHOW:
            return parseShowCmd(pos);
        case TIME:
            return parseTimeCmd(pos);
        case FN:
            return parseFnCmd(pos);
        default: 
            throw ParseException(("No command found at " + to_string(tokens[pos]->loc) + " (" + tokens[pos]->text + ")").c_str());
    }
}


std::pair<std::unique_ptr<Cmd>, int> Parser::parseReadCmd(int pos) {
    expectToken(&pos, READ);
    expectToken(&pos, IMAGE);
    string file = expectToken(&pos, STRING);
    expectToken(&pos, TO);
    unique_ptr<Argument> arg;
    tie(arg, pos) = parseArgument(pos);
    return make_tuple(make_unique<ReadCmd>(file, std::move(arg)), pos);
}

std::pair<std::unique_ptr<Cmd>, int> Parser::parseWriteCmd(int pos) {
    expectToken(&pos, WRITE);
    expectToken(&pos, IMAGE);
    unique_ptr<Expr> exp;
    tie(exp, pos) = parseExpr(pos);
    expectToken(&pos, TO);
    string str = expectToken(&pos, STRING);
    return make_tuple(make_unique<WriteCmd>(std::move(exp), str), pos);
}


std::pair<std::unique_ptr<Cmd>, int> Parser::parseTypeCmd(int pos) {
    expectToken(&pos, TYPE);
    unique_ptr<Variable> var;
    tie(var, pos) = parseVariable(pos);
    expectToken(&pos, EQUALS);
    unique_ptr<Type> ty;
    tie(ty, pos) = parseType(pos);
    return make_tuple(make_unique<TypeCmd>(std::move(var), std::move(ty)), pos);
}

std::pair<std::unique_ptr<Cmd>, int> Parser::parseLetCmd(int pos) {
    expectToken(&pos, LET);
    bool isMutable = (peekToken(pos) == MUTABLE);
    
    unique_ptr<Argument> lval;
    tie(lval, pos) = parseLValue(pos);
    expectToken(&pos, EQUALS);
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseExpr(pos);
    return make_tuple(make_unique<LetCmd>(std::move(lval), std::move(expr)), pos);
}

std::pair<std::unique_ptr<Cmd>, int> Parser::parseAssertCmd(int pos) {
    expectToken(&pos, ASSERT);
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseExpr(pos);
    expectToken(&pos, COMMA);
    string str = expectToken(&pos, STRING);
    return make_tuple(make_unique<AssertCmd>(std::move(expr), str), pos);
}

std::pair<std::unique_ptr<Cmd>, int> Parser::parsePrintCmd(int pos) {
    expectToken(&pos, PRINT);
    string str = expectToken(&pos, STRING);
    return make_pair(make_unique<PrintCmd>(str), pos);
}

std::pair<std::unique_ptr<Cmd>, int> Parser::parseShowCmd(int pos) {
    expectToken(&pos, SHOW);
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseExpr(pos);
    return make_pair(make_unique<ShowCmd>(std::move(expr)), pos);
}

std::pair<std::unique_ptr<Cmd>, int> Parser::parseTimeCmd(int pos) {
    expectToken(&pos, TIME);
    unique_ptr<Cmd> cmd;
    tie(cmd, pos) = parseCmd(pos);
    return make_pair(make_unique<TimeCmd>(std::move(cmd)), pos);
}

std::pair<std::unique_ptr<Cmd>, int> Parser::parseFnCmd(int pos) {
    unique_ptr<Variable> var;
    vector<unique_ptr<Binding>> parameters;
    unique_ptr<Type> retTy;
    vector<unique_ptr<Stmt>> statements;

    expectToken(&pos, FN);
    tie(var, pos) = parseVariable(pos);
    tie(parameters, pos) = parseParameterSequence(pos);
    expectToken(&pos, COLON);
    tie(retTy, pos) = parseType(pos);
    expectToken(&pos, LCURLY);
    expectToken(&pos, NEWLINE);
    tie(statements, pos) = parseStmtSequence(pos);
    expectToken(&pos, RCURLY);
    return make_tuple(make_unique<FnCmd>(std::move(var), std::move(parameters),
        std::move(retTy), std::move(statements)), pos);
}

std::pair<std::vector<std::unique_ptr<Stmt>>, int> Parser::parseStmtSequence(int pos) {
    vector<unique_ptr<Stmt>> statements;
    unique_ptr<Stmt> stmt;

    while(1) {
        if (peekToken(pos) == RCURLY) {
            break;
        }
        tie(stmt, pos) = parseStmt(pos);
        statements.push_back(std::move(stmt));
        expectToken(&pos, NEWLINE);        
    }
    return make_tuple(std::move(statements), pos);
}


std::pair<std::unique_ptr<Stmt>, int> Parser::parseStmt(int pos) {
    switch(peekToken(pos)) {
        case LET:
            return parseLetStmt(pos);
        case ASSERT:
            return parseAssertStmt(pos);
        case RETURN:
            return parseReturnStmt(pos);
        default:
            throw ParseException("No statement could be found at " + to_string(pos));
    }
}

std::pair<std::unique_ptr<Stmt>, int> Parser::parseLetStmt(int pos) {
    unique_ptr<Argument> lval;
    unique_ptr<Expr> expr;

    expectToken(&pos, LET);
    tie(lval, pos) = parseLValue(pos);
    expectToken(&pos, EQUALS);
    tie(expr, pos) = parseExpr(pos);
    return make_tuple(make_unique<LetStmt>(std::move(lval), std::move(expr)), pos);
}

std::pair<std::unique_ptr<Stmt>, int> Parser::parseAssertStmt(int pos) {
    unique_ptr<Expr> expr;

    expectToken(&pos, ASSERT);
    tie(expr, pos) = parseExpr(pos);
    expectToken(&pos, COMMA);
    string str = expectToken(&pos, STRING);
    return make_tuple(make_unique<AssertStmt>(std::move(expr), str), pos);
}

std::pair<std::unique_ptr<Stmt>, int> Parser::parseReturnStmt(int pos) {
    unique_ptr<Expr> expr;

    expectToken(&pos, RETURN);
    tie(expr, pos) = parseExpr(pos);
    return make_tuple(make_unique<ReturnStmt>(std::move(expr)), pos);
}

std::pair<std::unique_ptr<TupleBinding>, int> Parser::parseTupleBinding(int pos) {
    vector<unique_ptr<Binding>> bindList;
    unique_ptr<Binding> bind;
    
    expectToken(&pos, LCURLY);
    while(1){
        if (peekToken(pos) == RCURLY) {
            break;
        }
        tie(bind, pos) = parseBinding(pos);
        bindList.push_back(std::move(bind));
        if (peekToken(pos) != RCURLY) {
            expectToken(&pos, COMMA);
            if (peekToken(pos) == RCURLY) {
                throw ParseException("You cannot have a dangling comma: " + to_string(pos));
            }
        }
    }
    expectToken(&pos, RCURLY);
    return make_tuple(make_unique<TupleBinding>(std::move(bindList)), pos);
}

std::pair<std::unique_ptr<Binding>, int> Parser::parseBinding(int pos) {
    unique_ptr<Argument> varArg;
    unique_ptr<Type> ty;
    
    if (peekToken(pos) == LCURLY) {
        return parseTupleBinding(pos);
    }
    tie(varArg, pos) = parseArgument(pos);
    expectToken(&pos, COLON);
    tie(ty, pos) = parseType(pos);
    return make_tuple(make_unique<VarBinding>(std::move(varArg), std::move(ty)), pos);
}

std::pair<std::vector<std::unique_ptr<Binding>>, int> Parser::parseParameterSequence(int pos) {
    expectToken(&pos, LPAREN);
    vector<unique_ptr<Binding>> parameters;
    unique_ptr<Binding> bind;

    while(1) {
        if (peekToken(pos) == RPAREN) {
            break;
        }
        tie(bind, pos) = parseBinding(pos);
        parameters.push_back(std::move(bind));
        if (peekToken(pos) != RPAREN) {
            expectToken(&pos, COMMA);
            if (peekToken(pos) == RPAREN) {
                throw ParseException("You cannnot have a dangling comma: " + to_string(pos));
            }
        }
    }
    expectToken(&pos, RPAREN);
    return make_tuple(std::move(parameters), pos);
}

//TODO: lvl6 && lvl6cont may be irrelavent?
std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl6Cont(std::unique_ptr<Expr> inExp, int pos) {
    unique_ptr<Expr> expr;
    switch (peekToken(pos)){
        case LCURLY:
            tie(expr, pos) = parseTupleIndexExpr(std::move(inExp), pos);
            return parseExprLvl6Cont(std::move(expr), pos);
        case LSQUARE:
            tie(expr, pos) = parseArrayIndexExpr(std::move(inExp), pos);
            return parseExprLvl6Cont(std::move(expr), pos);
        default:
            return make_tuple(std::move(inExp), pos);
    }
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl6(int pos) {
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseLiteralExpr(pos);
    return parseExprLvl6Cont(std::move(expr), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl5Cont(std::unique_ptr<Expr> inExp, int pos) {
    if (peekToken(pos) == OP) {
        int mbpos = pos;
        string op = expectToken(&mbpos, OP);
        if (find(precedence[5].begin(), precedence[5].end(), op) != precedence[5].end()) {
            return parseExprLvl1Cont(std::move(inExp), pos); //todo pos correct?
        }
    }
    return make_tuple(std::move(inExp), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl5(int pos) {
    unique_ptr<Expr> expr;
    if (peekToken(pos) == OP) {
        int mbpos = pos;
        string op = expectToken(&mbpos, OP);
        if (find(precedence[5].begin(), precedence[5].end(), op) != precedence[5].end()) {
            tie(expr, pos) = parseExprLvl5(mbpos); //todo pos correct?
            return parseExprLvl5Cont(make_unique<UnopExpr>(op, std::move(expr)), pos);
        }
    }
    return parseExprLvl6(pos);

}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl4Cont(std::unique_ptr<Expr> inExp, int pos) {
    if (peekToken(pos) == OP) {
        int mbpos = pos;
        string op = expectToken(&mbpos, OP);
        if (find(precedence[4].begin(), precedence[4].end(), op) != precedence[4].end()) {
            unique_ptr<Expr> nExpr;
            tie(nExpr, pos) = parseExprLvl5(mbpos); //todo pos correct?
            return parseExprLvl4Cont(make_unique<BinopExpr>(op, std::move(inExp), std::move(nExpr)), pos);
        }
    }
    return make_tuple(std::move(inExp), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl4(int pos) {
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseExprLvl5(pos);
    return parseExprLvl4Cont(std::move(expr), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl3Cont(std::unique_ptr<Expr> inExp, int pos) {
    if (peekToken(pos) == OP) {
        int mbpos = pos;
        string op = expectToken(&mbpos, OP);
        if (find(precedence[3].begin(), precedence[3].end(), op) != precedence[3].end()) {
            unique_ptr<Expr> nExpr;
            tie(nExpr, pos) = parseExprLvl4(mbpos); //todo pos correct?
            return parseExprLvl3Cont(make_unique<BinopExpr>(op, std::move(inExp), std::move(nExpr)), pos);
        }
    }
    return make_tuple(std::move(inExp), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl3(int pos) {
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseExprLvl4(pos);
    return parseExprLvl3Cont(std::move(expr), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl2Cont(std::unique_ptr<Expr> inExp, int pos) {
    if (peekToken(pos) == OP) {
        int mbpos = pos;
        string op = expectToken(&mbpos, OP);
        if (find(precedence[2].begin(), precedence[2].end(), op) != precedence[2].end()) {
            unique_ptr<Expr> nExpr;
            tie(nExpr, pos) = parseExprLvl3(mbpos); //todo pos correct?
            return parseExprLvl2Cont(make_unique<BinopExpr>(op, std::move(inExp), std::move(nExpr)), pos);
        }
    }
    return make_tuple(std::move(inExp), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl2(int pos) {
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseExprLvl3(pos);
    return parseExprLvl2Cont(std::move(expr), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl1Cont(std::unique_ptr<Expr> inExp, int pos) {
    if (peekToken(pos) == OP) {
        int mbpos = pos;
        string op = expectToken(&mbpos, OP);
        if (find(precedence[1].begin(), precedence[1].end(), op) != precedence[1].end()) {
            unique_ptr<Expr> nExpr;
            tie(nExpr, pos) = parseExprLvl2(mbpos); //todo pos correct?
            return parseExprLvl1Cont(make_unique<BinopExpr>(op, std::move(inExp), std::move(nExpr)), pos);
        }
    }
    return make_tuple(std::move(inExp), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExprLvl1(int pos) {
    unique_ptr<Expr> expr;
    tie(expr, pos) = parseExprLvl2(pos);
    return parseExprLvl1Cont(std::move(expr), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseExpr(int pos) {
    return parseExprLvl1(pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseLiteralExpr(int pos) {
    unique_ptr<Variable> var;
    unique_ptr<Expr> exp;
    string vStr;

    switch (peekToken(pos)) {
        case INTVAL:
            vStr = expectToken(&pos, INTVAL);
            return make_tuple(make_unique<IntExpr>(vStr), pos);
        case FLOATVAL:
            vStr = expectToken(&pos, FLOATVAL);
            return make_tuple(make_unique<FloatExpr>(vStr), pos);
        case TRUE:
            return make_tuple(make_unique<TrueExpr>(), ++pos);
        case FALSE:
            return make_tuple(make_unique<FalseExpr>(), ++pos);
        case VARIABLE:
            tie(var, pos) = parseVariable(pos);
            if (peekToken(pos) == LPAREN) {
                return parseVariableExprCont(std::move(var), pos);
            }
            return make_tuple(make_unique<VarExpr>(std::move(var)), pos);
        case LCURLY:
             return parseTupleLiteralExpr(pos);
        case LSQUARE:
            return parseArrayLiteralExpr(pos);
        case LPAREN:
            expectToken(&pos, LPAREN);
            tie(exp, pos) = parseExpr(pos);
            expectToken(&pos, RPAREN);
            return make_tuple(std::move(exp), pos);
        case ARRAY:
            return parseArrayLoopExpr(pos);
        case SUM:
            return parseSumLoopExpr(pos);
        case IF:
            return parseIfExpr(pos);
        default:
            throw ParseException("expected an expression, got: " + tokens[pos]->text + " at: " + to_string(tokens[pos]->loc));

    }
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseLiteralExprCont(std::unique_ptr<Expr> inExp, int pos) {
    switch (peekToken(pos)) {
        case LCURLY:
            return parseTupleIndexExpr(std::move(inExp), pos);
        case LSQUARE:
            return parseArrayIndexExpr(std::move(inExp), pos);
        default:
            return make_tuple(std::move(inExp), pos);
    }
}

std::tuple<std::vector<std::unique_ptr<Variable>>, std::vector<std::unique_ptr<Expr>>, int> Parser::parseLoopBinds(int pos) {
    vector<unique_ptr<Variable>> vars;
    unique_ptr<Variable> var;
    vector<unique_ptr<Expr>> exprs;
    unique_ptr<Expr> expr;
    
    expectToken(&pos, LSQUARE);
    while(1) {
        if (peekToken(pos) == RSQUARE) {
            break;
        }

        tie(var, pos) = parseVariable(pos);
        expectToken(&pos, COLON);
        tie(expr, pos) = parseExpr(pos);
        vars.push_back(std::move(var));
        exprs.push_back(std::move(expr));

        if (peekToken(pos) != RSQUARE) {
            expectToken(&pos, COMMA);
            if (peekToken(pos) == RSQUARE) {
                throw ParseException("You cannot have a dangling comma at " + to_string(pos));
            }
        }
    }
    expectToken(&pos, RSQUARE);
    return make_tuple(std::move(vars), std::move(exprs), pos);
}


std::pair<std::unique_ptr<Expr>, int> Parser::parseArrayLoopExpr(int pos) {
    vector<unique_ptr<Variable>> vars;
    vector<unique_ptr<Expr>> exprs;
    unique_ptr<Expr> bExpr;

    expectToken(&pos, ARRAY);
    tie(vars, exprs, pos) = parseLoopBinds(pos);
    tie(bExpr, pos) = parseExpr(pos);
    return make_tuple(make_unique<ArrayLoopExpr>(std::move(vars), std::move(exprs), std::move(bExpr)), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseSumLoopExpr(int pos) {
    vector<unique_ptr<Variable>> vars;
    vector<unique_ptr<Expr>> exprs;
    unique_ptr<Expr> bExpr;

    expectToken(&pos, SUM);
    tie(vars, exprs, pos) = parseLoopBinds(pos);
    tie(bExpr, pos) = parseExpr(pos);
    return make_tuple(make_unique<SumLoopExpr>(std::move(vars), std::move(exprs), std::move(bExpr)), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseIfExpr(int pos) {
    unique_ptr<Expr> condExpr;
    unique_ptr<Expr> thenExpr;
    unique_ptr<Expr> elseExpr;

    expectToken(&pos, IF);
    tie(condExpr, pos) = parseExpr(pos);
    expectToken(&pos, THEN);
    tie(thenExpr, pos) = parseExpr(pos);
    expectToken(&pos, ELSE);
    tie(elseExpr, pos) = parseExpr(pos);

    return make_tuple(make_unique<IfExpr>(std::move(condExpr), std::move(thenExpr), std::move(elseExpr)), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseVariableExprCont(std::unique_ptr<Variable> inVar, int pos) {
    vector<unique_ptr<Expr>> parameters;
    tie(parameters, pos) = parseExprSequence(pos, LPAREN, RPAREN);
    return make_tuple(make_unique<CallExpr>(std::move(inVar), std::move(parameters)), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseTupleIndexExpr(std::unique_ptr<Expr> inExp, int pos) {
    expectToken(&pos, LCURLY);
    string index = expectToken(&pos, INTVAL);
    expectToken(&pos, RCURLY);
    return make_tuple(make_unique<TupleIndexExpr>(std::move(inExp), index), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseArrayIndexExpr(std::unique_ptr<Expr> inExp, int pos) {
    vector<unique_ptr<Expr>> indices;
    tie(indices, pos) = parseExprSequence(pos, LSQUARE, RSQUARE);
    return make_tuple(make_unique<ArrayIndexExpr>(std::move(inExp), std::move(indices)), pos);
}

std::pair<std::vector<std::unique_ptr<Expr>>, int>  Parser::parseExprSequence(int pos, Lex::Tokty start, Lex::Tokty end) {
    vector<unique_ptr<Expr>> expList;
    unique_ptr<Expr> exp;
    
    expectToken(&pos, start);
    while(1) {
        if (peekToken(pos) == end) {
            break;
        }
        tie(exp, pos) = parseExpr(pos);
        expList.push_back(std::move(exp));
        if (peekToken(pos) != end) {
            expectToken(&pos, COMMA);
            if (peekToken(pos) == end) {
                throw ParseException("You cannot have a dangling comma: " + to_string(pos));
            }
        }
    }
    expectToken(&pos, end);
    return make_tuple(std::move(expList), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseArrayLiteralExpr(int pos) {
    vector<unique_ptr<Expr>> expList;
    tie(expList, pos) = parseExprSequence(pos, LSQUARE, RSQUARE);
    return make_tuple(make_unique<ArrayLiteralExpr>(std::move(expList)), pos);
}

std::pair<std::unique_ptr<Expr>, int> Parser::parseTupleLiteralExpr(int pos) {
    vector<unique_ptr<Expr>> expList;
    tie(expList, pos) = parseExprSequence(pos, LCURLY, RCURLY);
    return make_tuple(make_unique<TupleLiteralExpr>(std::move(expList)), pos);
}



std::pair<std::unique_ptr<Type>, int> Parser::parseType(int pos) {
    unique_ptr<Variable> var;
    unique_ptr<Type> ty;

    switch (peekToken(pos)) {
        case INT:
            return parseTypeCont(make_unique<IntType>(), ++pos);
        case FLOAT:
            return parseTypeCont(make_unique<FloatType>(), ++pos);
        case BOOL:
            return parseTypeCont(make_unique<BoolType>(), ++pos);
        case VARIABLE:
            tie(var, pos) = parseVariable(pos);
            return parseTypeCont(make_unique<VarType>(std::move(var)), pos);
        case LCURLY:
            tie(ty, pos) = parseTupleType(pos);
            return parseTypeCont(std::move(ty), pos);
        default:
            throw ParseException(("No type found at " + to_string(tokens[pos]->loc) + " (" + tokens[pos]->text + ")").c_str());
    }
}

std::pair<std::unique_ptr<Type>, int> Parser::parseTypeCont(unique_ptr<Type> ty, int pos) {
    std::unique_ptr<Type> nTy;
    if (peekToken(pos) == LSQUARE) {
        tie(nTy, pos) = parseArrayType(std::move(ty), pos);
        return parseTypeCont(std::move(nTy), pos);
    }
    return make_tuple(std::move(ty), pos);
}

std::pair<std::unique_ptr<Type>, int> Parser::parseArrayType(std::unique_ptr<Type> ty, int pos) {
    expectToken(&pos, LSQUARE);
    int dimension = 1;
    while(peekToken(pos) == COMMA) { //TODO: iffy
        expectToken(&pos, COMMA);
        dimension++;
    }
    expectToken(&pos, RSQUARE);
    return make_tuple(make_unique<ArrayType>(std::move(ty), dimension), pos);
}

//TODO: in the rewrite, convert the sequence processor to a Generic type ala parseseq(T, start, end);
std::pair<std::unique_ptr<Type>, int> Parser::parseTupleType(int pos) {
    vector<unique_ptr<Type>> tyList;
    unique_ptr<Type> ty;

    expectToken(&pos, LCURLY);
    while(1) {
        if (peekToken(pos) == RCURLY) {
            break;
        }
        tie(ty, pos) = parseType(pos);
        tyList.push_back(std::move(ty));
        if (peekToken(pos) != RCURLY) {
            expectToken(&pos, COMMA);
            if (peekToken(pos) == RCURLY) {
                throw ParseException("You cannot have a dangling comma: " + to_string(pos));
            }
        }
    }
    expectToken(&pos, RCURLY);
    return make_tuple(make_unique<TupleType>(std::move(tyList)), pos);
}

std::pair<std::unique_ptr<Argument>, int> Parser::parseArrayArgument(std::unique_ptr<Variable> inVar, int pos) {
    vector<unique_ptr<Variable>> vars;
    unique_ptr<Variable> var;

    expectToken(&pos, LSQUARE);
    while(1) {
        if(peekToken(pos) == RSQUARE) {
            break;
        }
        tie(var, pos) = parseVariable(pos);
        vars.push_back(std::move(var));
        if(peekToken(pos) != RSQUARE) {
            expectToken(&pos, COMMA);
        }
    }
    expectToken(&pos, RSQUARE);
    return make_tuple(make_unique<ArrayArgument>(std::move(inVar), std::move(vars)), pos);
}        

std::pair<std::unique_ptr<Argument>, int> Parser::parseArgument(int pos) {
    unique_ptr<Variable> var;
    unique_ptr<Argument> arg;

    tie(var, pos) = parseVariable(pos);
    if (peekToken(pos) == LSQUARE) {
        return parseArrayArgument(std::move(var), pos);
    }
    return make_tuple(make_unique<VarArgument>(std::move(var)), pos);
}

std::pair<std::unique_ptr<Argument>, int> Parser::parseTupleLValue(int pos) {
    vector<unique_ptr<Argument>> args;
    unique_ptr<Argument> arg;

    expectToken(&pos, LCURLY);
    while(1) {
        if (peekToken(pos) == RCURLY) {
            break;
        }
        tie(arg, pos) = parseLValue(pos);
        args.push_back(std::move(arg));
        if (peekToken(pos) != RCURLY) {
            expectToken(&pos, COMMA);
            if (peekToken(pos) == RCURLY) {
                throw ParseException("You cannot have a dangling comma: " + to_string(pos));
            }
        }
    }
    expectToken(&pos, RCURLY);
    return make_tuple(make_unique<TupleLValue>(std::move(args)), pos);
}

std::pair<std::unique_ptr<Argument>, int> Parser::parseLValue(int pos) {
    unique_ptr<Argument> arg;

    if (peekToken(pos) == LCURLY) {
        return parseTupleLValue(pos);
    } 
    tie(arg, pos) = parseArgument(pos);
    return make_tuple(make_unique<ArgLValue>(std::move(arg)), pos);
}

std::pair<std::unique_ptr<Variable>, int> Parser::parseVariable(int pos) {
    string varName = expectToken(&pos, VARIABLE);
    return make_tuple(make_unique<Variable>(varName), pos);
}







