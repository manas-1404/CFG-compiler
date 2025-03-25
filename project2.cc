#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <map>
#include "lexer.h"

using namespace std;

struct Rule {
    string LHS;
    vector<string> RHS;
};

struct RuleWithLongestMatch {
    int longestMatch;
    Rule rule;
};

vector<Rule> allRules;

set<string> nonTerminalsSet;
set<string> terminalsSet;
set<string> nullableSet;

map<string, set<string>> firstsSetHashMap;
map<string, set<string>> followsSetHashMap;

vector<string> universe;

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
    void parseRightHandSide(string currentLHS);

    // Id-list -> ID Id-list | epsilon
    void parseIdList(vector<string> &rhsSymbols);

public:
    //constructor of the parser class
    void startParsing();
};

// Grammar -> Rule-list HASH
void Parser::parseGrammar() {

    //parse the rule-list
    parseRuleList();

    //consume the HASH token using expect function
    expect(HASH);
}

// Rule-list -> Rule Rule-list | Rule
void Parser::parseRuleList() {

    //CFG grammar has at least one Rule, so parsing the Rule
    parseRule();

    //seeing the next toekn after parsing the Rule
    Token nextToken = lexer.peek(1);

    //after parsing one Rule, we might or might not have more Rules. so I check if the 1st token of Rule which is ID is present,
    //then it means that we have more Rules to parse, so I call the parseRule function again inside the while loop
    while (nextToken.token_type == ID) {

        //parse the Rule
        parseRule();

        //seeing the next token after parsing the Rule, if it is ID then I can continue the loop and parse the next rule
        //if not then the loop will end.
        nextToken = lexer.peek(1);
    }
}

// Rule -> ID ARROW Right-hand-side STAR
void Parser::parseRule() {

    Rule newRule;

    //LHS must be ID based on the CFG grammar. so consume the ID token using expect function
    Token lhsToken = expect(ID);
    string currentLHS = lhsToken.lexeme;
    newRule.LHS = currentLHS;

    //pushing the LHS to the universe vector, i am doing it only to preserve the grammar order
    universe.push_back(currentLHS);

    //consume the ARROW token using expect function
    expect(ARROW);

    //now RHS starts, parse the right-hand side
    parseRightHandSide(currentLHS);

    //consume the STAR token using expect function, because every rule ends with STAR token
    expect(STAR);

}

// Right-hand-side -> Id-list | Id-list OR Right-hand-side
//right-hand side is one or more Id-list’s separated with OR’s
void Parser::parseRightHandSide(string currentLHS) {

    //initializing 1st token just to check for edge case
    Token firstToken = lexer.peek(1);

    //edge case: if the first RHS is epsilon only
    //what if while parsing the rule, A -> * , is present, here RHS = * ;LHS = A(passed down), then i will only need to add the empty rule and move ahead to next new rule
    if (firstToken.token_type == STAR) {

        //declaring rule called epsilonRule for epsilon case
        Rule epsilonRule;

        //LHS of epsilon rule is the current LHS passed down by the function
        epsilonRule.LHS = currentLHS;

        //RHS of epsilon rule is empty list because it is epsilon
        epsilonRule.RHS = {};

        //pushing the epsilon rule to the allRules vector
        allRules.push_back(epsilonRule);

        //you dont have to continue parsing because the rule has ended
        return;
    }

    //the 1st token is no epsilon, so its a regular case

    //declaring new rule for each rule of same LHS
    Rule newEachRule;

    //LHS of the new rule is the current LHS passed down by the function
    newEachRule.LHS = currentLHS;

    //parsing the first Id-list, because every right-hand-side has atleast 1 Id-list (Id-list meaning C D | E F | * etc etc)
    parseIdList(newEachRule.RHS);

    //pushing the new rule to the allRules vector
    allRules.push_back(newEachRule);


    //seeing the next toekn after parsing the Rule
    Token nextToken = lexer.peek(1);

    // If next token is '|', consume it and parse more Id-list
    //after parsing 1st IdList, we might or might not have more IdList. so I check if the next token after IdList is OR is present,
    //then it means that we have more IdList to parse, so I call the parseIdList function again inside the while loop
    while (nextToken.token_type == OR) {

        //consume the OR token using expect function after parsing the 1st IdList
        expect(OR);

        //initializing the next token after OR, now it could either be a STAR token or an ID token
        // A -> B C | D b | *
        //everytime after the OR token, it can either be the next duplicate rule (A -> D b) or it can be the epsilon rule (A -> *)
        Token maybeStar = lexer.peek(1);

        //if the next token is STAR, then its epsilon rule case and I just need to make RHS = {} and append the rule to allRules
        if (maybeStar.token_type == STAR) {

            //now, we have the epsilon rule, A -> *

            //declaring epsilon rule
            Rule epsilonRule;

            //LHS of epsilon rule is the current LHS passed down by the function
            epsilonRule.LHS = currentLHS;

            //RHS of epsilon rule is empty list because it is epsilon
            epsilonRule.RHS = {};

            //pushing the epsilon rule to the allRules vector
            allRules.push_back(epsilonRule);
        } else {

            //now, we have another duplicate rule of same LHS, A -> D b

            //declaring duplicate rule
            Rule duplicateRule;

            //LHS of duplicate rule is the current LHS passed down by the function
            duplicateRule.LHS = currentLHS;

            // parse the next Id-list, this Id-list will recursively append D, b to the RHS vector of the duplicate rule
            parseIdList(duplicateRule.RHS);

            //pushing the duplicate rule to the allRules vector
            allRules.push_back(duplicateRule);
        }


        //seeing the next token after parsing the IdList, if it is OR then I can continue the loop and parse the next IdList
        //if not then the loop will end.
        nextToken = lexer.peek(1);
    }
}

// Id-list -> ID Id-list | epsilon
//Id-list is a list of zero or more ID’s
void Parser::parseIdList(vector<string> &rhsSymbols) {

    //seeing the next token
    Token nextToken = lexer.peek(1);

    //checking if the next token is an ID, consume it and parse another Id-list
    //if the next toek is not ID, meaning epsilon, then I do nothing (epsilon) and the while loop ends
    if (nextToken.token_type == ID) {

        //consume the ID token using expect function, because every Id-list starts with ID
        Token rightSideToken = expect(ID);

        //pushing the ID to the RHS vector
        rhsSymbols.push_back(rightSideToken.lexeme);

        //pushing the ID to the universe vector, I am doing it only to preserve the grammar order
        universe.push_back(rightSideToken.lexeme);

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

string ruleToString(Rule &rule) {
    string result = rule.LHS + "->";
    for (int i = 0; i < rule.RHS.size(); i++) {
        result += rule.RHS[i];
        if (i < rule.RHS.size() - 1) {
            result += " ";
        }
    }
    return result;
}

vector<Rule> removeDuplicateRules(vector<Rule> &rules) {
    unordered_set<string> seen;
    vector<Rule> result;

    for (int i = 0; i < rules.size(); i++) {
        string key = ruleToString(rules[i]);

        if (seen.find(key) == seen.end()) {
            result.push_back(rules[i]);
            seen.insert(key);
        }
    }

    return result;
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

void printFirstSets() {
    for (int i = 0; i < (int)universe.size(); i++) {
        string nonTerminal = universe[i];

        //only printing for non-terminals
        if (nonTerminalsSet.find(nonTerminal) != nonTerminalsSet.end()) {
            cout << "FIRST(" << nonTerminal << ") = { ";

            bool first = true;
            for (int j = 0; j < (int)universe.size(); j++) {
                string symbol = universe[j];

                if (firstsSetHashMap[nonTerminal].find(symbol) != firstsSetHashMap[nonTerminal].end()) {
                    if (!first) cout << ", ";
                    cout << symbol;
                    first = false;
                }
            }
            cout << " }" << endl;
        }
    }
}


void calculateFirstSets() {
    //initialize each non-terminal's FIRST set to be empty
    for (auto &nt : nonTerminalsSet) {
        firstsSetHashMap[nt].clear();
    }


    bool changed = true;
    while (changed) {
        changed = false;

        //Going through each rule A -> α in allRules
        for (int i = 0; i < (int)allRules.size(); i++) {
            // Example: A -> X Y Z
            string A = allRules[i].LHS;
            vector<string> alpha = allRules[i].RHS;

            //If alpha is empty, then add "epsilon" to FIRST(A)
            if (alpha.empty()) {
                if (firstsSetHashMap[A].find("epsilon") == firstsSetHashMap[A].end()) {
                    firstsSetHashMap[A].insert("epsilon");
                    changed = true;
                }

                //done with this rule, so skip any future processin in this iteration
                continue;
            }

            //processing alpha from left to right
            bool allNullable = true;
            for (int j = 0; j < (int)alpha.size(); j++) {
                string symbol = alpha[j];

                //if symbol is a terminal, then add symbol to FIRST(A) and stop
                if (terminalsSet.find(symbol) != terminalsSet.end()) {

                    //inserting terminal
                    if (firstsSetHashMap[A].find(symbol) == firstsSetHashMap[A].end()) {
                        firstsSetHashMap[A].insert(symbol);
                        changed = true;
                    }

                    allNullable = false;
                    break;
                } else {

                    //if the symbol is a non-terminal, union its FIRST set minus epsilon into FIRST(A)
                    int oldSize = (int)firstsSetHashMap[A].size();

                    //inserting FIRST(symbol) - "epsilon" into FIRST(A)
                    for (auto &sym : firstsSetHashMap[symbol]) {
                        if (sym != "epsilon") {
                            firstsSetHashMap[A].insert(sym);
                        }
                    }

                    //if FIRST(A) becomes big, then mark changed
                    if ((int)firstsSetHashMap[A].size() > oldSize) {
                        changed = true;
                    }

                    //if symbol is not nullable, stop
                    if (nullableSet.find(symbol) == nullableSet.end()) {
                        allNullable = false;
                        break;
                    }
                    //else keep it goinggg
                }
            }

            //if it never "broke out", then all symbols in alpha are nullable and add epsilon
            if (allNullable) {
                if (firstsSetHashMap[A].find("epsilon") == firstsSetHashMap[A].end()) {
                    firstsSetHashMap[A].insert("epsilon");
                    changed = true;
                }
            }
        }
    }
}



bool unionInsert(set<string> &dest, const set<string> &src) {
    bool changed = false;
    for (auto &x : src) {
        if (dest.find(x) == dest.end()) {
            dest.insert(x);
            changed = true;
        }
    }
    return changed;
}

void calculateFollowSets() {

    if (!universe.empty()) {
        followsSetHashMap[universe[0]].insert("$");
    }

    bool changed = true;
    while (changed) {
        changed = false;

        //for each and every rule A -> α
        for (auto &rule : allRules) {
            string lhs = rule.LHS;
            vector<string> rhs = rule.RHS;

            //keeping trailer set
            //Initially = FOLLOW(A)
            set<string> trailer = followsSetHashMap[lhs];

            //traversing through RHS from right to left
            for (int i = rhs.size() - 1; i >= 0; i--) {
                string currSym = rhs[i];

                //if currSym is a non-terminal
                if (nonTerminalsSet.find(currSym) != nonTerminalsSet.end()) {

                    //adding trailer to FOLLOW(currSym)
                    int beforeSize = followsSetHashMap[currSym].size();
                    followsSetHashMap[currSym].insert(trailer.begin(), trailer.end());

                    if (followsSetHashMap[currSym].size() > beforeSize) {
                        changed = true;
                    }

                    //if FIRST(currSym) has epsilon, then trailer union =FIRST(currSym) - {epsilon}
                    //else trailer = FIRST(currSym) - epsilon
                    if (firstsSetHashMap[currSym].find("epsilon") != firstsSetHashMap[currSym].end()) {
                        // therefore currSym can vanish =>
                        // so union FIRST(currSym) minus epsilon goes into trailer
                        for (auto &x : firstsSetHashMap[currSym]) {
                            if (x != "epsilon") {
                                trailer.insert(x);
                            }
                        }
                    } else {
                        //symbol cannot vanish, so trailer = FIRST(currSym) minus epsilon
                        set<string> newTrailer;
                        for (auto &x : firstsSetHashMap[currSym]) {
                            if (x != "epsilon") {
                                newTrailer.insert(x);
                            }
                        }
                        trailer = newTrailer;
                    }
                }
                //if currSym is a terminal
                else {

                    trailer.clear();
                    trailer.insert(currSym);
                }
            }
        }
    }
}


void printFollowSets() {
    for (int i = 0; i < (int)universe.size(); i++) {
        string nonTerminal = universe[i];

        //printing only for non-terminals
        if (nonTerminalsSet.find(nonTerminal) != nonTerminalsSet.end()) {
            cout << "FOLLOW(" << nonTerminal << ") = { ";

            bool first = true;

            //$ aklways appears first if it's in the FOLLOW set
            if (followsSetHashMap[nonTerminal].find("$") != followsSetHashMap[nonTerminal].end()) {
                cout << "$";
                first = false;
            }

            //printing elements in the order they appear in the sorted universe
            for (int j = 0; j < (int)universe.size(); j++) {
                string symbol = universe[j];

                if (symbol != "$" && followsSetHashMap[nonTerminal].find(symbol) != followsSetHashMap[nonTerminal].end()) {
                    if (!first) cout << ", ";
                    cout << symbol;
                    first = false;
                }
            }

            cout << " }" << endl;
        }
    }
}


bool isRuleBefore(Rule r1, Rule r2) {

    vector<string> seq1 = {r1.LHS};
    vector<string> seq2 = {r2.LHS};

    seq1.insert(seq1.end(), r1.RHS.begin(), r1.RHS.end());
    seq2.insert(seq2.end(), r2.RHS.begin(), r2.RHS.end());

    int len = min(seq1.size(), seq2.size());
    for (int i = 0; i < len; i++) {
        if (seq1[i] < seq2[i]) {
            return true;
        } else if (seq1[i] > seq2[i]) {
            return false;
        }
    }

    return seq1.size() < seq2.size();
}


int getCommonPrefixLength(Rule r1, Rule r2) {
    // Only compare if LHS is the same
    if (r1.LHS != r2.LHS) return 0;

    int len = min(r1.RHS.size(), r2.RHS.size());
    int count = 0;

    for (int i = 0; i < len; i++) {
        if (r1.RHS[i] == r2.RHS[i]) {
            count++;
        } else {
            break;
        }
    }

    return count;
}

bool compareRuleLongMatch(RuleWithLongestMatch &a, RuleWithLongestMatch &b) {
    //first compare descending longestMatch
    if (a.longestMatch != b.longestMatch) {
        return a.longestMatch > b.longestMatch;
    }

    //then compare lex order by your existing utility isRuleBefore(r1, r2)
    return isRuleBefore(a.rule, b.rule);
}


vector<Rule> findLongestMatchesAndSort(vector<Rule> &grammar) {
    //creating a local vector of (longestMatch, Rule)
    vector<RuleWithLongestMatch> temp;

    //for each rule, find the maximum prefix length with any other rule having same LHS
    for (int i = 0; i < grammar.size(); i++) {
        int maxLen = 0;
        // Only compare with rules that share the same LHS
        for (int j = 0; j < grammar.size(); j++) {
            if (i == j) {
                continue;
            }

            if (grammar[i].LHS == grammar[j].LHS) {


                int len = getCommonPrefixLength(grammar[i], grammar[j]);
                if (len > maxLen) {
                    maxLen = len;
                }
            }
        }

        //storing the result
        RuleWithLongestMatch rlm;
        rlm.longestMatch = maxLen;
        rlm.rule = grammar[i];
        temp.push_back(rlm);
    }

    //sorting temp using our named compare function
    sort(temp.begin(), temp.end(), compareRuleLongMatch);

    //building the sorted grammar
    vector<Rule> sortedGrammar;
    for (auto &item : temp) {
        sortedGrammar.push_back(item.rule);
    }

    //returning the new grammar in correct order
    return sortedGrammar;
}

static int nextFactorIndex = 1; // e.g. used to generate "A1", "A2", etc. across multiple runs

// helper: returns e.g. "A1" or "A2" for an LHS "A"
string generateFactoredName(const string &lhs, int count) {
    // e.g. "A" + to_string(count) => "A1", "A2"
    return lhs + to_string(count);
}

// extractPrefix(r, length) => returns first 'length' symbols of r.RHS
vector<string> extractPrefix(const Rule &r, int length) {
    vector<string> result;
    for (int i = 0; i < length && i < (int)r.RHS.size(); i++) {
        result.push_back(r.RHS[i]);
    }
    return result;
}

// extractAllButPrefixOfSize(r, length) => returns RHS after skipping 'length' symbols
vector<string> extractAllButPrefixOfSize(const Rule &r, int length) {
    vector<string> result;
    for (int i = length; i < (int)r.RHS.size(); i++) {
        result.push_back(r.RHS[i]);
    }
    return result;
}

void printRules(const vector<Rule>& rules) {
    for (const Rule& rule : rules) {
        cout << rule.LHS << " -> ";

        if (rule.RHS.empty()) {
            cout << "#"; // Using "#" to denote epsilon
        } else {
            for (size_t i = 0; i < rule.RHS.size(); ++i) {
                cout << rule.RHS[i];
                if (i < rule.RHS.size() - 1) {
                    cout << " ";
                }
            }
        }

        cout << endl;
    }
}


void printRHS(const vector<string>& rhs) {
    if (rhs.empty()) {
        cout << "#";
        return;
    }

    for (size_t i = 0; i < rhs.size(); ++i) {
        cout << rhs[i];
        if (i < rhs.size() - 1) {
            cout << " ";
        }
    }
}

bool areRulesEqual(const Rule &r1, const Rule &r2) {


    if (r1.LHS != r2.LHS) return false;

    // Then compare RHS lengths
    if (r1.RHS.size() != r2.RHS.size()) return false;

    // Finally compare each symbol in the RHS
    for (int i = 0; i < (int)r1.RHS.size(); i++) {
        if (r1.RHS[i] != r2.RHS[i]) {
            return false;
        }
    }
    return true;
}




// performs the entire left factoring process
// vector<Rule> performLeftFactoring(vector<Rule> grammar) {
//
//     vector<Rule> newGrammar;
//
//     // cout << "Printing the grammar\n";
//     // printRules(grammar);
//     // cout << "----------------------\n";
//
//
//     bool done = false;
//     while (!done) {
//
//         vector<Rule> sorted = findLongestMatchesAndSort(grammar);
//
//         // cout << "Starting while loop of left factoring\n";
//         // printRules(sorted);
//         // cout << "----------------------\n";
//
//         if (sorted.empty()) {
//
//             // cout << "Sorted vector is empty so stopping now\n";
//             break;
//         }
//
//         int maxLen = 0;
//         Rule topRule = sorted[0];
//
//         //here you are finding out the maxLen of common sequence of RHS rules, nothing else is happening here
//         for (auto &r : grammar) {
//
//             // cout << "_________________" << endl;
//             // cout << "Comparing 2 rules of : \n";
//             // cout << "Current r: " << r.LHS << " -> ";
//             // printRHS(r.RHS);
//             // cout << endl;
//             //
//             // cout << "Toprule: " << topRule.LHS << " -> ";
//             // printRHS(topRule.RHS);
//             // cout << endl;
//
//             if (r.LHS == topRule.LHS && !areRulesEqual(r, topRule)) {
//
//                 // cout << "LHS of R and TopRule are same and R and Toprule are not eqauly\n";
//                 int len = getCommonPrefixLength(topRule, r);
//
//                 // cout << "Common prefix length(toprule, r) is: " << len << endl;
//
//                 if (len > maxLen) {
//
//                     // cout << "Common prefix length is greater than maxLen. So making new maxLen\n";
//                     maxLen = len;
//                 }
//             }
//         }
//
//         // cout << "MaxLen is: " << maxLen << endl;
//
//         if (maxLen == 0) {
//
//             // cout << "Inside maxLen == 0 or maxLen == topRule.RHS.size() condition\n";
//
//             sort(grammar.begin(), grammar.end(), isRuleBefore);
//
//             // cout << "Printing the sorted grammar\n";
//
//             // printRules(grammar);
//
//             for (auto &r : grammar) {
//                 newGrammar.push_back(r);
//             }
//
//             // cout << "printing the new grammar\n";
//
//             // printRules(newGrammar);
//
//             grammar.clear();
//             done = true;
//
//             // cout << "Done is true now. So while loop shoudl stop\n";
//             continue;
//         }
//         else {
//
//             // cout << "maxLen is not zero or topRule.RHS.size(). So we have to do left factoring\n";
//
//             // cout << "Toprule is: " << topRule.LHS << " -> ";
//             // printRHS(topRule.RHS);
//             // cout << endl;
//
//             // cout << "MaxLen is: " << maxLen << endl;
//
//             vector<string> thePrefix = extractPrefix(topRule, maxLen);
//
//             // cout << "The prefix is: ";
//             // printRHS(thePrefix);
//             // cout << endl;
//
//             string newNt = generateFactoredName(topRule.LHS, nextFactorIndex++);
//
//             // cout << "New non-terminal is: " << newNt << endl;
//
//             vector<Rule> sharedPrefixRules;
//
//
//             vector<Rule> keepInGrammar;
//
//             for (auto &r : grammar) {
//
//                 // cout << "~~~~~~~~~~~~~~~~~~~~" << endl;
//                 // cout << "Comparing current rule: " << r.LHS << " -> ";
//                 // printRHS(r.RHS);
//                 // cout << endl;
//
//                 // cout << "Toprule: " << topRule.LHS << " -> ";
//                 // printRHS(topRule.RHS);
//                 // cout << endl;
//
//                 if (r.LHS == topRule.LHS && !areRulesEqual(r, topRule)) {
//
//                     // cout << "LHS of R and TopRule are same\n";
//
//                     int len = getCommonPrefixLength(r, topRule);
//
//                     // cout << "Common prefix length(r, toprule) is: " << len << endl;
//
//
//                     if (len == maxLen) {
//
//                         // cout << "Common prefix length is equal to maxLen\n";
//                         sharedPrefixRules.push_back(r);
//                         sharedPrefixRules.push_back(topRule);
//                         // cout << "Pushed the rule to sharedPrefixRules\n";
//
//                     } else {
//
//                         // cout << "Common prefix length is not equal to maxLen, so pushing to keepInGrammar\n";
//                         keepInGrammar.push_back(r);
//                     }
//                 } else if (areRulesEqual(r, topRule)) {
//
//                     // cout << "R and TopRule are EQUAL, so DO NOT push anything to keepInGrammar!!!!!!!!!\n";
//                     // keepInGrammar.push_back(r);
//                 } else {
//
//                     // cout << "LHS of R and TopRule are not same, so pushing to keepInGrammar\n";
//                     keepInGrammar.push_back(r);
//                 }
//             }
//
//             // cout << "++++++++++++++++++++++++++++++\n";
//             // cout << "Printing the sharedPrefixRules\n";
//             // printRules(sharedPrefixRules);
//             // cout << endl;
//
//             // cout << "Printing the keepInGrammar\n";
//             // printRules(keepInGrammar);
//             // cout << endl;
//             // cout << "++++++++++++++++++++++++++++++\n";
//
//
//
//             Rule factoredRule;
//             factoredRule.LHS = topRule.LHS;
//             factoredRule.RHS = thePrefix;
//             factoredRule.RHS.push_back(newNt);
//
//             //here you actually push the new rule into the keepInGrammar
//             //A -> C B C B and A -> C B C D, here you ppush A -> C B C A1
//             keepInGrammar.push_back(factoredRule);
//
//             // cout << "After Pushing the factored rule to keepInGrammar\n";
//             // printRules(keepInGrammar);
//
//             sharedPrefixRules = removeDuplicateRules(sharedPrefixRules);
//             // cout << "After removing the duplicate rules from sharedPrefixRules\n";
//             // printRules(sharedPrefixRules);
//             // cout << endl;
//
//             for (auto &r : sharedPrefixRules) {
//                 Rule newNtRule;
//                 newNtRule.LHS = newNt;
//
//                 newNtRule.RHS = extractAllButPrefixOfSize(r, maxLen);
//
//                 // cout << "New rule is: " << newNtRule.LHS << " -> ";
//                 // printRHS(newNtRule.RHS);
//                 // cout << endl;
//
//                 newGrammar.push_back(newNtRule);
//
//                 // cout << "After Pushing the new NT rule to newGrammar\n";
//                 // printRules(newGrammar);
//             }
//
//             grammar = keepInGrammar;
//
//             // cout << "After updating the grammar with keepInGrammar\n";
//             // printRules(grammar);
//             // cout << "end of while loop\n";
//         }
//     }
//
//     sort(newGrammar.begin(), newGrammar.end(), isRuleBefore);
//
//     // cout << "Printing the final new grammar\n";
//     // printRules(newGrammar);
//     // cout << "---------------------------\n";
//
//     return newGrammar;
// }
//


vector<Rule> performLeftFactor() {

    //this vector is used to store the new rules like A -> B C D, A -> B C E, A -> B C F, A -> B C G
    vector<Rule> newRules;

    //this vector is used to store the rules which are left factored
    vector<Rule> leftFactoredRules;

    vector<Rule> rulesWithoutLeftFactor;

    //using this vector to store the remaining grammar and later equating it to the grammarRules
    vector<Rule> remainingGrammar;

    //this vector is used to return the final results
    vector<Rule> finalResultGrammar;

    //this vector contains the sorted grammar rules on basis of longest match and then lexically
    vector<Rule> grammarRules = findLongestMatchesAndSort(allRules);

    while (grammarRules.size() >= 2) {

        newRules.clear();
        rulesWithoutLeftFactor.clear();
        remainingGrammar.clear();


        Rule firstRule = grammarRules[0];

        int maxCommonLength = getCommonPrefixLength(firstRule, grammarRules[1]);

        if (maxCommonLength != 0) {

            for (int i = 1; i < grammarRules.size(); i++) {

                if (firstRule.LHS == grammarRules[i].LHS) {
                    int commonLength = getCommonPrefixLength(firstRule, grammarRules[i]);

                    if (maxCommonLength == commonLength) {
                        newRules.push_back(grammarRules[i]);
                    }
                }
            }

            //add the 1st rule only after adding all the other common same prefix rules
            newRules.push_back(firstRule);

            //removing the rules of A -> alpha X from the grammarRules
            for (int i = 0; i < grammarRules.size(); i++) {
                bool isMatch = false;

                for (int j = 0; j < newRules.size(); j++) {
                    if (areRulesEqual(grammarRules[i], newRules[j])) {
                        isMatch = true;
                        break;
                    }
                }

                if (!isMatch) {
                    rulesWithoutLeftFactor.push_back(grammarRules[i]);
                }
            }

            Rule factoredRule;
            factoredRule.LHS = firstRule.LHS;
            factoredRule.RHS = extractPrefix(firstRule, maxCommonLength);

            string newNT = generateFactoredName(firstRule.LHS, nextFactorIndex++);

            factoredRule.RHS.push_back(newNT);

            rulesWithoutLeftFactor.push_back(factoredRule);

            for (int k = 0; k < newRules.size(); k++) {
                Rule newRule;
                newRule.LHS = newRules[k].LHS;
                newRule.RHS = extractAllButPrefixOfSize(newRules[k], maxCommonLength);

                finalResultGrammar.push_back(newRule);
            }

            grammarRules = rulesWithoutLeftFactor;

        }

        else {

            for (int i = 0; i < grammarRules.size(); i++) {

                if (firstRule.LHS == grammarRules[i].LHS) {
                    finalResultGrammar.push_back(grammarRules[i]);
                }else {
                    remainingGrammar.push_back(grammarRules[i]);
                }

            }

            grammarRules = remainingGrammar;

        }
    }

    return finalResultGrammar;
}



void printTask5Grammar(const vector<Rule> &factoredGrammar) {
    //making a copy so we can sort
    vector<Rule> sortedGrammar = factoredGrammar;

    //sorting by your isRuleBefore function
    sort(sortedGrammar.begin(), sortedGrammar.end(), isRuleBefore);

    //printing each rule in the specified format
    for (auto &r : sortedGrammar) {
        cout << r.LHS << " -> ";
        if (r.RHS.empty()) {


            cout << "#\n";
        } else {
            //printing all symbols in the RHS, separated by space
            for (int i = 0; i < (int)r.RHS.size(); i++) {
                cout << r.RHS[i];
                if (i < (int)r.RHS.size() - 1) {
                    cout << " ";
                }
            }
            cout << " #\n";
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
    printSetUniverseOrder(terminalsSet);
    printSetUniverseOrder(nonTerminalsSet);
}

/*
 * Task 2:
 * Print out nullable set of the grammar in specified format.
*/
void Task2()
{
    cout << "Nullable = {";
    printNullableSetUniverseOrder(nullableSet);
    cout << "}";
}

// Task 3: FIRST sets
void Task3()
{
    printFirstSets();
}

// Task 4: FOLLOW sets
void Task4()
{
    printFollowSets();
}

// Task 5: left factoring
void Task5()
{
    vector<Rule> finalFactoredGrammar = performLeftFactor();
    printTask5Grammar(finalFactoredGrammar);
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

    //start calculating all the important sets right now and store them in the global sets
    getNonTerminals();
    getTerminals();

    calculateNullableNonTerminals();
    calculateFirstSets();
    calculateFollowSets();

    switch (task) {
        case 1:
            Task1();
            cout << endl;
            // cout << "Hello world from task1" << endl;
            break;

        case 2:
            Task2();
            break;

        case 3:
            Task3();
            break;

        case 4:
            Task4();
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

