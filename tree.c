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
#include <unistd.h>

#include "tokenizer.h"
#include "parser.h"
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
    /* node->head = head; */
    /* node->head->left = node->head->right = NULL; */
    node->left = NULL;
    node->right = NULL;
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
    // _init_tree_head(node);
    node->left = NULL;
    node->right = NULL;
    node->oldstreamfd = 0;
    token->spec = origin->spec;
    memset(token->element, '\0', ELEMENT_MAX);
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
    return _init_node(token);
}

/**
 * init_root - initialize tree root
 * @root: the root of syntax tree
 */
node_t *init_root(node_t *root)
{
    token_t *token = (token_t *) malloc(sizeof(token_t));
    
    if (token == NULL) {
        print_error("Bad allocation (token) \n", root);
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
    memset(token->element, '\0', ELEMENT_MAX);
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
    /* parent->head->left = (left != NULL)? left->head : NULL; */
    /* parent->head->right = (right != NULL)? right->head : NULL; */
    parent->left = (node_t *)left;
    parent->right = (node_t *)right;

    return parent;
}

void _init_redirect_in_stream(node_t *current) {
    node_t *word = current->left;
    FILE *stream;
    int streamfd;
    const char *streamname = word->token->element;
    if (!_is_abstract_node(current)) {
        stream = fopen(streamname, "r");
    } else {
        stream = stdin;
    }
    streamfd = fileno(stream);
    dup2(current->oldstreamfd, streamfd);
    close(current->oldstreamfd);
}

void _init_redirect_out_stream(node_t *current) {
    node_t *word = current->left;
    FILE *stream;
    int streamfd;
    char *mode;
    const char *streamname = current->token->element;
    switch (current->token->spec) {
    case REDIRECT_OUT: case REDIRECT_OUT_COMPOSITION:
        mode = "w";
        break;
    case REDIRECT_OUT_APPEND:
        mode = "a";
        break;
    default:
        break;
    }
    if (current->token->spec == REDIRECT_OUT_COMPOSITION) {
        if (!_is_abstract_node(current)) {
            streamfd = atoi(streamname);;
        } else {
            streamfd = STDOUT_FILENO;
        }
    } else {
        if (!_is_abstract_node(current)) {
            streamfd = atoi(streamname);
            stream = fdopen(streamfd, mode);
        } else {
            stream = stdout;
        }
        streamfd = fileno(stream);
    }
    dup2(current->oldstreamfd, streamfd);
    close(current->oldstreamfd);
}

void _init_redirect_stream(node_t *current) {
    if (_is_redirect_in(current->token))
        _init_redirect_in_stream(current);
    if (_is_redirect_out(current->token))
        _init_redirect_out_stream(current);
}

/**
 * free_nodes - free nodes which are children of `current` node recursively
 * @current: current node which will be freed
 */
void free_nodes(node_t *current, node_t *root)
{
    node_t *left, *right;
    // if (current->head->left != NULL) {
    _init_redirect_stream(current);
    if (current->left != NULL) {
        // left = container_of(&(current->head->left), node_t, head);
        left = (node_t *)current->left;
        free_nodes(left, root);
    }
    // if (current->head->left != NULL) {
    if (current->right != NULL) {
        // right = container_of(&(current->head->right), node_t, head);
        right = (node_t *)current->right;
        free_nodes(right, root);
    }    
    free(current);
}
