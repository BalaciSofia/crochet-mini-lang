#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>

#define ST_SIZE 211           

typedef enum { ST_IDENTIFIER, ST_INT, ST_COLOR, ST_STRING } StType;

typedef struct StNode {
    char *key;                
    StType type;
    int position;             
    struct StNode *next;      
} StNode;

typedef struct {
    StNode *bucket[ST_SIZE];
    int count;
} SymbolTable;

void st_init(SymbolTable *st);
unsigned st_hash(const char *key);
int st_insert(SymbolTable *st, const char *key, StType type);
int st_lookup(const SymbolTable *st, const char *key);
void st_print(const SymbolTable *st, FILE *f);
void st_free(SymbolTable *st);
const char *st_type_name(StType t);

#endif
