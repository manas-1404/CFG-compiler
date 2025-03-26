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


//method which will return true if the Rule R1 is before Rule R2 in dictionary order, else it will return false
bool isRuleBefore(Rule r1, Rule r2) {

    //declaring 2 vectors to store the sequence of symbols in the rules
    // initializing ghem with the LHS of both the rules
    vector<string> seq1 = {r1.LHS};
    vector<string> seq2 = {r2.LHS};

    //appending the entire RHS of the rules R1 and R2 into the seq1 and seq2 vectors
    seq1.insert(seq1.end(), r1.RHS.begin(), r1.RHS.end());
    seq2.insert(seq2.end(), r2.RHS.begin(), r2.RHS.end());

    //calculating the minimum length of the 2 sequences, because we can only travserse till the minimum length of both the rules
    int len = min(seq1.size(), seq2.size());

    //looping through the minimum length
    for (int i = 0; i < len; i++) {

        //check if the seq1 symbol is less than seq2 symbol
        if (seq1[i] < seq2[i]) {

            //returning true because seq1 symbol is less than seq2 symbol
            return true;
        }

        //check if the seq1 symbol is greater than seq2 symbol
        else if (seq1[i] > seq2[i]) {

            //returning false because seq1 symbol is greater than seq2 symbol
            return false;
        }
    }

    //if the Rules are equal up to the shorter one, then shorter rule comes first
    return seq1.size() < seq2.size();
}


//method whichi will return the length of the common prefixes by comparing the RHS of the rule R1 and R2
int getCommonPrefixLength(Rule r1, Rule r2) {

    //comparing if LHS is the same in both the rules
    if (r1.LHS != r2.LHS) {

        //return 0 because there is no common prefix if the rules have different LHS
        return 0;
    }

    //calculating the minimumm of the rhs size of both the rules
    int len = min(r1.RHS.size(), r2.RHS.size());

    //initializing the count of common prefix to 0
    int count = 0;

    //traversing through the RHS of the minimum length and comparing the symbols
    for (int i = 0; i < len; i++) {

        //checking if the RHS symbols are the same in both the rules
        if (r1.RHS[i] == r2.RHS[i]) {

            //incrementing the count of common prefix
            count++;
        } else {

            //the symbols are no longer the same, so i am breaking the for loop
            break;
        }
    }

    //returning the count of common prefix
    return count;
}


//method which will compare the Rules based on the longestMatch of common prefix and then lexically
bool compareRuleLongMatch(RuleWithLongestMatch &a, RuleWithLongestMatch &b) {

    //first comparing if the longestMatch of 2 rules is not same
    if (a.longestMatch != b.longestMatch) {

        //returning true if the longestMatch of a is greater than b, else false
        return a.longestMatch > b.longestMatch;
    }

    //then compare lexically which rule comes first using the isRuleBefore function, which will return true if a is before b, else false
    return isRuleBefore(a.rule, b.rule);
}


//method to find the longest matches of the rules in the grammar, then later sort is 1st on longestMatch and then lexically
vector<Rule> findLongestMatchesAndSort(vector<Rule> &grammar) {

    //creating a temp vector of struct (longestMatch, Rule)
    vector<RuleWithLongestMatch> temp;


    //the outer loop is used to take the Rule 1 and then compare it with the other rules one by one in the inner loop
    //looping through to find the maximum prefix length with any other rule having same the LHS
    for (int i = 0; i < grammar.size(); i++) {

        //initializing the maxLen to 0
        int maxLen = 0;

        //using the inner loop to traverse through the grammar and compare the Rule 1 with all the other rules
        for (int j = 0; j < grammar.size(); j++) {

            //if i am comparing the same rule, then i will skip the comparison
            if (i == j) {

                //skipping the comparison
                continue;
            }

            //comparing only if the LHS of the both the Rule 1 (outer loop) and Rule j (inner loop) is the same
            if (grammar[i].LHS == grammar[j].LHS) {

                //getting the length of the common prefix of the RHS of Rule 1 and Rule j of the grammar
                int len = getCommonPrefixLength(grammar[i], grammar[j]);

                //if the length of the common prefix is greater than the maxLen, then I will update the maxLen because i found a new maxiumum
                if (len > maxLen) {

                    //updating the maxLen
                    maxLen = len;
                }
            }
        }

        //storing the results of the comparison between Rule 1 and Rule j in a new struct
        RuleWithLongestMatch rlm;

        //storing the maxLen and the Rule 1 in the struct
        rlm.longestMatch = maxLen;
        rlm.rule = grammar[i];

        //appending the struct to the temp vector
        temp.push_back(rlm);
    }

    //now i will start the sorting of the rules based on longestMatch and then lexically

    //sorting temp using the compare function which will compare the rules based on the longestMatch and then lexically
    sort(temp.begin(), temp.end(), compareRuleLongMatch);

    //declaring a new vector of Rules to store the sorted grammar
    vector<Rule> sortedGrammar;

    //traversing through the temp vector and appending the rules to the sortedGrammar vector
    for (auto &item : temp) {

        //appending the rule to the sortedGrammar vector
        sortedGrammar.push_back(item.rule);
    }

    //returning the new grammar in correct order
    return sortedGrammar;
}


//method which will returns the nezt non-terminal name during left factoring, A1 or A2 for an LHS A
string generateFactoredName(const string &lhs, int count) {

    //concatenating the LHS with the count string,  A + to_string(count) => A1, A2
    return lhs + to_string(count);
}


//method which will return the prefix of a Rule till the length given
vector<string> extractPrefix(const Rule &r, int length) {

    //declaring a vector to store the result
    vector<string> result;

    //traversing through the exact prefix length
    for (int i = 0; i < length && i < r.RHS.size(); i++) {

        //appending the symbol to the result vector
        result.push_back(r.RHS[i]);
    }

    //returning the result vector
    return result;
}


//method which will return a next part after a certain lenght of the RHS of a Rule
vector<string> extractAllButPrefixOfSize(const Rule &r, int length) {

    //declaring a vector to store the result
    vector<string> result;

    //traversing through the i = length till the end of the RHS of the given Rule
    for (int i = length; i < r.RHS.size(); i++) {

        //appending the symbol to the result vector
        result.push_back(r.RHS[i]);
    }

    //returning the result vector
    return result;
}

//method to print the rules
void printRules(const vector<Rule>& rules) {
    for (const Rule& rule : rules) {
        cout << rule.LHS << " -> ";

        if (rule.RHS.empty()) {
            cout << "*";
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

//method to print the RHS of the rules
void printRHS(const vector<string>& rhs) {
    if (rhs.empty()) {
        cout << "*";
        return;
    }

    for (size_t i = 0; i < rhs.size(); ++i) {
        cout << rhs[i];
        if (i < rhs.size() - 1) {
            cout << " ";
        }
    }
}


//method whcih will return true if the 2 rules are equal, else false
bool areRulesEqual(const Rule &r1, const Rule &r2) {

    //checking if the LHS of both the rules are the same
    if (r1.LHS != r2.LHS) {

        //returning false because the LHS of both rules are not the same
        return false;
    }

    //checking if the RHS size of both the rules are the same
    if (r1.RHS.size() != r2.RHS.size()) {

        //returning false because the RHS size of both rules are not the same
        return false;
    }

    //checking if each and every element of the RHS of both the rules are the same
    for (int i = 0; i < r1.RHS.size(); i++) {

        //checking if the current RHS symbol are the same in both the rules
        if (r1.RHS[i] != r2.RHS[i]) {

            //returning false because the RHS symbol of both rules are not the same
            return false;
        }
    }

    //nothing went wrong, meaning till now everything is the same in both the rules
    //so returning true
    return true;
}


vector<Rule> performLeftFactor() {

    //this vector is used to store the new rules like A -> B C D, A -> B C E, A -> B C F, A -> B C G
    vector<Rule> newRules;

    //this vector is used to keep track of the rules without the left factored rules
    vector<Rule> rulesWithoutLeftFactor;

    //using this vector to store the remaining grammar and later equating it to the grammarRules
    vector<Rule> remainingGrammar;

    //this vector is used to return the final results
    vector<Rule> finalResultGrammar;

    //this vector contains the sorted grammar rules on basis of longest match and then lexically
    vector<Rule> grammarRules = findLongestMatchesAndSort(allRules);

    map<string, int> mapOfNonTerminalandIndex;

    for (auto nonTerminal : nonTerminalsSet) {
        mapOfNonTerminalandIndex[nonTerminal] = 0;
    }


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

            int currentIndexOfNonTerminal = mapOfNonTerminalandIndex[firstRule.LHS] + 1;

            string newNT = generateFactoredName(firstRule.LHS, currentIndexOfNonTerminal);

            mapOfNonTerminalandIndex[firstRule.LHS] = currentIndexOfNonTerminal;

            factoredRule.RHS.push_back(newNT);

            rulesWithoutLeftFactor.push_back(factoredRule);

            for (int k = 0; k < newRules.size(); k++) {
                Rule newRule;
                newRule.LHS = newNT;
                newRule.RHS = extractAllButPrefixOfSize(newRules[k], maxCommonLength);

                finalResultGrammar.push_back(newRule);
            }

            grammarRules = findLongestMatchesAndSort(rulesWithoutLeftFactor);

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

    if (grammarRules.size() == 1) {
        finalResultGrammar.push_back(grammarRules[0]);
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


vector<Rule> getRulesForNonTerminal(string &nonTerminal, vector<Rule> &grammar) {
    vector<Rule> result;

    for (Rule &rule : grammar) {
        if (rule.LHS == nonTerminal) {
            result.push_back(rule);
        }
    }

    return result;
}


//expanding a rule A -> B R using all the rules of B.
vector<Rule> expandRulesForNonterminal(const string &A, const vector<Rule> &bRules,const vector<string> &suffixR){

    vector<Rule> result;

    //for each rule of B: B -> something (call it Ri)
    //building a new rule A -> (Ri followed by R)
    for (const Rule &ruleB : bRules)
    {
        // ruleB.RHS is the sequence for that production of B
        Rule newRule;
        newRule.LHS = A;

        //copying all symbols from ruleB.RHS
        for (const string &symbol : ruleB.RHS) {
            newRule.RHS.push_back(symbol);
        }

        //appending the suffix R at the end
        for (const string &symbol : suffixR) {
            newRule.RHS.push_back(symbol);
        }

        //finaly i get the rule,  A -> ruleB.RHS + suffixR
        result.push_back(newRule);
    }

    return result;
}


//separate left-recursive and non-left-recursive rules for a given non-terminal A
pair<vector<Rule>, vector<Rule>> splitLeftRecursiveRules(string &nonTerminal, vector<Rule> &rulesForA){

    vector<Rule> leftRecursive;
    vector<Rule> nonLeftRecursive;

    for (const Rule &rule : rulesForA)
    {
        //just doing a sanity check for rule must belong to A
        if (rule.LHS != nonTerminal) {
            continue;
        }

        //rule is left-recursive if RHS is non-empty and starts with A
        if (!rule.RHS.empty() && rule.RHS[0] == nonTerminal) {
            leftRecursive.push_back(rule);
        } else {
            nonLeftRecursive.push_back(rule);
        }
    }

    return {leftRecursive, nonLeftRecursive};
}


pair<vector<Rule>, vector<Rule>> eliminateImmediateLeftRecursion(const string &A,const vector<Rule> &leftRec, const vector<Rule> &nonLeftRec, const string &AprimeName)
{
    //if there are no left-recursive rules, then no change is needed:
    if (leftRec.empty()) {
        //expansions for A remain the same (nonLeftRec),
        //no new expansions for A'.
        return make_pair(nonLeftRec, vector<Rule>());
    }

    vector<Rule> newRulesForA;
    for (const Rule &nr : nonLeftRec) {
        //here what if we have A -> beta. Then it becomes A -> beta A'
        Rule transformed;
        transformed.LHS = A;
        transformed.RHS = nr.RHS;

        //push back A'
        transformed.RHS.push_back(AprimeName);
        newRulesForA.push_back(transformed);
    }

    vector<Rule> newRulesForAprime;

    for (const Rule &lr : leftRec) {
        Rule transformed;
        transformed.LHS = AprimeName;

        for (int i = 1; i < (int)lr.RHS.size(); i++) {
            transformed.RHS.push_back(lr.RHS[i]);
        }

        transformed.RHS.push_back(AprimeName);
        newRulesForAprime.push_back(transformed);
    }

    {
        Rule epsilonRule;
        epsilonRule.LHS = AprimeName;
        // no symbols in RHS => epsilon
        newRulesForAprime.push_back(epsilonRule);
    }

    return make_pair(newRulesForA, newRulesForAprime);
}


vector<Rule> eliminateLeftRecursionAlgorithm()
{

    vector<string> sortedNT(nonTerminalsSet.begin(), nonTerminalsSet.end());
    sort(sortedNT.begin(), sortedNT.end());


    map<string, vector<Rule>> rulesMap;
    for (auto &A : sortedNT) {
        rulesMap[A] = {};
    }

    //filling them by grouping from allRules
    for (auto &r : allRules) {

        rulesMap[r.LHS].push_back(r);
    }

    static map<string,int> nextIndexForA;

    for (int i = 0; i < (int)sortedNT.size(); i++) {
        string Ai = sortedNT[i];

        for (int j = 0; j < i; j++) {
            string Aj = sortedNT[j];


            vector<Rule> oldAi = rulesMap[Ai];
            vector<Rule> newAi;

            for (auto &r : oldAi) {
                // check if it's of the form Ai->Aj gamma
                if (!r.RHS.empty() && r.RHS[0] == Aj) {

                    vector<string> gamma(r.RHS.begin() + 1, r.RHS.end());
                    for (auto &r0 : rulesMap[Aj]) {

                        Rule newRule;
                        newRule.LHS = Ai;
                        // copy delta
                        for (auto &sym : r0.RHS) {
                            newRule.RHS.push_back(sym);
                        }


                        for (auto &sym : gamma) {
                            newRule.RHS.push_back(sym);
                        }
                        newAi.push_back(newRule);
                    }
                } else {
                    //not offending rule, so keep it
                    newAi.push_back(r);
                }
            }

            //updating rulesMap[Ai]
            rulesMap[Ai] = newAi;
        }


        vector<Rule> AiRules = rulesMap[Ai];

        auto splitted = splitLeftRecursiveRules(Ai, AiRules);
        vector<Rule> leftRec = splitted.first;
        vector<Rule> nonLeftRec = splitted.second;

        if (!leftRec.empty()) {

            int indexCount = nextIndexForA[Ai] + 1;
            nextIndexForA[Ai] = indexCount;
            string AiPrime = generateFactoredName(Ai, indexCount);


            auto result = eliminateImmediateLeftRecursion(Ai, leftRec, nonLeftRec, AiPrime);


            rulesMap[Ai] = result.first;

            if (!result.second.empty()) {

                rulesMap[AiPrime] = result.second;
            }
        }


    }

    vector<Rule> finalRules;
    //for each A in sortedNT, I union them
    for (auto &A : sortedNT) {
        for (auto &r : rulesMap[A]) {
            finalRules.push_back(r);
        }
    }

    for (auto &p : rulesMap) {
        // if p.first not in sortedNT => new non-terminal
        if (find(sortedNT.begin(), sortedNT.end(), p.first) == sortedNT.end()) {
            for (auto &r : p.second) {
                finalRules.push_back(r);
            }
        }
    }

    return finalRules;
}


void printTask6Grammar(const vector<Rule> &finalGrammar)
{
    vector<Rule> sortedGrammar = finalGrammar;

    sort(sortedGrammar.begin(), sortedGrammar.end(), isRuleBefore);

    for (const Rule &r : sortedGrammar)
    {
        cout << r.LHS << " -> ";
        if (r.RHS.empty())
        {
            //no symbols on the right-hand side => epsilon => print just "#"
            cout << "#\n";
        }
        else
        {
            //printing each symbol in RHS separated by spaces
            for (int i = 0; i < (int)r.RHS.size(); i++)
            {
                cout << r.RHS[i];
                if (i < (int)r.RHS.size() - 1)
                    cout << " ";
            }

            //ending each rule with " #"
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
    // cout << "----------------------\n";
    printTask5Grammar(finalFactoredGrammar);
}

// Task 6: eliminate left recursion
void Task6()
{
    vector<Rule> finalLeftGrammar = eliminateLeftRecursionAlgorithm();
    printTask6Grammar(finalLeftGrammar);
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

        case 5:
            Task5();
            break;
        
        case 6: Task6();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

