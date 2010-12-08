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
static void fork_chain(tokenizer_t *t, int i)
{
    int file_pipes[2];
    pid_t fork_result;
    
    command_t current_command;
    command_t next_command;
    redirection_t redirect_in;
    redirection_t redirect_out;

    current_command = t->command[i];
    if (i < PIPE_MAX)
        next_command = t->command[i+1];
    if (i < t->p && strlen(current_command.cmd) != 0) {
        if (pipe(file_pipes) == 0) {
            fork_result = fork();
            switch (fork_result) {
            case -1:
                fprintf(stderr, "Fork failure");
                exit(EXIT_FAILURE);
                break;
            case 0:  // case of child
                close(0);
                dup(file_pipes[0]);
                close(file_pipes[0]);
                if (i + 1 < t->p) {
                    close(1);
                    dup(file_pipes[1]);
                }
                close(file_pipes[1]);
                
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
                
                execlp(current_command.cmd, current_command.cmd,
                       current_command.args,(char *) 0);
                fork_chain(t, i+1);
                exit(EXIT_SUCCESS);
                break;
            default:  // case of parent
                waitpid(fork_result, (int *) 0, WNOHANG);
                close(file_pipes[0]);
                close(file_pipes[1]); 
                break;
            }
        }
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
        fork_chain(t, 0);
        printf("[0;32;40mpsh-$ [0;37;40m");
    }
    exit(EXIT_SUCCESS);
}
