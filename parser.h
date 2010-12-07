/*
 * parser.h - simple parser for shell input
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#ifndef PSH_PARSER_H_
#define PSH_PARSER_H_

#include <stdbool.h>
#include <sys/types.h>

#define PIPE_MAX  8
#define ELEMENT_MAX  256
#define ARG_MAX  4096
#define INPUT_MAX (4096*4)

typedef enum {
    PIPED_COMMAND,
    COMMAND,
    REDIRECT_IN,
    REDIRECT_IN_APPEND,
    REDIRECT_OUT,
    REDIRECT_OUT_APPEND,
    REDIRECTION,
    REDIRECTION_LIST,
    COMMAND_ELEMENT,
    DIGIT,
    ALPHA,
    ALPHANUM,
    SPECIAL,
    HOME,
    ENV,
    NUM,
    ENV_ASSIGNMENT,
    WORD,
    ERROR,
    END_OF_LINE,
    END_OF_FILE
} token_spec_t;

typedef struct token {
    token_spec_t spec;
    char element[ELEMENT_MAX];
} token_t;

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

typedef struct tokenizer {
    token_t token;
    char c;
    int p; // the number of current pipe
    char input[INPUT_MAX];
    command_t command[PIPE_MAX];
} tokenizer_t;

/**
 * init_tokenizer - Initialize and set up scanninig from input.
 * @input: input from prompt
 */
tokenizer_t *init_tokenizer(const char *input);

/**
 * current_token - Get current token.
 * @t: Token information and next character.
 */
const token_t *current_token(tokenizer_t *t);

/**
 * current_token - Scan input and return next token.
 * @t: Token information and next character.
 */
const token_t *next_token(tokenizer_t *t);

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

