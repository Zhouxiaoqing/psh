#ifndef PSH_BUILTINS_H_
#define PSH_BUILTINS_H_

#include <unistd.h>

#include "executor.h"

bool check_builtins(command_t *current_command, node_t * root);

#endif   // PSH_BUILTINS_H_

