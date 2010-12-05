/*
  Copyright (c) 2010 Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef PSH_PARSER_H_
#define PSH_PARSER_H_

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
    REDIRECTTION_LIST,
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
    END_OF_FILE
} tokens_spec_t;

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
    int c;
    int p; // the number of current pipe
    char *input;
    commant_t command[PIPE_MAX];
} tokenizer_t;

/**
 * init_tokenizer - Initialize and set up scanninig from input.
 * @input: input from prompt
 */
const tokenizer_t init_tokenizer(const char *input);

/**
 * current_token - Get current token.
 * @t: Token information and next character.
 */
const token_t *current_token(tokenizer_t t);

/**
 * current_token - Scan input and return next token.
 * @t: Token information and next character.
 */
const token_t *next_token(tokenizer_t t);

/**
 * syntax_error - Deal with syntax error of input.
 * @t: Token information and next character.
 */
void syntax_error(tokenizer_t t);

#endif  // PSH_PARSER_H_
