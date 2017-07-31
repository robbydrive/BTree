#include <stdio.h>
#include "btree.h"


// Example of compar function passed to btree_init and then to stdlib qsort
int comp(const void * a, const void * b)
{
    return ((Cell *)a)->key - ((Cell *)b)->key;
}


int main()
{
    const size_t n = 2449;
    Cell *a[n];
    for (size_t i = 0; i < n; ++i)
    {
        a[i] = (Cell *) malloc(sizeof(Cell));
        a[i]->key = i + 1;
    }
    Node *root;
    // Creating a B-Tree from Cells array 'a'
    btree_init(&root, a, n, 4, 1, comp);

    Cell *found;
    key_type key = 1237;
    // Searching for a Cell
    btree_get_cell(root, &found, key);
    printf("Key: %d\n", found->key); // Output: 'Key: 1237'. Data is NULL at this point

    int some_number = 7;
    found = (Cell *) malloc(sizeof(Cell));
    found->key = key;
    found->data = &some_number;
    // Testing replace function with new Cell
    btree_replace_cell(root, &found);

    Cell *new_found;
    // Repeat getting a Cell with the same key
    btree_get_cell(root, &new_found, key);
    printf("Key: %d; Data: %d\n", new_found->key, *((int *)new_found->data)); // Output: 'Key: 1237; Data: 7'
    // Cleaning the entire B-Tree with single function call
    btree_clean_node(&root);
    return 0;
}