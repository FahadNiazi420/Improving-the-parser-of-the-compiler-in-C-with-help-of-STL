#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <map>

using namespace std;

enum TokenType {
    T_INT, T_FLOAT, T_DOUBLE, T_STRING, T_BOOL, T_CHAR, T_ID, 
    T_NUM, T_AGAR, T_ELSE, T_RETURN, T_WHILE, T_ASSIGN, 
    T_PLUS, T_MINUS, T_MUL, T_DIV, T_LPAREN, T_RPAREN, 
    T_LBRACE, T_RBRACE, T_SEMICOLON, T_GT, T_EQ, T_EOF
};

struct Token {
    TokenType type;
    string value;
    int line;
    int column;
};

class Lexer {
private:
    string src;
    size_t pos;
    int line;
    int column;

public:
    Lexer(const string &src) : src(src), pos(0), line(1), column(1) {}

    string consumeNumber() {
        size_t start = pos;
        while (pos < src.size() && (isdigit(src[pos]) || src[pos] == '.')) pos++;
        return src.substr(start, pos - start);
    }

    string consumeWord() {
        size_t start = pos;
        while (pos < src.size() && isalnum(src[pos])) pos++;
        return src.substr(start, pos - start);
    }

    vector<Token> tokenize() {
        vector<Token> tokens;
        while (pos < src.size()) {
            char current = src[pos];

            if (isspace(current)) {
                if (current == '\n') {
                    line++;
                    column = 0; // Reset column on new line
                }
                pos++;
                column++;
                continue;
            }

            if (isdigit(current)) {
                string num = consumeNumber();
                tokens.push_back(Token{T_NUM, num, line, column});
                column += num.length(); // Update column position
                continue;
            }

            if (isalpha(current)) {
                string word = consumeWord();
                TokenType type = T_ID;

                if (word == "int") type = T_INT;
                else if (word == "float") type = T_FLOAT;
                else if (word == "double") type = T_DOUBLE;
                else if (word == "string") type = T_STRING;
                else if (word == "bool") type = T_BOOL;
                else if (word == "char") type = T_CHAR;
                else if (word == "Agar") type = T_AGAR; // Custom conditional keyword
                else if (word == "else") type = T_ELSE;
                else if (word == "return") type = T_RETURN;
                else if (word == "while") type = T_WHILE; // Loop keyword

                tokens.push_back(Token{type, word, line, column});
                column += word.length(); // Update column position
                continue;
            }

            switch (current) {
                case '=': tokens.push_back(Token{T_ASSIGN, "=", line, column}); break;
                case '+': tokens.push_back(Token{T_PLUS, "+", line, column}); break;
                case '-': tokens.push_back(Token{T_MINUS, "-", line, column}); break;
                case '*': tokens.push_back(Token{T_MUL, "*", line, column}); break;
                case '/': tokens.push_back(Token{T_DIV, "/", line, column}); break;
                case '(': tokens.push_back(Token{T_LPAREN, "(", line, column}); break;
                case ')': tokens.push_back(Token{T_RPAREN, ")", line, column}); break;
                case '{': tokens.push_back(Token{T_LBRACE, "{", line, column}); break;
                case '}': tokens.push_back(Token{T_RBRACE, "}", line, column}); break;
                case ';': tokens.push_back(Token{T_SEMICOLON, ";", line, column}); break;
                case '>': tokens.push_back(Token{T_GT, ">", line, column}); break;
                case '"': // Handle string literals
                    {
                        size_t start = pos;
                        pos++;
                        column++;
                        while (pos < src.size() && src[pos] != '"') {
                            pos++;
                            column++;
                        }
                        if (pos < src.size()) {
                            pos++; // Consume closing quote
                            column++;
                            tokens.push_back(Token{T_STRING, src.substr(start, pos - start), line, column});
                        } else {
                            cout << "Syntax error: unclosed string literal at line " << line << ", column " << column << endl;
                            exit(1);
                        }
                        continue;
                    }
                case '\'': // Handle char literals
                    {
                        size_t start = pos;
                        pos++;
                        column++;
                        if (pos < src.size() && src[pos] != '\'') {
                            pos++; // Consume character
                            column++;
                            if (pos < src.size() && src[pos] == '\'') {
                                pos++; // Consume closing quote
                                column++;
                                tokens.push_back(Token{T_CHAR, src.substr(start, pos - start), line, column});
                            } else {
                                cout << "Syntax error: unclosed char literal at line " << line << ", column " << column << endl;
                                exit(1);
                            }
                        } else {
                            cout << "Syntax error: empty char literal at line " << line << ", column " << column << endl;
                            exit(1);
                        }
                        continue;
                    }
                default:
                    cout << "Unexpected character: '" << current << "' at line " << line << ", column " << column << endl;
                    exit(1);
            }
            pos++;
            column++;
        }
        tokens.push_back(Token{T_EOF, "", line, column});
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
        if (tokens[pos].type == T_INT || tokens[pos].type == T_FLOAT || 
            tokens[pos].type == T_DOUBLE || tokens[pos].type == T_STRING || 
            tokens[pos].type == T_BOOL || tokens[pos].type == T_CHAR) {
            parseDeclaration();
        } else if (tokens[pos].type == T_ID) {
            parseAssignment();
        } else if (tokens[pos].type == T_AGAR) {
            parseIfStatement();
        } else if (tokens[pos].type == T_RETURN) {
            parseReturnStatement();
        } else if (tokens[pos].type == T_WHILE) {
            parseWhileStatement(); // Parse while loop
        } else if (tokens[pos].type == T_LBRACE) {
            parseBlock();
        } else {
            cout << "Syntax error: unexpected token '" << tokens[pos].value << "' at line " << tokens[pos].line << ", column " << tokens[pos].column << endl;
            exit(1);
        }
    }

    void parseBlock() {
        expect(T_LBRACE);
        while (tokens[pos].type != T_RBRACE && tokens[pos].type != T_EOF) {
            parseStatement();
        }
        expect(T_RBRACE);
    }

    void parseDeclaration() {
        if (tokens[pos].type == T_INT || tokens[pos].type == T_FLOAT || 
            tokens[pos].type == T_DOUBLE || tokens[pos].type == T_STRING || 
            tokens[pos].type == T_BOOL || tokens[pos].type == T_CHAR) {
            expect(tokens[pos].type);
            expect(T_ID);
            expect(T_SEMICOLON);
        } else {
            cout << "Syntax error: expected data type but found '" << tokens[pos].value << "' at line " << tokens[pos].line << ", column " << tokens[pos].column << endl;
            exit(1);
        }
    }

    void parseAssignment() {
        expect(T_ID);
        expect(T_ASSIGN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseIfStatement() {
        expect(T_AGAR);
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

    void parseWhileStatement() {
        expect(T_WHILE);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();
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
        if (tokens[pos].type == T_NUM || tokens[pos].type == T_ID || 
            tokens[pos].type == T_STRING || tokens[pos].type == T_CHAR) {
            pos++;
        } else if (tokens[pos].type == T_LPAREN) {
            expect(T_LPAREN);
            parseExpression();
            expect(T_RPAREN);
        } else {
            cout << "Syntax error: unexpected token '" << tokens[pos].value << "' at line " << tokens[pos].line << ", column " << tokens[pos].column << endl;
            exit(1);
        }
    }

    void expect(TokenType type) {
        if (tokens[pos].type == type) {
            pos++;
        } else {
            cout << "Syntax error: expected token type " << type << " but found '" 
                 << tokens[pos].value << "' at line " << tokens[pos].line 
                 << ", column " << tokens[pos].column << endl;
            exit(1);
        }
    }

public:
    Parser(const vector<Token> &tokens) : tokens(tokens), pos(0) {}

    void parse() {
        parseProgram();
    }
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }

    // Read the file content
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << argv[1] << endl;
        return 1;
    }

    string input((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    parser.parse();

    return 0;
}
