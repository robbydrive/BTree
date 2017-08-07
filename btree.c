#include "btree.h"

typedef enum Mode {
    GET,
    REPLACE,
    INSERT
} Mode;

// Helper function to create and initialize node (used in btree_init)
int node_init(Node **node, size_t min_size, int (*comparator)(const void *, const void *))
{
    *node = (Node *) malloc(sizeof(Node));
    (*node)->cells = (Cell *) calloc(min_size * 2, sizeof(Cell));

    if (node == NULL)
    {
        strcpy(error_string, "Error allocating memory for new node");
        return 1;
    }
    if ((*node)->cells == NULL)
    {
        strcpy(error_string, "Error allocating memory for new node's cells");
        free(*node);
        return 1;
    }

    (*node)->min_size = min_size;
    (*node)->size = 0;
    (*node)->is_leaf = true;
    (*node)->compar = comparator;
    return 0;
}


// Helper function to fulfill change attributes of its' neighbours (used in btree_init)
int cell_init(Cell *new_cell, Node *owner)
{
    if (owner == NULL)
    {
        strcpy(error_string, "Wrong new_cell owner: it cannot be NULL");
        return 1;
    }

    new_cell->owner = owner;
    memcpy(&owner->cells[owner->size++], new_cell, sizeof(Cell));
    return 0;
}


// Helper function to finish node creation (used in btree_init)
void finish_node_packing(Cell *upper_array, size_t *index, Cell *key,
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
    for (int n_node = 0; n_node < children_array_size; ++n_node)
        if (target_cell->key < children_array[n_node]->cells[0].key)
        {
            children_array[n_node]->parent = target_cell->owner;
            target_cell->left_child = n_node != 0 ? children_array[n_node-1] : NULL;
            target_cell->right_child = children_array[n_node];
            target_cell->owner->is_leaf = false;
            break;
        }
}


int btree_init(Node **root, Cell *array, size_t array_size, size_t min_size, size_t to_leave,
               int (*compar)(const void *, const void *))
{
    qsort(array, array_size, sizeof(Cell), compar);

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

    Cell current_array[array_size];
    Cell upper_array[current_array_size / divider + 1];
    memcpy(current_array, array, sizeof(Cell) * array_size);

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
                    code = node_init(&current_node, min_size, compar);
                    if (code != 0)
                    {
                        printf("Error while creating node, i = %lu, error:\n%s\n", i, error_string);
                        return 1;
                    }
                }

                code = cell_init(&current_array[i], current_node);
                if (code != 0)
                {
                    printf("Error while creating key, i = %lu, error:\n%s\n", i, error_string);
                    return 1;
                }

                if (children_size > 0)
                    find_children_for_cell(&current_node->cells[current_node->size - 1], child_nodes, children_size);

                current_cell = &current_array[i];
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


int get_cell_or_leaf(Node *root, Cell **cell_ptr, Node **leaf_ptr, key_type key_value, Mode mode)
{
    int i = 0;
    while (key_value > root->cells[i].key && i < root->size - 1)
        i++;
    if (key_value == root->cells[i].key)
    {
        if (mode == GET)
            memcpy(*cell_ptr, &root->cells[i], sizeof(Cell));
        else
            *cell_ptr = &root->cells[i];
        return 0;
    }
    if (key_value < root->cells[i].key && root->cells[i].left_child != NULL)
        return get_cell_or_leaf(root->cells[i].left_child, cell_ptr, leaf_ptr, key_value, mode);
    if (key_value > root->cells[i].key && root->cells[i].right_child != NULL)
        return get_cell_or_leaf(root->cells[i].right_child, cell_ptr, leaf_ptr, key_value, mode);

    if (mode == INSERT && root->is_leaf)
        *leaf_ptr = root;
    return 1;
}


int btree_get_cell(Node *root, Cell *cell_ptr, key_type key_value)
{
    return get_cell_or_leaf(root, &cell_ptr, NULL, key_value, GET);
}


int btree_replace_cell(Node *root, Cell **new_cell)
{
    Cell *cell_to_replace;
    int code = get_cell_or_leaf(root, &cell_to_replace, NULL, (*new_cell)->key, REPLACE);
    if (code != 0)
    {
        strcpy(error_string, "No cell presented with provided key in the tree\n");
        return 1;
    }

    (*new_cell)->left_child = cell_to_replace->left_child;
    (*new_cell)->right_child = cell_to_replace->right_child;
    (*new_cell)->owner = cell_to_replace->owner;

    memcpy(cell_to_replace, *new_cell, sizeof(Cell));
    return 0;
}


void btree_clean_node(Node **root)
{
    Cell *cell;
    for (int i = 0; i < (*root)->size; ++i)
    {
        cell = &(*root)->cells[i];
        if (cell->left_child != NULL)
            btree_clean_node(&cell->left_child);
        // Check for NULL in right_cell to avoid double freeing of the same memory piece
        // It is true only for the most right cell of the node
        if (i == (*root)->size - 1 && cell->right_child != NULL)
            btree_clean_node(&cell->right_child);
    }
    free((*root)->cells);
    free(*root);
}


void insert_cell_internal(Node **root, Node **node_to_insert, Cell *new_cell)
{
    Node *node_ptr = *node_to_insert;
    new_cell->owner = *node_to_insert;
    if (node_ptr->size < node_ptr->min_size * 2)
    {
        memcpy(&node_ptr->cells[node_ptr->size++], new_cell, sizeof(Cell));
        qsort(node_ptr->cells, node_ptr->size, sizeof(Cell), node_ptr->compar);
    }
    else if (node_ptr->size == node_ptr->min_size * 2)
    {
        Cell *cells = (Cell *)calloc(node_ptr->size + 1, sizeof(Cell));
        memcpy(cells, node_ptr->cells, sizeof(Cell) * node_ptr->size);
        cells[node_ptr->size] = *new_cell;
        qsort(cells, node_ptr->size + 1, sizeof(Cell), node_ptr->compar);

        Node *left, *right;
        node_init(&left, node_ptr->min_size, node_ptr->compar);
        node_init(&right, node_ptr->min_size, node_ptr->compar);
        memcpy(left->cells, cells, sizeof(Cell) * node_ptr->min_size);
        memcpy(right->cells, &cells[node_ptr->min_size + 1], sizeof(Cell) * node_ptr->min_size);
        cells[node_ptr->min_size].left_child = left;
        cells[node_ptr->min_size].right_child = right;

        if (node_ptr->parent != NULL)
            insert_cell_internal(root, &node_ptr->parent, &cells[node_ptr->min_size]);
        else
        {
            Node *new_node;
            node_init(&new_node, node_ptr->min_size, node_ptr->compar);
            memcpy(new_node->cells, &cells[node_ptr->min_size], sizeof(Cell));
            node_ptr->parent = new_node;
            node_ptr->is_leaf = false;
            *root = new_node;
        }
        free(cells);
    }
}


void btree_insert_cell(Node **root, Cell *new_cell)
{
    Cell *cell_to_replace = NULL,
         *new_heap_cell = (Cell *)malloc(sizeof(Cell));
    memcpy(new_heap_cell, new_cell, sizeof(Cell));
    Node *leaf_to_insert = NULL;
    int code = get_cell_or_leaf(*root, &cell_to_replace, &leaf_to_insert, new_cell->key, INSERT);
    if (code == 0 && cell_to_replace != NULL)
    {
        memcpy(cell_to_replace, new_heap_cell, sizeof(Cell));
        free(new_heap_cell);
    }
    else if (code == 1 && leaf_to_insert != NULL)
        insert_cell_internal(root, &leaf_to_insert, new_heap_cell);
}
