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

/**
 * syntax_error - Deal with syntax error of input.
 * @t: Token information and next character.
 */
void syntax_error(tokenizer_t *t);

/**
 * parse input - Parse and set command information to command tables
 * @t: Token information and next character.
 */
const token_t *parse_input(tokenizer_t *t);

#endif  // PSH_PARSER_H_
