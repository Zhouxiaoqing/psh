/*
 * builtins.c - builtins commands
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"

/*
 * _check_cd - check whether cd is in `current_command' and if so, do chdir
 */
static bool _check_cd(command_t *current_command, node_t *root)
{
    bool result;
    const char *path;
    switch (strcmp(current_command->cmd, "cd")) {
    case 0:
        if (current_command->argv[1] == NULL)
            path = getenv("HOME");
        else
            path = current_command->argv[1];
        if (chdir(path) != 0) {
            print_error("cd: no such directry in pwd.\n", root);
        }
        result = true;
        break;
    default:
        result = false;
        break;
    }

    return result;
}

/*
 * _check_exit - check whether exit is in `current_command' and if so, do chdir
 */
static bool _check_exit(command_t *current_command, node_t *root)
{
    bool result;
    switch (strcmp(current_command->cmd, "exit")) {
    case 0:
        print_error("", root);
        result = true;
    default:
        result = false;
        break;
    }
    return result;
}


/**
 * check_builtins - check builtin commands
 */
bool check_builtins(command_t *current_command, node_t * root)
{
    return _check_cd(current_command, root) ||
        _check_exit(current_command, root);
}
