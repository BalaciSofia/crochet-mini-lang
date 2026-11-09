# Finite automata and regular grammars

Goal: define a finite automaton (FA) for identifiers and constants of CROCHET-MINI-LANG and transform each FA into a regular grammar (RG). Language: C++.

## Definitions

An FA is `M = (Q, Σ, δ, q0, F)`: states, alphabet, transition function, initial state, final states. A word is accepted if reading it from `q0` ends in a state of `F`; a missing transition rejects the word.

Each symbol of `Σ` is a character class, a name for a set of characters:

| Class | Characters |
|---|---|
| `letter` | a to z, A to Z |
| `digit` | 0 to 9 |
| `zero`, `nonzero` | 0, and 1 to 9 |
| `hex` | 0 to 9, a to f, A to F |
| `hash`, `quote` | `#`, `"` |
| `strchar` | any printable character except `"` |

A right linear grammar `G = (N, Σ, P, S)` has productions `A → aB` or `A → a`, plus `S → ε` if the empty word is accepted.

## The automata

All have `q0` as initial state.

| Token | Expression | Q | F | Transitions |
|---|---|---|---|---|
| identifier | `letter (letter \| digit)*` | q0, q1 | q1 | q0 letter q1; q1 letter q1; q1 digit q1 |
| integer | `0 \| nonzero digit*` | q0, q1, q2 | q1, q2 | q0 zero q1; q0 nonzero q2; q2 digit q2 |
| color | `# hex{6}` | q0 to q7 | q7 | q0 hash q1; q1 to q6 hex q(i+1) |
| string | `quote strchar* quote` | q0, q1, q2 | q2 | q0 quote q1; q1 strchar q1; q1 quote q2 |

## Transformation FA to RG

1. `N` has one nonterminal per state, `S` is the one of `q0`.
2. For every `δ(q, a) = p`, add `q → a p`.
3. If also `p ∈ F`, add `q → a`, so a derivation can stop. Without it every rule leaves a nonterminal and no word is ever derived.
4. If `q0 ∈ F`, add `q0 → ε`.

Results:

```
identifier                           integer
Q0 -> letter Q1 | letter             Q0 -> zero Q1 | zero | nonzero Q2 | nonzero
Q1 -> letter Q1 | letter             Q2 -> digit Q2 | digit
     | digit Q1 | digit

color                                string
Q0 -> hash Q1                        Q0 -> quote Q1
Q1 -> hex Q2                         Q1 -> strchar Q1 | quote Q2 | quote
...
Q6 -> hex Q7 | hex
```

States without outgoing transitions (`Q1` in integer, `Q2` in string, `Q7` in color) have no productions.

## Implementation

`tools/fa_to_rg.cpp` reads an FA from a text file and prints `N`, `Sigma`, `S` and the productions.

File format (`fa/identifier.fa`): states, initial state, final states, then one transition per line.

```
states: Q0 Q1
initial: Q0
final: Q1
Q0 letter Q1
Q1 letter Q1
Q1 digit Q1
```

The transitions are stored in three vectors (`from`, `symbol`, `to`). For each state, the program goes through its transitions and applies rules 2 to 4.

```
g++ -o fa_to_rg tools/fa_to_rg.cpp
./fa_to_rg fa/identifier.fa
```

```
N = {Q0, Q1}
Sigma = {letter, digit}
S = Q0
P:
  Q0 -> letter Q1 | letter
  Q1 -> letter Q1 | letter | digit Q1 | digit
```
