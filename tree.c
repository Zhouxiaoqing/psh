/*
 * tree.c - simple tree data structures and utils for them
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <string.h>
#include <stdlib.h>

#include "tokenizer.h"
#include "tree.h"

/*
 * _init_tree_head - initialize tree head
 */
static inline void *_init_tree_head(node_t *n)
{
    tree_t *t = (tree_t *) malloc(sizeof(tree_t));
    n->head = t;
    n->head->left = n->head->right = NULL;
}

/*
 * _init_node - initialize node with token `origin`
 */
static inline void *_init_node(node_t *n, const token_t *origin)
{
    token_t *token;

    _init_tree_head(n);
    
    token->spec = origin->spec;
    strncpy(token->element, origin->element, strlen(origin->element));
    n->token = token;
}

/**
 * init_node - initialize tree node
 * @token: token by which node will be initialized
 */
node_t *init_node(const token_t *token)
{
    node_t *n = (node_t *) malloc(sizeof(node_t));
    _init_node(n, token);
    
    return n;
}

/**
 * init_root - initialize tree root
 * @root: the root of syntax tree
 */
void init_root(node_t *root)
{
    token_t *token;
    token->spec = PIPED_COMMAND;
    root = init_node(token);
}

/**
 * init_abstract_node - initialize tree node with it's spec and without it's token
 * @spec: token's specifier
 */
node_t *init_abstract_node(token_spec_t spec)
{
    token_t *token;

    token->spec = spec;
    node_t *n = (node_t *) malloc(sizeof(node_t));
    _init_node(n, token);
    
    return n;
}

/**
 * create_tree - create tree from parent which left is `left` and right is `right`
 * @parent: parent node
 * @left: left side child of parent node
 * @right: right side child of parent node
 */
node_t *create_tree(node_t *parent, const node_t *left, const node_t *right)
{
    parent->head->left = left->head;
    parent->head->right = right->head;
    
    return parent;
}

/**
 * free_nodes - free nodes which are children of `current` node recursively
 * @current: current node which will be freed
 */
void free_nodes(node_t *current)
{
    node_t *left, *right;

    if (current->head->left != NULL) {
        left = container_of(&(current->head->left), node_t, head);
        free(left);
    }
    if (current->head->left != NULL) {
        right = container_of(&(current->head->right), node_t, head);
        free(right);
    }
    free(current);
}
