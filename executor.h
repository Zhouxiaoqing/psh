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
} command_t;

/**
 * print_error - print error message and finalize program
 */
void print_error(const char *error_message, node_t *root);

/*
 * _init_command - initialize command except for I/O fd
 */
static inline command_t *_init_command(node_t *root)
{
    command_t *command = (command_t *) malloc(sizeof(command_t));
    if (command == NULL) {
        print_error("Bad allocation (command).\n", root);
    }
    // memcpy(command->cmd, '\0', ELEMENT_MAX);
    // memcpy(command->argv, '\0', ARG_MAX);
    command->command_flag = false;
    return command;
}

/**
 * eat_root - execute commands in the given tree sequencially
 * @root: the root of syntax tree
 */
void eat_root(node_t *root);

#endif  // PSH_EXECUTOR_H_
