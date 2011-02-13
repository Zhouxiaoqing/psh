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
    char args[ARG_MAX];
    bool command_flag;
    int input_fd;
    int output_fd;
} command_t;

/*
 * _init_command - initialize command except for I/O fd
 */
static inline command_t *_init_command()
{
    command_t *command = (command_t *) malloc(sizeof(command_t));
    if (command == NULL) {
        fprintf(stderr, "Bad allocation (command).\n");
        exit(EXIT_FAILURE);
    }
    // strncat(command->cmd, '\0', ELEMENT_MAX);
    // strncat(command->args, '\0', ARG_MAX);
    command->command_flag = false;
    return command;
}

/**
 * eat_root - execute commands in the given tree sequencially
 * @root: the root of syntax tree
 */
void eat_root(const node_t *root);

#endif  // PSH_EXECUTOR_H_
