#include <lexer.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
using namespace std;
using namespace Lex;

Token::Token(Lex::Tokty tokty, int loc, std::string text):
    tokty(tokty), loc(loc), text(text) {}
Token::Token() = default;
Token::~Token() = default;


string Token::to_string() {
    switch (tokty) {
        case NEWLINE:
            return "NEWLINE\n";
        case END_OF_FILE:
            return "END_OF_FILE\n";
        default:
            return revTbl.find(tokty)->second + " '" + text + "'\n";
    }
}



Lexer::Lexer(string filename) {
    std::ifstream f(filename);
    std::stringstream buf;
    buf << f.rdbuf();
    file = buf.str();
    fsize = file.length();    
}

void Lexer::doLex(){
    int pos = lexWhiteSpc(0);
    if (pos == -1) {
        cout << "Compilation failed!" << endl;
        exit(-1);
    }    
    while (pos < fsize) {
        unique_ptr<Token> t;
        int l;
        tie(t, l) = lexItem(pos);
        if (l == -1) {
            cout << "Compilation failed! token invalid" << endl;
            exit(-1);
        }
        tokens.push_back(std::move(t));
        pos = lexWhiteSpc(l);
        if (pos == -1) {
            cout << "Compilation failed! white space invalid" << endl;
            exit(-1);
        }
    }
    unique_ptr<Token> eof = make_unique<Token>(END_OF_FILE, fsize-1, "EOF");
    tokens.push_back(std::move(eof));
}

void Lexer::prettyPrint() {
    for (int i = 0; i < tokens.size(); i++) {
        cout << tokens[i]->to_string();
    }
    cout << "Compilation succeeded: lexing complete\n";
}

std::vector<std::unique_ptr<Lex::Token>> Lex::Lexer::getTokens()
{
    return std::move(tokens);
}

int Lexer::lexWhiteSpc(int pos)
{
    bool hasNL = 0;
    
    while(pos < fsize) {
        if (file[pos] == ' '){
            pos++;
        } else if (file[pos] == '/') {
            if (file[pos+1] == '/'){
                while (pos < fsize && file[pos] != '\n'){
                    pos++;
                }
                if(pos == fsize) {
                    return -1;
                }
                if (!hasNL) {
                    hasNL = 1;
                    tokens.push_back(make_unique<Token>(NEWLINE, pos, "NEWLINE"));
                }
                pos++;
            } else if (file[pos+1] == '*') {
                pos += 2;
                while (pos < fsize) {
                    if (pos == fsize-1) {
                        return -1;
                    } 
                    if (file[pos] == '*' && file[pos+1] == '/'){
                        if (!hasNL) {
                            hasNL = 1;
                            tokens.push_back(make_unique<Token>(NEWLINE, pos, "NEWLINE"));
                        }
                        pos++;
                        break;
                    }
                    pos++;
                }
                pos += 2;
            } else {
                break;
            }
        } else if (file[pos] == '\\') {
            hasNL = 1;
            pos++;
        } else if (file[pos] == '\n') {
            if (!hasNL) {
                hasNL = 1;
                tokens.push_back(make_unique<Token>(NEWLINE, pos, "NEWLINE"));
            }
            pos++;
        } else if (pos == fsize-1) {
            break;
        } else {
            break;
        }
    }
    return pos;
}

std::tuple<std::unique_ptr<Lex::Token>, int> Lexer::lexItem(int pos) {
    smatch match;
    const string &curr = file.substr(pos);

    regex fl("(^[0-9]+\\.[0-9]*)|(^[0-9]*\\.[0-9]+)");
    regex nt("^[0-9]+");
    regex word("^[a-zA-Z]+[a-zA-Z0-9_\\.]*");
    regex op("^((&&)|(\\|\\|)|(<=)|(>=)|(<)|(>)|(==)|(!=)|(\\+)|(-)|(\\*)|(/)|(%)|(!))");
    regex st("\"[ !#-~]*\""); //TODO: works? ^ needed?
    regex punct("^[\\:\\{\\}\\(\\)\\[\\],=]");

    if (regex_search(curr.begin(), curr.end(), match, fl)) {
        return make_tuple(make_unique<Token>(FLOATVAL, pos, match[0]), match[0].length() + pos);
    } else if (regex_search(curr.begin(), curr.end(), match, nt)) {
        return make_tuple(make_unique<Token>(INTVAL, pos, match[0]), match[0].length() + pos);
    } else if (regex_search(curr.begin(), curr.end(), match, word)) {
        Tokty t = strToTokty(match[0]); //TODO: ???
        if (t == OP_OR_VAR) {
            t = VARIABLE;
        }
        return make_tuple(make_unique<Token>(t, pos, match[0]), match[0].length() + pos);
    } else if (regex_search(curr.begin(), curr.end(), match, op)) {
        return make_tuple(make_unique<Token>(OP, pos, match[0]), match[0].length() + pos);
    } else if (regex_search(curr.begin(), curr.end(), match, punct)) {
        return make_tuple(make_unique<Token>(strToTokty(match[0]), pos, match[0]), match[0].length() + pos);        
    } else if (regex_search(curr.begin(), curr.end(), match, st)) {
        return make_tuple(make_unique<Token>(STRING, pos, match[0]), match[0].length() + pos);
    }
    return make_tuple(make_unique<Token>(Token()), -1);
}


