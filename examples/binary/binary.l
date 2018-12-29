
%{
%}

%option noyywrap

%%
a             { return *yytext; }
.             { /* do nothing */ }
%%
