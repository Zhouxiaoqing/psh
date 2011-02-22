/*
 * tree.c - simple tree data structures and utils for them
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokenizer.h"
#include "tree.h"

/*
 * _init_tree_head - initialize tree head
 */
static inline void _init_tree_head(node_t *node)
{
    tree_t *head = (tree_t *) malloc(sizeof(tree_t));
    if (head == NULL) {
        fprintf(stderr, "Bad allocation (tree head) \n");
        exit(EXIT_FAILURE);
    }
    node->head = head;
    node->head->left = node->head->right = NULL;
}

/*
 * _init_node - initialize node with token `origin`
 */
static node_t *_init_node(const token_t *origin)
{
    token_t *token = (token_t *) malloc(sizeof(token_t));
    node_t *node = (node_t *) malloc(sizeof(node_t));

    if (token == NULL) {
        fprintf(stderr, "Bad allocation (token) \n");
        exit(EXIT_FAILURE);
    }
    if (node == NULL) {
        fprintf(stderr, "Bad allocation (node) \n");
        exit(EXIT_FAILURE);
    }
    _init_tree_head(node);
    token->spec = origin->spec;
    strncpy(token->element, origin->element, strlen(origin->element));
    node->token = token;
    // free(origin);
    
    return node;
}

/**
 * init_node - initialize tree node
 * @token: token by which node will be initialized
 */
node_t *init_node(const token_t *token)
{
    node_t *node = _init_node(token);

    return node;
}

/**
 * init_root - initialize tree root
 * @root: the root of syntax tree
 */
node_t *init_root(node_t *root)
{
    token_t *token = (token_t *) malloc(sizeof(token_t));
    
    if (token == NULL) {
        fprintf(stderr, "Bad allocation (token) \n");
        exit(EXIT_FAILURE);
    }
    token->spec = PIPED_COMMAND;

    return init_node(token);
}

/**
 * init_abstract_node - initialize tree node only with it's spec
 * @spec: token's specifier
 */
node_t *init_abstract_node(const token_spec_t spec)
{
    token_t *token = (token_t *) malloc(sizeof(token_t));
    node_t *node;

    if (token == NULL) {
        fprintf(stderr, "Bad allocation (token) \n");
        exit(EXIT_FAILURE);
    }
    token->spec = spec;
    node = _init_node(token);
    // free(token);
    
    return node;
}

/**
 * create_tree - create tree from parent which left is `left` and right is `right`
 * @parent: parent node
 * @left: left side child of parent node
 * @right: right side child of parent node
 */
node_t *create_tree(node_t *parent, const node_t *left, const node_t *right)
{
    parent->head->left = (left != NULL)? left->head : NULL;
    parent->head->right = (right != NULL)? right->head : NULL;

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
        free_nodes(left);
    }
    if (current->head->left != NULL) {
        right = container_of(&(current->head->right), node_t, head);
        free_nodes(right);
    }
    free(current);
}
