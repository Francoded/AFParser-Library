
%{
#define NUM 2
%}

num    [0-9]*([0-9]|\.[0-9]|[0-9]\.)[0-9]*

%option noyywrap

%%
[-+*/=()\nqQ] { return *yytext; }
{num}         { return NUM; }
.             { /* do nothing */ }
%%
