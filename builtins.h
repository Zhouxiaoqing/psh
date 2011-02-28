/*
 * builtins.h - builtins commands
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#ifndef PSH_BUILTINS_H_
#define PSH_BUILTINS_H_

#include <unistd.h>

#include "executor.h"

bool check_builtins(command_t *current_command, node_t * root);

#endif   // PSH_BUILTINS_H_

