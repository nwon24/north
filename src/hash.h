#ifndef HASH_H_
#define HASH_H_

typedef struct hash_entry {
    void *ptr;
    char *identifier;
    struct hash_entry *next;
    struct hash_entry *prev;
} HashEntry;

typedef struct hash_table {
    HashEntry **table;
    int size;
    int (*hashfn)(char *identifier, int hash_size);
} HashTable;

HashTable *new_hash_table(int (*fn)(char *identifier, int hash_size), int size);
int add_hash_entry(HashTable *table, HashEntry *entry);

#endif /* HASH_H_ */
