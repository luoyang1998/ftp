//hash.h
#ifndef _HASH_H_
#define _HASH_H_
typedef struct hash hash_t;
typedef unsigned int (*hashfunc_t)(unsigned int, void*);
hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func);
void* hash_lookup_entry(hash_t *hash, void* key, unsigned int key_size);
void hash_add_entry(hash_t *hash, void *key, unsigned int key_size, void *value, unsigned int value_size);
void hash_free_entry(hash_t *hash, void *key, unsigned int key_size);

typedef struct hash_node{
    void *key;  // 哈希表对象指针
    void *value;    // 查找键值
    struct hash_node *prev;
    struct hash_node *next;
} hash_node_t;
struct hash{
    unsigned int buckets;   // 哈希表数量
    hashfunc_t hash_func;   // 哈希函数指针，计算键的哈希值
    hash_node_t **nodes;
};
hash_node_t** hash_get_bucket(hash_t *hash, void *key);
hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size);
#endif