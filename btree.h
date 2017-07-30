#include <stdlib.h>

#ifndef B_TREE_BTREE_H
#define B_TREE_BTREE_H

extern char error_string[200];

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


//region node_init

// Helper function to fulfill just created node
int node_init(Node **node, Key *first_key, size_t min_size)
{
    *node = (Node *) malloc(sizeof(Node));

    if (node == NULL)
    {
        strcpy(error_string, "Error allocating memory for new node\0");
        return 1;
    }
    
    (*node)->first_key = first_key;
    (*node)->min_size = min_size;
    (*node)->size = (*node)->first_key == NULL ? 0 : 1;
    return 0;
}
//endregion

//region key_init

// Helper function to fulfill just created key and change attributes of its' neighbours
int key_init(Key **key, Key *left, Key *right, key_type key_value, Node *owner)
{
    if (owner == NULL)
    {
        strcpy(error_string, "Wrong key owner: it cannot be NULL\0");
        return 1;
    }

    *key = (Key *) malloc(sizeof(Key));
    if (key == NULL)
    {
        strcpy(error_string, "Error allocating memory for new key\0");
        return 1;
    }

    if (left != NULL) 
        left->right_key = *key;
    if (right != NULL)
        right->left_key = *key;

    (*key)->left_key = left;
    (*key)->right_key = right;
    (*key)->key = key_value;
    (*key)->owner = owner;
    return 0;
}
//endregion

void finish_node_packing(key_type *upper_array, size_t *index, int key,
                         Node **node_array, size_t *node_index, Node **current_node, Key **current_key)
{
    upper_array[(*index)++] = key;
    node_array[(*node_index)++] = *current_node;
    *current_node = NULL;
    *current_key = NULL;
}

void find_children_for_key(Key *target_key, Node **children_array, size_t children_array_size)
{
    for (int node_number = 0; node_number < children_array_size; ++node_number)
        if (target_key->key < children_array[node_number]->first_key->key)
        {
            children_array[node_number]->parent = target_key->owner;
            target_key->left_child = node_number != 0 ? children_array[node_number-1] : NULL;
            target_key->right_child = children_array[node_number];
            break;
        }
}


/**
 * @brief Initializes btree from a given @p array, putting a pointer to the root element in the @p root
 * @param root Pointer to the pointer to root Node, that is filled with address of constructed tree root
 * @param array Array of keys from which tree is constructed
 * @param array_size Size of @p array
 * @param min_size This number defines how many keys must be in the node at least and at most (2 * min_size)
 * @param to_leave This number defines how many keys in a node function will try to reserve
 * for possible future inserting - then the max number of most nodes equals 2 * min_size - to_leave
 * @param compar Pointer to function used to compare elements in quick sorting procedure
 * @return Exit code: if 0 - tree was constructed successfully, else there was a mistake
 */
int btree_init(Node **root, key_type *array, size_t array_size, size_t min_size,
               size_t to_leave, int (*compar)(const void *, const void *))
{
    qsort(array, array_size, sizeof(key_type), compar);

    printf("Array after qsort:\n");
    for (size_t i = 0; i < array_size; ++i)
        printf("%d ", array[i]);
    printf("\n");

    if (to_leave > min_size || to_leave < 0)
    {
        sprintf(error_string, "To_leave argument is wrong, value = %ld\0", to_leave);
        return 1;
    }

    size_t current_array_size = array_size,
           upper_index = 0,
           nodes_on_level,
           divider = min_size * 2 - to_leave,
           remainder,
           last_node_separator;

    key_type current_array[array_size];
    key_type upper_array[current_array_size / divider + 1];
    memcpy(current_array, array, sizeof(key_type) * array_size);

    size_t current_node_index = 0,
           child_node_index = 0;
    Node *child_nodes[current_array_size / divider + 2],
         *current_nodes[current_array_size / divider + 2];
    Node *current_node;
    Key *current_key, *new_key;
    int code, border;

    do
    {
        nodes_on_level = current_array_size / divider;
        border = (nodes_on_level - 1) * divider;
        border = border > 0 ? border : 0;
        remainder = current_array_size - border;
        last_node_separator = remainder <= min_size * 2 ? current_array_size + 1 : border + remainder / 2 + 1;

        current_node = NULL; 
        current_key = new_key = NULL;

        if (current_node_index > 0)
        {
            memcpy(child_nodes, current_nodes, sizeof(Node *) * current_node_index);
            memset(current_nodes, 0, sizeof(Node *) * current_node_index);
            child_node_index = current_node_index;
            current_node_index = 0;
        }

        for (size_t i = 0; i < current_array_size; ++i)
        {
            if ((i < border && (i + 1) % divider == 0)
                || (i >= border && (i + 1) == last_node_separator))
            {
                finish_node_packing(upper_array, &upper_index, current_array[i],
                                    current_nodes, &current_node_index, &current_node, &current_key);
            }
            else
            {
                if (current_node == NULL)
                {
                    code = node_init(&current_node, NULL, min_size);
                    if (code != 0)
                    {
                        printf("Error while creating node, i = %u, error:\n%s\n", i, error_string);
                        return 1;
                    }
                }

                code = key_init(&new_key, current_key, NULL, current_array[i], current_node);
                if (code != 0)
                {
                    printf("Error while creating key, i = %u, error:\n%s\n", i, error_string);
                    return 1;
                }

                if (child_node_index > 0)
                    find_children_for_key(new_key, child_nodes, child_node_index);

                if (current_node->first_key == NULL)
                    current_node->first_key = new_key;

                current_node->size++;
                current_key = new_key;
            }
        }

        if (current_node != NULL)
            current_nodes[current_node_index++] = current_node;


}


/**
 * @brief Finds the Key object with provided @p key_value
 * @param root Pointer to Node from which search should be started
 * @param key_ptr Pointer to the pointer to Key where the found Key is saved
 * @param key_value Value that desired Key should contain as key value
 * @return 0 if Key was found, 1 otherwise
 */
int btree_get_key(Node *root, Key **key_ptr, key_type key_value)
{
    Key *current_key = root->first_key;
    while (key_value > current_key->key && current_key->right_key != NULL)
        current_key = current_key->right_key;
    if (key_value == current_key->key)
    {
        *key_ptr = current_key;
        return 0;
    }
    if (key_value < current_key->key && current_key->left_child != NULL)
        return btree_get_key(current_key->left_child, key_ptr, key_value);
    if (key_value > current_key->key && current_key->right_child != NULL)
        return btree_get_key(current_key->right_child, key_ptr, key_value);
    return 1;
}


#endif //B_TREE_BTREE_H
