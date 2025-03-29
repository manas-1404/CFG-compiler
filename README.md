# CFG Grammar Analyzer and Transformer

This project is a C++ compiler for processing **Context-Free Grammars (CFGs)**. It analyzes and transforms a given grammar to make it suitable for **predictive (top-down) parsing**. The compiler is capable of computing Nullable, FIRST, FOLLOW sets, and performing left factoring and elimination of left recursion.

---

## 1. What is a Context-Free Grammar?

A **Context-Free Grammar (CFG)** is a set of recursive rewriting rules (productions) used to generate patterns of strings. It is defined as:

```
G = (N, T, R, S)
```

- **N**: Set of Non-terminals  
- **T**: Set of Terminals  
- **R**: Set of Production Rules (A → α where A ∈ N, α ∈ (N ∪ T)*)  
- **S**: Start Symbol  

CFGs are widely used in compiler design to define the syntax of programming languages.

---

## 2. Grammar Input Format

Each grammar rule in the input follows this format:

```
NonTerminal -> symbol1 symbol2 symbol3 *
```

- The arrow `->` separates the left-hand side (LHS) from the right-hand side (RHS).
- Each production ends with an asterisk `*`.
- If a non-terminal has multiple productions, they are separated using the `|` symbol.
- An epsilon (empty) production is written using just a `*` after the arrow.
- The entire input ends with a line containing only `#`.

### Example

```
S -> A B C *
A -> a | #
B -> b B | #
C -> c *
#
```

---

## 3. Formal Grammar Definition of the Input

The structure of the input grammar is defined by this CFG:

```
Grammar → Rule-list HASH  
Rule-list → Rule Rule-list | Rule  
Rule → ID ARROW Right-hand-side STAR  
Right-hand-side → Id-list | Id-list OR Right-hand-side  
Id-list → ID Id-list | ϵ
```

### Token Definitions

```
ID     = letter (letter | digit)*
STAR   = '*'
HASH   = '#'
OR     = '|'
ARROW  = '->'
```

Where:

- `letter` is any uppercase or lowercase letter (`a`–`z`, `A`–`Z`)
- `digit` is any numeral (`0`–`9`)
- Tokens are separated by at least one whitespace character

The input consists of a **rule list**. Each rule has a left-hand side which is an **ID** and a right-hand side composed of one or more **Id-list** separated by `|` and terminated by `*`. An **Id-list** is a sequence of zero or more **ID** tokens.

---

## 4. How the Compiler Works

This compiler has **six features** to analyze and transform a context-free grammar:

### Feature 1 – Identify Terminals and Non-Terminals

- Parses the grammar and identifies all terminal and non-terminal symbols.
- Maintains the order of appearance in the input.

### Feature 2 – Compute Nullable Set

- Finds all non-terminals that can derive an empty string (ϵ).
- Uses fixed-point iteration.

### Feature 3 – Compute FIRST Sets

- FIRST(X) includes terminals that can appear at the beginning of strings derived from X.

### Feature 4 – Compute FOLLOW Sets

- FOLLOW(X) includes terminals that can appear immediately after X.

### Feature 5 – Left Factoring

- Removes common prefixes among productions.
- Transforms ambiguous rules for predictive parsing.

**Example:**

```
A → a b c | a b d
```

Becomes:

```
A → a b A1
A1 → c | d
```

### Feature 6 – Eliminate Left Recursion

Handles both direct and indirect recursion by rewriting rules.

**Direct Recursion Example:**

```
A → A α | β
```

Becomes:

```
A → β A1
A1 → α A1 | #
```

Indirect recursion is handled by substituting and rewriting earlier non-terminal definitions.

---

## 5. Compilation & Execution

### Compile

```bash
g++ project2.cc lexer.cc inputbuf.cc -o grammar_tool
```

### Run

```bash
./grammar_tool <task_number> < input.txt
```

Where `<task_number>` is from 1 to 6.

---

## 6. Project Structure

| File              | Purpose                                         |
|-------------------|-------------------------------------------------|
| `project2.cc`     | Main compiler implementation and parser         |
| `lexer.cc/h`      | Lexical analyzer for tokenizing grammar         |
| `inputbuf.cc/h`   | Buffered input manager                          |

---

## 7. Output Format

- All output rules are sorted in **dictionary (lexicographic)** order.
- Every rule ends with a `#`.
- Empty rules (epsilon) are printed as `-> #`.

**Example:**

```
A -> a B A1 #
A1 -> c A1 #
A1 -> # 
B -> b #
```

---

## 8. Notes

- Uses **recursive descent parsing** for reading and analyzing grammar.
- All transformation steps (factoring, recursion removal) follow consistent naming and structure (A1, B1, etc.).
- Output is deterministic for automated testing or grading systems.

---

## 9. License

This compiler project was developed for academic purposes only.
