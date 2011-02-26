/*
 * executor.h - execute parsed tree
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#ifndef PSH_EXECUTOR_H_
#define PSH_EXECUTOR_H_

#include <stdio.h>
#include <sys/types.h>

#include "tokenizer.h"
#include "parser.h"
#include "tree.h"

typedef struct command {
    char cmd[ELEMENT_MAX];
    char *argv[ARG_MAX];
    // char *argv;
    int argc;
    bool command_flag;
    int input_fd;
    int output_fd;
    // FILE *ifstream[ELEMENT_MAX];
    // FILE *ofstream[ELEMENT_MAX];
} command_t;

/**
 * print_error - print error message and finalize program
 */
void print_error(const char *error_message, node_t *root);

/*
 * _init_command - initialize command except for I/O fd
 */
static inline void *_init_command(command_t *command)
{
    memset(command->cmd, '\0', ELEMENT_MAX);
    memset(command->argv, '\0', ARG_MAX);
    command->argc = 0;
    command->command_flag = false;
    return command;
}

/**
 * init_command - return initialized command
 */
static inline command_t *init_command(node_t *root)
{
    command_t *command = (command_t *) malloc(sizeof(command_t));
    if (command == NULL) {
        print_error("Bad allocation (command).\n", root);
    }
    _init_command(command);
    return command;
}

/**
 * eat_root - execute commands in the given tree sequencially
 * @root: the root of syntax tree
 */
void eat_root(node_t *root);

#endif  // PSH_EXECUTOR_H_
