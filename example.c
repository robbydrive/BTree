#include <stdio.h>
#include "btree.h"


// Example of compar function passed to btree_init and then to stdlib qsort
int comp(const void * a, const void * b)
{
    Cell *c1 = (Cell *)a;
    Cell *c2 = (Cell *)b;
    if (c1->key < c2->key)
        return -1;
    if (c1->key > c2->key)
        return 1;
    return 0;
}


int main()
{
    const size_t n = 25;
    Cell a[n];
    for (int i = 0; i < n; ++i)
    {
//        a[i] = (Cell *) malloc(sizeof(Cell));
        a[i].key = (i + 1) * 2 - 1;
    }
    a[24].key = 2;
    Node *root;
    // Creating a B-Tree from Cells array 'a'
    btree_init(&root, a, n, 4, 1, comp);

    Cell found;
    key_type key = 9;
    // Searching for a Cell
    btree_get_cell(root, &found, key);
    printf("Key: %d\n", found.key);

    int some_number = 7;
    Cell *new_cell = (Cell *) malloc(sizeof(Cell));
    new_cell->key = key;
    new_cell->data = &some_number;
    // Testing replace function with new Cell
    btree_replace_cell(root, &new_cell);

    Cell new_found;
    // Repeat getting a Cell with the same key
    btree_get_cell(root, &new_found, key);
    printf("Key: %d; Data: %d\n", new_found.key, *((int *)new_found.data));

    Cell cell_to_insert;
    cell_to_insert.key = 2;
    btree_insert_cell(&root, &cell_to_insert);
    btree_get_cell(root, &new_found, cell_to_insert.key);
    printf("Key: %d\n", new_found.key);
    // Cleaning the entire B-Tree with single function call
    btree_clean_node(&root);
    return 0;
}