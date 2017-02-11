/* FSMD Parser */

%{

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

 #include "FsmdaHeader.h"
 #include "parser.h"

%}

%union{
	char* string;
	struct normalized_cell* norm;
	struct Assign* data_trans;
	struct Assign_list* dt_list;
	struct Substatement* substmt;
	struct Substatement_list* substmt_list;
	struct Statement* stmt;
	struct Statement_list* stmt_list;
	}

%token <string> IDENTIFIER CONSTANT STRING_LITERAL
%token <string> LE_OP GE_OP EQ_OP NE_OP
%token <string> AND OR NOT

%token <string> READ OUT

%token <string> EQUAL COMMA SEMIC SEPARATOR
%token <string> LT_OP GT_OP LEFT_BR RIGHT_BR LEFT_SQBR RIGHT_SQBR

%type <string> variable relational_operator state_name
%type <string> fsmd_name
%type <string> transitions

%type <norm> primary_expression array_expression unary_expression multiplicative_expression expression
%type <norm> primary_conditional_expression secondary_conditional_expression conditional_expression
%type <norm> argument_list uninterpreted_function
%type <norm> function_name

%type <data_trans> assignment_expression
%type <dt_list> operation_list operations
%type <substmt> substatement
%type <substmt_list> substatement_list
%type <stmt> statement
%type <stmt_list> statement_list


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
	: variable						{ $$ = createVariable($1); }
	| CONSTANT						{ $$ = createConstant($1); }
	| LEFT_BR expression RIGHT_BR	{ $$ = $2; }
	| uninterpreted_function		{ $$ = $1; }
	| array_expression				{ $$ = $1; }
	;

array_expression
	: variable LEFT_SQBR expression RIGHT_SQBR				{ $$ = createArray($1, $3);       }
	| array_expression LEFT_SQBR expression RIGHT_SQBR		{ $$ = addArrayDimension($1, $3); }
	;

unary_expression
	: primary_expression			{ $$ = $1; }
	| MINUS primary_expression		{ $$ = negateExpression($2); }
	;

multiplicative_expression
	: unary_expression									{ $$ = $1; }
	| multiplicative_expression MULT unary_expression	{ $$ = Mult_Sum_With_Sum($1, $3, $$); }
	| multiplicative_expression DIV unary_expression	{ $$ = createExpression_Mod_Div($1, $3, '/'); }
	| multiplicative_expression MOD unary_expression	{ $$ = createExpression_Mod_Div($1, $3, '%'); }
	;

expression
	: multiplicative_expression							{ $$ = $1; }
	| expression PLUS multiplicative_expression			{ $$ = Add_Sums($1, $3, $$); }
	| expression MINUS multiplicative_expression		{ $$ = Add_Sums($1, negateExpression($3), $$); }
	;

assignment_expression
	: variable EQUAL expression							{ $$ = createDataTransVar($1, $3);   }
	| array_expression EQUAL expression 				{ $$ = createDataTransArray($1, $3); }
	| READ LEFT_BR variable COMMA variable RIGHT_BR		{ $$ = createDataTransVarRead($5, $3); /* port,  var: var  = port */ }
	| OUT LEFT_BR variable COMMA expression RIGHT_BR	{ $$ = createDataTransVarOut($3, $5);  /* port, expr: port = expr */ }
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
	: expression relational_operator expression							{ $$ = createConditionalExpression($1, $2, $3); }
	| NOT expression relational_operator expression						{ $$ = negateConditionalExpression( createConditionalExpression($2, $3, $4) ); }
	;

secondary_conditional_expression
	: primary_conditional_expression										{ $$ = $1; }
	| secondary_conditional_expression AND primary_conditional_expression	{ $$ = createAndExpression($1, $3);     }
	| secondary_conditional_expression OR primary_conditional_expression	{ $$ = createOrExpression($1, $3);      }
	| LEFT_BR secondary_conditional_expression RIGHT_BR						{ $$ = $2; }
	| NOT LEFT_BR secondary_conditional_expression RIGHT_BR					{ $$ = negateConditionalExpression($3); }
	;

conditional_expression
	: MINUS															{ $$ = NULL; }
	| secondary_conditional_expression								{ $$ = $1;   }
	;

state_name
	: IDENTIFIER	{ strcpy($$, $1); }
	;

transitions
	: CONSTANT		{ strcpy($$, $1); }
	;

operation_list
	: assignment_expression							{ $$ = (DT_LIST*)malloc(sizeof(DT_LIST)); $$->action[0].lhs = $1->lhs; $$->action[0].rhs = $1->rhs; $$->numOperations = 1; }
	| operation_list COMMA assignment_expression	{ $$->action[$$->numOperations].lhs = $3->lhs; $$->action[$$->numOperations].rhs = $3->rhs;  ($$->numOperations)++; }
	;

operations
	: MINUS				{ $$ = (DT_LIST*)malloc(sizeof(DT_LIST)); $$->numOperations = 0; }
	| operation_list	{ $$ = $1; }
	;

substatement
	: conditional_expression SEPARATOR operations state_name	{ $$ = (SUBSTMT*)malloc(sizeof(SUBSTMT)); $$->condition = $1; $$->assignments = $3; strcpy($$->stateName, $4); }
	;

substatement_list
	: substatement						{ $$ = (SUBSTMT_LIST*)malloc(sizeof(SUBSTMT_LIST)); $$->substmt[0] = $1; $$->numSubstmts = 1; }
	| substatement_list substatement	{ $$->substmt[$$->numSubstmts] = $2; ($$->numSubstmts)++; }
	;

statement
	: state_name transitions substatement_list SEMIC	{ $$ = (STMT*)malloc(sizeof(STMT)); strcpy($$->stateName, $1);
															if(constval($2) != $3->numSubstmts){printf("\n\nMismatch in number of transitions for State %s\nExiting System\n", $1); exit(0);}
															strcpy($$->numTransitions, $2);  $$->substmts = $3;   }
	| state_name transitions SEMIC						{ $$ = (STMT*)malloc(sizeof(STMT)); strcpy($$->stateName, $1);
															if(constval($2) != 0){printf("\n\nMismatch in number of transitions for State %s\nExiting System\n", $1); exit(0);}
															strcpy($$->numTransitions, "0"); $$->substmts = NULL; }
	;

statement_list
	: statement							{ $$ = (STMT_LIST*)malloc(sizeof(STMT_LIST)); $$->statements[0] = $1; $$->numStatements = 1; }
	| statement_list statement			{ $$->statements[$$->numStatements] = $2; ($$->numStatements)++;                               }
	;

translation_fsmd
	: fsmd_name additional_declaration statement_list	{ createFSMD($1, $3); /* additional_declaration is needed for SAST tool */ }
	| fsmd_name statement_list							{ createFSMD($1, $2); }
	;

fsmd_name
	: STRING_LITERAL	{ strcpy($$, $1); }
	;

additional_declaration
	: CONSTANT CONSTANT variable_list SEMIC
	| CONSTANT CONSTANT SEMIC
	;

argument_list
	: expression										{ $$ = $1; }
	| argument_list COMMA expression					{ $$ = addArgument($1, $3);    }
	;

uninterpreted_function
	: function_name LEFT_BR argument_list RIGHT_BR		{ $$ = createFunction($1, $3); }
	;

function_name
	: IDENTIFIER		{ $$ = createVariable($1); /* 'v' will later be replaced by 'f' */ }
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

extern boolean flagVar_List;

void callParser(char *fileName)
{

 	printf("\nCalling Parser....");

 	printf("\nFile opening: %s\n", fileName);

 	yyin = fopen(fileName, "r");

 	if(yyin == NULL)
	{
 		printf("\nFile open error\n");
		exit(0);
	}

	indexof_symtab("-");
	//The next statement adds the variable to corresponding var_list
	if(!(flagVar_List))
		indexof_varlist("-", &V0);
	else
		indexof_varlist("-", &V1);

 	yyparse();
}
