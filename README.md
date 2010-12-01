psh - Naive shell with recursive descent parser
================================================

Requirements
-------------

1. Enable to redirect
   - e.g.) `<command> > <filename>`
2. Enable to pipe
   - e.g.) `<command1> | <command2>`
   - Pipe should enable to be nested
     + e.g.) `<command1> | <command2> | <command3>`
     + Depth of nests can be limited by realistic restriction
3. Enable to interpret `~` as a part of a path
4. Redirect and pipe can be recognized even if they have no space arround them
5. Enable to treat with both absolute and relative path
6. Enable to read environment variables


Options
--------

1. Enable to modify environment variables (5 points)
2. Enable to interpret back quoted commands and return its result to outer command (20 points)
3. Enable to interpret `if` statements and loops (15 points)


BNF syntax for shell inputs
----------------------------

        command_sequence := piped_commands [ '|' command_sequence ]
        piped_commands := command_line [ redirect_in ]* [ redirect_out ]*
        rediret_in := [ fd ] '<' [ '&' ][ fd ]
        rediret_out := [ fd ] '>' [ '>'] [ '&' ][ fd ]
        command_line := command [ option ]*
        command := symbol 
        option := symbol

        nonzero := [ '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ]
        zero := [ '0' ] 
        num := zero | nonzero
        fd := nonzero [ num ]
        alpha := 'a'  - 'z' | 'A' - 'Z'
        alphanum := alpha | num
        special := '!' | '"' | '#' | '$' | '%' | ''' | '(' | ')' | '*' | '+' | ',' |
                   '-' | '.' | '/' | ':' | ';" | '=' | '?' | '@' | '[' | ']' |
                   '\' | '^' | '_' | '`' | '{' | '|' | '}' 
        home := '~'
        symbol := alphanum | special | home
        
