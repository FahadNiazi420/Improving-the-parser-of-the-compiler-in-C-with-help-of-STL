#include <iostream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

enum TokenType {
    T_INT, T_ID, T_NUM, T_IF, T_ELSE, T_RETURN,
    T_ASSIGN, T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE,
    T_SEMICOLON, T_GT, T_EQ, T_EOF, T_WHILE
};

struct Token {
    TokenType type;
    string value;
    int line;
    int col;
};

class Lexer {
private:
    string src;
    size_t pos;
    int line;
    int col;

public:
    Lexer(const string &src) : src(src), pos(0), line(1), col(1) {}

    string consumeNumber() {
        size_t start = pos;
        int startCol = col;
        while (pos < src.size() && isdigit(src[pos])) {
            pos++;
            col++;
        }
        return src.substr(start, pos - start);
    }

    string consumeWord() {
        size_t start = pos;
        int startCol = col;
        while (pos < src.size() && isalnum(src[pos])) {
            pos++;
            col++;
        }
        return src.substr(start, pos - start);
    }

    vector<Token> tokenize() {
        vector<Token> tokens;
        while (pos < src.size()) {
            char current = src[pos];

            if (isspace(current)) {
                if (current == '\n') {
                    line++;
                    col = 1;
                } else {
                    col++;
                }
                pos++;
                continue;
            }

            if (isdigit(current)) {
                tokens.push_back(Token{T_NUM, consumeNumber(), line, col});
                continue;
            }

            if (isalpha(current)) {
                string word = consumeWord();
                if (word == "int") tokens.push_back(Token{T_INT, word, line, col});
                else if (word == "if") tokens.push_back(Token{T_IF, word, line, col});
                else if (word == "else") tokens.push_back(Token{T_ELSE, word, line, col});
                else if (word == "return") tokens.push_back(Token{T_RETURN, word, line, col});
                else if (word == "while") tokens.push_back(Token{T_WHILE, word, line, col});
                else tokens.push_back(Token{T_ID, word, line, col});
                continue;
            }

            switch (current) {
                case '=': tokens.push_back(Token{T_ASSIGN, "=", line, col}); break;
                case '+': tokens.push_back(Token{T_PLUS, "+", line, col}); break;
                case '-': tokens.push_back(Token{T_MINUS, "-", line, col}); break;
                case '*': tokens.push_back(Token{T_MUL, "*", line, col}); break;
                case '/': tokens.push_back(Token{T_DIV, "/", line, col}); break;
                case '(': tokens.push_back(Token{T_LPAREN, "(", line, col}); break;
                case ')': tokens.push_back(Token{T_RPAREN, ")", line, col}); break;
                case '{': tokens.push_back(Token{T_LBRACE, "{", line, col}); break;
                case '}': tokens.push_back(Token{T_RBRACE, "}", line, col}); break;
                case ';': tokens.push_back(Token{T_SEMICOLON, ";", line, col}); break;
                case '>': tokens.push_back(Token{T_GT, ">", line, col}); break;
                default: 
                    cout << "Unexpected character '" << current << "' at line " << line << ", column " << col << endl;
                    exit(1);
            }
            pos++;
            col++;
        }
        tokens.push_back(Token{T_EOF, "", line, col});
        return tokens;
    }
};

class Parser {
private:
    vector<Token> tokens;
    int pos;

    void parseProgram() {
        while (tokens[pos].type != T_EOF) {
            parseStatement();
        }
        cout << "Parsing completed successfully! No Syntax Error" << endl;
    }

    void parseStatement() {
        switch (tokens[pos].type) {
            case T_INT:
                parseDeclaration();
                break;
            case T_ID:
                parseAssignment();
                break;
            case T_IF:
                parseIfStatement();
                break;
            case T_RETURN:
                parseReturnStatement();
                break;
            case T_LBRACE:
                parseBlock();
                break;
            case T_WHILE:
                parseWhileStatement();
                break;
            default:
                syntaxError("unexpected token");
        }
    }

    void parseWhileStatement() {
        expect(T_WHILE);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();
    }

    void parseBlock() {
        expect(T_LBRACE);
        while (tokens[pos].type != T_RBRACE && tokens[pos].type != T_EOF) {
            parseStatement();
        }
        expect(T_RBRACE);
    }

    void parseDeclaration() {
        expect(T_INT);
        expect(T_ID);
        expect(T_SEMICOLON);
    }

    void parseAssignment() {
        expect(T_ID);
        expect(T_ASSIGN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseIfStatement() {
        expect(T_IF);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();
        if (tokens[pos].type == T_ELSE) {
            expect(T_ELSE);
            parseStatement();
        }
    }

    void parseReturnStatement() {
        expect(T_RETURN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseExpression() {
        parseTerm();
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS) {
            pos++;
            parseTerm();
        }
        if (tokens[pos].type == T_GT) {
            pos++;
            parseExpression();
        }
    }

    void parseTerm() {
        parseFactor();
        while (tokens[pos].type == T_MUL || tokens[pos].type == T_DIV) {
            pos++;
            parseFactor();
        }
    }

    void parseFactor() {
        if (tokens[pos].type == T_NUM || tokens[pos].type == T_ID) {
            pos++;
        } else if (tokens[pos].type == T_LPAREN) {
            expect(T_LPAREN);
            parseExpression();
            expect(T_RPAREN);
        } else {
            syntaxError("unexpected token");
        }
    }

    void expect(TokenType type) {
        if (tokens[pos].type == type) {
            pos++;
        } else {
            syntaxError("expected different token");
        }
    }

    void syntaxError(const string& message) {
        cout << "Syntax error: " << message
             << " at line " << tokens[pos].line
             << ", column " << tokens[pos].col
             << ", found token: '" << tokens[pos].value << "'" << endl;
        exit(1);
    }

public:
    Parser(const vector<Token> &tokens) : tokens(tokens), pos(0) {}

    void parse() {
        parseProgram();
    }
};

int main() {
    string input = R"(
        int a;
        a = 5;
        int b;
        b = a + 10;
        if (b > 10) {
            return b;
        } else {
            return 0;
        }
        while (a < 10) {
            a = a + 1;
        }
    )";

    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    parser.parse();

    return 0;
}
