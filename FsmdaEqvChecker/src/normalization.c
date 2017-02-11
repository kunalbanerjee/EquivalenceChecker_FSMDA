#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FsmdaHeader.h"
#include "parser.h"

extern boolean flagVar_List;
extern FSMD *M0, *M1;

//Functions related normalized arithmetic expressions
//Creation of normalized "primary" expressions

//Input:  A string x
//Output: A normalized expression with x as primary
NC* createVariable( char *variableName )
{
	NC *sum, *term, *primary;
	int symbolIndex;

	symbolIndex = indexof_symtab(variableName);
	primary = (NC*) malloc (sizeof(NC));
	primary->list = NULL;
	primary->type = 'v';
	primary->inc = symbolIndex;
	primary->link = NULL;

	term = (NC*) malloc (sizeof(NC));
	term->list = NULL;
	term->type = 'T';
	term->inc = 1;
	term->link = primary;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = NULL;
	sum->type = 'S';
	sum->inc = 0;
	sum->link = term;

	//The next statement adds the variable to corresponding var_list
	if(!(flagVar_List))
		indexof_varlist(variableName, &V0);
	else
		indexof_varlist(variableName, &V1);

	return sum;
}


//Input:  A string x (representing a constant integer)
//Output: A normalized expression with x as sum
NC* createConstant( char *constant )
{
	NC *sum;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = NULL;
	sum->type = 'S';
	sum->inc = constval(constant);
	sum->link = NULL;

	//The next statement adds the constant to corresponding var_list
	//Although it is seemingly unnecessary, it is done to remain compliant
	//with previous code
	if(!(flagVar_List))
		indexof_varlist(constant, &V0);
	else
		indexof_varlist(constant, &V1);

	return sum;
}


//Input:  A string x and a normalized expression y
//Output: A normalized expression with x as as an array
//        and y as its first index expression
NC* createArray( char *arrayName, NC *expression )
{
	NC *sum, *term, *array, *newExpression;
	int symbolIndex;

	symbolIndex = indexof_symtab(arrayName);
	newExpression = copylist(expression);

	array = (NC*) malloc (sizeof(NC));
	array->list = NULL;
	array->type = 'a';
	array->inc = symbolIndex;
	array->link = newExpression;

	term = (NC*) malloc (sizeof(NC));
	term->list = NULL;
	term->type = 'T';
	term->inc = 1;
	term->link = array;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = NULL;
	sum->type = 'S';
	sum->inc = 0;
	sum->link = term;

	//The next statement adds the array to corresponding var_list
	if(!(flagVar_List))
		indexof_varlist(arrayName, &V0);
	else
		indexof_varlist(arrayName, &V1);

	return sum;
}


//Input:  An array x (of some dimension) and a normalized expression y
//Output: The array x with an increased dimension (with index value y)
NC* addArrayDimension( NC *array, NC *expression )
{
	NC *temp, *newExpression;

	newExpression = copylist(expression);

	//currently array points to Sum (S), so traverse downward
	//to reach array's first dimension
	temp = array->link->link->link;

	//traverse the list until the last dimension is reached
	while(temp->list != NULL)
		temp = temp->list;

	temp->list = newExpression;

	return array;
}


//Input:  Two normalized expressions x and y, and a flag (type: character)
//Output: A normalized expression x/y or x%y depending upon the flag
NC* createExpression_Mod_Div( NC *neumerator, NC *denominator, char flag )
{
	NC *finalExpression, *newNeumerator, *newDenominator;
	NC *term, *sum;

	newNeumerator = copylist(neumerator);
	newDenominator = copylist(denominator);

	finalExpression = (NC*) malloc (sizeof(NC));
	finalExpression->list = NULL;
	finalExpression->inc = 0;
	finalExpression->link = newNeumerator;
	newNeumerator->list = newDenominator;


	if(flag == '/')
		finalExpression->type = 'D';
	else
		finalExpression->type = 'M';

	term = (NC*) malloc (sizeof(NC));
	term->list = NULL;
	term->type = 'T';
	term->inc = 1;
	term->link = finalExpression;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = NULL;
	sum->type = 'S';
	sum->inc = 0;
	sum->link = term;

	return sum;
}



//Input:  An argument list x and an argument y
//Output: An argument list x.y (ie, y is appended to x)
NC* addArgument( NC *argList, NC *newArg )
{
	NC *traverseArgList;

	if(argList == NULL && newArg == NULL)
		return NULL;
	if(argList == NULL)
		return newArg;
	if(newArg == NULL)
		return argList;

	traverseArgList = argList;
	while(traverseArgList->list != NULL)
		traverseArgList = traverseArgList->list;

	traverseArgList->list = newArg;

	return argList;
}


//Input:  A function name (type: NC) and an argument list
//Output: The function with its argument list set
NC* createFunction( NC *functionName, NC *argList )
{
	NC *functionPrimary;

	//Note that the function was created using createVariable
	//So, the 'v' in its Primary needs to be replaced by 'f'
	functionPrimary = functionName->link->link;
	functionPrimary->type = 'f';

	functionPrimary->link = argList;

	return functionName;
}


//Creation of data transformations

//Input:  A variable as lhs and an expression as rhs
//Output: A data transformation with rhs assigned to lhs
DATA_TRANS* createDataTransVar( char *lhs, NC *rhs )
{
	DATA_TRANS *newDataTrans;
	int symbolIndex;

	symbolIndex = indexof_symtab(lhs);

	newDataTrans = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));
	newDataTrans->lhs = symbolIndex;
	newDataTrans->rhs = rhs;

	//The next statement adds the lhs variable to corresponding var_list
	if(!(flagVar_List))
		indexof_varlist(lhs, &V0);
	else
		indexof_varlist(lhs, &V1);

	return newDataTrans;
}


//Input:  An array as lhs and an expression as rhs
//Output: A data transformation with rhs assigned to lhs
DATA_TRANS* createDataTransArray( NC *lhs_a, NC *rhs )
{
	DATA_TRANS *newDataTrans;
	NC *newRHS;

	newRHS = (NC*) malloc (sizeof(NC));
	newRHS->type = 'w'; //'w' stands for 'write'
	newRHS->inc = 0;
	newRHS->list = NULL;
	//lhs_a has a structure like (0 + 1 * a) because while generating
	//the array_expression 'a', the parser does not know whether 'a'
	//occurs on the lhs or on the rhs
	//Now, 'a' should occur as the link of 'w'
	newRHS->link = lhs_a->link->link;
	//rhs (2nd argument) should occur as the list of 'a'
	newRHS->link->list = rhs;

	//We need to differenciate the arrays (of type 'a') to denote
	//'read' and 'write' explicitly.
	//In general, 'a' is used to denote the 'read' operation
	//so as to avoid a separate normalized cell for 'read'.
	//This necessitates that we change the type of 'w'->link
	//to something other than 'a'.
	newRHS->link->type = 'y';

	newDataTrans = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));
	//newDataTrans->lhs should contain the (lhs) array's index in the symbol table
	newDataTrans->lhs = newRHS->link->inc;
	newDataTrans->rhs = newRHS;

	return newDataTrans;
}


//Input:  A variable as lhs and another variable as rhs
//Output: A data transformation with rhs assigned to lhs
DATA_TRANS* createDataTransVarRead( char *lhs, char *rhs )
{
	DATA_TRANS *newDataTrans;
	int symbolIndexLHS;

	symbolIndexLHS = indexof_symtab(lhs);

	newDataTrans = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));
	newDataTrans->lhs = symbolIndexLHS;
	newDataTrans->rhs = createVariable(rhs);

	//The next statement adds the lhs variable to corresponding var_list
	if(!(flagVar_List))
		indexof_varlist(lhs, &V0);
	else
		indexof_varlist(lhs, &V1);

	return newDataTrans;
}


//Input:  A variable as lhs and an expression as rhs
//Output: A data transformation with rhs assigned to lhs
//The variables that occur in rhs are added to the list
//of output variables
DATA_TRANS* createDataTransVarOut( char *lhs, NC *rhs )
{
	DATA_TRANS *newDataTrans;
	int symbolIndexLHS;

	symbolIndexLHS = indexof_symtab(lhs);

	newDataTrans = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));
	newDataTrans->lhs = symbolIndexLHS;
	newDataTrans->rhs = rhs;

	//The next statement adds the lhs variable to corresponding var_list
	if(!(flagVar_List))
		indexof_varlist(lhs, &V0);
	else
		indexof_varlist(lhs, &V1);

	markOutputVariables(symbolIndexLHS);

	return newDataTrans;
}


//This function adds the variable x in the list of output variables
//if not already present
void markOutputVariables( int x )
{
	int i;
	boolean flagPresent;
	var_list *tempOutputVar;

	//This check is for avoiding - (minus)
	if(x == 0)
		return;

	//Add the variable in outputVar if not already present
	flagPresent = FALSE;

	if(!(flagVar_List))
		tempOutputVar = &outputVar0;
	else
		tempOutputVar = &outputVar1;

	for(i = 0; i < tempOutputVar->no_of_elements; i++)
	{
		if(tempOutputVar->var_val[i] == x)
		{
			flagPresent = TRUE;
			break;
		}
	}//end for

	if(!(flagPresent))
	{
		tempOutputVar->var_val[tempOutputVar->no_of_elements] = x;
		(tempOutputVar->no_of_elements)++;
	}
}



//Input:  A normalized expression x
//Output: Negate of x (Sum's and Term's of x are multiplied by -1)
NC* negateExpression( NC* expression )
{
	NC *temp, *newExpression;

	newExpression = copylist(expression);

	newExpression->inc = (newExpression->inc) * -1;
	temp = newExpression->link;


	while(temp != NULL)
	{
		temp->inc = (temp->inc) * -1;
		temp = temp->list;
	}

	return newExpression;
}



//Input:  A list of actions (type: DT_LIST) and a transition (type: TRANSITION_ST)
//Output: The transition associated with the list of actions
void generateAssignments( DT_LIST *actions_list, TRANSITION_ST *trans )
{
	int i;

	for(i = 0; i < actions_list->numOperations; i++)
	{
		//Copy DATA_TRANS
		trans->action[i].lhs = actions_list->action[i].lhs;
		trans->action[i].rhs = copylist(actions_list->action[i].rhs);
	}

	//The following assignments are needed to denote
	//"end of action" has been reached
	trans->action[i].lhs = -1;
	trans->action[i].rhs = NULL;
}


//Functions related to normalized conditional expressions
//Creation of normalized conditional expressions


//Input:  A relational operator (type: character)
//Output: The operator's index (type: int)
int getRelOperatorIndex( char *relOperator )
{
	if(strcmp(relOperator,"!=") == 0)
		return 5;
	if(strcmp(relOperator,"==") == 0)
		return 4;
	if(strcmp(relOperator,">=") == 0)
		return 0;
	if(strcmp(relOperator,">") == 0)
		return 1;
	if(strcmp(relOperator,"<=") == 0)
		return 2;
	if(strcmp(relOperator,"<") == 0)
		return 3;
	return -1;
}


//Input:  Two expressions x and y, and a relational operator op
//Output: One conditional expression x op y
NC* createConditionalExpression( NC *expr1, char *relOp, NC *expr2 )
{
	NC *andExpr, *orExpr, *relExpr;
	NC *newExpr1, *newExpr2;
	NC *newExpr;
	int operator;

	operator = getRelOperatorIndex(relOp);
	newExpr1 = copylist(expr1);
	newExpr2 = copylist(expr2);
	newExpr2 = negateExpression(newExpr2);
	newExpr = (NC*) malloc (sizeof(NC));
	Add_Sums(newExpr1, newExpr2, newExpr);


	relExpr = (NC*) malloc (sizeof(NC));
	relExpr->list = NULL;
	relExpr->type = 'R';
	relExpr->inc = operator;
	relExpr->link = newExpr;

	orExpr = (NC*) malloc (sizeof(NC));
	orExpr->list = NULL;
	orExpr->type = 'O';
	orExpr->inc = 0;
	orExpr->link = relExpr;

	andExpr = (NC*) malloc (sizeof(NC));
	andExpr->list = NULL;
	andExpr->type = 'A';
	andExpr->inc = 0;
	andExpr->link = orExpr;

	return andExpr;
}


//Functions for performing Boolean operations - NOT, AND, OR


//Input:  A conditional expression x (at OR level)
//Output: The conditional expression !x
NC* computeNegation( NC *cond )
{
	NC *retOr, *oldOr, *newOr;
	NC *iRel, *oneRel;
	NC *retNegCond, *jRet;

	//base case
	if(cond->list == NULL)
	{
		for(iRel = cond->link; iRel != NULL; iRel = iRel->list)
		{
			oneRel = (NC*) malloc (sizeof(NC));
			oneRel->list = NULL;
			oneRel->type = 'R';
			oneRel->inc = negateoperator(iRel->inc);
			oneRel->link = copylist(iRel->link);

			newOr = (NC*) malloc (sizeof(NC));
			newOr->list = NULL;
			newOr->type = 'O';
			newOr->inc = 0;
			newOr->link = oneRel;

			if(iRel == cond->link)
				retOr = newOr;
			else
				oldOr->list = newOr;

			oldOr = newOr;
		}

		return retOr;
	}

	//inductive case
	retNegCond = computeNegation( cond->list );

	for(iRel = cond->link; iRel != NULL; iRel = iRel->list)
	{
		for(jRet = retNegCond; jRet != NULL; jRet = jRet->list)
		{
			oneRel = (NC*) malloc (sizeof(NC));
			oneRel->list = copylist(jRet->link);
			oneRel->type = 'R';
			oneRel->inc = negateoperator(iRel->inc);
			oneRel->link = copylist(iRel->link);

			newOr = (NC*) malloc (sizeof(NC));
			newOr->list = NULL;
			newOr->type = 'O';
			newOr->inc = 0;
			newOr->link = oneRel;

			if(iRel == cond->link)
				retOr = newOr;
			else
				oldOr->list = newOr;

			oldOr = newOr;
		}
	}

	return retOr;
}


//Input:  A conditional expression x (at AND level)
//Output: The conditional expression !x
NC* negateConditionalExpression( NC *cond )
{
	NC *negCond;

	if(cond == NULL)
		return NULL;

	negCond = (NC*) malloc (sizeof(NC));
	negCond->list = NULL;
	negCond->type = 'A';
	negCond->inc = 0;
	negCond->link = computeNegation(cond->link);

	return negCond;
}



//Input:  Two conditions x and y
//Output: One condition x AND y (i.e., x && y)
NC* createAndExpression( NC *cond1, NC *cond2 )
{
	NC *traverseOr;
	NC *t1, *t2, *node;
	boolean flag;

	if(cond1 == NULL && cond2 == NULL)
		return NULL;
	if(cond1 == NULL)
		return cond2;
	if(cond2 == NULL)
		return cond1;

	traverseOr = cond1->link;
	while(traverseOr->list != NULL)
		traverseOr = traverseOr->list;

	//Appending (ANDing) cond1 and cond2
	//traverseOr->list = cond2->link;
	//The above-mentioned simplest strategy is neglected
	//to avoid duplicating the same conjuncts

	t2 = cond2->link;
	while( t2 != (NC*)NULL )
	{
		flag = TRUE;
		t1 = cond1->link;
		while( t1 != (NC*)NULL )
		{
			if(compare_trees(t1->link, t2->link) == 1)
			{
				flag = FALSE;
				break;
			}
			t1 = t1->list;
		}

		if(flag)
		{
			node = (NC*)malloc(sizeof(NC));
			node->list = (NC*)NULL;
			node->type = 'O';
			node->inc = 0;
			node->link = copylist( t2->link );

			traverseOr->list = node;
			traverseOr = node;
		}

		t2 = t2->list;
	}

	return cond1;
}



//Input:  Two relations x = (a or b or c), and y = (b or d)
//Output: One relation z = (a or b or c or d) -- devoid of identical clauses
NC* compareRelations( NC *rel1, NC *rel2 )
{
	NC *copyRel1, *copyRel2;
	NC *r1, *r2, *tempR;
	boolean flag;

	if(rel1 == NULL && rel2 == NULL)
		return NULL;
	if(rel1 == NULL)
		return rel2;
	if(rel2 == NULL)
		return rel1;

	copyRel1 = copylist(rel1);
	copyRel2 = copylist(rel2);

	tempR = copyRel1;
	while(tempR->list != NULL)
		tempR = tempR->list;

	for(r2 = copyRel2; r2 != NULL; r2 = r2->list)
	{
		flag = TRUE;
		for(r1 = copyRel1; r1 != NULL; r1 = r1->list)
		{
			if(compare_trees(r1, r2) == 1)
			{
				flag = FALSE;
				break;
			}
		}

		if(flag)
		{
			tempR->list = copylist( r2->link );
			tempR = tempR->list;
		}
	}

	return copyRel1;
}


//Input:  Two conditions x and y
//Output: One condition x OR y (i.e., x || y)
NC* createOrExpression( NC *cond1, NC *cond2 )
{
	NC *iCond1, *jCond2;
	NC *orCond, *node, *oldNode;

	if(cond1 == NULL && cond2 == NULL)
		return NULL;
	if(cond1 == NULL)
		return cond2;
	if(cond2 == NULL)
		return cond1;

	orCond = (NC*) malloc (sizeof(NC));
	orCond->list = NULL;
	orCond->type = 'A';
	orCond->inc = 0;

	for(iCond1 = cond1->link; iCond1 != NULL; iCond1 = iCond1->list)
	{
		for(jCond2 = cond2->link; jCond2 != NULL; jCond2 = jCond2->list)
		{
			node = (NC*) malloc (sizeof(NC));
			node->list = NULL;
			node->type = 'O';
			node->inc = 0;
			node->link = compareRelations(iCond1->link, jCond2->link);

			if(iCond1 == cond1->link && jCond2 == cond2->link)
				orCond->link = node;
			else
				oldNode->link = node;

			oldNode = node;
		}
	}

	return orCond;
}


//Creation of FSMD


//Input:  A name (type: string) and all characteristics of an FSMD
//Output: An FSMD with proper states and transitions
void createFSMD( char* FSMDname, STMT_LIST *stmts )
{
	FSMD *M;
	TRANSITION_ST *trans;
	STMT *oneStmt;
	int i, j, k;
	int current_state, temp, numtrans;

	if(!(flagVar_List))
	{
		M0 = (FSMD*) malloc (sizeof(FSMD));
		M = M0;
	}
	else
	{
		M1 = (FSMD*) malloc (sizeof(FSMD));
		M = M1;
	}

	strcpy(M->name, FSMDname);

	current_state = 0;

	for(i = 0; i < stmts->numStatements; i++)
	{
		oneStmt = stmts->statements[i];
		temp = create_state( oneStmt->stateName, M, current_state );

		numtrans = constval(oneStmt->numTransitions);

		M->states[temp].numtrans = numtrans;

		//value propagation code start
		M->states[temp].VAPFLAG = FALSE;

		M->states[temp].propVector.cond = NULL;
		for(j=0; j < SYMTABSIZE; j++)
		{
			M->states[temp].propVector.value[j] = NULL;

			//array start
			M->states[temp].propVector.subVector[j].countValues = 0;
			for(k = 0; k < MAX_ARRAY_VALUES; k++)
			{
				M->states[temp].propVector.subVector[j].arrayValue[k] = NULL;
			}
			//array end
		}
		//value propagation code end


		if( numtrans == 0 )
		{
			M->states[temp].node_type = 1;
			// state with no outward transition. This is the last state of the fsmd.
			// it has a dummy transition from itself to the start state of the fmsd,
			// with condition = '-' and action = '-'.
		}
		else
			M->states[temp].node_type = 2;

		if( current_state == 0 )
		{   // This is the start state of the FSMD
			M->states[temp].node_type = 0;
		}
		if( temp == current_state )
		{
			current_state++;
			strcpy(M->states[temp].state_id, oneStmt->stateName);
		}

		M->states[temp].translist = NULL;
		for(k = 0; k < numtrans; k++)
		{
		    // This loop will read the transtions of each state

			trans = (TRANSITION_ST *) malloc (sizeof(TRANSITION_ST));
			trans->next = M->states[temp].translist;
			M->states[temp].translist = trans;
			trans->condition = oneStmt->substmts->substmt[k]->condition;

		    generateAssignments(oneStmt->substmts->substmt[k]->assignments, trans);

			trans->outtrans = create_state(oneStmt->substmts->substmt[k]->stateName, M, current_state);
			if(trans->outtrans == current_state)
			{
			    current_state++;
				M->states[trans->outtrans].translist = NULL;
				M->states[trans->outtrans].numtrans = 0;
				M->states[trans->outtrans].node_type = 1;
				strcpy(M->states[trans->outtrans].state_id, oneStmt->substmts->substmt[k]->stateName);
			}
		}
	}

	M->numstates = current_state;
}
