#ifndef _LEX_
#define _LEX_

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

namespace Lex
{

    enum Tokty{
        ARRAY, 
        ASSERT, 
        BOOL, 
        ELSE,
        FALSE, 
        FLOAT, 
        FN, 
        IF, 
        IMAGE, 
        INT, 
        LET, 
        PRINT,
        READ, 
        RETURN, 
        SHOW, 
        SUM, 
        THEN, 
        TIME, 
        TO, 
        TRUE, 
        TYPE, 
        WRITE,
        COLON, 
        LCURLY, 
        RCURLY, 
        LPAREN, 
        RPAREN, 
        COMMA, 
        LSQUARE, 
        RSQUARE, 
        EQUALS,
        STRING, 
        INTVAL, 
        FLOATVAL, 
        VARIABLE, 
        OP, 
        NEWLINE, 
        END_OF_FILE,
        OP_OR_VAR
    };

    static std::unordered_map<std::string, Tokty> const tbl = {
        {"array", ARRAY}, {"assert", ASSERT}, {"bool", BOOL}, {"else", ELSE}, {"to", TO},
        {"false", FALSE}, {"float", FLOAT}, {"fn", FN}, {"if", IF}, {"image", IMAGE},
        {"int", INT}, {"let", LET}, {"print", PRINT}, {"read", READ}, {"return", RETURN},
        {"show", SHOW}, {"sum", SUM}, {"then", THEN}, {"to", TO}, {"true", TRUE},
        {"type", TYPE}, {"write", WRITE}, {":", COLON}, {"{", LCURLY}, {"}", RCURLY},
        {"(", LPAREN}, {")", RPAREN}, {",", COMMA}, {"[", LSQUARE}, {"]", RSQUARE},
        {"=", EQUALS}, {"string", STRING}, {"intval", INTVAL}, {"floatval", FLOATVAL},
        {"variable", VARIABLE}, {"op", OP}, {"read", READ}, {"\n", NEWLINE}, {"eof", END_OF_FILE}, {"time", TIME}
    };

    static std::unordered_map<Tokty, std::string> const revTbl = {
        {ARRAY, "ARRAY"}, {ASSERT, "ASSERT"}, {BOOL, "BOOL"}, {ELSE, "ELSE"}, {TIME, "TIME"},
        {FALSE, "FALSE"}, {FLOAT, "FLOAT"}, {FN, "FN"}, {IF, "IF"}, {IMAGE, "IMAGE"}, {TO, "TO"},
        {INT, "INT"}, {LET, "LET"}, {PRINT, "PRINT"}, {READ, "READ"}, {RETURN, "RETURN"},
        {SHOW, "SHOW"}, {SUM, "SUM"}, {THEN, "THEN"}, {TO, "TO"}, {TRUE, "TRUE"},
        {TYPE, "TYPE"}, {WRITE, "WRITE"}, {COLON, "COLON"}, {LCURLY, "LCURLY"}, {RCURLY, "RCURLY"},
        {LPAREN, "LPAREN"}, {RPAREN, "RPAREN"}, {COMMA, "COMMA"}, {LSQUARE, "LSQUARE"}, {RSQUARE, "RSQUARE"},
        {EQUALS, "EQUALS"}, {STRING, "STRING"}, {INTVAL, "INTVAL"}, {FLOATVAL, "FLOATVAL"},
        {VARIABLE, "VARIABLE"}, {OP, "OP"}, {READ, "READ"}, {NEWLINE, "NEWLINE"}, {END_OF_FILE, "END_OF_FILE"}

    };


    inline Tokty strToTokty(std::string match){
        auto it = tbl.find(match);
        if (it == tbl.end()) {
            return OP_OR_VAR;
        }
        return it->second;
    }

    class Token{
        public:
            Lex::Tokty tokty;
            int row;
            int col;
            int loc;
            std::string text;

            //Token(Lex::Tokty tok, int loc, std::string txt);
            Token(Lex::Tokty tokty, int loc, std::string text);
            Token();
            ~Token();

            std::string to_string();

    };

    class Lexer {
        private:
            std::string file;
            int fsize;
            std::vector<std::unique_ptr<Lex::Token>> tokens; //TODO: move tokens to public or find a way to transfer vector of unique ptrs

            std::tuple<std::unique_ptr<Lex::Token>, int> lexItem(int);
            int lexWhiteSpc(int);

        public:
            Lexer(std::string);
            void doLex();
            void prettyPrint();
            std::vector<std::unique_ptr<Lex::Token>> getTokens();
    };      



} // namespace Lex


#endif