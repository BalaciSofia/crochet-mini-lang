#include <stdlib.h>
#include <string.h>
#include "symtab.h"

void st_init(SymbolTable *st) {
    for (int i = 0; i < ST_SIZE; i++) st->bucket[i] = NULL;
    st->count = 0;
}

/* hash = sum of the ASCII codes of the characters, modulo the number of buckets */
unsigned st_hash(const char *key) {
    unsigned long sum = 0;
    for (const unsigned char *p = (const unsigned char *)key; *p; p++)
        sum += *p;
    return (unsigned)(sum % ST_SIZE);
}

int st_lookup(const SymbolTable *st, const char *key) {
    for (StNode *n = st->bucket[st_hash(key)]; n; n = n->next)
        if (strcmp(n->key, key) == 0) return n->position;
    return -1;
}

int st_insert(SymbolTable *st, const char *key, StType type) {
    unsigned h = st_hash(key);
    for (StNode *n = st->bucket[h]; n; n = n->next)
        if (strcmp(n->key, key) == 0) return n->position;   

    StNode *n = malloc(sizeof(StNode));
    n->key = malloc(strlen(key) + 1);
    strcpy(n->key, key);
    n->type = type;
    n->position = st->count++;
    n->next = st->bucket[h];          
    st->bucket[h] = n;
    return n->position;
}

const char *st_type_name(StType t) {
    switch (t) {
        case ST_IDENTIFIER: return "identifier";
        case ST_INT:        return "int constant";
        case ST_COLOR:      return "color constant";
        default:            return "string constant";
    }
}

void st_print(const SymbolTable *st, FILE *f) {
    StNode **byPos = calloc(st->count ? st->count : 1, sizeof(StNode *));
    for (int i = 0; i < ST_SIZE; i++)
        for (StNode *n = st->bucket[i]; n; n = n->next)
            byPos[n->position] = n;

    fprintf(f, "Symbol table (hash table, %d buckets, %d entries)\n", ST_SIZE, st->count);
    fprintf(f, "%-9s %-8s %-16s %s\n", "position", "bucket", "type", "lexeme");
    for (int i = 0; i < st->count; i++)
        fprintf(f, "%-9d %-8u %-16s %s\n", byPos[i]->position,
                st_hash(byPos[i]->key), st_type_name(byPos[i]->type), byPos[i]->key);
    free(byPos);
}

void st_free(SymbolTable *st) {
    for (int i = 0; i < ST_SIZE; i++) {
        StNode *n = st->bucket[i];
        while (n) {
            StNode *next = n->next;
            free(n->key);
            free(n);
            n = next;
        }
        st->bucket[i] = NULL;
    }
    st->count = 0;
}
