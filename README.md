# CROCHET-MINI-LANG

A small domain-specific language for writing crochet patterns, and the compiler front end for it.

You declare colors, reusable stitch groups (motifs) and patterns, then build the pattern row by row with loops, conditions and input/output.

```
//scarf
pattern scarf
color red = #ff0000
motif edge = { sc, dc ^ 2 }
int n = 3
int i

new row in scarf : ch ^ 10
for i from 1 to n :
    new row in scarf : sc, { dc(red), dc } rep 4, edge
end
if rows(scarf) > 3 :
    remove row from scarf
else :
    output "short scarf"
end
output rows(scarf)
output stitches(scarf)
```

## Language at a glance

| Construct | Example |
|---|---|
| declarations | `int n = 3`, `color red = #ff0000`, `motif edge = { sc, dc ^ 2 }`, `pattern scarf` |
| stitches | `ch sc hdc dc trc slst skip`, colored: `dc(red)` |
| repetition | `dc ^ 3` (one stitch), `{ sc, dc } rep 4` (a group) |
| rows | `new row in scarf : sc, edge`, `add row r to scarf`, `remove row from scarf` |
| control flow | `if … : … else : … end`, `for i from 1 to n : … end`, `while … : … end` |
| I/O | `input rounds`, `output "text"`, `output rows(scarf)` |

The full grammar is in [docs/grammar.txt](docs/grammar.txt).

## Repository layout

| Path | Contents |
|---|---|
| `docs/` | grammar (BNF), use cases, lexer and finite automata documentation |
| `examples/` | example programs (`.cml`) |
| `tokens.txt` | token list with the codes used in the PIF |
| `fa/` | finite automata for identifiers, integers, colors and strings |
| `compiler/` | flex scanner with a hash table symbol table (C) |
| `fa-scanner/` | scanner driven by the finite automata in `fa/` (C++) |
| `tools/fa_to_rg.cpp` | converts a finite automaton into a regular grammar |

## Build and run

flex scanner (gcc, flex, make):

```
cd compiler
make
make test          # scans every program in examples/, writes ../out/
```

FA-based scanner (CMake, C++20):

```
cmake -S fa-scanner -B build
cmake --build build
./build/fa_scanner examples/scarf.cml
```

FA to regular grammar:

```
g++ -o fa_to_rg tools/fa_to_rg.cpp
./fa_to_rg fa/identifier.fa
```

Both scanners write `<program>.PIF.out`, `<program>.ST.out` and `<program>.errors.out` to `out/`.

## Roadmap

- [x] Language definition (BNF, use cases)
- [x] Lexical analysis (flex + hash table symbol table)
- [x] Finite automata for identifiers and constants, FA to RG
- [x] FA-driven scanner
- [ ] Parser (yacc/bison)
- [ ] LL(1) parser with parse tree
- [ ] Translation to C
