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

#include "consts.h"
#include "executor.h"

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

static void finalize(tokenizer_t *t, parser_t *p, node_t *root)
{
    free(t);
    free(p);
    free_nodes(root);
}
    
int main(int argc, char **argv)
{
    tokenizer_t *t;
    parser_t *p;
    node_t *root;
    char input[INPUT_MAX];
    
    say_hello();
    printf("[0;32mpsh-$[0;37m ");
    while (*fgets(input, INPUT_MAX, stdin)) {
        t = init_tokenizer(input);
        p = init_parser();
        root = (node_t *)parse_input(p, t);
        eat_root(root);
        finalize(t, p, root);
        printf("[0;32mpsh-$[0;37m" );
    }
    return 0;
}
