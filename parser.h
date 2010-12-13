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
    char src[ELEMENT_MAX];
    char dst[ELEMENT_MAX];
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

/*
 * _is_num - chech whether token is <num>
 */
static inline const bool _is_num(const token_t *t)
{
    return (t->spec == NUM) ? true : false;
}

/*
 * _is_alpha - chech whether token is <alpha>
 */
static inline const bool _is_alpha(const token_t *t)
{
    return (t->spec == ALPHA) ? true : false;
}
/*
 * _is_word - chech whether token is <word>
 */
static inline const bool _is_word(const token_t *t)
{
    return (t->spec == WORD) ? true : false;
}

/*
 * _is_redirect_in - chech whether token is <redirect_in>
 */
static inline const bool _is_redirect_in(const token_t *t)
{
    return (t->spec == REDIRECT_IN) ? true : false;
}

/*
 * _is_redirect_out - chech whether token is <redirect_out>
 */
static inline const bool _is_redirect_out(const token_t *t)
{
    return (t->spec == REDIRECT_OUT) ? true : false;
}

/*
 * _is_redirection - chech whether token is <redirection>
 */
static inline const bool _is_redirection(const token_t *t)
{
    return (_is_redirect_in(t) || _is_redirect_out(t)) ? true : false;
}

/*
 * _is_command_element - chech whether token is <command_element>
 */
static inline const bool _is_command_element(const token_t *t)
{
    return (t->spec == WORD ||
            t->spec == ENV_ASSIGNMENT || 
            t->spec == REDIRECT_IN || 
            t->spec == REDIRECT_OUT) ? true : false;
}

/**
 * init_parser - Initialize parser and command tables.
 */
parser_t *init_parser(void);

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
