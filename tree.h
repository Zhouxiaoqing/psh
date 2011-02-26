/*
 * tree.h - simple tree data structure and utils for them
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#ifndef PSH_TREE_H_
#define PSH_TREE_H_

#include <stddef.h>

typedef struct tree_head {
    struct tree_head *left;
    struct tree_head *right;
} tree_t;

typedef struct node {
    token_t *token;
    // tree_t *head;
    struct node *left;
    struct node *right;
    int oldstreamfd;
} node_t;

/*
 * conteiner_of - macro to get parent structure
 */
#define container_of(ptr, type, member) ({        \
            const typeof ( ((type *)0)->member )*__mptr = (ptr);    \
            (type *)( (char *)__mptr - offsetof(type, member) );})

/*
 * _is_abstract_node - check whether the node is abstract node or not.
 */
static inline bool _is_abstract_node(const node_t *node) {
    // return (node->token == NULL)? true : false;
    return (strlen(node->token->element) == 0)? true : false;
}

/**
 * init_node - initialize tree node
 * @token: token by which node will be initialized
 */
// node_t *init_node(const token_t *token);
node_t *init_node(const token_t *token);
/**
 * init_root - initialize tree root
 * @root: the root of syntax tree
 */
node_t *init_root();
/**
 * init_abstract_node - initialize tree node with it's spec and without it's token
 * @spec: token's specifier
 */
node_t *init_abstract_node(token_spec_t spec);
/**
 * create_tree - create tree from parent which left is `left` and right is `right`
 * @parent: parent node
 * @left: left side child of parent node
 * @right: right side child of parent node
 */
node_t *create_tree(node_t *parent, const node_t *left, const node_t *right);
/**
 * free_nodes - free nodes which are children of `current` node recursively
 * @current: current node which will be freed
 */
void free_nodes(node_t *current, node_t *root);

#endif  // PSH_TREE_H_
