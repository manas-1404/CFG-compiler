#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include "lexer.h"

using namespace std;

class Parser {
private:
    LexicalAnalyzer lexer;
    Token token;

    void syntax_error() {
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!\n";
        exit(1);
    }

    // This helper consumes the next token and checks its type.
    void expect(TokenType expected) {
        token = lexer.GetToken();
        if (token.token_type != expected) {
            syntax_error();
        }
    }

    // Grammar → Rule-list HASH
    void parseGrammar();

    // Rule-list → Rule Rule-list | Rule
    void parseRuleList();

    // Rule → ID ARROW Right-hand-side STAR
    void parseRule();

    // Right-hand-side → Id-list | Id-list OR Right-hand-side
    void parseRightHandSide();

    // Id-list → ID Id-list | ε
    void parseIdList();

public:
    void startParsing();
};

// Grammar → Rule-list HASH
void Parser::parseGrammar() {
    parseRuleList();
    expect(HASH);
}

// Rule-list → Rule Rule-list | Rule
void Parser::parseRuleList() {

    // The grammar requires at least one Rule
    parseRule();

    // After parsing one Rule, we may have more Rules
    // as long as the next token is an ID
    Token nextToken = lexer.peek(1);
    while (nextToken.token_type == ID) {
        parseRule();
        nextToken = lexer.peek(1);
    }
}

// Rule → ID ARROW Right-hand-side STAR
void Parser::parseRule() {
    // LHS must be an ID
    expect(ID);
    // Then '->'
    expect(ARROW);
    // Then parse the right-hand side
    parseRightHandSide();
    // Then expect a '*'
    expect(STAR);
}

// Right-hand-side → Id-list | Id-list OR Right-hand-side
//right-hand side which is one or more Id-list’s separated with OR’ s
void Parser::parseRightHandSide() {
    // Parse the first Id-list
    parseIdList();

    // If next token is '|', consume it and parse more Id-list
    Token nextToken = lexer.peek(1);
    while (nextToken.token_type == OR) {
        expect(OR);
        parseIdList();
        nextToken = lexer.peek(1);
    }
}

// Id-list → ID Id-list | ε
//Id-list is a list of zero or more ID’s
void Parser::parseIdList() {
    // If the next token is an ID, consume it and parse another Id-list
    // Otherwise, we do nothing (ε)

    Token nextToken = lexer.peek(1);
    if (nextToken.token_type == ID) {
        expect(ID);
        // Recursively parse the remainder of the Id-list
        parseIdList();
    }
    // If it's not an ID, we take ε, which means "do nothing"
}

// This function is your public "entry point" for parsing.
void Parser::startParsing() {
    parseGrammar();
}

// read grammar
void ReadGrammar() {


}

/* 
 * Task 1: 
 * Printing the terminals, then nonterminals of grammar in appearing order
 * output is one line, and all names are space delineated
*/
void Task1()
{
}

/*
 * Task 2:
 * Print out nullable set of the grammar in specified format.
*/
void Task2()
{
}

// Task 3: FIRST sets
void Task3()
{
}

// Task 4: FOLLOW sets
void Task4()
{
}

// Task 5: left factoring
void Task5()
{
}

// Task 6: eliminate left recursion
void Task6()
{
}
    
int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);


    Parser parser;
    parser.startParsing();
    
    //ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    switch (task) {
        case 1: Task1();
            cout << "Hello world from task1" << endl;
            break;

        case 2: Task2();
            break;

        case 3: Task3();
            break;

        case 4: Task4();
            break;

        case 5: Task5();
            break;
        
        case 6: Task6();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

