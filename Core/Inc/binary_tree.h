#ifndef __BINARY_TREE_H
#define __BINARY_TREE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef config_static_malloc
// fix me!
#define STATIC_MALLOC(class_size, num)
#else
#define STATIC_MALLOC(total_size) malloc((total_size))
#endif

#define MEM_FREE(ptr) free(ptr)

#define MEM_OVERFLOW 1
#define MEM_OK       0

#define MALLOC_CHECK(ptr, class_size, num)\
        do\
        {\
            uint32_t total_size = (class_size) * (num);\
            (ptr) = STATIC_MALLOC(total_size);\
            if (NULL == (ptr))\
            {\
                return MEM_OVERFLOW;\
            }\
            memset((ptr), 0, total_size);\
        }while(0)

struct binary_tree;

typedef void *element;
typedef struct binary_tree *node;

struct binary_tree
{
    element value;
    node    l_child;
    node    r_child;
};

typedef void (*hook_func)(node);

extern hook_func g_cb_leaf_traverse;
extern hook_func g_cb_level_traverse;

void level_traverse(node root);
void leaf_traverse(node root);
uint8_t tree_creater(node *root, uint16_t element_size);
void tree_destructor(node root);

#endif /*__BINARY_TREE_H*/
