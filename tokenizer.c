/*
 * tokenizer.c - simple tokenizer for shell input
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <ctype.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"

static token_t *_init_token(token_t *t);
static void _append_token(token_t *token, const char *c);
static const token_t *_scan_word(tokenizer_t *t);
static const token_t *_scan_alphanum(tokenizer_t *t);
static const token_t *_scan_only_alphanum(tokenizer_t *t);
static const token_t *_scan_letter(tokenizer_t *t);
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
static const char _getc(char *input)
{
    const char c = input[0];
    char *substring;

    // if (c != '\0' || c != '\n') {
    if (c != '\0') {
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
    tokenizer_t *t = (tokenizer_t *) malloc(sizeof(tokenizer_t));
    if (t == NULL) {
        fprintf(stderr, "Bad allocation (toknizer) \n");
        exit(EXIT_FAILURE);
    }

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
        t->token.spec = WORD;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_word(t);
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
 * _scan_letter - Scan <letter>
 */
static const token_t *_scan_letter(tokenizer_t *t)
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
    case '^': case '_': case '`': case '{': case '|': case '}': /* case '~': */
        t->token.spec = LETTER;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_letter(t);
        break;
    default: break;
    }
    
    return &(t->token);
}

/*
 * _scan_alphanum - Scan <alphanum>
 */
static const token_t *_scan_only_alphanum(tokenizer_t *t)
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
        // t->token.spec = ALPHANUM;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_only_alphanum(t);
        break;
    default:  break;
    }
    return &(t->token);
}


/*
 * _scan_alphanum - Scan <alphanum>
 */
static const token_t *_scan_alphanum(tokenizer_t *t)
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
        t->token.spec = ALPHANUM;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_alphanum(t);
        break;
    case '!': case '"': case '#': case '%': case '\'': case '(': case ')':
    case '*': case '+': case ',': case '-': case '.': case '/': case ':':
    case ';': case '?': case '@': case '[': case ']': case '&': case '\\':
    case '^': case '_': case '`': case '{': case '}': case '~':
        t->token.spec = LETTER;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_letter(t);
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
        t->c = _getc(t->input);
        _scan_num(t);
        break;
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z': 
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': 
        t->token.spec = ALPHANUM;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_alphanum(t);
        break;
    case '!': case '"': case '#': case '%': case '\'': case '(': case ')':
    case '*': case '+': case ',': case '-': case '.': case '/': case ':':
    case ';': case '?': case '@': case '[': case ']': case '&': case '\\':
    case '^': case '_': case '`': case '{': case '}': case '~':
        t->token.spec = LETTER;
        _append_token(&(t->token), &(t->c));
        t->c = _getc(t->input);
        _scan_letter(t);
        break;
    case '>':
        /* if (t->token.spec == REDIRECT_OUT) */
        /*     t->token.spec = REDIRECT_OUT_APPEND; */
        /* else */
        /*     t->token.spec = REDIRECT_OUT; */
        t->token.spec = REDIRECT_OUT;
        _scan_redirect_out(t);
        // t->c = _getc(t->input);
        break;
    case '<':
        t->token.spec = REDIRECT_IN;
        _scan_redirect_in(t);
        // t->c = _getc(t->input);
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
    switch (t->c) {
    case '$':
        t->token.spec = ENV;
        t->c = _getc(t->input);
        __scan_env(t, key);
        break;
    case '{':
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
        strncat(key, &(t->c), 1);
        t->c = _getc(t->input);
        __scan_env(t, key);
        break;
    case '}':
        /* if (t->token.spec != ENV) {
           strncat(t->token.element, key, strlen(key));
           t->c = _getc(t->input);
           __scan_env(t, key);
        } else {*/
        // t->c = _getc(t->input);
        // break;
    default:
        strncat(t->token.element, key, strlen(key));
        t->c = _getc(t->input);
        __scan_env(t, key);
            /*
            t->token.spec = WORD;
            char *value = getenv(key);
            if (value)
                strncat(t->token.element, value, strlen(value));
            if (t->c != '/')
                t->c = _getc(t->input);
            _scan_word(t);
            break;
            } */
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
    switch (t->c) {
    case '~':
        t->token.spec = HOME;
        t->c = _getc(t->input);
        _scan_only_alphanum(t);
        if (!isspace(t->c)) t->token.spec = HOME_WORD;
        break;
    default: break;
    }
    
    return &(t->token);
}

/*
 * _scan_assignment - Scan <env_assignment>
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
        t->c = _getc(t->input);
        switch (t->c) {
        case '>':
            t->token.spec = REDIRECT_IN_OUT;
            // t->c = _getc(t->input);
            break;
        default: break;
        }
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
        t->c = _getc(t->input);
        switch (t->c) {
        case '>':
            t->token.spec = REDIRECT_OUT_APPEND;
            // t->c = _getc(t->input);
            break;
        case '&':
            t->token.spec = REDIRECT_OUT_COMPOSITION;
            // t->c = _getc(t->input);
            break;
        default: break;
        }
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
    while (isspace(t->c) && t->c != '\n') t->c = _getc(t->input);
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
        t->token.spec = ALPHANUM;
        _scan_alphanum(t);
        t->c = _getc(t->input);
        break;
    case '!': case '"': case '#': case '%': case '\'': case '(': case ')':
    case '*': case '+': case ',': case '-': case '.': case '/': case ':':
    case ';': case '?': case '@': case '[': case ']': case '&': case '\\':
    case '^': case '_': case '`': case '{': case '}': 
        t->token.spec = LETTER;
        _scan_letter(t);
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
        break;
    case '<':
        t->token.spec = REDIRECT_IN;
        _scan_redirect_in(t);
        break;
    case '>':
        t->token.spec = REDIRECT_OUT;
        _scan_redirect_out(t);
        break;
    case '|':
        t->token.spec = PIPED_COMMAND;
        t->c = _getc(t->input);
        break;
    default: break;
    }
    
    return &(t->token);
}
