/*
 * tokenizer.h - simple tokenizer for shell input
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#ifndef PSH_PARSER_H_
#define PSH_PARSER_H_

#include "tokenizer.h"

typedef struct redirection {
    bool input;
    char filename[ELEMENT_MAX];
} redirection_t;

typedef struct command {
    char cmd[ELEMENT_MAX];
    redirection_t redirection[ELEMENT_MAX];
    char args[ARG_MAX];
    bool command_flag;
} command_t;

typedef struct parser {
    int p; // the number of current pipe
    command_t command[PIPE_MAX];
} parser_t;

/**
 * syntax_error - Deal with syntax error of input.
 * @p: Parser and command tables
 * @t: Token information and next character.
 */
void syntax_error(parser_t *p, tokenizer_t *t);

/**
 * parse input - Parse and set command information to command tables
 * @p: Parser and command tables
 * @t: Token information and next character.
 */
const token_t *parse_input(parser_t *p, tokenizer_t *t);

#endif  // PSH_PARSER_H_
