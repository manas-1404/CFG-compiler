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

//defining a Rule struct for storing the rules of the grammar
struct Rule {

    //i am storing the LHS of the rule as a string
    string LHS;

    //i am storing the RHS of the rule as a vector of strings
    vector<string> RHS;
};

//defining a new struct to store only the Rule struct and the longestMatch of the Rule
//i am only using this struct for task5
struct RuleWithLongestMatch {

    //defining an int to store the longest match of a particular rule
    int longestMatch;

    //defining a Rule to store the rule struct
    Rule rule;
};

//declaring all the global variables

//declaring a vector of Rule struct to store all the rules of the grammar
vector<Rule> allRules;

//declaring a set of strings to store all the nonterminals of the grammar
set<string> nonTerminalsSet;

//declaring a set of strings to store all the terminals of the grammar
set<string> terminalsSet;

//declaring a set of strings to store all the nullable nonterminals of the grammar
set<string> nullableSet;

//declaring a hash map to store the first sets of all the nonterminals
//the key of the hash map is the nonterminal and the value is the set of strings
map<string, set<string>> firstsSetHashMap;

//declaring a hash map to store the follow sets of all the nonterminals
map<string, set<string>> followsSetHashMap;

//declaring a vector of string called the universe. I am using this vector to keep track of the order in which each symbol comes in the grammar
//i use this vector at the very end as a reference while printing out order
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

        //return the correct token object
        return t;
    }

    //declaring all the function headers for the parser
    //Grammar -> Rule-list HASH
    void parseGrammar();

    //Rule-list -> Rule Rule-list | Rule
    void parseRuleList();

    //Rule -> ID ARROW Right-hand-side STAR
    void parseRule();

    //Right-hand-side -> Id-list | Id-list OR Right-hand-side
    void parseRightHandSide(string currentLHS);

    //Id-list -> ID Id-list | epsilon
    void parseIdList(vector<string> &rhsSymbols);

// declaring public memebrs of parser class
public:
    //constructor of the parser class
    void startParsing();
};

//Grammar -> Rule-list HASH
void Parser::parseGrammar() {

    //parse the rule-list
    parseRuleList();

    //consume the HASH token using expect function
    expect(HASH);
}

//Rule-list -> Rule Rule-list | Rule
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

//Rule -> ID ARROW Right-hand-side STAR
void Parser::parseRule() {

    //declaring new rule for rule
    Rule newRule;

    //LHS must be ID based on the CFG grammar. so consume the ID token using expect function
    Token LHSToken = expect(ID);

    //storing the LHS of the rule in the newRule struct
    string currentLHS = LHSToken.lexeme;
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

    //if next token is |, consume it and parse more Id-list
    //after parsing 1st IdList, we might or might not have more IdList. so I check if the next token after IdList is OR is present,
    //then it means that we have more IdList to parse, so I call the parseIdList function again inside the while loop
    while (nextToken.token_type == OR) {

        //consume the OR token using expect function after parsing the 1st IdList
        expect(OR);

        //initializing the next token after OR, now it could either be a STAR token or an ID token
        // A -> B C | D b | *
        //everytime after the OR token, it can either be the next duplicate rule (A -> D b) or it can be the epsilon rule (A -> *)
        Token starToken = lexer.peek(1);

        //if the next token is STAR, then its epsilon rule case and I just need to make RHS = {} and append the rule to allRules
        if (starToken.token_type == STAR) {

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

//Id-list -> ID Id-list | epsilon
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

//utility function to remove duplicates from a vector of strings
//i used it to remove the duplicates from the universe vector
void removeDuplicates(vector<string>& vec) {

    //declaring a set to store the seen strings
    unordered_set<string> seenStrings;

    //declaring a vector to store the result
    vector<string> result;

    //looping through the vector using a for loop with indexes
    for (size_t i = 0; i < vec.size(); ++i) {

        //if the element is not in the set, then add it to the result and mark it as seen
        if (seenStrings.find(vec[i]) == seenStrings.end()) {

            //pushing the element to the result vector
            result.push_back(vec[i]);

            //marking the element as seen
            seenStrings.insert(vec[i]);
        }
    }

    //assigning the result vector to the original vector
    vec = result;
}

//utility function which is used to convert the rules into a string
string ruleToString(Rule &rule) {

    //initializing a string to store the result. I am using this string to store the rule in the format LHS -> RHS
    string result = rule.LHS + "->";

    //looping through the RHS of the rule and appending the symbols to the result string
    for (int i = 0; i < rule.RHS.size(); i++) {

        //appending the symbol to the result string
        result += rule.RHS[i];

        //if the symbol is not the last symbol, then append a space
        if (i < rule.RHS.size() - 1) {

            //appending a space
            result += " ";
        }
    }

    //returning the result string
    return result;
}

//utility function which is used to remove the duplicate rules from the vector of rules
//i used this function while debugging task5, now simply thought of keeping it in the code XD
vector<Rule> removeDuplicateRules(vector<Rule> &rules) {

    //declaring a set to store the seen rules
    unordered_set<string> seenStrings;

    //declaring a vector to store the result
    vector<Rule> result;

    //looping through the vector of rules
    for (int i = 0; i < rules.size(); i++) {

        //converting the rule to a string
        string key = ruleToString(rules[i]);

        //if the rule is not seen, then add it to the result and mark it as seen
        if (seenStrings.find(key) == seenStrings.end()) {

            //pushing the rule to the result vector
            result.push_back(rules[i]);

            //marking the rule as seen
            seenStrings.insert(key);
        }
    }

    //returning the result vector
    return result;
}


//method which is used to check if a symbol is a non-terminal or not
bool isNonTerminal(const string &symbol) {

    //checking if the symbol is in the non-terminals set
    if (nonTerminalsSet.find(symbol) != nonTerminalsSet.end()) {

        //returning true because the symbol is a non-terminal
        return true;
    } else {

        //returning false because the symbol is not a non-terminal
        return false;
    }
}

//method which is used to find all the nonterminals in the grammar
void getNonTerminals() {

    //looping through all the rules
    for (int i = 0; i < allRules.size(); i++) {

        //pushing the LHS of the rule to the non-terminals set
        nonTerminalsSet.insert(allRules[i].LHS);
    }
}

//method which is used to find all the terminals in the grammar
void getTerminals() {

    //looping through all the rules
    for (int i = 0; i < allRules.size(); i++) {

        //looping through the RHS of each the rule
        for (int j = 0; j < allRules[i].RHS.size(); j++) {

            //checking if the symbol is a non-terminal or not
            if (!isNonTerminal(allRules[i].RHS[j])) {

                //the symbol is a terminal, so I push it to the terminals set
                terminalsSet.insert(allRules[i].RHS[j]);
            }
        }
    }
}

//method which is used to print the set in the order in which they appear in the universe
void printSetUniverseOrder(set<string> commonSet)
{
    //initializing a boolean to keep track of the first word
    bool firstWord = true;

    //initializing an index to loop through the universe vector
    int i = 0;

    //looping through the universe vector to get the proper order of the symbols
    while (i < universe.size()) {

        //checking if the symbol is in the set or not
        if (commonSet.find(universe[i]) != commonSet.end()) {

            //if it is not the first word, then add a space
            if (!firstWord) {

                //adding a space to output
                cout << " ";
            }

            //printing the symbol
            cout << universe[i];

            //marking the first word as false
            firstWord = false;
        }

        //incrementing the index to loop through the universe vector
        i++;
    }
}

//method which is used to print the nullable set in the order in which they appear in the universe
void printNullableSetUniverseOrder(set<string> commonSet)
{
    //initializing a boolean to keep track of the first word
    bool firstWord = true;

    //initializing an index to loop through the universe vector
    int i = 0;

    //looping through the universe vector to get the proper order of the symbols
    while (i < universe.size()) {

        //checking if the symbol is in the set or not
        string word = universe[i];

        //checking if the symbol is in the set, then print it
        if (commonSet.find(word) != commonSet.end()) {

            //if it is not the first word, then add a space
            if (!firstWord) {

                //adding a space to output
                cout << " , ";
            }

            //printing the symbol
            cout << word;

            //marking the first word as false
            firstWord = false;
        }

        //incrementing the index to loop through the universe vector
        i++;
    }
}

//method which is calculating the Nullables of each non terminals
void calculateNullableNonTerminals() {

    //initializing a flag updated to true, to continunous check if the nullable is present or not
    bool updated = true;

    //looping through the rules to find the nullable non-terminals
    while (updated) {

        //marking the updated flag as false
        updated = false;

        //looping through all the rules
        for (int j = 0; j < allRules.size(); j++) {

            //declaring a rule to store the current rule
            Rule rule = allRules[j];

            //storing the LHS of the current rule
            string currentLHS = rule.LHS;

            //checking if the LHS is in the nullable set or not
            if (nullableSet.find(currentLHS) == nullableSet.end()) {

                //initializing a boolean to keep track of all nullable
                bool allNullable = true;

                //checking if the RHS of the rule is empty or not
                if (rule.RHS.empty()) {

                    //if the RHS is empty, then set allNullable as true
                    allNullable = true;
                } else {

                    //looping through the RHS of the current rule
                    for (int i = 0; i < rule.RHS.size(); i++) {

                        //storing the current symbol in the RHS vector of strings
                        string currentRHSSymbol = rule.RHS[i];

                        //checking if the currentSymbol is in terminalSet or not or if it is in the nullable set or not
                        if (terminalsSet.find(currentRHSSymbol) != terminalsSet.end() || nullableSet.find(currentRHSSymbol) == nullableSet.end()) {

                            //setting allNullable as false because the symbol did not pass the check
                            allNullable = false;

                            //breaking the loop because the symbol did not pass the check
                            break;
                        }
                    }
                }

                //if the allNullable is true, then add the LHS to the nullable set and mark updated as true
                if (allNullable) {

                    //adding the LHS to the nullable set
                    nullableSet.insert(currentLHS);

                    //marking updated as true
                    updated = true;
                }
            }
        }
    }
}

//method which is used to print the first sets
void printFirstSets() {

    //looping through the universe vector to print the first sets in the proper order
    for (int i = 0; i < universe.size(); i++) {

        //storing the current symbol of the universe
        string nonTerminal = universe[i];

        //checking if the nonterminal is in the nonterminals set or not
        if (nonTerminalsSet.find(nonTerminal) != nonTerminalsSet.end()) {

            //printing it in the proper format, FIRST(nonterminal) = {
            cout << "FIRST(" << nonTerminal << ") = { ";

            //initializing a boolean to keep track of the first word
            bool firstWord = true;

            //looping through the universe vector to print the first set in the proper order
            for (int j = 0; j < universe.size(); j++) {

                //storing the symbol in the universe vector
                string symbol = universe[j];

                //checking if the nonterminal is in the first set or not
                if (firstsSetHashMap[nonTerminal].find(symbol) != firstsSetHashMap[nonTerminal].end()) {

                    //if it is not the first word, then add a comma
                    if (!firstWord) {
                        cout << ", ";
                    }

                    //printing the symbol
                    cout << symbol;

                    //marking the first word as false
                    firstWord = false;
                }
            }

            //printing the closing bracket
            cout << " }" << endl;
        }
    }
}

//method which is used to calculate the first sets of all the non-terminals
void calculateFirstSets() {

    //initialize each non-terminal's FIRST set to be empty
    for (auto &nt : nonTerminalsSet) {
        firstsSetHashMap[nt].clear();
    }

    //initializing the updated flag to true
    bool updated = true;

    //looping through the rules to find the first sets of all the non-terminals
    while (updated) {

        //marking the updated flag as false
        updated = false;

        //looping through all the rules in grammar
        //going through each rule A -> α in allRules
        for (int i = 0; i < allRules.size(); i++) {

            //storing the current LHS of the rule
            string currentLHS = allRules[i].LHS;

            //storing the RHS of the rule
            vector<string> currentRHSVector = allRules[i].RHS;

            //if currentRHSVector is empty, then adding epsilon to FIRST(currentLHS)
            if (currentRHSVector.empty()) {

                //checking if currentLHS is already in the FIRST set or not
                if (firstsSetHashMap[currentLHS].find("epsilon") == firstsSetHashMap[currentLHS].end()) {

                    //inserting epsilon to the FIRST set of currentLHS
                    firstsSetHashMap[currentLHS].insert("epsilon");

                    //marking updated as true
                    updated = true;
                }

                //done with this rule, so skip any future processin in this iteration
                continue;
            }

            //processing currentRHSVector from left to right

            //checking if all symbols in currentRHSVector are nullable or not
            bool allNullable = true;

            //looping through the RHS of the rule
            for (int j = 0; j < currentRHSVector.size(); j++) {

                //storing the current letter of the RHS vector
                string currentLetter = currentRHSVector[j];

                //if currentLetter is a terminal, then add currentLetter to FIRST(currentLHS) and stop dont do anything
                if (terminalsSet.find(currentLetter) != terminalsSet.end()) {

                    //checking if the currentLetter is already in the FIRST set of currentLhS or not
                    if (firstsSetHashMap[currentLHS].find(currentLetter) == firstsSetHashMap[currentLHS].end()) {

                        //inserting the currentLetter to the FIRST set of currentLHS
                        firstsSetHashMap[currentLHS].insert(currentLetter);

                        //marking updated as true
                        updated = true;
                    }

                    //if currentLetter is not nullable, then stop by setting allNullable flag to false
                    allNullable = false;

                    //stop the loop
                    break;
                } else {

                    //if the currentLetter is a non-terminal, union its FIRST set minus epsilon into FIRST(currentLHS)
                    int oldSize = firstsSetHashMap[currentLHS].size();

                    //inserting FIRST(currentLetter) - "epsilon" into FIRST(currentLHS)
                    for (auto &index : firstsSetHashMap[currentLetter]) {

                        //checking if the symbol is epsilon or not
                        if (index != "epsilon") {

                            //inserting the symbol to the FIRST set of currentLHS
                            firstsSetHashMap[currentLHS].insert(index);
                        }
                    }

                    //if FIRST(currentLHS) becomes big, then mark updated as true
                    if (firstsSetHashMap[currentLHS].size() > oldSize) {

                        //marking updated as true
                        updated = true;
                    }

                    //if currentLetter is not in nullable set, then stop
                    if (nullableSet.find(currentLetter) == nullableSet.end()) {

                        //setting allNullable flag as false
                        allNullable = false;

                        //brekaing the loop
                        break;
                    }

                    //else keep it goinggg!!!
                }
            }

            //if the allNullable is true, then add epsilon to the FIRST set of currentLHS
            if (allNullable) {

                //checking if epsilon is already in the FIRST set of currentLHS or not
                if (firstsSetHashMap[currentLHS].find("epsilon") == firstsSetHashMap[currentLHS].end()) {

                    //inserting epsilon to the FIRST set of currentLHS
                    firstsSetHashMap[currentLHS].insert("epsilon");

                    //marking updated as true
                    updated = true;
                }
            }
        }
    }
}


//method which is used to insert the elements from the set small to the set big, it returns true if the set big is changed, else it returns false
bool unionInsert(set<string> &smallSet, const set<string> &bigSet) {

    //initializing a flag didSetChange to false
    bool didSetChange = false;

    //looping through the bigSet
    for (auto &element : bigSet) {

        //if the element is not in the smallSet, then insert it and mark didSetChange as true
        if (smallSet.find(element) == smallSet.end()) {

            //inserting the element to the smallSet
            smallSet.insert(element);

            //marking didSetChange as true
            didSetChange = true;
        }
    }

    //returning didSetChange
    return didSetChange;
}

//method which is used to calculate the follow sets of all the non-terminals
void calculateFollowSets() {

    //if the universe is not empty, then add $ to the FOLLOW set of the first non-terminal in universe
    if (!universe.empty()) {

        //adding $ to the FOLLOW set of the first non-terminal in universe
        followsSetHashMap[universe[0]].insert("$");
    }

    //initializing the updated flag to true
    bool updatedFlag = true;

    //looping through the rules to find the follow sets of all the non-terminals
    while (updatedFlag) {

        //marking the updated flag as false
        updatedFlag = false;

        //for each and every rule A -> α
        for (int i = 0; i < allRules.size(); i++) {

            //storing the current rule in a rule
            Rule rule = allRules[i];

            //storing the LHS of the rule
            string currentLHS = rule.LHS;

            //storing the RHS of the rule
            vector<string> currentRHSvector = rule.RHS;

            //keeping trackerOfFollowSets set
            //initializing the tracker to FOLLOW(currentLHS)
            set<string> trackerOfFollowSets = followsSetHashMap[currentLHS];

            //traversing through RHS from right to left
            for (int i = currentRHSvector.size() - 1; i >= 0; i--) {

                //storing the current symbol of rhs vector
                string currentLetter = currentRHSvector[i];

                //if currentLetter is a non-terminal
                if (nonTerminalsSet.find(currentLetter) != nonTerminalsSet.end()) {

                    //adding trackerOfFollowSets to FOLLOW(currentLetter)
                    int beforeSize = followsSetHashMap[currentLetter].size();

                    //adding trackerOfFollowSets to FOLLOW(currentLetter)
                    followsSetHashMap[currentLetter].insert(trackerOfFollowSets.begin(), trackerOfFollowSets.end());

                    //checking if the size of the FOLLOW set of currentLetter is changed or not
                    if (followsSetHashMap[currentLetter].size() > beforeSize) {

                        //marking updatedFlag as true
                        updatedFlag = true;
                    }

                    //if firstSetHaspmap of currentLetter contains epsilon
                    if (firstsSetHashMap[currentLetter].find("epsilon") != firstsSetHashMap[currentLetter].end()) {

                        //looping through the firstsSetHashMap of currentLetter
                        for (auto &currentString : firstsSetHashMap[currentLetter]) {

                            //if currentString is not epsilon, then add it to trackerOfFollowSets
                            if (currentString != "epsilon") {

                                //adding the currentString to trackerOfFollowSets
                                trackerOfFollowSets.insert(currentString);
                            }
                        }
                    } else {

                        //declaring a new set to store the strings
                        set<string> newSet;

                        //looping through the firstsSetHashMap of currentLetter
                        for (auto &currentString : firstsSetHashMap[currentLetter]) {

                            //checking if the currentString is not epsilon, then add it to newSet
                            if (currentString != "epsilon") {

                                //adding the currentString to newSet
                                newSet.insert(currentString);
                            }
                        }

                        //reassigning the trackerOfFollowSets to newSet
                        trackerOfFollowSets = newSet;
                    }
                }

                //if currentLetter is a terminal
                else {

                    //clearing the trackerOfFollowSets
                    trackerOfFollowSets.clear();

                    //adding currentLetter to trackerOfFollowSets
                    trackerOfFollowSets.insert(currentLetter);
                }
            }
        }
    }
}

//method which is used to print the follow sets
void printFollowSets() {

    //looping through the universe vector to print the follow sets in the proper order
    for (int i = 0; i < universe.size(); i++) {

        //storing the current nonterminal of the universe
        string nonTerminal = universe[i];

        //checking if the nonterminal is in the nonterminals set or not
        if (nonTerminalsSet.find(nonTerminal) != nonTerminalsSet.end()) {

            //printing the follow set in the proper format, FOLLOW(nonterminal) = {
            cout << "FOLLOW(" << nonTerminal << ") = { ";

            //initializing a boolean to keep track of the first word
            bool firstWord = true;

            //$ aklways appears first if it's in the FOLLOW set
            //checking if $ is in the follow set of the nonterminal or not
            if (followsSetHashMap[nonTerminal].find("$") != followsSetHashMap[nonTerminal].end()) {

                //printing the $ symbol
                cout << "$";

                //marking the first word as false
                firstWord = false;
            }

            //printing elements in the order they appear in the sorted universe
            for (int j = 0; j < universe.size(); j++) {

                //storing the current letter of the universe
                string currentLetterOfUniverse = universe[j];

                //checking if the currentLetterOfUniverse is in the follow set of the nonterminal or not
                if (currentLetterOfUniverse != "$" && followsSetHashMap[nonTerminal].find(currentLetterOfUniverse) != followsSetHashMap[nonTerminal].end()) {

                    //if it is not the first word, then add a comma
                    if (!firstWord) {
                        cout << ", ";
                    }

                    //printing the symbol
                    cout << currentLetterOfUniverse;

                    //marking the first word as false
                    firstWord = false;
                }
            }

            //printing the closing bracket
            cout << " }" << endl;
        }
    }
}


//method which will return true if the Rule R1 is before Rule R2 in dictionary order, else it will return false
bool isRuleBefore(Rule r1, Rule r2) {

    //declaring 2 vectors to store the sequence of symbols in the rules
    //initializing ghem with the LHS of both the rules
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

    //initializing the index to 0
    int i = 0;

    //traversing through the RHS of the minimum length and comparing the symbols
    while (i < len) {
        //checking if the RHS symbols are not the same in both the rules
        if (r1.RHS[i] != r2.RHS[i]) {
            //the symbols are no longer the same, so i am breaking the while loop
            break;
        } else {
            //incrementing the count of common prefix
            count++;
        }
        i++;
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

            //comparing only if the LHS of the both the Rule 1 (outer loop) and Rule j (inner loop) is the same
            if (grammar[i].LHS == grammar[j].LHS && i != j) {

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
        RuleWithLongestMatch biggerRuleStruct;

        //storing the maxLen and the current Rule in the bigger struct
        biggerRuleStruct.longestMatch = maxLen;
        biggerRuleStruct.rule = grammar[i];

        //appending the struct to the temp vector
        temp.push_back(biggerRuleStruct);
    }

    //now i will start the sorting of the rules based on longestMatch and then lexically

    //sorting temp using the compare function which will compare the rules based on the longestMatch and then lexically
    sort(temp.begin(), temp.end(), compareRuleLongMatch);

    //declaring a new vector of Rules to store the sorted grammar
    vector<Rule> sortedGrammar;

    //traversing through the temp vector and appending the rules to the sortedGrammar vector
    for (int i = 0; i < temp.size(); ++i) {

        //appending the rule to the sortedGrammar vector
        sortedGrammar.push_back(temp[i].rule);
    }

    //returning the new grammar in correct order
    return sortedGrammar;
}


//method which will returns the nezt non-terminal name during left factoring, A1 or A2 for an LHS A
string makeUpNewFactorName(string currentLHS, int count) {

    //concatenating the LHS with the count string,  A + to_string(count) => A1, A2
    return currentLHS + to_string(count);
}


//method which will return the prefix of a Rule till the length given
vector<string> extractPrefix(Rule &r, int length) {

    //declaring a vector to store the result
    vector<string> result;

    //initializing the index to 0
    int i = 0;

    //traversing through the exact prefix length
    while (i < length && i < r.RHS.size()) {

        //appending the symbol to the result vector
        result.push_back(r.RHS[i]);

        //incrementing the index
        i++;
    }

    //returning the result vector
    return result;
}


//method which will return a next part after a certain lenght of the RHS of a Rule
vector<string> extractAllButPrefixOfSize(Rule &r, int length) {

    //declaring a vector to store the result
    vector<string> result;

    //initializing the index to the length
    int i = length;

    //traversing through the i = length till the end of the RHS of the given Rule
    while (i < r.RHS.size()) {

        //appending the symbol to the result vector
        result.push_back(r.RHS[i]);

        //incrementing the index
        i++;
    }

    //returning the result vector
    return result;
}

//debugging method used to print the rules
//i used it while debugging, now simply thought of keeping it in the code XD
void printRules(vector<Rule>& rules) {

    for (int i = 0; i < rules.size(); i++) {

        Rule rule = rules[i];

        cout << rule.LHS << " -> ";

        if (rule.RHS.empty()) {
            cout << "*";
        } else {
            for (int i = 0; i < rule.RHS.size(); ++i) {
                cout << rule.RHS[i];
                if (i < rule.RHS.size() - 1) {
                    cout << " ";
                }
            }
        }
        cout << endl;
    }
}

//debuggninf method to print the RHS of the rules
//i used it while debugging, now simply thought of keeping it in the code XD
void printRHS(const vector<string>& RuleRHS) {
    if (RuleRHS.empty()) {
        cout << "*";
        return;
    }

    for (int i = 0; i < RuleRHS.size(); ++i) {
        cout << RuleRHS[i];
        if (i < RuleRHS.size() - 1) {
            cout << " ";
        }
    }
}


//method whcih will return true if the 2 rules are equal, else false
bool areTwoRulesEqual(Rule &r1, Rule &r2) {

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

//method which is used to perform the left factoring on the grammar
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

    //declaring a map to store the index of a particular non-terminal
    //like for A it is A1, A2, A3. so for B it is B1, B2, B3
    map<string, int> mapOfNonTerminalandIndex;

    //initializing the index of all the non-terminals to 0
    for (auto nonTerminal : nonTerminalsSet) {

        //setting the index of the non-terminal to 0
        mapOfNonTerminalandIndex[nonTerminal] = 0;
    }

    //looping through the grammarRules to perform the left factoring
    //i am using >= 2 because i will always need to have atleast 2 rules to compare them for longest match
    while (grammarRules.size() >= 2) {

        //clearing the newRules, rulesWithoutLeftFactor and remainingGrammar vectors
        newRules.clear();
        rulesWithoutLeftFactor.clear();
        remainingGrammar.clear();

        //storing the 1st rule in the grammarRules vector
        Rule firstRule = grammarRules[0];

        //getting the maximum common prefix length of the 1st rule with 2nd the other rules having the same LHS
        int maxCommonLength = getCommonPrefixLength(firstRule, grammarRules[1]);

        //if the maxCommonLength is not 0, then i will perform the left factoring
        //because if the max common length itself is 0, then there is nothing common to factor out between the 2 rules
        if (maxCommonLength != 0) {

            //looping through the grammarRules to filter out all the rules which have the same common prefix
            //starting from i = 1 because i already have the 1st rule in the firstRule
            for (int i = 1; i < grammarRules.size(); i++) {

                //checking if the LHS of the 1st rule is the same as the LHS of the other rules
                if (firstRule.LHS == grammarRules[i].LHS) {

                    //getting the common prefix length of the 1st rule with the other rules having the same LHS and same common prefix
                    int commonLength = getCommonPrefixLength(firstRule, grammarRules[i]);

                    //if the commonLength is the same as the maxCommonLength, then i will add the rule to the newRules vector
                    if (maxCommonLength == commonLength) {

                        //adding the rule to the newRules vector
                        newRules.push_back(grammarRules[i]);
                    }
                }
            }

            //add the 1st rule only after adding all the other common same prefix rules
            newRules.push_back(firstRule);

            //removing the rules of A -> alpha X from the grammarRules
            //basically i am filtering out the rules without the common prefix rules
            for (int i = 0; i < grammarRules.size(); i++) {

                //intializing a flag to keep track of the rules without the left factored rules
                bool areTheRulesSame = false;

                //looping through the newRules to check if the sorted rules are in the newRules or not
                for (int j = 0; j < newRules.size(); j++) {

                    //checking if the rules are the same or not
                    if (areTwoRulesEqual(grammarRules[i], newRules[j])) {

                        //marking the flag as true because the rules are the same
                        areTheRulesSame = true;

                        //breaking the loop because the rule is found
                        break;
                    }
                }

                //if the rules are not the same, then add the rule to the rulesWithoutLeftFactor (clean) vector
                if (!areTheRulesSame) {

                    //adding the rule to the rulesWithoutLeftFactor vector
                    rulesWithoutLeftFactor.push_back(grammarRules[i]);
                }
            }

            //declaring a factoredRule to store the new rule like A -> B C B A1
            Rule factoredRule;

            //setting the LHS of the factoredRule to the LHS of the 1st rule
            factoredRule.LHS = firstRule.LHS;

            //extracting the common prefix of the 1st rule and storing it in the RHS of the factoredRule
            //like getting only B C B
            factoredRule.RHS = extractPrefix(firstRule, maxCommonLength);

            //getting the current index of the non-terminal
            int currentIndexOfNonTerminal = mapOfNonTerminalandIndex[firstRule.LHS] + 1;

            //getting the new non-terminal name like A1, A2, A3
            string newNonTerminal = makeUpNewFactorName(firstRule.LHS, currentIndexOfNonTerminal);

            //updating the index of the non-terminal
            mapOfNonTerminalandIndex[firstRule.LHS] = currentIndexOfNonTerminal;

            //adding the new non-terminal to the RHS of the factoredRule. like adding A1
            factoredRule.RHS.push_back(newNonTerminal);

            //adding the factoredRule to the rulesWithoutLeftFactor
            rulesWithoutLeftFactor.push_back(factoredRule);

            //adding the newRules to the rulesWithoutLeftFactor
            for (int k = 0; k < newRules.size(); k++) {

                //declaring a rule to remove the common prefix from the RHS of the rule
                Rule newRule;

                //setting the LHS of the newRule to the new non-terminal
                newRule.LHS = newNonTerminal;

                //extracting everything but the common prefix of the rule and storing it in the RHS of the newRule
                newRule.RHS = extractAllButPrefixOfSize(newRules[k], maxCommonLength);

                //adding the newRule to the rulesWithoutLeftFactor
                finalResultGrammar.push_back(newRule);
            }

            //updating the grammarRules to the sorted rulesWithoutLeftFactor
            grammarRules = findLongestMatchesAndSort(rulesWithoutLeftFactor);

        }

        //the maxCommonLength is 0, then there is nothing is common to factor out
        else {

            //initializin the index i to 0
            int i = 0;

            //looping through the grammarRules to find the rules which are not the same as the 1st rule
            while (i < grammarRules.size()) {

                //checking if the LHS of the 1st rule is the same as the LHS of the other rules
                if (firstRule.LHS != grammarRules[i].LHS) {

                    //appending the rule to the remainingGrammar vector to process further
                    remainingGrammar.push_back(grammarRules[i]);
                } else {

                    //appending the rule to the finalResultGrammar vector
                    finalResultGrammar.push_back(grammarRules[i]);
                }

                //incrementing the i
                i++;
            }

            //updating the grammarRules to the remainingGrammar
            grammarRules = remainingGrammar;

        }
    }

    //if there is only 1 rule left in the grammarRules, then add it to the finalResultGrammar
    //i had forgotten to add this case in the while loop and i was failing all the test cases because the last rule was not added
    if (grammarRules.size() == 1) {

        //adding the remaining rule to the finalResultGrammar
        finalResultGrammar.push_back(grammarRules[0]);
    }

    //returning the finalResultGrammar
    return finalResultGrammar;
}


//method which is used to print the grammar in the specified format
void printTask5Grammar(vector<Rule> &factoredGrammar) {

    //initializing a vector to store the factored grammar
    vector<Rule> sortedGrammar = factoredGrammar;

    //sorting by your isRuleBefore function
    sort(sortedGrammar.begin(), sortedGrammar.end(), isRuleBefore);

    //printing each rule in the specified format
    for (int i = 0; i < sortedGrammar.size(); i++) {

        //declaring a rule to store the current rule
        Rule rule = sortedGrammar[i];

        //printing the LHS of the rule ->
        cout << rule.LHS << " -> ";

        //if the RHS of the rule is empty, then print #
        if (rule.RHS.empty()) {

            //printing #
            cout << "#\n";
        } else {

            //looping to print all letters in the RHS, separated by space
            for (int i = 0; i < rule.RHS.size(); i++) {

                //printing the RHS letter
                cout << rule.RHS[i];

                //if it's not the last letter, then print a space
                if (i < rule.RHS.size() - 1) {

                    //printing a space
                    cout << " ";
                }
            }

            //printing the # at the end
            cout << " #\n";
        }
    }
}

//method which is used to get the Rules for a given non-terminal
vector<Rule> getRulesForNonTerminal(string &nonTerminal, vector<Rule> &grammar) {

    //declaring a vector to store the result
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

    //for each rule of B: B -> something (calling it Ri)
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


//method which will separate left recursive and non leftrecursive rules for a given non-terminal
pair<vector<Rule>, vector<Rule>> splitLeftRecursiveRules(string nonTerminal, vector<Rule> &rulesToSeparate){

    //declaring 2 vectors to store the left recursive and non left recursive rules
    vector<Rule> leftRecursiveRule;
    vector<Rule> nonLeftRecursiveRule;

    //initializing the index to 0
    int i = 0;

    //looping through the rules to separate them
    while (i < rulesToSeparate.size()) {

        //storing the current rule in a rule
        Rule rule = rulesToSeparate[i];

        //just doing a sanity check for rule must belong to A
        if (rule.LHS != nonTerminal) {

            //incrementing the index
            i++;

            //skipping the further current iteration
            continue;
        }

        //checkig if the RHS of the rule is empty or not or not starting with the non-terminal
        if (rule.RHS.empty() || rule.RHS[0] != nonTerminal) {

            //appeding the rule to the nonLeftRecursiveRule vector
            nonLeftRecursiveRule.push_back(rule);
        } else {

            //appending the rule to the leftRecursiveRule vector
            leftRecursiveRule.push_back(rule);
        }

        //incrementing the index
        i++;
    }

    //returning the pair of leftRecursiveRule and nonLeftRecursiveRule rules
    return {leftRecursiveRule, nonLeftRecursiveRule};
}

//method which will eliminate the immediate left recursion for a given non-terminal
pair<vector<Rule>, vector<Rule>> eliminateImmediateLeftRecursion(vector<Rule> &leftRecurRules, vector<Rule> &nonLeftRecurRules, string &nonTerminal, string &nonTerminal2)
{
    //if there are no leftrecursive rules, then no change is needed
    if (leftRecurRules.empty()) {

        //return the nonLeftRecurRules as it is in a pair
        return make_pair(nonLeftRecurRules, vector<Rule>());
    }

    //declaring a vector to store the new rules
    vector<Rule> newRulesForNonTerminals;

    //looping through the nonLeftRecurRules to add the new rules
    for (int i = 0; i < nonLeftRecurRules.size(); i++) {

        //storing the current non left recursive rule in a rule
        Rule currentNonRecursiveRule = nonLeftRecurRules[i];

        //declaring a rule to store the transformed rule
        Rule transformedRule;

        //setting the LHS of the transformed rule to the non-terminal
        transformedRule.LHS = nonTerminal;

        //copying the RHS of the current non left recursive rule to the transformed rule
        transformedRule.RHS = currentNonRecursiveRule.RHS;

        //appedning the nonTerminal2 to the RHS of the transformed rule
        transformedRule.RHS.push_back(nonTerminal2);

        //appending the transformed rule to the newRulesForNonTerminals vector
        newRulesForNonTerminals.push_back(transformedRule);
    }

    //declaring a vector to store the new rules for the nonTerminal2
    vector<Rule> newRulesForNonTerminals2;

    //looping through the leftRecurRules to add the new rules for the nonTerminal2
    for (int j = 0; j < leftRecurRules.size(); j++) {

        //storing the current left recursive rule in a rule
        Rule currentRecursiveRule = leftRecurRules[j];

        //declaring a rule to store the transformed rule
        Rule transformedRule;

        //setting the LHS of the transformed rule to the nonTerminal2
        transformedRule.LHS = nonTerminal2;

        //intializing the index to 1
        int k = 1;

        //looping through the RHS of the current left recursive rule to append the letters to the transformed rule
        while (k < currentRecursiveRule.RHS.size()) {

            //appending the letters to the RHS of the transformed rule
            transformedRule.RHS.push_back(currentRecursiveRule.RHS[k]);

            //incrementing the index k
            k++;
        }

        //appending the nonTerminal2 to the RHS of the transformed rule
        transformedRule.RHS.push_back(nonTerminal2);

        //appending the transformed rule to the newRulesForNonTerminals2 vector
        newRulesForNonTerminals2.push_back(transformedRule);
    }

    //declaring a rule to store the epsilon rule
    Rule epsilonRule;

    //setting the LHS of the epsilon rule to the nonTerminal2
    epsilonRule.LHS = nonTerminal2;

    //no symbols in RHS => epsilon
    //appending the epsilon to the RHS of the epsilon rule
    newRulesForNonTerminals2.push_back(epsilonRule);

    //returning the pair of newRulesForNonTerminals and newRulesForNonTerminals2
    return make_pair(newRulesForNonTerminals, newRulesForNonTerminals2);
}

//method which will eliminate the left recursion from the grammar
vector<Rule> eliminateLeftRecursionAlgorithm()
{

    //declaring a vector to store the sorted non-terminals
    vector<string> sortedNonTerminal;

    //looping through the nonTerminalsSet to store the sorted non-terminals
    for (string nt : nonTerminalsSet) {

        //appending the non-terminal to the sortedNonTerminal vector
        sortedNonTerminal.push_back(nt);
    }

    //sorting the sortedNonTerminal vector
    sort(sortedNonTerminal.begin(), sortedNonTerminal.end());

    //declaring a map to store the rules and nonterminals of the non-terminals
    map<string, vector<Rule>> rulesMap;

    //declaring a map to store the next index of the non-terminal
    map<string,int> nextIndexForNonTerminal;

    //looping through the sortedNonTerminal to fill the rulesMap with the rules of the non-terminals
    for (int a = 0; a < sortedNonTerminal.size(); a++) {

        //initializing the rulesMap with nothing
        rulesMap[sortedNonTerminal[a]] = {};
    }

    //looping through the allRules to fill the rulesMap
    for (int i = 0; i < allRules.size(); i++) {

        //storing the current rule in a rule
        Rule rule = allRules[i];

        //appending the rule to the rulesMap
        rulesMap[rule.LHS].push_back(rule);
    }

    //looping through the sortedNonTerminal to eliminate the left recursion
    for (int i = 0; i < sortedNonTerminal.size(); i++) {

        //storing the current non-terminal in a string
        string currentNonTerminal = sortedNonTerminal[i];

        //looping through the non-terminal before the current non-terminal
        for (int j = 0; j < i; j++) {

            //storing the non-terminal in a string before the current non-terminal
            string beforeCurrentNonTerminal = sortedNonTerminal[j];

            //storing the rules of the current non-terminal in a vector
            vector<Rule> currentNonTerminalVector = rulesMap[currentNonTerminal];

            //declaring a vector to store the new rules of the current non-terminal
            vector<Rule> newNonTerminalVector;


            for (int j = 0; j < currentNonTerminalVector.size(); j++) {

                //storing the current rule in a rule
                Rule rule = currentNonTerminalVector[j];

                //cehcking if the RHS of the rule is not empty and the first symbol of the RHS is the beforeCurrentNonTerminal
                if (!rule.RHS.empty() && rule.RHS[0] == beforeCurrentNonTerminal) {

                    //declaring the gamma vector
                    vector<string> gamma;

                    //looping through the RHS of the rule to append the symbols to the gamma vector
                    for (int i = 1; i < rule.RHS.size(); i++) {

                        //appending the letters to the gamma vector
                        gamma.push_back(rule.RHS[i]);
                    }


                    //looping through the rules map of the beforeCurrentNonTerminal
                    for (auto &anotherRule : rulesMap[beforeCurrentNonTerminal]) {

                        //declaring a new rule to store the new rule
                        Rule newRule;

                        //setting the LHS of the newRule to the current non-terminal
                        newRule.LHS = currentNonTerminal;

                        //looping through the RHS of the rule to append the symbols to the newRule RHS
                        for (int m = 0; m < anotherRule.RHS.size(); m++) {

                            //appending the letters to the newRule RHS
                            newRule.RHS.push_back(anotherRule.RHS[m]);
                        }

                        //looping through the gamma vector to append the symbols to the newRule RHS
                        for (int n = 0; n < gamma.size(); n++) {

                            //appending the letters to the newRule RHS
                            newRule.RHS.push_back(gamma[n]);
                        }

                        //appending the newRule to the newNonTerminalVector
                        newNonTerminalVector.push_back(newRule);
                    }
                } else {

                    //not offending rule, so keep it
                    //appending the rule to the newNonTerminalVector
                    newNonTerminalVector.push_back(rule);
                }
            }

            //updating rulesMap[currentNonTerminal] with the newNonTerminalVector
            rulesMap[currentNonTerminal] = newNonTerminalVector;
        }

        //storing the rules of the current non-terminal in a vector
        vector<Rule> currentRulesVector = rulesMap[currentNonTerminal];

        //splitting the left recursive and non left recursive rules
        pair<vector<Rule>, vector<Rule>> splitted = splitLeftRecursiveRules(currentNonTerminal, currentRulesVector);

        //storing the left recursive rules in a vector
        vector<Rule> leftRecurRule = splitted.first;

        //storing the non left recursive rules in a vector
        vector<Rule> nonRecurRule = splitted.second;

        //if there are left recursive rules, then eliminate the left recursion
        if (!leftRecurRule.empty()) {

            //getting the next index of the non-terminal
            int indexCount = nextIndexForNonTerminal[currentNonTerminal] + 1;

            //updating the next index of the current non-terminal
            nextIndexForNonTerminal[currentNonTerminal] = indexCount;

            //storing the new non-terminal name
            string newNonTerminalName = makeUpNewFactorName(currentNonTerminal, indexCount);

            //eliminating the immediate left recursion
            pair<vector<Rule>, vector<Rule>> resultPair = eliminateImmediateLeftRecursion( leftRecurRule, nonRecurRule, currentNonTerminal, newNonTerminalName);

            //storing the new rules for the current non-terminal in rulesMap
            rulesMap[currentNonTerminal] = resultPair.first;

            //storing the new rules for the new non-terminal in rulesMap
            if (!resultPair.second.empty()) {

                //appending the new rules to the rulesMap
                rulesMap[newNonTerminalName] = resultPair.second;
            }
        }
    }

    //declaring a vector to store the final rules
    vector<Rule> finalRules;

    //looping through the sortedNonTerminal to append the rules to the finalRules vector
    for (int p1 = 0; p1 < sortedNonTerminal.size(); p1++) {

        //looping through the rulesMap in the inside
        for (int p2 = 0; p2 < rulesMap[sortedNonTerminal[p1]].size(); p2++) {

            //appending the rules to the finalRules vector
            finalRules.push_back(rulesMap[sortedNonTerminal[p1]][p2]);
        }
    }

    for (auto &currentElementInMap : rulesMap) {

        //checking if the non-terminal is not in the sortedNonTerminal
        if (find(sortedNonTerminal.begin(), sortedNonTerminal.end(), currentElementInMap.first) == sortedNonTerminal.end()) {

            //looping through the currentElementInMap to append the rules to the finalRules vector
            for (int d = 0; d < currentElementInMap.second.size(); d++) {

                //appending the rules to the finalRules vector
                finalRules.push_back(currentElementInMap.second[d]);
            }
        }
    }

    //returning the finalRules
    return finalRules;
}

//method which is used to print the task6 grammar in the specified format
void printTask6Grammar(vector<Rule> &finalGrammar)
{
    //declaring the sortedGrammar vector to store the finalGrammar
    vector<Rule> sortedGrammar = finalGrammar;

    //sorting the sortedGrammar vector using the isRuleBefore comparison
    sort(sortedGrammar.begin(), sortedGrammar.end(), isRuleBefore);

    for (int i = 0; i < sortedGrammar.size(); i++){

        //storing the current rule
        Rule rule = sortedGrammar[i];

        //printing the rule LHS ->
        cout << rule.LHS << " -> ";

        //checking if the RHS of the rule is not empty
        if (!rule.RHS.empty())
        {
            //looping through to print each symbol in RHS separated by spaces
            for (int i = 0; i < rule.RHS.size(); i++)
            {
                //printing the letter of RHS
                cout << rule.RHS[i];

                //if it's not the last letter, then print a space
                if (i < rule.RHS.size() - 1) {

                    //printing a space
                    cout << " ";
                }
            }

            //ending each rule with " #"
            cout << " #\n";
        }

        else
        {
            //printing # if the RHS is empty
            cout << "#\n";
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
        
        case 6:
            Task6();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

