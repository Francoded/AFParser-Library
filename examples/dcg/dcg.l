
%{
#define NUM 2
%}

exp    [Ee][-+]?[0-9]+
num    [0-9]*([0-9]|\.[0-9]|[0-9]\.)[0-9]*{exp}?
%option noyywrap

%%
{num}j?       { return NUM; }
[-+*/=()]     { return *yytext; }
[ \t\r\n]     // ignore space
.             { fprintf(stderr, "mystery character"); }

%%
