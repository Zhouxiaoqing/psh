/*
 * psh.c - naive shell with recursive descendent parser
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"

static const char *logo[] = {
    "==========================\n",
    "                _         \n",
    "               | |        \n",
    "      _ __  ___| |__      \n",
    "     | '_ \\/ __| '_ \\     \n",
    "     | |_) \\__ \\ | | |    \n",
    "     | .__/|___/_| |_|    \n",
    "     | |                  \n",
    "     |_|                  \n",
    "==========================\n",
};

static void dummy()
{
    int i = 0;
}

/*
 * say_hello - display greeting for user
 */
static void say_hello()
{
    time_t timer;
    timer = time(NULL);
    printf("%s", ctime(&timer));
}

/*
 * fork_chain - fork and exec while command exists and joint them with pipes
 */
static void _fork_chain(tokenizer_t *t, int i, int *pipes)
{
    int fork_pipes[2];
    pid_t fork_result, child;
    
    command_t current_command;
    redirection_t redirect_in, redirect_out;
    
    if (i > 0 && i <= t->p) {
        current_command = t->command[i-1];
        if (i > 1)
            dup2(pipes[0], 0);
        close(pipes[0]);
        fork_result = fork();
        switch (fork_result) {
        case -1:
            fprintf(stderr, "fork failure");
            exit(EXIT_FAILURE);
            break;
        case 0:  // case of child
            if (i >= 0 && i < t->p) {
                dup2(pipes[1], 1);
                close(pipes[1]);
            }
            // Input redirection
            redirect_in = current_command.redirection[0];
            if (strlen(redirect_in.filename) != 0 &&
                redirect_in.input == true ) {
                if (!freopen(redirect_in.filename, "r", stdin)) {
                    fprintf(stderr,
                            "could not redirect stdin from file %s\n",
                            redirect_in.filename);
                    exit(2);
                }
            }
            // Output redirection
            redirect_out = current_command.redirection[1];
            if (strlen(redirect_out.filename) != 0 &&
                redirect_out.input == false ) {
                if (!freopen(redirect_out.filename, "w", stdout)) {
                    fprintf(stderr,
                            "could not redirect stdout from file %s\n",
                            redirect_out.filename);
                    exit(2);
                }
            }
            if (i > 1) {
                if (pipe(fork_pipes) == 0) {
                    _fork_chain(t, i-1, pipes);
                } else {
                    fprintf(stderr,
                            "could not create pipes.\n");
                        exit(2);
                }
            }
            execlp(current_command.cmd, current_command.cmd,
                   current_command.args,(char *) 0);
            exit(EXIT_SUCCESS);
            break;
        default:  // case of parent
            child = wait((int *) 0);
            break;
        }
    }
}

static void fork_chain(tokenizer_t *t)
{
    int pipes[2];
    
    if (pipe(pipes) == 0) {
        _fork_chain(t, t->p, pipes);
    } else {
        fprintf(stderr,
                "could not create pipes.\n");
        exit(2);
    }
}
    
int main(int argc, char **argv)
{
    int i;
    tokenizer_t *t;
    char input[INPUT_MAX];
    
    say_hello();
    printf("[0;32;40mpsh-$ [0;37;40m");
    while (fgets(input, INPUT_MAX, stdin) != EOF) {
        t = init_tokenizer(input);
        parse_input(t);
        dummy();
        fork_chain(t);
        printf("[0;32;40mpsh-$ [0;37;40m");
    }
    exit(EXIT_SUCCESS);
}
