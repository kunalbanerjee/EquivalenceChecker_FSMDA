/* FSMD Lexer */

D			[0-9]
L			[a-zA-Z_]


%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fsmda.tab.h"

#define COPY_STRING { yylval.string=(char*)malloc((yyleng+1)*(sizeof(char))); strncpy(yylval.string, yytext, yyleng + 1); }

void count();
void comment_multiple_line();
void comment_single_line();
%}

%%
"/*"			{ comment_multiple_line(); }
"//"			{ comment_single_line(); }

"read"			{ count(); COPY_STRING return(READ); }
"write"			{ count(); COPY_STRING return(OUT);  }


{L}({L}|{D})*	{ count(); COPY_STRING return(IDENTIFIER); }
{D}+			{ count(); COPY_STRING return(CONSTANT);   }


\"(\\.|[^\\"])*\"	{ count(); COPY_STRING return(STRING_LITERAL); }


"&&"		{ count(); COPY_STRING return(AND); }
"||"		{ count(); COPY_STRING return(OR);  }
"!"			{ count(); COPY_STRING return(NOT); }

"<="		{ count(); COPY_STRING return(LE_OP); }
">="		{ count(); COPY_STRING return(GE_OP); }
"=="		{ count(); COPY_STRING return(EQ_OP); }
"!="		{ count(); COPY_STRING return(NE_OP); }
"<"			{ count(); COPY_STRING return(LT_OP); }
">"			{ count(); COPY_STRING return(GT_OP); }

","			{ count(); COPY_STRING return(COMMA); }
"="			{ count(); COPY_STRING return(EQUAL); }

"("			{ count(); COPY_STRING return(LEFT_BR);  }
")"			{ count(); COPY_STRING return(RIGHT_BR); }

"["			{ count(); COPY_STRING return(LEFT_SQBR);  }
"]"			{ count(); COPY_STRING return(RIGHT_SQBR); }

"-"			{ count(); COPY_STRING return(MINUS); }
"+"			{ count(); COPY_STRING return(PLUS);  }
"*"			{ count(); COPY_STRING return(MULT);  }
"/"			{ count(); COPY_STRING return(DIV);   }
"%"			{ count(); COPY_STRING return(MOD);   }

"|"			{ count(); COPY_STRING return(SEPARATOR); }
";"			{ count(); COPY_STRING return(SEMIC); }


[ \t\v\n\f]		{ count(); }
.			{ /* ignore bad characters */ }

%%

int yywrap()
{
	return(1);
}


void comment_multiple_line()
{
	//for handling multiple line /*comments*/
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
	{
		;
	}
	if( c != 0 )
		c1 = input();
	if( c1 != 0 && c1 != '/' )
		goto loop;
}

void comment_single_line()
{
	//for handling //one line comments
	char c;

	while((c=input())!='\n' && c!=0)
		;
}

int column = 0;

void count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

	//Uncomment the following command to detect syntax errors
	//ECHO;
}

