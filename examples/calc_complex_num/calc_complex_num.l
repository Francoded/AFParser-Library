
%{
#define NUM 2
%}

num    [0-9]*([0-9]|\.[0-9]|[0-9]\.)[0-9]*
%option noyywrap

%%
{num}j?         { return NUM; }
[-+*/=()\nqQ]   { return *yytext; }
.               /* do nothing */
%%
