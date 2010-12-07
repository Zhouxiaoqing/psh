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

static const char _getc(char* input);
static token_t *_init_token(tokenizer_t *t);
static void _append_token(token_t *token, const char *c);
static const token_t *_parse_word(tokenizer_t *t);
static const token_t *_parse_num(tokenizer_t *t);
static const token_t *_parse_env(tokenizer_t *t);
static const token_t *_parse_home(tokenizer_t *t);
static const token_t *_parse_env_assignment(tokenizer_t *t);
static const token_t *_parse_redirect_in(tokenizer_t *t);
static const token_t *_parse_redirect_out(tokenizer_t *t);
static const token_t *_next_token(tokenizer_t *t);
static const token_t *eat_word(tokenizer_t *t);
static const token_t *eat_env(tokenizer_t *t);
static const token_t *eat_env_assignment(tokenizer_t *t);
static const token_t *eat_home(tokenizer_t *t);
static const token_t *eat_redirect_out(tokenizer_t *t);
static const token_t *eat_redirect_in(tokenizer_t *t);
static const token_t *eat_redirection(tokenizer_t *t);
static const token_t *eat_command_element(tokenizer_t *t);
static const token_t *eat_redirection_list(tokenizer_t *t);
static const token_t *eat_command(tokenizer_t *t);
static const token_t *eat_piped_command(tokenizer_t *t);

/*
 * _getc - get a character from input
 */
static const char _getc(char *input)
{
    char c = input[0];
    input++;
    return c;
}

/**
 * init_tokenizer - Initialize and set up scanninig from input.
 * @input: input from prompt
 */
tokenizer_t *init_tokenizer(const char *input)
{
    int i = 0;
    tokenizer_t *t = (tokenizer_t *) malloc(sizeof(tokenizer_t));
    
    t->p = 0;
    for (i = 0; i < PIPE_MAX; i++)
        t->command[i].command_flag = false;
    strncpy(t->input, input, INPUT_MAX);
    // Read first character from input.
    t->c = _getc(t->input);
    next_token(t);
    return t;
}
/*
 * _init_token - Initialize token
 */
static token_t *_init_token(tokenizer_t *t)
{
    t->token.spec = ERROR;
    memset((void *) t->token.element, '\0', sizeof(char) * ELEMENT_MAX);
    
    return &(t->token);
}

/**
 * current_token - Get current token.
 * @t: Token information and next character.
 */
const token_t *current_token(tokenizer_t *t)
{
    return &(t->token);
}

/*
 * _append_token - append a character to the tail of token's element
 */
static void _append_token(token_t *token, const char *c)
{
    if (strlen(token->element) < ELEMENT_MAX) {
        strncat(token->element, c, 1);
    } else {
        fprintf(stderr, "error: length of command element is too long.");
        exit(-1);
    }
}

/*
 * _parse_word - Parse <word>
 */
static const token_t *_parse_word(tokenizer_t *t)
{
    switch (t->c) {
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z': 
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': 
    case '!': case '"': case '#': case '%': case '\'': case '(': case ')':
    case '*': case '+': case ',': case '-': case '.': case '/': case ':':
    case ';': case '?': case '@': case '[': case ']': case '&': case '\\':
    case '^': case '_': case '`': case '{': case '|': case '}':
        t->token.spec = WORD;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _parse_word(t);
        break;
    case '$':
        t->token.spec = ENV;
        t->c = _getc(t->input);
        _parse_env(t);
        break;
    case '~':
        t->token.spec = HOME;
        t->c = _getc(t->input);
        _parse_home(t);
        break;
    case '=':
        t->token.spec = ENV_ASSIGNMENT;
        t->c = _getc(t->input);
        _parse_env_assignment(t);
        break;
    case '<':
        next_token(t);
        t->token.spec = REDIRECT_IN;
        t->c = _getc(t->input);
        _parse_redirect_in(t);
        break;
    case '>':
        next_token(t);
        t->token.spec = REDIRECT_OUT;
        t->c = _getc(t->input);
        _parse_redirect_out(t);
        break;
    default: break;
    }
    
    return &(t->token);
}

/*
 * _parse_num - Parse <num>
 */
static const token_t *_parse_num(tokenizer_t *t)
{
    switch (t->c) {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9':
        t->token.spec = NUM;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _parse_num(t);
        break;
    /*
    case '<':
        next_token(t);
        t->token.spec = REDIRECT_IN;
        _parse_redirect_in(t);
        break;
    case '>':
        next_token(t);
        t->token.spec = REDIRECT_OUT;
        _parse_redirect_out(t);
        break;
    */
    default: break;
    }
    return &(t->token);
}

/*
 * _parse_env - Parse <env>
 */
static const token_t *_parse_env(tokenizer_t *t)
{
    const token_t *word;
    switch (t->c) {
    case '$':
        t->token.spec = ENV;
        t->c = _getc(t->input);
        word = _parse_word(t);
        char *env_v = getenv(word->element);
        if (env_v) {
            t->token.spec = WORD;
            strncat(t->token.element, env_v, strlen(env_v));
        } else {
            t->token.spec = WORD;
        }
    default: break;
    }
    
    return &(t->token);
}

/*
 * _parse_home - Parse <home>
 */
static const token_t *_parse_home(tokenizer_t *t)
{
    switch (t->c) {
    case '~':
        t->token.spec = HOME;
        register struct passwd *pw;
        register uid_t uid;
        
        uid = geteuid();
        pw = getpwuid(uid);
        if (pw) {
            const char *homedir = "/home/";
            strncpy(t->token.element, homedir, strlen(homedir));
            strncat(t->token.element, pw->pw_name, strlen(pw->pw_name));
            strncat(t->token.element, "/", 1);
        } else {
            fprintf(stderr,"error: cannot find username for UID %u\n",
                    (unsigned) uid);
        }
        break;
    default: break;

    }
    return &(t->token);
}

/*
 * _parse_num - Parse <env_assignment>
 */
static const token_t *_parse_env_assignment(tokenizer_t *t)
{
    switch (t->c) {
    case '=':
        t->token.spec = ENV_ASSIGNMENT;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _parse_word(t);
        break;
    default: break;
    }

    return &(t->token);
}

/*
 * _parse_redirect_in - Parse <redirect_in>
 */
static const token_t *_parse_redirect_in(tokenizer_t *t)
{
    switch (t->c) {
    case '<':
        t->token.spec = REDIRECT_IN;
        t->c = _getc(t->input);
        // next_token(t);
        // _parse_word(t);
        break;
    default: break;
    }
    
    return &(t->token);
}

/*
 * _parse_redirect_out - Parse <redirect_out>
 */
static const token_t *_parse_redirect_out(tokenizer_t *t)
{
    switch (t->c) {
    case '>':
        t->token.spec = REDIRECT_OUT;
        t->c = _getc(t->input);
        // next_token(t);
        // _parse_word(t);
        break;
    /*
    case '&':
        // Composite multi-descriptor
        break;
    */
    default: break;
    }
    
    return &(t->token);
}

/**
 * _next_token - Scan input and return next token.
 * @t: Token information and next character.
 */
const token_t *next_token(tokenizer_t *t)
{
    _init_token(t);
    return _next_token(t);
}

/*
 * _next_token - Real scanner.
 */
static const token_t  *_next_token(tokenizer_t *t)
{
    while (isspace(t->c)) t->c = _getc(t->input);
    switch (t->c) {
    case EOF:
        t->token.spec = END_OF_FILE;
        break;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z': 
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': 
    case '!': case '"': case '#': case '%': case '\'': case '(': case ')':
    case '*': case '+': case ',': case '-': case '.': case '/': case ':':
    case ';': case '?': case '@': case '[': case ']': case '&': case '\\':
    case '^': case '_': case '`': case '{': case '}': 
        t->token.spec = WORD;
        t->c = _getc(t->input);
        _parse_word(t);
        break;
    case '$':
        t->token.spec = ENV;
        t->c = _getc(t->input);
        _parse_env(t);
        break;
    case '~':
        t->token.spec = HOME;
        t->c = _getc(t->input);
        _parse_home(t);
        break;
    case '<':
        t->token.spec = REDIRECT_IN;
        t->c = _getc(t->input);
        _parse_redirect_in(t);
        break;
    case '>':
        t->token.spec = REDIRECT_OUT;
        t->c = _getc(t->input);
        _parse_redirect_out(t);
        break;
    case '|':
        t->token.spec = PIPED_COMMAND;
        t->c = _getc(t->input);
        break;
    default: break;
    }
}

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
 * eat_word - Eat <word>
 */
static const token_t *eat_word(tokenizer_t *t)
{
    const token_t *word;
    command_t current_command;

    word = current_token(t);
    if (word->spec != WORD) syntax_error(t);
    current_command = t->command[t->p];
    if (!current_command.command_flag) {
        strncpy(current_command.cmd, word->element, ELEMENT_MAX);
    } else {
        strncat(current_command.args, " ", 1);
        strncat(current_command.args, word->element, strlen(word->element));
    }

    return &(t->token);
}

/*
 * eat_env - Eat <env>
 */
static const token_t *eat_env(tokenizer_t *t)
{
    const token_t *env;
    env = current_token(t);
    // Environment variable will be converted into real value in environ.
    if (env->spec != WORD) syntax_error(t);
    _next_token(t);

    return eat_word(t);
}

/*
 * eat_env_assignment - Eat <env_assignment>
 */
static const token_t *eat_env_assignment(tokenizer_t *t)
{
    const token_t *env;
    
    env = current_token(t);
    if (env->spec != ENV_ASSIGNMENT) syntax_error(t);
    if (putenv(env->element) != 0) {
        fprintf(stderr, "putenv failed\n");
    }
    // Environment variable will be converted into real value in environ.
    if (env->spec != WORD) syntax_error(t);

    return eat_word(t);
}

/*
 * eat_redirect_out - Eat <redirect_out>
 */
static const token_t *eat_home(tokenizer_t *t)
{
    const token_t *home;
    
    home = current_token(t);
    if (home->spec != HOME) syntax_error(t);
    // Convert `~` into home directory

    return _next_token(t);
}

/*
 * eat_redirect_out - Eat <redirect_out>
 */
static const token_t *eat_redirect_out(tokenizer_t *t)
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
        token_t *word = eat_redirect_in_append(t);
    }
    */
    if (redirect_head->spec != REDIRECT_OUT) syntax_error(t);
    redirect_file= next_token(t);
    t->command[t->p].redirection[0].input = true;
    strcpy(t->command[t->p].redirection[0].filename, redirect_file->element);
    return redirect_file;
}


/*
 * eat_redirect_in - Eat <redirect_in>
 */
static const token_t *eat_redirect_in(tokenizer_t *t)
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
        token_t *word = eat_redirect_in_append(t);
    }
    */
    if (redirect_head->spec != REDIRECT_IN) syntax_error(t);
    redirect_file = next_token(t);
    t->command[t->p].redirection[0].input = false;
    strcpy(t->command[t->p].redirection[1].filename, redirect_file->element);
    return redirect_file;
}

/*
 * eat_redirection - Eat <redirection>
 */
static const token_t *eat_redirection(tokenizer_t *t)
{
    const token_t *redirect_head;

    redirect_head = current_token(t);
    if (redirect_head->spec != REDIRECT_IN
        && redirect_head->spec != REDIRECT_OUT)
        syntax_error(t);
    switch (redirect_head->spec) {
    case REDIRECT_IN:
        eat_redirect_in(t);
        break;
    case REDIRECT_OUT:
        eat_redirect_out(t);
        break;
    }
    return next_token(t);
}

/*
 * eat_command_element - Eat <command_element>
 */
static const token_t *eat_command_element(tokenizer_t *t)
{
    const token_t *command_element;
    
    command_element = current_token(t);
    switch (command_element->spec) {
    case WORD:
        eat_word(t);
        break;
    case ENV_ASSIGNMENT:
        eat_env_assignment(t);
        break;
    case REDIRECT_IN: case REDIRECT_OUT:
        eat_redirection_list(t);
        break;
    default: break;
    }
    
    return command_element;
}

/*
 * eat_redirection_list - Eat <redirection_list>
 */
static const token_t *eat_redirection_list(tokenizer_t *t)
{
    const token_t *redirection;
    const token_t *redirection_list;

    redirection = current_token(t);
    if (redirection->spec != REDIRECT_IN &&
        redirection->spec != REDIRECT_OUT)
        syntax_error(t);
    eat_redirection(t);
    redirection_list = next_token(t);
    eat_redirection_list(t);

    return redirection_list;
}

/*
 * eat_command - Eat <command> and make pipe.
 */
static const token_t *eat_command(tokenizer_t *t)
{
    const token_t *command_element;
    const token_t *command;
    
    command_element = current_token(t);
    if (command_element->spec != WORD &&
        command_element->spec != ENV_ASSIGNMENT &&
        command_element->spec != REDIRECTION_LIST)
        syntax_error(t);
    eat_command_element(t);
    command = next_token(t);
    if (command_element->spec == WORD ||
        command_element->spec == ENV_ASSIGNMENT ||
        command_element->spec == REDIRECT_IN ||
        command_element->spec == REDIRECT_OUT)
        eat_command(t);
    
    return command_element;
}

/*
 * eat_piped_command - Eat <piped_command>
 */
static const token_t *eat_piped_command(tokenizer_t *t)
{
    const token_t *command;
    const token_t *piped_command;

    command = current_token(t);
    // Eat command and bind it to 
    eat_command(t);
    if (t->p <= PIPE_MAX) {
        t->p++;
    } else {
        fprintf(stderr, "At most 8 pipes only can be created.");
        exit(-1);
    }

    piped_command = next_token(t);
    if (piped_command->spec == PIPED_COMMAND) {
        next_token(t);
        eat_piped_command(t);
    }
    
    return command;
}

/**
 * parse input - Parse and set command information to command tables
 * @t: Token information and next character.
 */
const token_t *parse_input(tokenizer_t *t)
{
    return eat_piped_command(t);
}
