typedef struct {
    char **keys;
    void **values;
    int count;
    int capacity;
} Hash;

unsigned str_hash(char *str) {
    unsigned hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

int hash_find_pos(Hash *hash, char *name) {
    int pos;
    for (pos = str_hash(name) % hash->capacity;
         hash->keys[pos] != NULL && strcmp(name, hash->keys[pos]) != 0;
         pos++);

    return pos;
}

void hash_grow_table(Hash *hash) {
    char **oldKeys = hash->keys;
    void **oldValues = hash->values;
    int oldCapacity = hash->capacity;

    hash->capacity = hash->capacity * 2 + 10;
    hash->keys = calloc(hash->capacity, sizeof(char *));
    hash->values = calloc(hash->capacity, sizeof(void *));

    for (int i = 0; i < oldCapacity; i++) {
        if (oldKeys[i]) {
            int pos = hash_find_pos(hash, oldKeys[i]);
            hash->keys[pos] = oldKeys[i];
            hash->values[pos] = oldValues[i];
        }
    }
}

void *hash_get(Hash *hash, char *name) {
    if (hash->capacity == 0) {
        return NULL;
    }

    int pos = hash_find_pos(hash, name);
    return hash->values[pos];
}

void hash_set(Hash *hash, char *name, void *value) {
    if (!hash->capacity || hash->capacity / hash->count < 2) {
        hash_grow_table(hash);
    }

    int pos = hash_find_pos(hash, name);

    hash->keys[pos] = name;
    hash->values[pos] = value;
    hash->count++;
}

