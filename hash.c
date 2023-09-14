//hash.c
#include "hash.h"
#include "common.h"
#include <assert.h>

// 创建一个指定大小的哈希表
hash_t *hash_alloc(unsigned int buckets, hashfunc_t hash_func) {
    hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
    //assert(hash != NULL);
    hash->buckets = buckets;
    hash->hash_func = hash_func;
    int size = buckets * sizeof(hash_node_t *); // 需要分配内存大小
    hash->nodes = (hash_node_t **)malloc(size); // 表头地址
    memset(hash->nodes, 0, size);   // 将内存空间初始化为0
    return hash;
}
// 在哈希表中查找给定键的值
void* hash_lookup_entry(hash_t *hash, void* key, unsigned int key_size) {
    hash_node_t *node = hash_get_node_by_key(hash, key, key_size);  // 查找指向键所在的哈希表节点指针
    if (node == NULL){
        return NULL;
    }
    return node->value;
}
// 哈希表增加条目
void hash_add_entry(hash_t *hash, void *key, unsigned int key_size, void *value, unsigned int value_size) {
    if (hash_lookup_entry(hash, key, key_size)) {
        fprintf(stderr, "duplicate hash key\n");
        return;
    }
    hash_node_t *node = malloc(sizeof(hash_node_t));
    node->prev = NULL;
    node->next = NULL;
    node->key = malloc(key_size);
    memcpy(node->key, key, key_size);
    node->value = malloc(value_size);
    memcpy(node->value, value, value_size);
    hash_node_t **bucket = hash_get_bucket(hash, key);
    if (*bucket == NULL){
        *bucket = node;
    } else {
        // 将新结点插入到链表头部
        node->next = *bucket;
        (*bucket)->prev = node;
        *bucket = node;
    }
}
// 释放哈希表指定条目数
void hash_free_entry(hash_t *hash, void *key, unsigned int key_size) {
    hash_node_t *node = hash_get_node_by_key(hash, key, key_size);
    if (node == NULL){
        return;
    }
    free(node->key);
    free(node->value);
    if (node->prev){
        node->prev->next = node->next;
    } else {
        hash_node_t **bucket = hash_get_bucket(hash, key);
        *bucket = node->next;
    }
    if (node->next)
        node->next->prev = node->prev;
    free(node);
}
// 获取存储给定键的桶指针
hash_node_t** hash_get_bucket(hash_t *hash, void *key){
    unsigned int bucket = hash->hash_func(hash->buckets, key);  // 根据键值和桶数查找对应索引
    if (bucket >= hash->buckets){
        fprintf(stderr, "bad bucket lookup\n");
        exit(EXIT_FAILURE);
    }
    return &(hash->nodes[bucket]);
}
// 根据键值获取哈希表节点
hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size){
    hash_node_t **bucket = hash_get_bucket(hash, key);  // 获取对应索引
    hash_node_t *node = *bucket;
    if (node == NULL){
        return NULL;
    }
    while (node != NULL && memcmp(node->key, key, key_size) != 0){
        node = node->next;
    }
    return node;
}