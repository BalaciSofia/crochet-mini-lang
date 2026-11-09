%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

#define MAX_TOKENS 128
#define MAX_PIF 100000

typedef struct { char name[32]; int code; } TokenDef;
typedef struct { int code; int stPos; int line; } PifEntry;

static TokenDef tokens[MAX_TOKENS];
static int tokenCount = 0;
static int CODE_ID = -1, CODE_CONST = -1;

static SymbolTable st;
static PifEntry *pif;
static int pifCount = 0;
static int errorCount = 0;
static FILE *errFile;

static int token_code(const char *name) {
    for (int i = 0; i < tokenCount; i++)
        if (strcmp(tokens[i].name, name) == 0) return tokens[i].code;
    return -1;
}

static void add_pif(int code, int stPos) {
    if (pifCount >= MAX_PIF) { fprintf(stderr, "PIF is full\n"); exit(2); }
    pif[pifCount].code = code;
    pif[pifCount].stPos = stPos;
    pif[pifCount].line = yylineno;
    pifCount++;
}

static void lex_error(const char *msg) {
    errorCount++;
    fprintf(errFile, "Lexical error at line %d: %s: \"%s\"\n", yylineno, msg, yytext);
    fprintf(stderr,  "Lexical error at line %d: %s: \"%s\"\n", yylineno, msg, yytext);
}

static void constant(StType type) {
    add_pif(CODE_CONST, st_insert(&st, yytext, type));
}
%}

%option noyywrap
%option yylineno
%option noinput nounput

LETTER      [A-Za-z]
DIGIT       [0-9]
HEX         [0-9A-Fa-f]

%%

[ \t\r]+                        { /* whitespace */ }
\n                              { /* line counting is done by yylineno */ }
"//".*                          { /* comment */ }

{LETTER}({LETTER}|{DIGIT})*     {
                                  /* reserved word / stitch name if it is in tokens.txt, otherwise identifier */
                                  int code = token_code(yytext);
                                  if (code >= 0) add_pif(code, -1);
                                  else add_pif(CODE_ID, st_insert(&st, yytext, ST_IDENTIFIER));
                                }

0|[1-9]{DIGIT}*                 { constant(ST_INT); }
0{DIGIT}+                       { lex_error("number with a leading zero"); }
{DIGIT}+({LETTER}|_)({LETTER}|{DIGIT}|_)*  { lex_error("identifier cannot start with a digit"); }

#{HEX}{6}                       { constant(ST_COLOR); }
#({LETTER}|{DIGIT})*            { lex_error("malformed color, expected # followed by exactly 6 hex digits"); }

\"[^"\r\n]*\"                   { constant(ST_STRING); }
\"[^"\r\n]*                     { lex_error("unterminated string"); }

"=="|"!="|"<="|">="|"<"|">"|"="|"+"|"-"|"*"|"/"|","|":"|"("|")"|"{"|"}"|"^" {
                                  add_pif(token_code(yytext), -1);
                                }

.                               { lex_error("unexpected character"); }

%%

static int load_tokens(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return 0; }
    char line[128], name[64];
    int code;
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '/' && line[1] == '/') continue;     /* comment line */
        if (sscanf(line, "%63s %d", name, &code) != 2) continue;
        if (tokenCount >= MAX_TOKENS) { fprintf(stderr, "too many tokens\n"); return 0; }
        strncpy(tokens[tokenCount].name, name, sizeof tokens[tokenCount].name - 1);
        tokens[tokenCount].code = code;
        tokenCount++;
    }
    fclose(f);
    CODE_ID = token_code("identifier");
    CODE_CONST = token_code("constant");
    if (CODE_ID < 0 || CODE_CONST < 0) {
        fprintf(stderr, "tokens.txt must define identifier and constant\n");
        return 0;
    }
    return 1;
}

static const char *token_name(int code) {
    for (int i = 0; i < tokenCount; i++)
        if (tokens[i].code == code) return tokens[i].name;
    return "?";
}

/* "dir/program 1.txt" -> "program 1" */
static void base_name(const char *path, char *out, size_t size) {
    const char *s = strrchr(path, '/');
    const char *b = strrchr(path, '\\');
    if (b && (!s || b > s)) s = b;
    s = s ? s + 1 : path;
    strncpy(out, s, size - 1);
    out[size - 1] = '\0';
    char *dot = strrchr(out, '.');
    if (dot) *dot = '\0';
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s tokens.txt source.txt [output_dir]\n", argv[0]);
        return 2;
    }
    const char *outDir = argc > 3 ? argv[3] : ".";

    if (!load_tokens(argv[1])) return 2;
    yyin = fopen(argv[2], "r");
    if (!yyin) { perror(argv[2]); return 2; }

    char base[128], path[512];
    base_name(argv[2], base, sizeof base);
    snprintf(path, sizeof path, "%s/%s.errors.out", outDir, base);
    errFile = fopen(path, "w");
    if (!errFile) { perror(path); return 2; }

    pif = malloc(MAX_PIF * sizeof(PifEntry));
    st_init(&st);
    yylex();

    /* PIF */
    snprintf(path, sizeof path, "%s/%s.PIF.out", outDir, base);
    FILE *f = fopen(path, "w");
    if (!f) { perror(path); return 2; }
    fprintf(f, "%-6s %-12s %-6s %s\n", "code", "token", "ST pos", "line");
    for (int i = 0; i < pifCount; i++) {
        const char *name = token_name(pif[i].code);
        if (pif[i].stPos >= 0) fprintf(f, "%-6d %-12s %-6d %d\n", pif[i].code, name, pif[i].stPos, pif[i].line);
        else                   fprintf(f, "%-6d %-12s %-6s %d\n", pif[i].code, name, "-", pif[i].line);
    }
    fclose(f);

    /* ST */
    snprintf(path, sizeof path, "%s/%s.ST.out", outDir, base);
    f = fopen(path, "w");
    if (!f) { perror(path); return 2; }
    st_print(&st, f);
    fclose(f);

    if (errorCount == 0) {
        printf("%s: lexically correct (%d PIF entries, %d ST entries)\n", argv[2], pifCount, st.count);
        fprintf(errFile, "no lexical errors\n");
    } else {
        printf("%s: %d lexical error(s) found\n", argv[2], errorCount);
    }

    fclose(errFile);
    fclose(yyin);
    st_free(&st);
    free(pif);
    return errorCount ? 1 : 0;
}
