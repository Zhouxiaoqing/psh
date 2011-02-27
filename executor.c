/*
 * executor.c - execute parsed tree
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "executor.h"

static int _fork_exec(command_t *current_command,
                      const bool head_flag, const bool tail_flag, node_t *root);
static void _eat_letter(const node_t *current,
                        command_t *current_command, node_t *parent, node_t *root);
static void _eat_num(const node_t *current,
                     command_t *current_command, node_t *parent, node_t *root);
static void _eat_alphanum(const node_t *current,
                          command_t *current_command, node_t *parent, node_t *root);
static void _eat_home(const node_t *current,
                      command_t *current_command, node_t *parent, node_t *root);
static void _eat_env(const node_t *current,
                     command_t *current_command, node_t *parent, node_t *root);
static void _eat_word(node_t *current, command_t *current_command, bool fstflag, node_t *root);
static void _eat_env_assignment(const node_t *current,
                                command_t *current_command, node_t *root);
static void _eat_redirection_out(node_t *current, command_t *current_command,
                                 node_t *root);
static void _eat_redirection_in(node_t *current, command_t *current_command,
                                node_t *root);
static void _eat_redirection(const node_t *current, command_t *current_command,
                             node_t *root);
static void _eat_redirection_list(const node_t *current,
                                  command_t *current_command, node_t *root);
static void _eat_command_element(const node_t *current,
                                 command_t *current_command, node_t *root);
static void _eat_command(const node_t *current, command_t *current_command,
                         node_t *root);;
static void _eat_piped_command(const node_t *current,
                               command_t *current_command, const bool head,
                               node_t *root);
/**
 * print_error - print error message and finalize program
 */
void print_error(const char *error_message, node_t *root)
{
    fprintf(stderr, error_message);
    free_nodes(root, root);
    exit(EXIT_FAILURE);
}

/*
 * _fork_exec - fork and exec
 */
static int _fork_exec(command_t *current_command,
                      const bool head_flag, const bool tail_flag, node_t *root)
{
    pid_t fork_result, child;
    int next_pipe[2];

    if (pipe(next_pipe) == 0) {
        fork_result = fork();
        switch (fork_result) {
        case -1:
            print_error("fork failure", root);
            break;
        case 0:  // case of child
            if (!head_flag) {
                dup2(current_command->input_fd, STDIN_FILENO);
                close(current_command->input_fd);
            }            
            if (tail_flag) {
                close(next_pipe[1]);
                current_command->output_fd = open("/dev/stdout", O_WRONLY);
            } else {
                current_command->output_fd = next_pipe[1];
            }
            dup2(current_command->output_fd, STDOUT_FILENO);
            close(current_command->output_fd);
            execvp(current_command->cmd, current_command->argv);
            print_error("psh: command not found.\n", root);
            break;
        default:  // case of parent
            close(next_pipe[1]);
            child = wait(NULL);
            if (!tail_flag)
                current_command->input_fd = next_pipe[0];
            break;
        }
        
    } else {
        print_error("psh: pipe creation failure", root);
    }

    return 0;
}


/*
 * _eat_terminal - eat <letter>, <num> or <alphanum>
 */
static inline void __eat_terminal(const node_t *current,
                                  command_t *current_command, node_t *parent,
                                  node_t *root) {
    strncat(parent->token->element,
            current->token->element, strlen(current->token->element));
}

/*
 * _eat_letter - eat <letter>
 */
static void _eat_letter(const node_t *current,
                        command_t *current_command, node_t *parent,
                        node_t *root) {
    __eat_terminal(current, current_command, parent, root);
}

/*
 * _eat_num - eat <num>
 */
static void _eat_num(const node_t *current,
                     command_t *current_command, node_t *parent,
                     node_t *root) {
    __eat_terminal(current, current_command, parent, root);
    
}

/*
 * _eat_alphanum - eat <alphanum>
 */
static void _eat_alphanum(const node_t *current,
                          command_t *current_command, node_t *parent,
                          node_t *root) {
    __eat_terminal(current, current_command, parent, root);
}

/*
 * _eat_home - eat <home>
 */
static void _eat_home(const node_t *current,
                      command_t *current_command, node_t *parent,
                      node_t *root) {
    const node_t *home = current;
    const char *word;

    word = getenv("HOME");
    if (_is_word(parent->token)) {
        strncat(parent->token->element, word, strlen(word));
    } else {
        print_error("cannot eat home.", root);
    }
}

/*
 * _eat_env - eat <env>
 */
static void _eat_env(const node_t *current,
                     command_t *current_command, node_t *parent,
                     node_t *root) {
    const node_t *env = current;
    const char *word;

    word = getenv(env->token->element);
    if (word == NULL)  return;
    if (_is_word(parent->token)) {
        strncat(parent->token->element, word, strlen(word));
    } else {
        fprintf(stderr,"psh: error: cannot eat env %s\n", env->token->element);
    }
}

/*
 * _eat_word - eat <word>
 */
static void _eat_word(node_t *current, command_t *current_command,
                      bool fstflag, node_t *root) {
    const node_t *elh = current->left;  // container_of(&(current->head->left), node_t, head);
    node_t *word = current->right;  // container_of(&(current->head->right), node_t, head);

    if (elh == NULL)  return;
    switch (elh->token->spec) {
    case ENV: case ENV_WORD:
        _eat_env(elh, current_command, current, root);
        break;
    case LETTER:
        _eat_letter(elh, current_command, current, root);
        break;
    case ALPHANUM:
        _eat_alphanum(elh, current_command, current, root);
        break;
    case NUM:
        _eat_num(elh, current_command, current, root);
        break;
    case HOME: case HOME_WORD:
        _eat_home(elh, current_command, current, root);
        break;
    default:
        break;
    }
    
    if (word != NULL && _is_word(word->token)) {
        _eat_word(word, current_command, false, root);
        strncat(current->token->element,
                word->token->element, strlen(word->token->element));
    }
    
    if (fstflag) {
        if (!current_command->command_flag) {
            strncat(current_command->cmd, current->token->element, ELEMENT_MAX);
            if (current_command->argc < ARG_MAX)
                current_command->argv[current_command->argc++] = current->token->element;
            else
                print_error("psh: too many arguments.", root);
            current_command->command_flag = true;
        } else {
            if (current_command->argc < ARG_MAX)
                current_command->argv[current_command->argc++] = current->token->element;
            else
                print_error("psh: too many arguments.", root);
        }
    }
}

/*
 * _eat_env_assignment - eat <env_assignment>
 */
static void _eat_env_assignment(const node_t *current,
                                command_t *current_command, node_t *root) {
    const node_t *env_assignment = current;
    
    char *assign = env_assignment->token->element;
    if (!putenv(assign))
        print_error("psh: can't assgin environment variable.\n", root);
}

/*
 * _eat_redirection_out - eat <redirection_out>
 */
static void _eat_redirection_out(node_t *current,
                                 command_t *current_command, node_t *root) {
    const node_t *redirection_out = current;
    const node_t *word = current->left;  // container_of(&(current->head->left), node_t, head);
    const char* filename;
    char *mode;
    FILE *stream;

    switch (redirection_out->token->spec) {
    case REDIRECT_OUT: case REDIRECT_OUT_COMPOSITION:
        mode = "w";
        break;
    case REDIRECT_OUT_APPEND:
        mode = "a";
        break;
    default:
        break;
    }

    if (!_is_abstract_node(redirection_out)) {
        const char* streamname = redirection_out->token->element;
        int streamfd = atoi(streamname);
        stream = fdopen(streamfd, mode);
    } else {
        stream = stdout;
    }
    current->oldstreamfd = fcntl(fileno(stream), F_DUPFD, STDIN_FILENO);
    filename = word->token->element;

    if (redirection_out->token->spec == REDIRECT_OUT_COMPOSITION) {
        int redirectfd = atoi(filename);
        dup2(redirectfd, fileno(stream));
        fclose(stream);
    } else {
        if (!freopen(filename, mode, stream)) {
            print_error("psh: could not redirect stdout from file.\n", root);
        }
    }
}

/*
 * _eat_redirection_in - eat <redirection_in>
 */
static void _eat_redirection_in(node_t *current,
                                command_t *current_command, node_t *root) {
    const node_t *redirection_in = current;
    const node_t *word = current->left;  // container_of(&(current->head->left), node_t, head);
    const char* filename;
    FILE *stream;
    char *mode;
    
    if (!_is_abstract_node(redirection_in)) {
        const char* streamname = redirection_in->token->element;
        int streamfd = atoi(streamname);
        stream = fdopen(streamfd, "r");
    } else {
        stream = stdin;
    }
    
    switch (redirection_in->token->spec) {
    case REDIRECT_IN:
        mode = "r";
        break;
    case REDIRECT_IN_OUT:
        mode = "r+";
    default:  break;
    }
    
    current->oldstreamfd = fcntl(fileno(stream), F_DUPFD, STDIN_FILENO);
    filename = word->token->element;

    if (!freopen(filename, mode, stream)) {
        print_error("psh: could not redirect stdin from file. \n", root);
    }
}

/*
 * _eat_redirection - eat <redirection>
 */
static void _eat_redirection(const node_t *current,
                             command_t* current_command, node_t *root) {
    node_t *redirect_in_out = 
        current->left;  // container_of(&(current->head->left), node_t, head);
    
    switch (redirect_in_out->token->spec) {
    case REDIRECT_IN_PATTERN:
        _eat_redirection_in(redirect_in_out, current_command, root);
        break;
    case REDIRECT_OUT_PATTERN:
        _eat_redirection_out(redirect_in_out, current_command, root);
        break;
    default:
        break;
    }
}

/*
 * _eat_redirection_list - eat <redirect_list>
 */
static void _eat_redirection_list(const node_t *current,
                                  command_t *current_command, node_t *root) {
    const node_t *redirection = current->left;  // container_of(&(current->head->left), node_t, head);
    const node_t *redirection_list = 
        current->right;  // container_of(&(current->head->right), node_t, head);

    _eat_redirection(redirection, current_command, root);
    
    if (redirection_list != NULL && _is_redirection(redirection_list->token))
        _eat_redirection_list(redirection_list, current_command, root);
}

/*
 * _eat_command_element - eat <command_element>
 */
static void _eat_command_element(const node_t *current,
                                 command_t *current_command, node_t *root) {
    const node_t *wer = current->left;  // container_of(&(current->head->left), node_t, head);
    const node_t *command_element =
        current->right;  // container_of(&(current->head->right), node_t, head);

    if (wer == NULL)  return;
    switch (wer->token->spec) {
    case WORD_PATTERN:
        _eat_word((node_t *)wer, current_command, true, root);
        break;
    case ENV_ASSIGNMENT:
        _eat_env_assignment(wer, current_command, root);
        break;
    case REDIRECT_PATTERN:
        _eat_redirection_list(wer, current_command, root);
        break;
    default:
        break;
    }

    if (command_element != NULL && _is_command_element(command_element->token))
        _eat_command_element(command_element, current_command, root);
}

/*
 * _eat_command - eat <command>
 */
static void _eat_command(const node_t *current, command_t *current_command,
                         node_t *root)
{
    const node_t *command_element, *command;
    command_element = current->left;  // container_of(&(current->head->left), node_t, head);
    command = current->right;  // container_of(&(current->head->right), node_t, head);
    
    _eat_command_element(command_element, current_command, root);
    if (command != NULL && _is_command(command->token))
        _eat_command(command, current_command, root);
}

/*
 * _eat_piped_command - eat <piped_command>
 */
static void _eat_piped_command(const node_t *current, command_t *current_command,
                               const bool head_flag, node_t *root)
{
    const node_t *command_element, *command;
    command_element = current->left;  // container_of(&(current->head->left), node_t, head);
    if (_is_eof(command_element->token) || _is_eol(command_element->token))
        return ;
    command = current->right;  // container_of(&(current->head->right), node_t, head);
    _eat_command_element(command_element, current_command, root);
    if (command != NULL && _is_command(command->token)) {
        _fork_exec(current_command, head_flag, false, root);
        _init_command(current_command);
        _eat_piped_command(command, current_command, false, root);
    } else {
        _fork_exec(current_command, head_flag, true, root);
    }
}

/**
 * eat_root - execute commands in the given tree sequencially
 * @root: the root of syntax tree
 */
void eat_root(node_t *root)
{
    command_t *current_command = init_command(root);
    _eat_piped_command(root, current_command, true, root);
    free(current_command);
}
