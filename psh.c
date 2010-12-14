/*
 * psh.c - naive shell with recursive descent parser
 *
 * This source code is licensed under the MIT License.
 * See the file COPYING for more details.
 *
 * @author: Taku Fukushima <tfukushima@dcl.info.waseda.ac.jp>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
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
 * _set_redirections - set redirection
 */
static void _set_redirections(command_t *current_command)
{
    int i;
    redirection_t redirection;
    FILE *oldfile;
    char *mode;
    
    for (i = 0; i < ELEMENT_MAX; i++) {
        redirection = current_command->redirection[i];
        if (strlen(redirection.dst) != 0) {
            switch (redirection.input) {
            case true:
                oldfile = stdin;
                mode = "r";
                break;
            case false:
                oldfile = stdout;
                mode = "w";
                break;
            }
            if (!freopen(redirection.dst, mode, oldfile)) {
                fprintf(stderr,
                        "could not redirect stdin from file %s\n",
                        redirection.dst);
                exit(2);
            }
        }
    }
}

/*
 * fork_chain - fork and exec while command exists and joint them with pipes
 */
static int _fork_chain(parser_t *p, int i, int prev_pipe_in)
{
    int next_pipe[2];
    pid_t fork_result, child;
    command_t current_command;

    if (i >= 0 && i < p->p) {
        if (pipe(next_pipe) == 0) {
            current_command = p->command[i];
            fork_result = fork();
            switch (fork_result) {
            case -1:
                fprintf(stderr, "fork failure");
                exit(EXIT_FAILURE);
                break;
            case 0:  // case of child
                if (i > 0 && i <= p->p) {
                    dup2(prev_pipe_in, 0);
                    close(prev_pipe_in);
                }
                // redirection
                _set_redirections(&(current_command));
                if (i + 1 < p->p && i >= 0 && i < p->p)
                    dup2(next_pipe[1], 1);
                close(next_pipe[1]);
                execlp(current_command.cmd, current_command.cmd,
                       current_command.args, 0);
                exit(EXIT_SUCCESS);
                break;
            default:  // case of parent
                child = wait((int *) 0);
                break;
            }
        } else {
            fprintf(stderr, "could not create pipe.\n");
            exit(2);
        }
    }
    
    return next_pipe[0];
}

static void fork_chain(parser_t *p)
{
    int i, prev_pipe_in;
    
    for (i = 0; i < p->p; i++) {
        prev_pipe_in = _fork_chain(p, i, prev_pipe_in);
    }
    close(prev_pipe_in);
}
    
int main(int argc, char **argv)
{
    int i;
    tokenizer_t *t;
    parser_t *p;
    char input[INPUT_MAX];
    
    say_hello();
    printf("[0;32mpsh-$ [0;37m");
    while (fgets(input, INPUT_MAX, stdin) != EOF) {
        t = init_tokenizer(input);
        p = init_parser();
        parse_input(p, t);
        fork_chain(p);
        printf("[0;32mpsh-$ [0;37m");
    }
    exit(EXIT_SUCCESS);
}
