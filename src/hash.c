/*
 * Generic hash table code.
 * Will be used for variables and perhaps language keywords/inbuilt
 * words if there are too many of them.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "hash.h"

HashTable *new_hash_table(int (*fn)(char *identifier, int hash_size), int size)
{
    HashEntry **table;
    HashTable *new;

    if ((new = malloc(sizeof(*new))) == NULL) {
	fatal("new_hash_table: malloc returned NULL!");
    }
    if ((table = calloc(size, sizeof(*table))) == NULL) {
	fatal("new_hash_table: malloc returned NULL!");
    }
    new->table = table;
    new->size = size;
    new->hashfn = fn;
    return new;
}

/*
 * Returns -1 if entry already in hash, 0 otherwise.
 */
int add_hash_entry(HashTable *table, HashEntry *entry)
{
    int hashn;
    HashEntry **tab, *tmp;

    hashn = table->hashfn(entry->identifier, table->size);
    tab = table->table;
    assert(hashn >= 0 && hashn < table->size);
    if (tab[hashn] == NULL) {
	tab[hashn] = entry;
	entry->next = entry->prev = NULL;
	return 0;
    } else {
	for (tmp = tab[hashn]; tmp->next != NULL; tmp = tmp->next) {
	    if (strcmp(tmp->identifier, entry->identifier) == 0) {
		return -1;
	    }
	}
	tmp->next = entry;
	entry->prev = tmp;
	entry->next = NULL;
	return 0;
    }
}

int string_hashfn(char *str, int hash_size)
{
    char *p;
    int sum;

    for (sum = 0, p = str; *p != '\0'; p++) {
	sum += *p;
    }
    return sum % hash_size;
}
