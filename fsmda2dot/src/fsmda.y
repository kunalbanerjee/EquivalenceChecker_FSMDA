/* FSMD Parser */

%{

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 #include "parserDot.h"

 FILE *fp;
 int i;
%}

%union{
	char* string;
	struct SubStatement* sub_stmt;
	struct SubStatement_List* sub_stmt_list;
	}

%token <string> IDENTIFIER CONSTANT STRING_LITERAL
%token <string> LE_OP GE_OP EQ_OP NE_OP
%token <string> AND OR NOT

%token <string> READ OUT

%token <string> EQUAL COMMA SEMIC SEPARATOR
%token <string> LT_OP GT_OP LEFT_BR RIGHT_BR LEFT_SQBR RIGHT_SQBR

%type <string> variable relational_operator state_name fsmd_name


%type <string> primary_expression array_expression unary_expression multiplicative_expression expression
%type <string> primary_conditional_expression secondary_conditional_expression conditional_expression
%type <string> argument_list uninterpreted_function function_name
%type <string> assignment_expression operation_list operations 

%type <sub_stmt> substatement 
%type <sub_stmt_list> substatement_list


%left MULT DIV MOD
%left PLUS MINUS

%start translation_fsmd
%%

variable
	: IDENTIFIER	{ strcpy($$,$1); }
	;

variable_list
	: variable
	| variable_list variable
	;

primary_expression
	: variable						{ strcpy($$,$1); }
	| CONSTANT						{ strcpy($$,$1); }
	| LEFT_BR expression RIGHT_BR	{ strcpy($$,"("); strcat($$,$2); strcat($$,")"); }
	| uninterpreted_function		{ strcpy($$,$1); }
	| array_expression				{ strcpy($$,$1); }
	;

array_expression
	: variable LEFT_SQBR expression RIGHT_SQBR				{ strcpy($$,$1); strcat($$,"["); strcat($$,$3); strcat($$,"]"); }
	| array_expression LEFT_SQBR expression RIGHT_SQBR		{ strcpy($$,$1); strcat($$,"["); strcat($$,$3); strcat($$,"]"); }
	;

unary_expression
	: primary_expression			{ strcpy($$,$1); }
	| MINUS primary_expression		{ strcpy($$,"-"); strcat($$,$2); } 
	;

multiplicative_expression
	: unary_expression									{ strcpy($$,$1); }
	| multiplicative_expression MULT unary_expression	{ strcpy($$,$1); strcat($$,"*"); strcat($$,$3); }
	| multiplicative_expression DIV unary_expression	{ strcpy($$,$1); strcat($$,"/"); strcat($$,$3); }
	| multiplicative_expression MOD unary_expression	{ strcpy($$,$1); strcat($$,"%"); strcat($$,$3); }
	;

expression
	: multiplicative_expression							{ strcpy($$,$1); }
	| expression PLUS multiplicative_expression			{ strcpy($$,$1); strcat($$,"+"); strcat($$,$3); }
	| expression MINUS multiplicative_expression		{ strcpy($$,$1); strcat($$,"-"); strcat($$,$3); }
	;

assignment_expression
	: variable EQUAL expression							{ strcpy($$,$1); strcat($$,"="); strcat($$,$3); }
	| array_expression EQUAL expression 				{ strcpy($$,$1); strcat($$,"="); strcat($$,$3); }			
	| READ LEFT_BR variable COMMA variable RIGHT_BR		{ strcpy($$,"read("); strcat($$,$3); strcat($$,","); strcat($$,$5); strcat($$,")");/* port,  var: var  = port */ }
	| OUT LEFT_BR variable COMMA expression RIGHT_BR	{ strcpy($$,"OUT("); strcat($$,$3); strcat($$,","); strcat($$,$5); strcat($$,")"); /* port, expr: port = expr */ }
	;
	
relational_operator
	: LT_OP		{ strcpy($$,$1); }
	| GT_OP		{ strcpy($$,$1); }
	| LE_OP		{ strcpy($$,$1); }
	| GE_OP		{ strcpy($$,$1); }
	| EQ_OP		{ strcpy($$,$1); }
	| NE_OP		{ strcpy($$,$1); }
	;

primary_conditional_expression
	: expression relational_operator expression			{ strcpy($$,$1); strcat($$,$2); strcat($$,$3); }
	| NOT expression relational_operator expression		{ strcpy($$,"!"); strcat($$,$2); strcat($$,$3); strcat($$,$4); }
	;
	
secondary_conditional_expression
	: primary_conditional_expression										{ strcpy($$,$1); }
	| secondary_conditional_expression AND primary_conditional_expression	{ strcpy($$,$1); strcat($$,"&&"); strcat($$,$3); }
	| secondary_conditional_expression OR primary_conditional_expression	{ strcpy($$,$1); strcat($$,"||"); strcat($$,$3); }
	| LEFT_BR secondary_conditional_expression RIGHT_BR						{ strcpy($$,"("); strcat($$,$2); strcat($$,")"); }
	| NOT LEFT_BR secondary_conditional_expression RIGHT_BR					{ strcpy($$,"!"); strcat($$,"("); strcat($$,$3); strcat($$,")"); }
	;
	
conditional_expression
	: MINUS															{ strcpy($$,"-"); }
	| secondary_conditional_expression								{ strcpy($$,$1);  }
	;

state_name
	: IDENTIFIER	{ strcpy($$,$1); }
	;

transitions
	: CONSTANT
	;

operation_list
	: assignment_expression							{ strcpy($$,$1); }
	| operation_list COMMA assignment_expression	{ strcpy($$,$1); strcat($$,",\\n"); strcat($$,$3); }
	;
	
operations
	: MINUS				{ strcpy($$,"-"); }
	| operation_list	{ strcpy($$,$1); }
	;

substatement
	: conditional_expression SEPARATOR operations state_name	{ $$ = (SUB_STMT*)malloc(sizeof(SUB_STMT)); 
	                                                              strcpy($$->characteristics,$1); strcat($$->characteristics," | "); 
	                                                              strcat($$->characteristics,$3); strcpy($$->destState,$4); }
	;

substatement_list
	: substatement						{ $$ = (SUB_STMT_LIST*)malloc(sizeof(SUB_STMT_LIST)); $$->substmtList[0] = $1; $$->numSubstmts = 1; }
	| substatement_list substatement	{ $$->substmtList[$$->numSubstmts] = $2; ($$->numSubstmts)++; }
	;

statement
	: state_name transitions substatement_list SEMIC	{ fprintf(fp,"%s [shape=circle];\n",$1);
		                                                  for( i=0; i<($3->numSubstmts);i++ ) { 
										                      fprintf(fp,"%s [shape=circle];\n",($3->substmtList[i])->destState);
								                              fprintf(fp,"%s -> %s [label=\"%s\"];\n",$1,($3->substmtList[i])->destState,($3->substmtList[i])->characteristics);
								                              } /*end for*/ } 
	| state_name transitions SEMIC						{ /* Back to reset state */ }
	;

statement_list
	: statement	
	| statement_list statement
	;

translation_fsmd
	: fsmd_name additional_declaration statement_list
	| fsmd_name statement_list
	;

fsmd_name
	: STRING_LITERAL	{ fprintf(fp,"%s [shape=note];\n",$1); }
	;

additional_declaration
	: CONSTANT CONSTANT variable_list SEMIC	
	| CONSTANT CONSTANT SEMIC
	;

argument_list
	: expression										{ strcpy($$,$1); }
	| argument_list COMMA expression					{ strcpy($$,$1); strcat($$,","); strcat($$,$3); }
	;

uninterpreted_function
	: function_name LEFT_BR argument_list RIGHT_BR		{ strcpy($$,$1); strcat($$,"("); strcat($$,$3); strcat($$,")"); }
	;

function_name
	: IDENTIFIER		{ strcpy($$,$1); }
	;

%%


extern char yytext[];
extern int column;

extern FILE *yyin;


int yyerror(char *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}


int main ( int argc, char* argv[] )
{
	char fileName[40];
	
	if( argc != 2 )
	{
		printf("\nInadequate number of arguments provided. You must provide an FSMD input file.\n");
		exit(0);
	}
	
	strcpy(fileName,argv[1]);
	strcat(fileName,".dot");
	
	fp = fopen(fileName,"w");
	if(fp == NULL)
	{
		printf("\nFile open error\n");
		exit(0);
	}
	fprintf(fp,"digraph fsmdIndot {\n\n");
	
	printf("\nCalling Parser....");
 	printf("\nFile opening: %s\n",argv[1]);	
 		
 	yyin = fopen(argv[1],"r");
 	
 	if(yyin == NULL)
	{
 		printf("\nFile open error\n");
		exit(0);
	}
	
	 yyparse();
	 
	 fclose(yyin);

	 fprintf(fp,"\n}\n");
	 fclose(fp);

	 return 0;
}
