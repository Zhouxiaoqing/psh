/*
 * executor.c - execute parsed tree
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "executor.h"

static int _fork_exec(command_t *current_command,
                      const bool head, const bool tail);
static void _eat_letter(const node_t *current,
                        command_t *current_command, node_t *parent);
static void _eat_num(const node_t *current,
                     command_t *current_command, node_t *parent);
static void _eat_alphanum(const node_t *current,
                          command_t *current_command, node_t *parent);
static void _eat_home(const node_t *current,
                      command_t *current_command, node_t *parent);
static void _eat_env(const node_t *current,
                     command_t *current_command, node_t *parent);
// static void _eat_word(const node_t *current, command_t *current_command);
static void _eat_word(node_t *current, command_t *current_command);
static void _eat_env_assignment(const node_t *current,
                                command_t *current_command);
static void _eat_redirect_out(const node_t *current, command_t *current_command);
static void _eat_redirect_in(const node_t *current, command_t *current_command);
static void _eat_redirection(const node_t *current, command_t *current_command);
static void _eat_redirection_list(const node_t *current,
                                  command_t *current_command);
static void _eat_command_element(const node_t *current,
                                 command_t *current_command);
static void _eat_command(const node_t *current, command_t *current_command);
static void _eat_piped_command(const node_t *current,
                               command_t *current_command, const bool head);

/*
 * _fork_exec - fork and exec
 */
static int _fork_exec(command_t *current_command,
                      const bool head, const bool tail)
{
    pid_t fork_result, child;
    int next_pipe[2];

    if (pipe(next_pipe) == 0) {
        fork_result = fork();
        switch (fork_result) {
        case -1:
            fprintf(stderr, "fork failure");
            exit(EXIT_FAILURE);
            break;
        case 0:  // case of child
            if (!head || !tail) {
                if (head)
                    current_command->input_fd = open("/dev/stdin", O_RDONLY);
                else
                    dup2(current_command->input_fd, 0);

                dup2(current_command->input_fd, 0);
                close(current_command->input_fd);

                if (tail) {
                    close(current_command->output_fd);
                    current_command->output_fd = open("/dev/stdout", O_WRONLY);
                } else {
                    current_command->output_fd = next_pipe[1];
                }
                dup2(current_command->output_fd, 1);
                close(current_command->output_fd);

                current_command->input_fd = next_pipe[0];
            }
            execlp(current_command->cmd, current_command->cmd,
                   current_command->args, NULL);
            exit(EXIT_SUCCESS);
            break;
        default:  // case of parent
            child = wait((int *) 0);
            break;
        }
    } else {
        fprintf(stderr, "pipe creation failure");
        exit(EXIT_FAILURE);
    }
}


/*
 * _eat_terminal - eat <letter>, <num> or <alphanum>
 */
static inline void __eat_terminal(const node_t *current,
                           command_t *current_command, node_t *parent) {
    strncat(parent->token->element,
            current->token->element, strlen(current->token->element));
}

/*
 * _eat_letter - eat <letter>
 */
static void _eat_letter(const node_t *current,
                        command_t *current_command, node_t *parent) {
    __eat_terminal(current, current_command, parent);
}

/*
 * _eat_num - eat <num>
 */
static void _eat_num(const node_t *current,
                     command_t *current_command, node_t *parent) {
    __eat_terminal(current, current_command, parent);
    
}

/*
 * _eat_alphanum - eat <alphanum>
 */
static void _eat_alphanum(const node_t *current,
                          command_t *current_command, node_t *parent) {
    __eat_terminal(current, current_command, parent);
}

/*
 * _eat_home - eat <home>
 */
static void _eat_home(const node_t *current,
                      command_t *current_command, node_t *parent) {
    const node_t *home = current;
    const char *word;

    word = getenv("HOME");
    if (_is_word(parent->token)) {
        strncat(parent->token->element, word, strlen(word));
    } else {
        fprintf(stderr,"error: cannot eat home %s\n", home->token->element);
    }
}

/*
 * _eat_env - eat <env>
 */
static void _eat_env(const node_t *current,
                     command_t *current_command, node_t *parent) {
    const node_t *env = current;
    const char *word;

    word = getenv(env->token->element);
    if (_is_word(parent->token)) {
        strncat(parent->token->element, word, strlen(word));
    } else {
        fprintf(stderr,"error: cannot eat env %s\n", env->token->element);
    }
}

/*
 * _eat_word - eat <word>
 */
static void _eat_word(node_t *current,
                      command_t *current_command) {
    const node_t *elh = container_of(&(current->head->left), node_t, head);
    node_t *word = container_of(&(current->head->right), node_t, head);

    if (word != NULL)  return;
    switch (word->token->spec) {
    case ENV:
        _eat_env(elh, current_command, current);
        break;
    case LETTER:
        _eat_letter(elh, current_command, current);
        break;
    case HOME:
        _eat_home(elh, current_command, current);
        break;
    }
    
    if (!current_command->command_flag) {
        strncat(current_command->cmd, current->token->element, ELEMENT_MAX);
        current_command->command_flag = true;
    } else {
        strncat(current_command->args, current->token->element, ARG_MAX);
    }

    if (word != NULL & _is_word(word->token))
        _eat_word(word, current_command);
}

/*
 * _eat_env_assignment - eat <env_assignment>
 */
static void _eat_env_assignment(const node_t *current,
                                command_t *current_command) {
    const node_t *env_assignment = current;
    
    char *assign = env_assignment->token->element;
    if (!putenv(assign))
        fprintf(stderr, "can't assgin environment variable: %s", assign);
}

/*
 * _eat_redirection_out - eat <redirection_out>
 */
static void _eat_redirection_out(const node_t *current,
                                command_t *current_command) {
    const node_t *redirection_out = current;
    const node_t *word = container_of(&(current->head->left), node_t, head);
    const char* filename;
    char *mode;
    FILE *stream;

    switch (redirection_out->token->spec) {
    case REDIRECT_OUT:
        mode = "w+";
        break;
    case REDIRECT_OUT_APPEND:
        mode = "a+";
        break;
    }

    if (!_is_abstract_node(redirection_out)) {
        const char* streamname = redirection_out->token->element;
        stream = fopen(streamname, mode);
    } else {
        stream = stdin;
    }
    filename = word->token->element;
    if (!freopen(filename, mode, stream)) {
        fprintf(stderr,
                "could not redirect stdout from file %s\n", filename);
        exit(2);
    }
}

/*
 * _eat_redirection_in - eat <redirection_in>
 */
static void _eat_redirection_in(const node_t *current,
                                command_t *current_command) {
    const node_t *redirection_in = current;
    const node_t *word = container_of(&(current->head->left), node_t, head);
    const char* filename;
    FILE *stream;
    
    if (!_is_abstract_node(redirection_in)) {
        const char* streamname = redirection_in->token->element;
        stream = fopen(streamname, "r");
    } else {
        stream = stdin;
    }
    filename = word->token->element;
    if (!freopen(filename, "r", stream)) {
        fprintf(stderr,
                "could not redirect stdin from file %s\n", filename);
        exit(2);
    }
}

/*
 * _eat_redirection - eat <redirection>
 */
static void _eat_redirection(const node_t *current,
                             command_t* current_command) {
    const node_t *redirect_in_out = 
        container_of(&(current->head->left), node_t, head);
    
    switch (redirect_in_out->token->spec) {
    case REDIRECT_IN:
        _eat_redirection_in(redirect_in_out, current_command);
        break;
    case REDIRECT_OUT: case REDIRECT_OUT_APPEND:
        _eat_redirection_out(redirect_in_out, current_command);
        break;
    }
}

/*
 * _eat_redirection_list - eat <redirect_list>
 */
static void _eat_redirection_list(const node_t *current,
                                  command_t *current_command) {
    const node_t *redirection = container_of(&(current->head->left), node_t, head);
    const node_t *redirection_list = 
        container_of(&(current->head->right), node_t, head);

    _eat_redirection(redirection, current_command);
    
    if (redirection_list != NULL && _is_redirection(redirection_list->token))
        _eat_redirection_list(redirection_list, current_command);
}

/*
 * _eat_command_element - eat <command_element>
 */
static void _eat_command_element(const node_t *current,
                                 command_t *current_command) {
    const node_t *wer = container_of(&(current->head->left), node_t, head);
    const node_t *command_element =
        container_of(&(current->head->right), node_t, head);

    if (wer == NULL)  return;
    switch (wer->token->spec) {
    case WORD:
        _eat_word((node_t *)wer, current_command);
        break;
    case ENV_ASSIGNMENT:
        _eat_env_assignment(wer, current_command);
        break;
    case REDIRECT_IN:
    case REDIRECT_OUT: case REDIRECT_OUT_APPEND:
        _eat_redirection_list(wer, current_command);
        break;
    }

    if (command_element != NULL && _is_command_element(command_element->token))
        _eat_command_element(command_element, current_command);
}

/*
 * _eat_command - eat <command>
 */
static void _eat_command(const node_t *current, command_t *current_command)
{
    const node_t *command_element, *command;
    command_element = container_of(&(current->head->left), node_t, head);
    command = container_of(&(current->head->right), node_t, head);
    
    _eat_command_element(command_element, current_command);
    if (command != NULL && _is_command(command->token))
        _eat_command(command, current_command);
}

/*
 * _eat_piped_command - eat <piped_command>
 */
static void _eat_piped_command(const node_t *current,
                               command_t *current_command, const bool head)
{
    const node_t *command, *piped_command;
    command = container_of(&(current->head->left), node_t, head);
    piped_command = container_of(&(current->head->right), node_t, head);
    int next_pipe[2];

    _eat_command(command, current_command);
    if (piped_command != NULL &&
        _is_piped_command(piped_command->token)) {
        _fork_exec(current_command, head, false);
    } else {
        _fork_exec(current_command, head, true);
    }
    _init_command(current_command);
    _eat_piped_command(piped_command, current_command, false);
}

/**
 * eat_root - execute commands in the given tree sequencially
 * @root: the root of syntax tree
 */
void eat_root(const node_t *root)
{
    command_t *current_command;
    _init_command(current_command);
    _eat_piped_command(root, current_command, true);
}