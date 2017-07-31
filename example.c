#include <stdio.h>
#include "btree.h"

int comp(const void * a, const void * b)
{
    return ((Cell *)a)->key - ((Cell *)b)->key;
}

int main()
{
    const size_t n = 25;
    Cell *a[n];
    for (size_t i = 0; i < n; ++i)
    {
        a[i] = (Cell *) malloc(sizeof(Cell));
        a[i]->key = i + 1;
    }
    Node *root;
    btree_init(&root, a, n, 4, 1, comp);
    printf("%d\n", root->first_cell->key);

    Cell *found;
    btree_get_cell(root, &found, 17);
    printf("Key: %d\n", found->key);

    int some_number = 7;
    found = (Cell *) malloc(sizeof(Cell));
    found->key = 17;
    found->data = &some_number;
    btree_replace_cell(root, &found);

    Cell *new_found;
    btree_get_cell(root, &new_found, 17);
    printf("Key: %d; Data: %d\n", new_found->key, *((int *)new_found->data));
    btree_clean_node(&root);
    return 0;
}