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

static const token_t *parse_word(tokenizer_t *t);
static const token_t *parse_env(tokenizer_t *t);
static const token_t *parse_env_assignment(tokenizer_t *t);
static const token_t *parse_home(tokenizer_t *t);
static const token_t *parse_redirect_out(tokenizer_t *t);
static const token_t *parse_redirect_in(tokenizer_t *t);
static const token_t *parse_redirection(tokenizer_t *t);
static const token_t *parse_command_element(tokenizer_t *t);
static const token_t *parse_redirection_list(tokenizer_t *t);
static const token_t *parse_command(tokenizer_t *t);
static const token_t *parse_piped_command(tokenizer_t *t);

/**
 * syntax_error - Deal with syntax error of input.
 * @t: Token information and next character.
 */
void syntax_error(tokenizer_t *t)
{
    fprintf(stderr, "syntax error: \n");
    free(t);
    exit(-1);
}

/*
 * parse_word - Eat <word>
 */
static const token_t *parse_word(tokenizer_t *t)
{
    const token_t *word;
    command_t *current_command;

    word = current_token(t);
    if (word->spec != WORD) syntax_error(t);
    current_command = &(t->command[t->p]);
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
 * parse_env - Eat <env>
 */
static const token_t *parse_env(tokenizer_t *t)
{
    const token_t *doller, *env;

    doller = current_token(t);
    if (doller->spec != ENV) syntax_error(t);
    env = next_token(t);
    _next_token(t);

    return parse_word(t);
}

/*
 * parse_env_assignment - Eat <env_assignment>
 */
static const token_t *parse_env_assignment(tokenizer_t *t)
{
    const token_t *key, *assign, *value;
    
    key = current_token(t);
    if (key->spec != WORD) syntax_error(t);
    assign = next_token(t);
    if (assign->spec !=ENV_ASSIGNMENT) syntax_error(t);
    value = next_token(t);
    if (assign->spec !=WORD) syntax_error(t);
    
    if (putenv(value->element) != 0) {
        fprintf(stderr, "putenv failed\n");
    }
    
    return value;
}

/*
 * parse_redirect_in - Eat <redirect_in>
 */
static const token_t *parse_redirect_in(tokenizer_t *t)
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
        token_t *word = parse_redirect_in_append(t);
    }
    */
    if (redirect_head->spec != REDIRECT_IN) syntax_error(t);
    redirect_file = next_token(t);
    t->command[t->p].redirection[0].input = false;
    strcpy(t->command[t->p].redirection[1].filename, redirect_file->element);
    return redirect_file;
}

/*
 * parse_redirect_out - Eat <redirect_out>
 */
static const token_t *parse_home(tokenizer_t *t)
{
    const token_t *home;
    
    home = current_token(t);
    if (home->spec != HOME) syntax_error(t);
    // Convert `~` into home directory

    return _next_token(t);
}

/*
 * parse_redirect_out - Eat <redirect_out>
 */
static const token_t *parse_redirect_out(tokenizer_t *t)
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
        token_t *word = parse_redirect_in_append(t);
    }
    */
    if (redirect_head->spec != REDIRECT_OUT) syntax_error(t);
    redirect_file= next_token(t);
    t->command[t->p].redirection[1].input = false;
    strcpy(t->command[t->p].redirection[1].filename, redirect_file->element);
    return redirect_file;
}

/*
 * parse_redirection - Eat <redirection>
 */
static const token_t *parse_redirection(tokenizer_t *t)
{
    const token_t *redirect_head;

    redirect_head = current_token(t);
    if (redirect_head->spec != REDIRECT_IN
        && redirect_head->spec != REDIRECT_OUT)
        syntax_error(t);
    switch (redirect_head->spec) {
    case REDIRECT_IN:
        parse_redirect_in(t);
        break;
    case REDIRECT_OUT:
        parse_redirect_out(t);
        break;
    }
    return next_token(t);
}

/*
 * parse_redirection_list - Eat <redirection_list>
 */
static const token_t *parse_redirection_list(tokenizer_t *t)
{
    const token_t *redirection;
    const token_t *redirection_list;

    redirection = current_token(t);
    if (redirection->spec != REDIRECT_IN &&
        redirection->spec != REDIRECT_OUT)
        syntax_error(t);
    parse_redirection(t);
    redirection_list = next_token(t);
    if (redirection->spec == REDIRECT_IN &&
        redirection->spec == REDIRECT_OUT)
        parse_redirection_list(t);

    return redirection_list;
}

/*
 * parse_command_element - Eat <command_element>
 */
static const token_t *parse_command_element(tokenizer_t *t)
{
    const token_t *command_element;
    
    command_element = current_token(t);
    switch (command_element->spec) {
    case WORD:
        parse_word(t);
        break;
    case ENV_ASSIGNMENT:
        parse_env_assignment(t);
        break;
    case REDIRECT_IN: case REDIRECT_OUT:
        parse_redirection_list(t);
        break;
    default: break;
    }
    
    return command_element;
}

/*
 * parse_command - Eat <command> and make pipe.
 */
static const token_t *parse_command(tokenizer_t *t)
{
    const token_t *command_element;
    const token_t *command;
    
    command_element = current_token(t);
    if (command_element->spec != WORD &&
        command_element->spec != ENV_ASSIGNMENT &&
        command_element->spec != REDIRECT_IN &&
        command_element->spec != REDIRECT_OUT)
        syntax_error(t);
    parse_command_element(t);
    command = next_token(t);
    if (command->spec == WORD ||
        command->spec == ENV_ASSIGNMENT ||
        command->spec == REDIRECT_IN ||
        command->spec == REDIRECT_OUT)
        parse_command(t);
    
    return command;
}

/*
 * parse_piped_command - Eat <piped_command>
 */
static const token_t *parse_piped_command(tokenizer_t *t)
{
    const token_t *command;
    const token_t *pipe;

    command = current_token(t);
    // Eat command and bind it to 
    pipe = parse_command(t);
    if (t->p <= PIPE_MAX) {
        t->p++;
    } else {
        fprintf(stderr, "At most 8 pipes only can be created.");
        exit(-1);
    }

    if (pipe->spec == PIPED_COMMAND) {
        next_token(t);
        parse_piped_command(t);
    }
   
    return command;
}

/**
 * parse input - Parse and set command information to command tables
 * @t: Token information and next character.
 */
const token_t *parse_input(tokenizer_t *t)
{
    return parse_piped_command(t);
}
