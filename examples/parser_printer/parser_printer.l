
%{
%}

%option noyywrap

%%

[abcd]     { return *yytext; }
.          { /* do nothing */ }

%%
