
%{
#define ID 1
#define NUM 2
#define LOG 3
#define SIN 4
#define COS 5
#define TAN 6
%}

num    [0-9]*([0-9]|\.[0-9]|[0-9]\.)[0-9]*
id     [a-zA-Z]+

%option noyywrap

%%
[-+*/=()^;\n]   { return *yytext; }
{num}           { return NUM; }
log             { return LOG; }
sin             { return SIN; }
cos             { return COS; }
tan             { return TAN; }
{id}            { return ID; }
.               /* do nothing  */   
%%
