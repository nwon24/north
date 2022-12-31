#ifndef HASH_H_
#define HASH_H_

#include <stdbool.h>

typedef enum hash_type {
    HASH_KEYWORD,
    HASH_DIRECTIVE,
    HASH_MACRO,
    HASH_FUNCTION,
    HASH_VAR,
} HashType;

typedef struct hash_entry {
    void *ptr;
    char *identifier;
    HashType type;
    struct hash_entry *next;
    struct hash_entry *prev;
} HashEntry;

typedef struct hash_table {
    HashEntry **table;
    int size;
    int (*hashfn)(char *identifier, int hash_size);
} HashTable;

HashTable *new_hash_table(int (*fn)(char *identifier, int hash_size), int size);
HashEntry *new_hash_entry(char *identifier, void *ptr);
int add_hash_entry(HashTable *table, HashEntry *entry);
HashEntry *in_hash(HashTable *table, char *identifier);
int string_hashfn(char *str, int hash_size);

#endif /* HASH_H_ */
