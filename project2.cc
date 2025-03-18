#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include "lexer.h"

using namespace std;

struct Rule {
    string LHS;
    vector<string> RHS;
};

vector<Rule> allRules;

set<string> nonTerminalsSet;
set<string> terminalsSet;
set<string> nullableSet;

vector<string> universe;

void printAllRules() {
    for (const auto &rule : allRules) {
        cout << rule.LHS << " -> ";
        if (rule.RHS.empty()) {
            cout << "(empty)";
        } else {
            for (const string &symbol : rule.RHS) {
                cout << symbol << " ";
            }
        }
        cout << endl;
    }
}

void printRHSSymbols(const vector<string> &rhsSymbols) {
    if (rhsSymbols.empty()) {
        cout << "(empty)";  // Indicate epsilon explicitly
    } else {
        for (const string &symbol : rhsSymbols) {
            cout << symbol << " ";
        }
    }
    cout << endl;
}


//declaring parser class for parsing the grammar
class Parser {

// declaring private memebrs of parser class
private:

    //declaring lexer object and token object
    LexicalAnalyzer lexer;
    Token token;

    //error functrion which is used to print the syntax error command and stop the program
    void syntax_error() {

        //print the syntax error message
        cout << "SYNTAX ERROR !!!!!!!!!!!!!!\n";

        //stoping the program
        exit(1);
    }

    //function which will consumes the current token and checks if the token type is correct
    Token expect(TokenType expectedToken) {

        //initializing the token object with the current token
        Token t = lexer.GetToken();

        //checking if the token type is correct, then don't do anything. else call the syntax error function
        if (t.token_type != expectedToken) {

            //calling the syntax error function because token type is not matching the expetcted
            syntax_error();
        }

        return t;
    }

    // Grammar -> Rule-list HASH
    void parseGrammar();

    // Rule-list -> Rule Rule-list | Rule
    void parseRuleList();

    // Rule -> ID ARROW Right-hand-side STAR
    void parseRule();

    // Right-hand-side -> Id-list | Id-list OR Right-hand-side
    void parseRightHandSide(vector<string> &rhsSymbols);

    // Id-list -> ID Id-list | epsilon
    void parseIdList(vector<string> &rhsSymbols);

public:
    //constructor of the parser class
    void startParsing();
};

// Grammar -> Rule-list HASH
void Parser::parseGrammar() {

    cout << "STARTING parsing THE GRAMMAR\n";

    //parse the rule-list
    parseRuleList();

    cout << "FINISHED parsing THE GRAMMAR\n";

    //consume the HASH token using expect function
    expect(HASH);
}

// Rule-list -> Rule Rule-list | Rule
void Parser::parseRuleList() {

    cout << "STARTING parsing THE RULE LIST\n";

    //CFG grammar has at least one Rule, so parsing the Rule
    parseRule();

    //seeing the next toekn after parsing the Rule
    Token nextToken = lexer.peek(1);

    //after parsing one Rule, we might or might not have more Rules. so I check if the 1st token of Rule which is ID is present,
    //then it means that we have more Rules to parse, so I call the parseRule function again inside the while loop
    while (nextToken.token_type == ID) {

        cout << "More Rules present, so callinf parseRule() again from parseRuleList()\n";

        //parse the Rule
        parseRule();

        //seeing the next token after parsing the Rule, if it is ID then I can continue the loop and parse the next rule
        //if not then the loop will end.
        nextToken = lexer.peek(1);
    }

    cout << "FINISHED parsing THE RULE LIST\n";
}

// Rule -> ID ARROW Right-hand-side STAR
void Parser::parseRule() {

    cout << "STARTING parsing THE RULE\n";

    Rule newRule;

    //LHS must be ID based on the CFG grammar. so consume the ID token using expect function
    Token lhsToken = expect(ID);
    newRule.LHS = lhsToken.lexeme;
    universe.push_back(lhsToken.lexeme);

    cout << "\nPrinting IN-PROGRESS Rule so far\n";
    cout << "-------> LHS: " << newRule.LHS << endl;
    printAllRules();

    //consume the ARROW token using expect function
    expect(ARROW);

    //now RHS starts, parse the right-hand side
    parseRightHandSide(newRule.RHS);

    cout << "Parsing RHS done in Rule method\n";

    //consume the STAR token using expect function, because every rule ends with STAR token
    expect(STAR);

    cout << "\nPrinting COMPLETE Rule so far\n";
    allRules.push_back(newRule);

    printAllRules();

    cout << "FINISHED parsing THE RULE\n";
}

// Right-hand-side -> Id-list | Id-list OR Right-hand-side
//right-hand side is one or more Id-list’s separated with OR’s
void Parser::parseRightHandSide(vector<string> &rhsSymbols) {

    cout << "STARTING parsing  RIGHT HAND SIDE\n";

    //parsing the first Id-list, because every right-hand-side has atleast 1 Id-list
    parseIdList(rhsSymbols);


    //seeing the next toekn after parsing the Rule
    Token nextToken = lexer.peek(1);

    // If next token is '|', consume it and parse more Id-list
    //after parsing 1st IdList, we might or might not have more IdList. so I check if the next token after IdList is OR is present,
    //then it means that we have more IdList to parse, so I call the parseIdList function again inside the while loop
    while (nextToken.token_type == OR) {

        //consume the OR token using expect function after parsing the 1st IdList
        expect(OR);

        Token maybeStar = lexer.peek(1);

        if (maybeStar.token_type == STAR) {
            // if the alternative is epsilon (no symbols)
            // Do nothing, just continue after OR
        } else {
            // parse the next Id-list
            parseIdList(rhsSymbols);
        }


        //seeing the next token after parsing the IdList, if it is OR then I can continue the loop and parse the next IdList
        //if not then the loop will end.
        nextToken = lexer.peek(1);
    }
}

// Id-list -> ID Id-list | epsilon
//Id-list is a list of zero or more ID’s
void Parser::parseIdList(vector<string> &rhsSymbols) {

    cout << "STARTING parsing  ID LIST\n";

    //seeing the next token
    Token nextToken = lexer.peek(1);

    //checking if the next token is an ID, consume it and parse another Id-list
    //if the next toek is not ID, meaning epsilon, then I do nothing (epsilon) and the while loop ends
    if (nextToken.token_type == ID) {

        cout << "Another Id-list present, so parsing the Id-List from parseIdList() using parseIdList()\n";

        //consume the ID token using expect function, because every Id-list starts with ID
        Token rightSideToken = expect(ID);
        rhsSymbols.push_back(rightSideToken.lexeme);
        universe.push_back(rightSideToken.lexeme);

        cout << "Inside parseIDList(), printing rhsSymbols -------> RHS: ";
        printRHSSymbols(rhsSymbols);

        cout << "Inside parseIDList(), printing universe -------> RHS: ";
        printRHSSymbols(universe);


        //parse the Id-list which is present for sure
        parseIdList(rhsSymbols);

    }

    //if the next toek is not ID, meaning epsilon, then I do nothing (epsilon) and exit the function
}

//declaring the public function which will start the parsing of grammar
void Parser::startParsing() {

    //parse the grammar
    parseGrammar();
}

// read grammar
void ReadGrammar() {


}

void removeDuplicates(vector<string>& vec) {
    unordered_set<string> seen;
    vector<string> result;

    // Traverse the vector using a for loop with indices
    for (size_t i = 0; i < vec.size(); ++i) {

        // If the element is not in the set, add it to the result and mark it as seen
        if (seen.find(vec[i]) == seen.end()) {
            result.push_back(vec[i]);
            seen.insert(vec[i]);
        }
    }

    vec = result;
}


bool isNonTerminal(const string &symbol) {
    return nonTerminalsSet.find(symbol) != nonTerminalsSet.end();
}

void getNonTerminals() {
    for (auto &rule : allRules) {
        nonTerminalsSet.insert(rule.LHS);
    }
}

void getTerminals() {
    for (auto &rule : allRules) {
        for (auto &symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                terminalsSet.insert(symbol);
            }
        }
    }
}

void printSetUniverseOrder(set<string> s)
{
    bool first = true;

    // in which they appear in the universe
    for (const string &word : universe) {
        if (s.find(word) != s.end()) {
            if (!first) cout << " ";
            cout << word;
            first = false;
        }
    }
}

void printNullableSetUniverseOrder(set<string> s)
{
    bool first = true;

    // in which they appear in the universe
    for (const string &word : universe) {
        if (s.find(word) != s.end()) {
            if (!first) cout << " , ";
            cout << word;
            first = false;
        }
    }
}

void calculateNullableNonTerminals() {
    bool changed = true;

    while (changed) {
        changed = false;

        for (auto &rule : allRules) {
            const string &lhs = rule.LHS;


            if (nullableSet.find(lhs) == nullableSet.end()) {

                bool allNullable = true;


                if (rule.RHS.empty()) {
                    allNullable = true;
                } else {

                    for (auto &symbol : rule.RHS) {

                        if (terminalsSet.find(symbol) != terminalsSet.end() || nullableSet.find(symbol) == nullableSet.end()) {
                            allNullable = false;
                            break;
                        }
                    }
                }

                if (allNullable) {
                    nullableSet.insert(lhs);
                    changed = true;
                }
            }
        }
    }
}


/* 
 * Task 1: 
 * Printing the terminals, then nonterminals of grammar in appearing order
 * output is one line, and all names are space delineated
*/
void Task1()
{
    getNonTerminals();
    getTerminals();

    printSetUniverseOrder(terminalsSet);
    printSetUniverseOrder(nonTerminalsSet);
}

/*
 * Task 2:
 * Print out nullable set of the grammar in specified format.
*/
void Task2()
{

    calculateNullableNonTerminals();

    cout << "Nullable = { ";
    printNullableSetUniverseOrder(nullableSet);
    cout << " }";
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

    //declaring the parser object
    Parser parser;

    //starting the parsing of the grammar
    parser.startParsing();
    
    //ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    removeDuplicates(universe);

    switch (task) {
        case 1:
            Task1();
            cout << endl;
            // cout << "Hello world from task1" << endl;
            break;

        case 2:
            Task2();
            cout << endl;
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

