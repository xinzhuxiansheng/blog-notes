# Antlr4 - 利用访问器构造一个计算器         

****
```bash
grammar LabeledExpr;

prog:   stat+ ;

stat:   CLEAR NEWLINE               # clear
    |   expr NEWLINE                # printExpr
    |   ID '=' expr NEWLINE         # assign
    |   NEWLINE                     # blank
    ;

expr:   expr op=('*'|'/') expr      # MulDiv
    |   expr op=('+'|'-') expr      # AddSub
    |   INT                         # int
    |   ID                          # id
    |   '(' expr ')'                # parens
    ;


MUL :   '*' ; // assigns token name to '*' used above in grammar
DIV :   '/' ;
ADD :   '+' ;
SUB :   '-' ;
CLEAR: 'clear';
ID  :   [a-zA-Z]+? ;      // match identifiers
INT :   [0-9]+ ;         // match integers
NEWLINE: '\r'? '\n'
       | EOF;     // return newlines to parser (is end-statement signal)
WS  :   [ \t]+ -> skip ; // toss out whitespace
```


refer   
1.《ANTLR 4 权威指南》     