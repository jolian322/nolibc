#include "jlibc.h"

// linked lists
typedef struct jll
{
    struct jll *prev;
    struct jll *next;
    void *value;
} jll;

jll *jllRoot(jll *node)
{
    if (node == NULL)
        return NULL;

    while (node->prev != NULL)
        node = node->prev;

    return node;
}
jll *jllLast(jll *node)
{
    if (node == NULL)
        return NULL;

    while (node->next != NULL)
        node = node->next;

    return node;
}
int jllInsert(jll *node, jll *innode)
{
    if (node == NULL || innode == NULL)
        return -1;

    jll *holdnext = node->next;

    node->next = innode;
    innode->prev = node;
    innode->next = holdnext;

    if (holdnext != NULL)
        holdnext->prev = innode;

    return 0;
}
jll *jllDel(jll *node)
{
    if (node == NULL)
        return NULL;
    if (node->next)
        node->next->prev = node->prev;
    if (node->prev)
        node->prev->next = node->next;
    node->value = NULL;
    node->next = NULL;
    node->prev = NULL;
    return node;
}
jll *jllSearch(jll *node, void *value)
{
    jll *right = node;
    while (!0)
    {
        if (right == NULL && node == NULL)
            return NULL;

        if (right != NULL)
        {
            if (right->value == value)
                return right;
            right = right->next;
        }
        if (node != NULL)
        {
            if (node->value == value)
                return node;
            node = node->prev;
        }
    }
}
