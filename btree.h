#include <stdlib.h>

#ifndef B_TREE_BTREE_H
#define B_TREE_BTREE_H

typedef int key_type;

typedef struct Key
{
    key_type key;
    void* data;
    struct Key *left_key, *right_key;
    struct Node *left_child, *right_child;
} Key;

typedef struct Node
{
    struct Node *parent;
    struct Key *first_key;
    size_t min_size;
} Node;


void init_btree(Node *root, key_type *array, size_t array_size, size_t min_size, int (*compar)(const void*, const void*))
{
    qsort(array, array_size, sizeof(key_type), compar);

    root->min_size = min_size;


}

#endif //B_TREE_BTREE_H
