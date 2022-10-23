#include "binary_tree.h"
#include "queue.h"

#define MAX_ELEMENTS     16 // we can get the elements_num by traverse,
                            // so MAX_ELEMENTS may not be a good idea.

hook_func g_cb_leaf_traverse  = NULL;
hook_func g_cb_level_traverse = NULL;

void level_traverse(node root)
{
    queue_handler queueHandler        = {0};
    uint32_t      queue[MAX_ELEMENTS] = {0};
    node          p_node              = NULL;
    
    initQueue(&queueHandler, queue, MAX_ELEMENTS);
    
    enQueue(&queueHandler, (uint32_t)root);
    while (!isEmpty(&queueHandler))
    {
        deQueue(&queueHandler, (uint32_t *)&p_node);
        if (g_cb_level_traverse)
        {
            g_cb_level_traverse(p_node);
        }
        
        if (p_node->l_child)
        {
            enQueue(&queueHandler, (uint32_t)p_node->l_child);
        }
        if (p_node->r_child)
        {
            enQueue(&queueHandler, (uint32_t)p_node->r_child);
        }
    }
}

void leaf_traverse(node root)
{
    if ((NULL == root->l_child) && (NULL == root->r_child))
    {
        if (g_cb_leaf_traverse)
        {
            g_cb_leaf_traverse(root);
        }
    }
    else
    {
        leaf_traverse(root->l_child);
        leaf_traverse(root->r_child);
    }
}

uint8_t tree_creater(node *root, uint16_t element_size)
{
    if (*root)
    {
        tree_destructor(*root);
    }
    
    MALLOC_CHECK(*root, sizeof(struct binary_tree), 1);
    MALLOC_CHECK((*root)->value, element_size, 1);
    
    return MEM_OK;
}

void tree_destructor(node root)
{
    if (root != NULL)
    {
        tree_destructor(root->l_child);
        tree_destructor(root->r_child);
        
        if (root->value != NULL)
        {
            MEM_FREE(root->value);
            root->value = NULL;
        }
        
        MEM_FREE(root);
        root = NULL;
    }
}
