# Lexical analysis (flex scanner)

The scanner reads `tokens.txt` and a source program and produces the **PIF**, the **ST** and the **lexical errors**. It is written with **flex** (C), and the symbol table is a **hash table** implemented in C.

## Files

| File | Role |
|---|---|
| `compiler/lexer.l` | flex rules, PIF and error handling, `main()` |
| `compiler/symtab.h`, `compiler/symtab.c` | symbol table (hash table) |
| `tokens.txt` | input list of tokens and their codes |
| `examples/` | `scarf.cml`, `blanket.cml` (correct), `lexical-errors.cml` (errors) |

## Tools

- **flex** turns the regular expressions in `lexer.l` into a C scanner (`lex.yy.c`). It always takes the **longest match**, and on a tie the **first rule**. Both rules are used on purpose (see Errors).
- **gcc** and **make** build it (tested on Ubuntu under WSL).

```
cd compiler
make
./lexer ../tokens.txt ../examples/scarf.cml ../out
make test
```

Outputs go to `out/` (repository root) as `<program>.PIF.out`, `<program>.ST.out` and `<program>.errors.out`. The exit code is 0 without errors, 1 with lexical errors, 2 for usage problems.

## Tokens

`tokens.txt` has one `token code` pair per line.

| Group | Tokens | Codes |
|---|---|---|
| classes | `identifier`, `constant` | 0, 1 |
| operators | `==  !=  <=  >=  <  >  =  +  -  *  /` | 2 to 12 |
| separators | `,  :  (  )  {  }  ^` | 13 to 19 |
| reserved words (24) | `int motif add remove color in new row for from to rows while and or not stitches if else end input output pattern rep` | 20 to 43 |
| stitch names (7) | `ch sc hdc dc trc slst skip` | 44 to 50 |

Whitespace and `//` comments are skipped. Keywords are case sensitive (`if` is a keyword, `IF` is an identifier).

## Detection

**Identifiers and reserved words.** One pattern, `[A-Za-z]([A-Za-z]|[0-9])*`. After a match the lexeme is looked up in the token table loaded from `tokens.txt`. If found, it is a reserved word or stitch (PIF code only). If not, it is an identifier: inserted in the ST, PIF gets code 0 and the ST position.

**Constants.** All get PIF code 1 and are inserted in the ST with a type.

| Constant | Pattern |
|---|---|
| number | `0 \| [1-9][0-9]*` |
| color | `#[0-9A-Fa-f]{6}` |
| string | `"[^"\r\n]*"` (stored with its quotes) |

**Operators and separators** are matched by one rule. Longest match makes `==` one token and not two `=`, and makes `//` a comment and not two divisions.

**Errors.** Each error has its own pattern. It wins over the valid pattern when it matches more text, so a bad lexeme is reported as one unit. The scanner prints line number, message and lexeme, then keeps scanning.

| Error | Example |
|---|---|
| number with a leading zero | `007` |
| identifier starting with a digit | `3abc` |
| malformed color | `#ff00`, `#12345g`, `#ff00000` |
| unterminated string | `"abc` |
| unexpected character | `@`, `!`, `_` |

## PIF

One entry per token, in source order: the token code, plus the ST position for identifiers and constants (`-` for the others). The line number is extra information.

```
code   token        ST pos line
42     pattern      -      2
0      identifier   0      2
24     color        -      3
0      identifier   1      3
8      =            -      3
1      constant     2      3
```

## Symbol table

A **hash table with separate chaining**: 211 buckets (prime), each holding a linked list. One table stores identifiers and constants, each node has `key`, `type`, `position` and `next`.

- **Hash:** the sum of the ASCII codes of the characters of the key, `h = (c1 + c2 + ... + cn) mod 211`. Words with the same letters in a different order collide (for example `ab` and `ba`), and the chains handle that.
- **Position:** insertion order, never changes, so the PIF can refer to it.
- **Operations:**

| Operation | Description |
|---|---|
| `st_init` | empty all buckets |
| `st_insert(key, type)` | return the position if the key exists, otherwise add a node at the head of its chain and return the new position |
| `st_lookup(key)` | position or -1 |
| `st_print` | dump ordered by position, with bucket and type |
| `st_free` | free all nodes |

Insert and lookup are O(1) on average. `valgrind` reports no leaks.

```
position  bucket   type             lexeme
0         105      identifier       scarf
1         104      identifier       red
2         9        color constant   #ff0000
```

## Tests

- `scarf.cml`: no errors, 86 PIF entries, 13 ST entries.
- `blanket.cml`: no errors, 164 PIF entries, 23 ST entries.
- `lexical-errors.cml`: 8 errors reported with their lines (3, 4, 5, 6, 9, 11, 12, 13). `IF` is accepted as an identifier, and `under_score` is split into `under`, an error on `_`, and `score`, because the BNF has no underscore.
