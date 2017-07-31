#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef B_TREE_BTREE_H
#define B_TREE_BTREE_H

char error_string[100];

typedef int key_type;

typedef struct Cell
{
    key_type key;
    void* data;
    struct Cell *left_cell, *right_cell;
    struct Node *left_child, *right_child, *owner;
} Cell;

typedef struct Node
{
    struct Node *parent;
    struct Cell *first_cell;
    size_t min_size;
    size_t size;
} Node;


/**
 * @brief Initializes B-Tree from a given @p array, putting a pointer to the root element in the @p root
 * @param root Pointer to the pointer to root Node, that is filled with address of constructed tree root
 * @param array Array of Cells from which tree is constructed. Elements should be allocated in heap
 * @param array_size Size of @p array
 * @param min_size This number defines how many cells must be in a node at least and at most (2 * min_size)
 * @param to_leave This number defines how many cells in a node function will try to reserve
 * for possible future inserting - then the max number of most nodes equals 2 * min_size - to_leave
 * @param compar Pointer to function used to compare elements in quick sorting procedure
 * @return Exit code: 0 if tree was constructed successfully, 1 otherwise (string representation of an error is
 * available through global @c error_string variable)
 *
 * @details Function sorts all cells in designated with @p compar function order (usually from min to max),
 * then iteratively repeats next steps:
 * 1. Moves nodes from the previous level to current level's children if there are any
 * 2. Desides how many nodes should be constructed according to formula:
 *      number of remaining cells / (2 * @p min_size - @p to_leave).
 *    This formula guarantees that most of the nodes will have required reserve, but all of them,
 *    because it is possible that the remainder from the division in the formula is less than min_size
 *    and the sum of divider from the formula and the remainder is less than max size of the node (2 * @p min_size).
 *    In this scenario last node must be filled with more cells than required
 * 3. Forms current level nodes and sends every (divider + 1)th node to the array of cells to be used on higher level
 * 4. Performs parameters swapping between levels
 * 5. If there are more than 1 Node left, repeats from step #1, otherwise there is a root element,
 *    which is put into @p root
 */
int btree_init(Node **root, Cell **array, size_t array_size, size_t min_size,
               size_t to_leave, int (*compar)(const void *, const void *));


/**
 * @brief Finds the Cell object with key equals to @p key_value
 * @param root Pointer to Node from which search should be started
 * @param cell_ptr Pointer to the pointer to Cell where the found Cell is saved
 * @param key_value Value that desired Cell should contain as key value
 * @return Exit code: 0 if Cell was found, 1 otherwise
 */
int btree_get_cell(Node *root, Cell **cell_ptr, key_type key_value);


/**
 * @brief Replaces the cell with the same to new_cell's key if there is one in tree with @p root as root element
 * @param root Pointer to the root Node element of the tree
 * @param new_cell Pointer to the pointer to replacement cell
 * @return Exit code: 0 if substitution succeeded, 1 if there is no cell in the tree with the same to new_cell's key
 */
int btree_replace_cell(Node *root, Cell **new_cell);


/**
 * @brief Frees all nodes and cells allocated in heap
 * IMPORTANT! Cells MUST be allocated in heap in order to use this function
 * @param root Pointer to the pointer to root Node of tree (or subtree)
 */
void btree_clean_node(Node **root);

#endif //B_TREE_BTREE_H
