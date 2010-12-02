psh - Naive shell with recursive descent parser
================================================

Exercise on C and system programming

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

        <command_sequence> ::= <piped_command> { '|' <command_sequence> }
        <piped_command> ::= <command_line> { <redirect_in> } { <redirect_out> } { <option> }
        <redirect_in> ::=  { <num> } '<' { '&' }{ <num> } { <redirect_in> }
        <redirect_out> ::= { <num> } '>' { ('>' | '&') }{ <num> } { <redirect_out> }
        <command_line> ::= { <env_assignment> } <command> { <option> }
        <command> ::= <word> 
        <option> ::= <word> | <env_assignment> { <option> }

        <digit> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' 
        <alpha> ::= 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' 
                  | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' 
                  | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' 
                  | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' 
                  | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' 
                  | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z' 
        <alphanum> ::= (<alpha> | <num>) { <alphanum> }
        <num> ::= <digit> | <digit> <num>
        <special> ::= '!' | '"' | '#' | '$' | '%' | ''' | '(' | ')' | '*' | '+' | ',' 
                    | '-' | '.' | '/' | ':' | ';" | '?' | '@' | '[' | ']' | '&' 
                    | '\' | '^' | '_' | '`' | '{' | '|' | '}' 
        <home> ::= '~'
        <env> ::= '$' <word>
        <env_assignment> ::= <word> '=' <word>
        <word> ::= (<alphanum> | <special> | <home>) { (<word> | <env> }
        

Copyright
---------

Copyright (c) 2010 Taku Fukushima. All rights reserved.
