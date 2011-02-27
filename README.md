psh - Naive shell with recursive descent parser
================================================

Exercise on C and system programming.


Prerequisites
-------------

* [GNU readline](http://cnswww.cns.cwru.edu/php/chet/readline/)


Build
------

        $ make clean; make all
        $ ./psh

Goal
-----

1. Enable to redirect
   - e.g.) `<command> > <filename>`
2. Enable to pipe
   - e.g.) `<command 1> | <command 2>`
   - Pipe should enable to be nested
     + e.g.) `<command 1> | <command 2> | <command 3> | ... <command n>`
     + Depth of nests can be limited by realistic restriction
3. Enable to interpret `~` as a part of paths
4. Redirect and pipe can be recognized even if they have no space arround them
5. Enable to deal with both absolute and relative path
6. Enable to read environment variables


Options
--------

1. Enable to modify environment variables (5 points)
2. Enable to interpret back quoted commands and return its result to outer command (20 points)
3. Enable to interpret `if` statements and loops (15 points)


Shell input syntax in BNF
---------------------------

        <piped_commands> ::= <command> { '|' <piped_commands> }
        <command> ::= <command_element> { <command> }
        <redirect_in> ::=  { <num> } '<' { '&' } <word>
        <redirect_out> ::= { <num> } '>' { ('>' | '&') } <word>
        <redirection> ::= <redirect_in>
                        | <redirect_out>
        <redirection_list> ::= <redirection> { <redirection_list> } 
        <command_element> ::= <word>
                            | <env_assignment>
                            | <redirection_list>

        <digit> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' 
        <alpha> ::= 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' 
                  | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' 
                  | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' 
                  | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' 
                  | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' 
                  | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' 
        <alphanum> ::= (<alpha> | <num> | '_' ) { <alphanum> }
        <special> ::= '!' | '"' | '#' | '%' | ''' | '(' | ')' | '*' | '+' | ',' 
                    | '-' | '.' | '/' | ':' | ';" | '?' | '@' | '[' | ']' | '&' 
                    | '\' | '^' | '_' | '`' | '{' | '|' | '}' 
        <word> ::= (<env> | <letter> | <home>) { <word> }
        <home> ::= '~' { <alphanum> }
        <env> ::= '$' { '{' } <word> { '}' }
        <num> ::= <digit> { <num> }
        <env_assignment> ::= <alphanum> '=' <word>
        <letter> ::= (<alphanum> | <special>) { <letter> }
        

Copyright
---------

Copyright (c) 2010 Taku Fukushima. All rights reserved.


License
--------

psh is [MIT Lisense](http://www.opensource.org/licenses/mit-license.php)
