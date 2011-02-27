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
#include "tree.h"

static const node_t *_parse_terminal(parser_t *p, tokenizer_t *t);
static const node_t *_parse_num(parser_t *p, tokenizer_t *t);
static const node_t *_parse_letter(parser_t *p, tokenizer_t *t);
static const node_t *_parse_alphanum(parser_t *p, tokenizer_t *t);
static const node_t *_parse_env(parser_t *p, tokenizer_t *t);
static const node_t *_parse_word(parser_t *p, tokenizer_t *t, node_t *parent);
static const node_t *
_parse_env_assignment(parser_t *p, tokenizer_t *t, node_t *parent);
// static const node_t *_parse_home(parser_t *p, tokenizer_t *t, node_t *parent);
static const node_t *_parse_home(parser_t *p, tokenizer_t *t);
static const node_t *
_parse_redirect_out(parser_t *p, tokenizer_t *t, node_t *parent);
static const node_t *
_parse_redirect_in(parser_t *p, tokenizer_t *t, node_t *parent);
static const node_t *
_parse_redirection(parser_t *p, tokenizer_t *t, node_t *parent);
static const node_t *
_parse_command_element(parser_t *p, tokenizer_t *t, node_t *parent);
static const node_t *
_parse_redirection_list(parser_t *p, tokenizer_t *t, node_t *parent);
static const node_t *
_parse_piped_command(parser_t *p, tokenizer_t *t, node_t *parent);

/**
 * init_parsr - Initialize parser and command tables.
 */
parser_t *init_parser(void)
{
    parser_t *p = (parser_t *) malloc(sizeof(parser_t));
    if (p == NULL) {
        fprintf(stderr, "Bad allocation (parser) \n");
        exit(EXIT_FAILURE);
    }
    p->root = init_root();

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
    free_nodes(p->root, p->root);
    free(p);
    exit(EXIT_FAILURE);
}

static const node_t *_parse_terminal(parser_t *p, tokenizer_t *t)
{
    const token_t *_terminal;
    node_t *terminal;

    _terminal = current_token(t);
    if (!_is_letter(_terminal) && !_is_alphanum(_terminal)
        && !_is_num(_terminal)  && !_is_env(_terminal))  syntax_error(p, t);
    terminal = init_node(_terminal);

    return terminal;
}

/*
 * _parse_num - Parse <num>
 */
static const node_t *_parse_num(parser_t *p, tokenizer_t *t)
{
    return _parse_terminal(p, t);
}


/*
 * _parse_alphanum - Parse <alphanum>
 */
static const node_t *_parse_alphanum(parser_t *p, tokenizer_t *t)
{
    return _parse_terminal(p, t);
}

/*
 * _parse_letter - Parse <letter>
 */
static const node_t *_parse_letter(parser_t *p, tokenizer_t *t)
{
    return _parse_terminal(p, t);
}

/*
 * _parse_env - Parse <env>
 */
static const node_t *_parse_env(parser_t *p, tokenizer_t *t)
{
    return _parse_terminal(p, t);
}



/*
 * _parse_word - Parse <word>
 */
static const node_t *_parse_word(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_elh, *_word;
    const node_t *elh, *word;

    _elh = current_token(t);
    if (!_is_word(_elh))  syntax_error(p, t);
    word = NULL;
    switch (_elh->spec) {
    case ENV:
        elh = _parse_env(p, t);
        break;
    case LETTER:
        elh = _parse_letter(p, t);
        break;
    case ALPHANUM:
        elh = _parse_alphanum(p, t);
        break;
    case NUM:
        elh = _parse_num(p, t);
        break;
    case HOME:
        // elh = _parse_home(p, t, parent);
        elh = _parse_home(p, t);
        break;
    case HOME_WORD:
        elh = _parse_home(p, t);
        _word = next_token(t);
        if (!_is_word(_word))  syntax_error(p, t);
        word = _parse_word(p, t, init_abstract_node(WORD));
        break;
    default:
        break;
    }
    
    create_tree(parent, elh, word);
    
    return parent;
}

/*
 * _parse_env_assignment - Parse <env_assignment>
 */
static const node_t *
_parse_env_assignment(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_env_assignment;
    const node_t *env_assignment;
    
    _env_assignment = current_token(t);
    if (!_is_env_assignment(_env_assignment))  syntax_error(p, t);
    env_assignment = init_node(_env_assignment);
    create_tree(parent, env_assignment, NULL);
    
    return parent;
}

/*
 * _parse_home - Parse <home>
 */
// static const node_t *_parse_home(parser_t *p, tokenizer_t *t, node_t *parent)
static const node_t *_parse_home(parser_t *p, tokenizer_t *t)
{    
    const token_t *_home = current_token(t);
    const node_t *home;

    if (!_is_home(_home))  syntax_error(p, t);
    home = init_node(_home);
    // create_tree(parent, home, NULL);
    
    // return parent;
    return home;
}

/*
 * _parse_redirect_in - Parse <redirect_in>
 */
static const node_t *
_parse_redirect_in(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_redirect_in, *_word;
    // node_t *redirect_in;
    const node_t *word;

    _redirect_in = current_token(t);
    if (!_is_redirect_in(_redirect_in))  syntax_error(p, t);
    
    /* switch (_redirect_in->spec) { */
    /* case REDIRECT_IN: */
    /*     redirect_in = init_node(_redirect_in); */
    /*     break; */
    /* default: */
    /*     redirect_in = NULL; */
    /*     break; */
    /* } */
    _word = next_token(t);
    if (!_is_word(_word))  syntax_error(p, t);
    word = _parse_word(p, t, parent);
    // create_tree(parent, redirect_in, NULL);

    return parent;
}

/*
 * _parse_redirect_out - Parse <redirect_out>
 */
static const node_t *_parse_redirect_out(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_redirect_out, *_wn;
    // node_t *redirect_out;
    const node_t *wn;
    
    _redirect_out = current_token(t);
    if (!_is_redirect_out(_redirect_out))  syntax_error(p, t);
    switch (_redirect_out->spec) {
    case REDIRECT_OUT_COMPOSITION:
        _wn = next_token(t);
        if (!_is_num(_wn))  break; // syntax_error(p, t);
        wn = _parse_num(p, t);
        create_tree(parent, wn, NULL);
        break;
    case REDIRECT_OUT: case REDIRECT_OUT_APPEND:
        _wn = next_token(t);
        if (!_is_word(_wn))  syntax_error(p, t);
        wn = _parse_word(p, t, parent);
        break;
    default:
        break;
    }
    
    // create_tree(parent, redirect_out, NULL);

    return parent;
}

/*
 * _parse_redirection - Parse <redirection>
 */
static const node_t *
_parse_redirection(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_redirection;
    const node_t *redirection;

    _redirection = current_token(t);
    if (!_is_redirection(_redirection))  syntax_error(p, t);
    switch (_redirection->spec) {
        // case REDIRECT_OUT: case REDIRECT_OUT_APPEND: case REDIRECT_OUT_COMPOSITION:
    case REDIRECT_OUT_PATTERN:
        redirection = _parse_redirect_out(p, t, init_node(_redirection));
        break;
        // case REDIRECT_IN: case REDIRECT_IN_OUT:
    case REDIRECT_IN_PATTERN:
        redirection = _parse_redirect_in(
            p, t, init_node(_redirection));
        break;
    default:
        redirection = NULL;
    }
    
    create_tree(parent, redirection, NULL);
    return parent;
}

/*
 * _parse_redirection_list - Parse <redirection_list>
 */
static const node_t *
_parse_redirection_list(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_redirection, *_redirection_list;
    const node_t *redirection, *redirection_list;
    
    _redirection = current_token(t);
    if (!_is_redirection(_redirection))  syntax_error(p, t);
    redirection = _parse_redirection(p, t, init_abstract_node(REDIRECTION));
    /* _redirection_list = next_token(t); */
    /* if (_is_redirection_list(_redirection_list)) */
    /*     redirection_list = _parse_redirection_list( */
    /*         p, t, init_abstract_node(REDIRECTION_LIST)); */
    /* else */
    /*     redirection_list = NULL; */
    /* create_tree(parent, redirection, redirection_list); */
    create_tree(parent, redirection, NULL);

    return parent;
}

/*
 * _parse_command_element - Parse <command_element>
 */
static const node_t *
_parse_command_element(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_wer, *_command_element;
    const node_t *wer, *command_element;
    
    _wer = current_token(t);
    if (!_is_command_element(_wer))  syntax_error(p, t);
    switch (_wer->spec) {
        // case WORD: case ENV: case LETTER: case ALPHANUM: case NUM:
    case WORD_PATTERN:
        wer = _parse_word(p, t, init_abstract_node(WORD));
        break;
    case ENV_ASSIGNMENT:
        wer = _parse_env_assignment(p, t, init_abstract_node(ENV_ASSIGNMENT));
        break;
        // case REDIRECT_OUT: case REDIRECT_OUT_APPEND: case REDIRECT_OUT_COMPOSITION:
        // case REDIRECT_IN: case REDIRECT_IN_OUT:
    case REDIRECT_PATTERN:
        wer = _parse_redirection_list(p, t, init_abstract_node(REDIRECTION_LIST));
        break;
    default: break;
    }
    
    _command_element = next_token(t);
    if (_command_element != NULL && _is_command_element(_command_element))
        command_element = _parse_command_element(
            p, t, init_abstract_node(COMMAND_ELEMENT));
    else
        command_element = NULL;
    
    create_tree(parent, wer, command_element);
    
    return parent;
}

/*
 * _parse_command - Parse <command> and make pipe.
 */
static const node_t *
_parse_piped_command(parser_t *p, tokenizer_t *t, node_t *parent)
{
    const token_t *_command_element, *_command;
    const node_t *command_element, *command;
    
    _command_element = current_token(t);
    if (!_is_command_element(_command_element))  syntax_error(p, t);
    command_element = _parse_command_element(
        p, t, init_abstract_node(COMMAND_ELEMENT));
    _command = next_token(t);
    if (_is_command_element(_command))
        command = _parse_piped_command(p, t, init_abstract_node(COMMAND));
    else
        command = NULL;
    create_tree(parent, command_element, command);
    
    return parent;
}

/**
 * parse_input - Parse and set command information to command tables
 * @t: Token information and next character.
 */
const node_t *parse_input(parser_t *p, tokenizer_t *t)
{
    const node_t *root = _parse_piped_command(p, t, p->root);
    return root;
}
