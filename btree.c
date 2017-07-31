#include "btree.h"


// Helper function to create and initialize node (used in btree_init)
int node_init(Node **node, Cell *first_cell, size_t min_size)
{
    *node = (Node *) malloc(sizeof(Node));

    if (node == NULL)
    {
        strcpy(error_string, "Error allocating memory for new node");
        return 1;
    }

    (*node)->first_cell = first_cell;
    (*node)->min_size = min_size;
    (*node)->size = (*node)->first_cell == NULL ? 0 : 1;
    return 0;
}


// Helper function to fulfill change attributes of its' neighbours (used in btree_init)
int cell_init(Cell *new_cell, Cell *left, Cell *right, Node *owner)
{
    if (owner == NULL)
    {
        strcpy(error_string, "Wrong new_cell owner: it cannot be NULL");
        return 1;
    }

    if (left != NULL)
        left->right_cell = new_cell;
    if (right != NULL)
        right->left_cell = new_cell;

    new_cell->left_cell = left;
    new_cell->right_cell = right;
    new_cell->owner = owner;
    return 0;
}


// Helper function to finish node creation (used in btree_init)
void finish_node_packing(Cell **upper_array, size_t *index, Cell **key,
                         Node **node_array, size_t *node_index, Node **current_node, Cell **current_cell)
{
    upper_array[(*index)++] = *key;
    node_array[(*node_index)++] = *current_node;
    *current_node = NULL;
    *current_cell = NULL;
}


// Helper function to find children nodes for target_cell in children_array (used in btree_init)
void find_children_for_cell(Cell *target_cell, Node **children_array, size_t children_array_size)
{
    for (int node_number = 0; node_number < children_array_size; ++node_number)
        if (target_cell->key < children_array[node_number]->first_cell->key)
        {
            children_array[node_number]->parent = target_cell->owner;
            target_cell->left_child = node_number != 0 ? children_array[node_number-1] : NULL;
            target_cell->right_child = children_array[node_number];
            break;
        }
}


int btree_init(Node **root, Cell **array, size_t array_size, size_t min_size, size_t to_leave,
               int (*compar)(const void *, const void *))
{
    qsort(array, array_size, sizeof(Cell *), compar);

    if (to_leave > min_size || to_leave < 0)
    {
        sprintf(error_string, "To_leave argument is wrong, value = %ld", to_leave);
        return 1;
    }

    size_t current_array_size = array_size,
            upper_index = 0,
            nodes_on_level,
            divider = min_size * 2 - to_leave + 1,
            remainder,
            last_node_separator;

    Cell *current_array[array_size];
    Cell *upper_array[current_array_size / divider + 1];
    memcpy(current_array, array, sizeof(Cell *) * array_size);

    size_t current_node_index = 0,
            children_size = 0;
    Node *child_nodes[current_array_size / divider + 2],
            *current_nodes[current_array_size / divider + 2];
    Node *current_node;
    Cell *current_cell;
    int code, border;

    do
    {
        nodes_on_level = current_array_size / divider;
        // Border variable sets the limit until nodes are filled with required number of cells
        border = (nodes_on_level - 1) * divider;
        border = border > 0 ? border : 0;
        remainder = current_array_size - border;
        // If the remainder of the formula allows to create another node, then last 2 nodes should be similar-sized
        // See details in function docs
        last_node_separator = remainder <= min_size * 2 ? current_array_size + 1 : border + remainder / 2 + 1;

        current_node = NULL;
        current_cell = NULL;

        if (current_node_index > 0)
        {
            memset(child_nodes, 0, sizeof(Node *) * children_size);
            memcpy(child_nodes, current_nodes, sizeof(Node *) * current_node_index);
            memset(current_nodes, 0, sizeof(Node *) * current_node_index);
            children_size = current_node_index;
            current_node_index = 0;
        }

        for (size_t i = 0; i < current_array_size; ++i)
        {
            if ((i < border && (i + 1) % divider == 0)
                || (i >= border && (i + 1) == last_node_separator))
            {
                finish_node_packing(upper_array, &upper_index, &current_array[i],
                                    current_nodes, &current_node_index, &current_node, &current_cell);
            }
            else
            {
                if (current_node == NULL)
                {
                    code = node_init(&current_node, NULL, min_size);
                    if (code != 0)
                    {
                        printf("Error while creating node, i = %lu, error:\n%s\n", i, error_string);
                        return 1;
                    }
                }

                code = cell_init(current_array[i], current_cell, NULL, current_node);
                if (code != 0)
                {
                    printf("Error while creating key, i = %lu, error:\n%s\n", i, error_string);
                    return 1;
                }

                if (children_size > 0)
                    find_children_for_cell(current_array[i], child_nodes, children_size);

                if (current_node->first_cell == NULL)
                    current_node->first_cell = current_array[i];

                current_node->size++;
                current_cell = current_array[i];
            }
        }

        // Finishes formation of the last node
        if (current_node != NULL)
            current_nodes[current_node_index++] = current_node;

        // Swapping parameters between levels
        memcpy(current_array, upper_array, sizeof(Cell *) * upper_index);
        memset(upper_array, 0, sizeof(Cell *) * upper_index);
        current_array_size = upper_index;
        upper_index = 0;
    }
    while (nodes_on_level > 1);

    *root = current_nodes[0];
    return 0;
}


int btree_get_cell(Node *root, Cell **cell_ptr, key_type key_value)
{
    Cell *current_cell = root->first_cell;
    while (key_value > current_cell->key && current_cell->right_cell != NULL)
        current_cell = current_cell->right_cell;
    if (key_value == current_cell->key)
    {
        *cell_ptr = current_cell;
        return 0;
    }
    if (key_value < current_cell->key && current_cell->left_child != NULL)
        return btree_get_cell(current_cell->left_child, cell_ptr, key_value);
    if (key_value > current_cell->key && current_cell->right_child != NULL)
        return btree_get_cell(current_cell->right_child, cell_ptr, key_value);
    return 1;
}


int btree_replace_cell(Node *root, Cell **new_cell)
{
    Cell *cell_to_replace;
    int code = btree_get_cell(root, &cell_to_replace, (*new_cell)->key);
    if (code != 0)
    {
        strcpy(error_string, "No cell presented with provided key in the tree\n");
        return 1;
    }
    (*new_cell)->left_cell = cell_to_replace->left_cell;
    (*new_cell)->right_cell = cell_to_replace->right_cell;
    (*new_cell)->left_child = cell_to_replace->left_child;
    (*new_cell)->right_child = cell_to_replace->right_child;
    (*new_cell)->owner = cell_to_replace->owner;

    if ((*new_cell)->owner->first_cell == cell_to_replace)
        (*new_cell)->owner->first_cell = *new_cell;
    if ((*new_cell)->left_cell != NULL)
        (*new_cell)->left_cell->right_cell = (*new_cell);
    if ((*new_cell)->right_cell != NULL)
        (*new_cell)->right_cell->left_cell = (*new_cell);
    free(cell_to_replace); // Otherwise there is a memory leak as all links to this Cell are lost
    return 0;
}


void btree_clean_node(Node **root) {
    Cell *current_cell = (*root)->first_cell,
            *helper;
    while (current_cell != NULL)
    {
        if (current_cell->left_child != NULL)
            btree_clean_node(&current_cell->left_child);
        // Check for NULL in right_cell to avoid double freeing of the same memory piece
        // It is true only for the most right cell of the node
        if (current_cell->right_cell == NULL && current_cell->right_child != NULL)
            btree_clean_node(&current_cell->right_child);
        helper = current_cell->right_cell;
        free(current_cell);
        current_cell = helper;
    }
    free(*root);
}
