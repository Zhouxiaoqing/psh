/*
 * parser.c - simple parser for shell input
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"

static const token_t *_parse_word(parser_t *p, tokenizer_t *t);
static const token_t *_parse_env(parser_t *p, tokenizer_t *t);
static const token_t *_parse_env_assignment(parser_t *p, tokenizer_t *t);
static const token_t *_parse_home(parser_t *p, tokenizer_t *t);
static const token_t *_parse_redirect_out(parser_t *p, tokenizer_t *t);
static const token_t *_parse_redirect_in(parser_t *p, tokenizer_t *t);
static const token_t *_parse_redirection(parser_t *p, tokenizer_t *t);
static const token_t *_parse_command_element(parser_t *p, tokenizer_t *t);
static const token_t *_parse_redirection_list(parser_t *p, tokenizer_t *t);
static const token_t *_parse_command(parser_t *p, tokenizer_t *t);
static const token_t *_parse_piped_command(parser_t *p, tokenizer_t *t);


/**
 * init_parsr - Initialize parser and command tables.
 */
parser_t *init_parser(void)
{
    int i = 0;
    parser_t *p = (parser_t *) malloc(sizeof(parser_t));

    p->p = 0;
    for (i = 0; i < PIPE_MAX; i++)
        p->command[i].command_flag = false;
    // strncpy(t->input, input, INPUT_MAX);
    // Read first character from input.
    // t->c = _getc(t->input);
    // _append_token(&(t->token), &(t->c));
    // next_token(t);
    
    return p;
}

/**
 * syntax_error - Deal with syntax error of input.
 * @t: Token information and next character.
 */
void syntax_error(parser_t *p, tokenizer_t *t)
{
    fprintf(stderr, "syntax error: \n");
    free(t);
    free(p);
    exit(-1);
}

/*
 * _parse_word - Eat <word>
 */
static const token_t *_parse_word(parser_t *p, tokenizer_t *t)
{
    const token_t *word;
    command_t *current_command;

    word = current_token(t);
    if (word->spec != WORD) syntax_error(p, t);
    current_command = &(p->command[p->p]);
    if (!current_command->command_flag) {
        strncpy(current_command->cmd, word->element, ELEMENT_MAX);
        current_command->command_flag = true;
    } else {
        if (strlen(current_command->args) != 0)
            strncat(current_command->args, " ", 1);
        strncat(current_command->args, word->element, strlen(word->element));
    }

    return &(t->token);
}

/*
 * _parse_env - Eat <env>
 */
static const token_t *_parse_env(parser_t *p, tokenizer_t *t)
{
    const token_t *doller, *env;

    doller = current_token(t);
    if (doller->spec != ENV) syntax_error(p, t);
    env = next_token(t);
    _next_token(t);

    return _parse_word(p, t);
}

/*
 * _parse_env_assignment - Eat <env_assignment>
 */
static const token_t *_parse_env_assignment(parser_t *p, tokenizer_t *t)
{
    const token_t *key, *assign, *value;
    
    key = current_token(t);
    if (key->spec != WORD) syntax_error(p, t);
    assign = next_token(t);
    if (assign->spec !=ENV_ASSIGNMENT) syntax_error(p, t);
    value = next_token(t);
    if (assign->spec !=WORD) syntax_error(p, t);
    
    if (putenv(value->element) != 0) {
        fprintf(stderr, "putenv failed\n");
    }
    
    return value;
}

/*
 * _parse_redirect_in - Eat <redirect_in>
 */
static const token_t *_parse_redirect_in(parser_t *p, tokenizer_t *t)
{
    const token_t *redirect_head;
    const token_t *redirect_file;

    redirect_head = current_token(t);
    /*
    if (redirect_head->spec != NUM &&
        redirect_head->spec != REDIRECT_IN) {
        syntax_error(t);
    } else {
        token_t *redirect_next = next_token(t);
    }
    if (redirect_next->spec == REDIRECT_IN_APPEND) {
        token_t *word = _parse_redirect_in_append(t);
    }
    */
    if (redirect_head->spec != REDIRECT_IN) syntax_error(p, t);
    redirect_file = next_token(t);
    p->command[p->p].redirection[0].input = false;
    strcpy(p->command[p->p].redirection[1].filename, redirect_file->element);
    return redirect_file;
}

/*
 * _parse_redirect_out - Eat <redirect_out>
 */
static const token_t *_parse_home(parser_t *p, tokenizer_t *t)
{
    const token_t *home;
    
    home = current_token(t);
    if (home->spec != HOME) syntax_error(p, t);
    // Convert `~` into home directory

    return _next_token(t);
}

/*
 * _parse_redirect_out - Eat <redirect_out>
 */
static const token_t *_parse_redirect_out(parser_t *p, tokenizer_t *t)
{
    const token_t *redirect_head;
    const token_t *redirect_file;
    
    redirect_head = current_token(t);
    /*
    if (redirect_head->spec != NUM &&
        redirect_head->spec != REDIRECT_OUT) {
        syntax_error(t);
    } else {
        token_t *redirect_next = next_token(t);
    }
    if (redirect_next->spec == REDIRECT_OUT_APPEND) {
        token_t *word = _parse_redirect_in_append(t);
    }
    */
    if (redirect_head->spec != REDIRECT_OUT) syntax_error(p, t);
    redirect_file = next_token(t);
    p->command[p->p].redirection[1].input = false;
    strcpy(p->command[p->p].redirection[1].filename, redirect_file->element);
    return redirect_file;
}

/*
 * _parse_redirection - Eat <redirection>
 */
static const token_t *_parse_redirection(parser_t *p, tokenizer_t *t)
{
    const token_t *redirect_head;

    redirect_head = current_token(t);
    if (redirect_head->spec != REDIRECT_IN
        && redirect_head->spec != REDIRECT_OUT)
        syntax_error(p, t);
    switch (redirect_head->spec) {
    case REDIRECT_IN:
        _parse_redirect_in(p, t);
        break;
    case REDIRECT_OUT:
        _parse_redirect_out(p, t);
        break;
    }
    return next_token(t);
}

/*
 * _parse_redirection_list - Eat <redirection_list>
 */
static const token_t *_parse_redirection_list(parser_t *p, tokenizer_t *t)
{
    const token_t *redirection;
    const token_t *redirection_list;

    redirection = current_token(t);
    if (redirection->spec != REDIRECT_IN &&
        redirection->spec != REDIRECT_OUT)
        syntax_error(p, t);
    _parse_redirection(p, t);
    redirection_list = next_token(t);
    if (redirection->spec == REDIRECT_IN &&
        redirection->spec == REDIRECT_OUT)
        _parse_redirection_list(p, t);

    return redirection_list;
}

/*
 * _parse_command_element - Eat <command_element>
 */
static const token_t *_parse_command_element(parser_t *p, tokenizer_t *t)
{
    const token_t *command_element;
    
    command_element = current_token(t);
    switch (command_element->spec) {
    case WORD:
        _parse_word(p, t);
        break;
    case ENV_ASSIGNMENT:
        _parse_env_assignment(p, t);
        break;
    case REDIRECT_IN: case REDIRECT_OUT:
        _parse_redirection_list(p, t);
        break;
    default: break;
    }
    
    return command_element;
}

/*
 * _parse_command - Eat <command> and make pipe.
 */
static const token_t *_parse_command(parser_t *p, tokenizer_t *t)
{
    const token_t *command_element;
    const token_t *command;
    
    command_element = current_token(t);
    if (command_element->spec != WORD &&
        command_element->spec != ENV_ASSIGNMENT &&
        command_element->spec != REDIRECT_IN &&
        command_element->spec != REDIRECT_OUT)
        syntax_error(p, t);
    _parse_command_element(p, t);
    command = next_token(t);
    if (command->spec == WORD ||
        command->spec == ENV_ASSIGNMENT ||
        command->spec == REDIRECT_IN ||
        command->spec == REDIRECT_OUT)
        _parse_command(p, t);
    
    return command;
}

/*
 * _parse_piped_command - Eat <piped_command>
 */
static const token_t *_parse_piped_command(parser_t *p, tokenizer_t *t)
{
    const token_t *command;
    const token_t *pipe;

    command = current_token(t);
    // Eat command and bind it to 
    pipe = _parse_command(p, t);
    if (p->p <= PIPE_MAX) {
        p->p++;
    } else {
        fprintf(stderr, "At most 8 pipes only can be created.");
        exit(-1);
    }

    if (pipe->spec == PIPED_COMMAND) {
        next_token(t);
        _parse_piped_command(p, t);
    }
   
    return command;
}

/**
 * parse input - Parse and set command information to command tables
 * @t: Token information and next character.
 */
const token_t *parse_input(parser_t *p, tokenizer_t *t)
{
    return _parse_piped_command(p, t);
}
