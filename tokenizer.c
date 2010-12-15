/*
 * tokenizer.c - simple tokenizer for shell input
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

static inline const char _getc(char* input);
static token_t *_init_token(token_t *t);
static void _append_token(token_t *token, const char *c);
static const token_t *_scan_word(tokenizer_t *t);
static const token_t *_scan_num(tokenizer_t *t);
static const token_t *_scan_env(tokenizer_t *t);
static const token_t *_scan_home(tokenizer_t *t);
static const token_t *_scan_env_assignment(tokenizer_t *t);
static const token_t *_scan_redirect_in(tokenizer_t *t);
static const token_t *_scan_redirect_out(tokenizer_t *t);
// static const token_t *_next_token(tokenizer_t *t);

/*
 * _getc - get a character from input
 */
static inline const char _getc(char *input)
{
    const char c = input[0];
    int index;
    char *substring;

    if (c != '\0' || c != '\n') {
        substring = input + 1;
        strncpy(input, substring, ELEMENT_MAX);
    }

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

    strncpy(t->input, input, INPUT_MAX);
    // Read first character from input.
    t->c = _getc(t->input);
    next_token(t);
    
    return t;
}

/*
 * _init_token - Initialize token
 */
static token_t *_init_token(token_t *t)
{
    t->spec = ERROR;
    memset((void *) t->element, '\0', sizeof(char) * ELEMENT_MAX);
    
    return t;
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
 * _scan_word - Scan <word>
 */
static const token_t *_scan_word(tokenizer_t *t)
{
    switch (t->c) {
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
    case '^': case '_': case '`': case '{': case '|': case '}': case '~':
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_word(t);
        break;
    case '$':
        _scan_env(t);
        // t->c = _getc(t->input);
        break;
    case '=':
        _scan_env_assignment(t);
        // t->c = _getc(t->input);
        break;
    default: break;
    }
    
    return &(t->token);
}

/*
 * _scan_num - Scan <num>
 */
static const token_t *_scan_num(tokenizer_t *t)
{
    switch (t->c) {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9':
        t->token.spec = NUM;
        _append_token(&(t->token), &(t->c));
        _scan_num(t);
        t->c = _getc(t->input);
        break;
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
    case '^': case '_': case '`': case '{': case '}': case '~':
        t->token.spec = WORD;
        _scan_word(t);
        t->c = _getc(t->input);
        break;
    case '$':
        t->token.spec = ENV;
        _scan_env(t);
        t->c = _getc(t->input);
        break;
    default: break;
    }
    return &(t->token);
}

/*
 * __scan_env - real scanner for <env>
 */
static const token_t *__scan_env(tokenizer_t *t, char *key)
{
    const token_t *word;

    switch (t->c) {
    case '$':
        t->token.spec = WORD;
        t->c = _getc(t->input);
        __scan_env(t, key);
        break;
    case '{':
        t->token.spec = ENV;
        t->c = _getc(t->input);
        __scan_env(t, key);
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
    case '*': case '+': case ',': case '-': case '.': /* case '/': */case ':':
    case ';': case '?': case '@': case '[': case ']': case '&': case '\\':
    case '^': case '_': case '`': /* case '{':  case '}': */
        strncat(key, &(t->c), 1);
        t->c = _getc(t->input);
        __scan_env(t, key);
        break;
    case '}': 
        if (t->token.spec != ENV) {
            strncat(t->token.element, key, strlen(key));
            t->c = _getc(t->input);
            __scan_env(t, key);
        } else {
        default:
            // t->token.spec = WORD;
            // char *value = getenv(key);
            // if (value)
            //     strncat(t->token.element, value, strlen(value));
            strncat(t->token.element, value, strlen(value));
            if (t->c != '/')
                t->c = _getc(t->input);
            _scan_word(t);
            break;
        }
        break;
    }
    
    return &(t->token);
}

/*
 * _scan_env - Scan <env>
 */
static const token_t *_scan_env(tokenizer_t *t)
{
    char c[ELEMENT_MAX];
    memset(c, 0, ELEMENT_MAX);
        
    return __scan_env(t, c);
}

/*
 * _scan_home - Scan <home>
 */
static const token_t *_scan_home(tokenizer_t *t)
{
    register struct passwd *pw;
    register uid_t uid;
    char *env_home;
        
    switch (t->c) {
    case '~':
        env_home = getenv("HOME");
        if (env_home) {
            strncat(t->token.element, env_home, strlen(env_home));
        } else {
            uid = geteuid();
            pw = getpwuid(uid);
            if (pw) {
                const char *homedir = "/home/";
                strncat(t->token.element, homedir, strlen(homedir));
                strncat(t->token.element, pw->pw_name, strlen(pw->pw_name));
            } else {
                fprintf(stderr,"error: cannot find username for UID %u\n",
                        (unsigned) uid);
            }
        }
        break;
    default: break;
    }
    
    return &(t->token);
}

/*
 * _scan_num - Scan <env_assignment>
 */
static const token_t *_scan_env_assignment(tokenizer_t *t)
{
    switch (t->c) {
    case '=':
        t->token.spec = ENV_ASSIGNMENT;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_word(t);
        // t->c = _getc(t->input);
        break;
    default: break;
    }

    return &(t->token);
}

/*
 * _scan_redirect_in - Scan <redirect_in>
 */
static const token_t *_scan_redirect_in(tokenizer_t *t)
{
    switch (t->c) {
    case '<':
        t->token.spec = REDIRECT_IN;
        break;
    default: break;
    }
    
    return &(t->token);
}

/*
 * _scan_redirect_out - Scan <redirect_out>
 */
static const token_t *_scan_redirect_out(tokenizer_t *t)
{
    switch (t->c) {
    case '>':
        t->token.spec = REDIRECT_OUT;
        break;
    default: break;
    }
    
    return &(t->token);
}

/**
 * next_token - Scan input and return next token.
 * @t: Token information and next character.
 */
const token_t *next_token(tokenizer_t *t)
{
    _init_token(&(t->token));
    return _next_token(t);
}

/**
 * _next_token - Real scanner.
 * @t: Token information and next character.
 */
const token_t  *_next_token(tokenizer_t *t)
{
    while (isspace(t->c)) t->c = _getc(t->input);
    switch (t->c) {
    case EOF:
        t->token.spec = END_OF_FILE;
        break;
    case '\n':
        t->token.spec = END_OF_LINE;
        break;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9':
        t->token.spec = NUM;
        _scan_num(t);
        t->c = _getc(t->input);
        break;
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
        _scan_word(t);
        t->c = _getc(t->input);
        break;
    case '$':
        t->token.spec = ENV;
        _scan_env(t);
        t->c = _getc(t->input);
        break;
    case '~':
        t->token.spec = HOME;
        _scan_home(t);
        t->token.spec = WORD;
        t->c = _getc(t->input);
        if (!isspace(t->c))
            _next_token(t);
        break;
    case '<':
        t->token.spec = REDIRECT_IN;
        _scan_redirect_in(t);
        t->c = _getc(t->input);
        break;
    case '>':
        t->token.spec = REDIRECT_OUT;
        _scan_redirect_out(t);
        t->c = _getc(t->input);
        break;
    case '|':
        t->token.spec = PIPED_COMMAND;
        t->c = _getc(t->input);
        break;
    default: break;
    }
    
    return &(t->token);
}
