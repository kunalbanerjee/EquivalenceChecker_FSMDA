#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FsmdaHeader.h"
#include "parser.h"
#include "valPropHeader.h"


extern int NO_OF_VARIABLES;
extern int flagVar_List;

//This file contains code for Live Variable Analysis (LVA)
//required to verify non-uniform code motions


//Comment the following #define if you want conventional LVA
//Uncomment the following #define if you want LVA for output variables only
//#define OUTPUT_ONLY
//The following functions are affected by commenting / uncommenting the definition OUTPUT_ONLY
//1. void computeDefUse_CFG_Node( CFG_NODE* );
//2. void updateLiveS( CFG_NODE* );
//The modified versions of these functions are given at the bottom of this file


//The following function constructs the CFG of an FSMD
//Input:  A path cover
//Output: Control Flow Graph of P
//(the predecessor / successor relationship is made explicit in CFG)
CFG* constructCFG( PATHS_LIST *P )
{
	int i, j;
	CFG_NODE_LIST *trav, *prev_trav;
	CFG_NODE *newNode;
	CFG *cfg;

	if(P == (PATHS_LIST*)NULL)
		return (CFG*)NULL;

	cfg = (CFG*) malloc (sizeof(CFG));
	cfg->countNodes = 0;
	for(i = 0; i < MAX_TAIL; i++)
		cfg->tail[i] = (CFG_NODE*)NULL;

	cfg->fsmd = P->fsmd;

	//traverse all paths in P
	for(i = 0; i < P->numpaths; i++)
	{
		newNode = (CFG_NODE*) malloc (sizeof(CFG_NODE));
		newNode->path = &(P->paths[i]);
		newNode->pred = (CFG_NODE_LIST*)NULL;
		newNode->succ = (CFG_NODE_LIST*)NULL;
		newNode->indexAtPathCover = i;

		//Find for which nodes already present in cfg, is this new node
		//a predecessor or a successor
		for(j = 0; j < cfg->countNodes; j++)
		{
			if(cfg->node[j]->path->start == newNode->path->end)
			{
				//newNode is predecessor and cfg->node[j] is successor
				trav = newNode->succ;
				prev_trav = (CFG_NODE_LIST*)NULL;
				while(trav != (CFG_NODE_LIST*)NULL)
				{
					prev_trav = trav;
					trav = trav->next;
				}

				trav = (CFG_NODE_LIST*) malloc (sizeof(CFG_NODE_LIST));
				trav->nlist = cfg->node[j];
				trav->next = (CFG_NODE_LIST*)NULL;
				if(prev_trav == (CFG_NODE_LIST*)NULL)
					newNode->succ = trav;
				else
					prev_trav->next = trav;


				trav = cfg->node[j]->pred;
				prev_trav = (CFG_NODE_LIST*)NULL;
				while(trav != (CFG_NODE_LIST*)NULL)
				{
					prev_trav = trav;
					trav = trav->next;
				}

				trav = (CFG_NODE_LIST*) malloc (sizeof(CFG_NODE_LIST));
				trav->nlist = newNode;
				trav->next = (CFG_NODE_LIST*)NULL;
				if(prev_trav == (CFG_NODE_LIST*)NULL)
					cfg->node[j]->pred = trav;
				else
					prev_trav->next = trav;

			}

			if(cfg->node[j]->path->end == newNode->path->start)
			{
				//cfg->node[j] is predecessor and newNode is successor
				trav = newNode->pred;
				prev_trav = (CFG_NODE_LIST*)NULL;
				while(trav != (CFG_NODE_LIST*)NULL)
				{
					prev_trav = trav;
					trav = trav->next;
				}

				trav = (CFG_NODE_LIST*) malloc (sizeof(CFG_NODE_LIST));
				trav->nlist = cfg->node[j];
				trav->next = (CFG_NODE_LIST*)NULL;
				if(prev_trav == (CFG_NODE_LIST*)NULL)
					newNode->pred = trav;
				else
					prev_trav->next = trav;


				trav = cfg->node[j]->succ;
				prev_trav = (CFG_NODE_LIST*)NULL;
				while(trav != (CFG_NODE_LIST*)NULL)
				{
					prev_trav = trav;
					trav = trav->next;
				}

				trav = (CFG_NODE_LIST*) malloc (sizeof(CFG_NODE_LIST));
				trav->nlist = newNode;
				trav->next = (CFG_NODE_LIST*)NULL;
				if(prev_trav == (CFG_NODE_LIST*)NULL)
					cfg->node[j]->succ = trav;
				else
					prev_trav->next = trav;
			}
		}//end for j

		//newNode is a self-loop
		if(newNode->path->start == newNode->path->end)
		{
			//newNode is its own successor / predecessor
			trav = newNode->pred;
			prev_trav = (CFG_NODE_LIST*)NULL;
			while(trav != (CFG_NODE_LIST*)NULL)
			{
				prev_trav = trav;
				trav = trav->next;
			}

			trav = (CFG_NODE_LIST*) malloc (sizeof(CFG_NODE_LIST));
			trav->nlist = newNode;
			trav->next = (CFG_NODE_LIST*)NULL;
			if(prev_trav == (CFG_NODE_LIST*)NULL)
				newNode->pred = trav;
			else
				prev_trav->next = trav;


			trav = newNode->succ;
			prev_trav = (CFG_NODE_LIST*)NULL;
			while(trav != (CFG_NODE_LIST*)NULL)
			{
				prev_trav = trav;
				trav = trav->next;
			}

			trav = (CFG_NODE_LIST*) malloc (sizeof(CFG_NODE_LIST));
			trav->nlist = newNode;
			trav->next = (CFG_NODE_LIST*)NULL;
			if(prev_trav == (CFG_NODE_LIST*)NULL)
				newNode->succ = trav;
			else
				prev_trav->next = trav;
		}//end self loop

		cfg->node[cfg->countNodes] = newNode;
		(cfg->countNodes)++;
	}//end for i

	//cfg->countNodes == P->numpaths should hold at this point
	#ifdef DEbug
	if(cfg->countNodes != P->numpaths)
		printf("\nWARNING: Number of paths in path cover and CFG do not match!\n");
	#endif

	//Find the tail (final) nodes
	j = 0;
	for(i = 0; i < cfg->countNodes; i++)
	{
		//tail nodes do not have any successor
		if(cfg->node[i]->succ == (CFG_NODE_LIST*)NULL)
		{
			cfg->tail[j] = cfg->node[i];
			cfg->tail[j]->path->finalPath = TRUE;
			j++;
		}
	}

	return cfg;
}


//To print a CFG
void printCFG( CFG *G )
{
	int i, j;
	char *sym_value;

	if(G == (CFG*)NULL)
		return;

	sym_value = (char*)malloc(100 * sizeof( char ));

	printf("\nDisplaying the CFG along with Live Variable Analysis\n");

	for(i = 0; i < G->countNodes; i++)
	{
		displayApath( G->node[i]->path );
		printf("\nUse: ");
		for(j = 0; j < G->node[i]->use->countVars; j++)
		{
			symbol_for_index( G->node[i]->use->variable[j], sym_value );
			printf("%s  ", sym_value);
		}//end for Use
		printf("\nDef: ");
		for(j = 0; j < G->node[i]->def->countVars; j++)
		{
			symbol_for_index( G->node[i]->def->variable[j], sym_value );
			printf("%s  ", sym_value);
		}//end for Def
		printf("\nLive_s: ");
		for(j = 0; j < G->node[i]->live_s->countLiveVar; j++)
		{
			symbol_for_index( G->node[i]->live_s->liveVar[j], sym_value );
			printf("%s  ", sym_value);
		}//end for Live_s
		printf("\nLive_f: ");
		for(j = 0; j < G->node[i]->live_f->countLiveVar; j++)
		{
			symbol_for_index( G->node[i]->live_f->liveVar[j], sym_value );
			printf("%s  ", sym_value);
		}//end for Live_f
		printf("\n-----\n\n");
	}//end for i
}

//To mark variables and put them into "Use"
void markVariables( NC *expression, VARIABLE_LIST *vlist )
{
	int i;
	boolean flagPresent;

	if(expression != NULL)
	{
		if(expression->type == 'v' || expression->type == 'a')
		{
			flagPresent = FALSE;
			for(i = 0; i < vlist->countVars; i++)
			{
				if(vlist->variable[i] == expression->inc)
				{
					flagPresent = TRUE;
					break;
				}
			}//end for i

			if(!(flagPresent))
			{
				vlist->variable[ vlist->countVars ] = expression->inc;
				(vlist->countVars)++;
			}
		}//end if 'v' or 'a'

		markVariables(expression->list, vlist);
		markVariables(expression->link, vlist);
	}
}


#ifndef OUTPUT_ONLY
//To compute Def-Use of a node (path) in a CFG
void computeDefUse_CFG_Node( CFG_NODE *node )
{
	int i;
	r_alpha *trans;
	boolean flagPresent;

	if(node == (CFG_NODE*)NULL)
		return;

	if(node->path == (PATH_ST*)NULL)
		return;

	node->def = (VARIABLE_LIST*) malloc (sizeof(VARIABLE_LIST));
	node->def->countVars = 0;
	node->use = (VARIABLE_LIST*) malloc (sizeof(VARIABLE_LIST));
	node->use->countVars = 0;

	//All variables appearing in the condition of the path occur in "Use"
	markVariables(node->path->condition, node->use);

	trans = node->path->transformations;

	while(trans != (r_alpha*)NULL)
	{
		//Put the lhs in Def
		if(trans->Assign.rhs->type != 'w')
		{
			//For arrays we take a conservative approach and do not include them in Def
			//The following loop is executed for scalars only

			flagPresent = FALSE;
			for(i = 0; i < node->def->countVars; i++)
			{
				if(node->def->variable[i] == trans->Assign.lhs)
				{
					flagPresent = TRUE;
					break;
				}
			}

			if(!(flagPresent))
			{
				node->def->variable[ node->def->countVars ] = trans->Assign.lhs;
				(node->def->countVars)++;
			}
		}

		//Put the variables in rhs into Use
		markVariables(trans->Assign.rhs, node->use);

		trans = trans->next;
	}//end while r_alpha
}
#endif


//To compute Def-Use of each node (path) in a CFG
void computeDefUse_CFG( CFG *G )
{
	int i;

	for(i = 0; i < G->countNodes; i++)
	{
		computeDefUse_CFG_Node(G->node[i]);

		//The list of live variables are also inititalized
		G->node[i]->live_s = (LV*) malloc (sizeof(LV));
		G->node[i]->live_s->countLiveVar = 0;
		G->node[i]->live_s->stateIndex = G->node[i]->path->start;

		G->node[i]->live_f = (LV*) malloc (sizeof(LV));
		G->node[i]->live_f->countLiveVar = 0;
		G->node[i]->live_f->stateIndex = G->node[i]->path->end;
	}//end for i
}


//Updates Live_F of a CFG_NODE
void updateLiveF( CFG_NODE *node )
{
	int i, j;
	CFG_NODE_LIST *trav;
	boolean flagPresent;

	//Live_F( node ) = UNION( Live_S( succ ) | succ is a successor of node )

	trav = node->succ;

	while(trav != (CFG_NODE_LIST*)NULL)
	{
		for(i = 0; i < trav->nlist->live_s->countLiveVar; i++)
		{
			flagPresent = FALSE;

			for(j = 0; j < node->live_f->countLiveVar; j++)
			{
				if(trav->nlist->live_s->liveVar[i] == node->live_f->liveVar[j])
				{
					flagPresent = TRUE;
					break;
				}
			}//end for j

			if( !(flagPresent) )
			{
				node->live_f->liveVar[ node->live_f->countLiveVar ] = trav->nlist->live_s->liveVar[i];
				(node->live_f->countLiveVar)++;
			}
		}//end for i

		trav = trav->next;
	}//end while trav != NULL
}


#ifndef OUTPUT_ONLY
//Updates Live_S of a CFG_NODE
void updateLiveS( CFG_NODE *node )
{
	int i, j;
	boolean flagPresent;

	//Live_S( node ) = Use( node ) Union ( Live_F( node ) - Def( node ) )

	if(node->live_s->countLiveVar == 0)
	{
		//The above mentioned check avoids assigning
		//Live_S( node ) = Use( node ) more than once

		for(i = 0; i < node->use->countVars; i++)
		{
			node->live_s->liveVar[i] = node->use->variable[i];
		}
		node->live_s->countLiveVar = node->use->countVars;
	}

	for(i = 0; i < node->live_f->countLiveVar; i++)
	{
		//Add this variable from live_f to live_s if it does not occur in Def
		//and is not already present

		flagPresent = FALSE;

		for(j = 0; j < node->def->countVars; j++)
		{
			if(node->live_f->liveVar[i] == node->def->variable[j])
			{
				flagPresent = TRUE;
				break;
			}
		}//end for j

		if( !(flagPresent) )
		{
			for(j = 0; j < node->live_s->countLiveVar; j++)
			{
				if(node->live_f->liveVar[i] == node->live_s->liveVar[j])
				{
					flagPresent = TRUE;
					break;
				}
			}//end for j
		}

		if( !(flagPresent) )
		{
			node->live_s->liveVar[ node->live_s->countLiveVar ] = node->live_f->liveVar[i];
			(node->live_s->countLiveVar)++;
		}
	}//end for i
}
#endif


//Updates liveness information for all nodes in CFG
//flagChange contains the value FALSE if no information is updated
//flagChange contains the value TRUE if at least one node's live_s is upadted
void updateLivenessInfo( CFG *G, CFG_NODE *tail, boolean *flagChange )
{
	int i;
	CFG_NODE *queueNodes[MAXSTATES];
	CFG_NODE_LIST *pred;
	LV *oldLV;
	int qHead, qTail;
	boolean flagPresent;

	#ifdef DEbug
	printf("\nIn updateLivenessInfo\n");
	#endif

	*flagChange = FALSE;

	//Empty queue
	qHead = 0;
	qTail = -1;

	//Enqueue the first element into the queue
	queueNodes[ ++qTail ] = tail;

	//The following iterates until the queue is empty
	while(qHead <= qTail) //check for emptiness
	{
		oldLV = copyLV( queueNodes[qHead]->live_s );

		updateLiveF( queueNodes[qHead] );
		updateLiveS( queueNodes[qHead] );

		if(equalLV(oldLV, queueNodes[qHead]->live_s) == FALSE)
			*flagChange = TRUE; //once set to TRUE, flagChange cannot be
			                    //reverted back to FALSE within the same invocation

		//enqueue the predecessors of queue[ head ]
		//which have not already been visited and
		//are also not present in the queue
		pred = queueNodes[ qHead ]->pred;
		while(pred != (CFG_NODE_LIST*)NULL)
		{
			flagPresent = FALSE;

			for(i = 0; i <= qTail; i++)
			{
				if(queueNodes[i] == pred->nlist)
				{
					flagPresent = TRUE;
					break;
				}
			}//enf for i

			if(!(flagPresent))
			{
				queueNodes[ ++qTail ] = pred->nlist;
			}

			pred = pred->next;
		}

		//dequeue
		qHead++;
	}//end while (queue !empty)

	#ifdef DEbug
	printf("\nOut of updateLivenessInfo\n");
	#endif
}



//Live Variable Analysis
void liveVariableAnalysis( CFG *G )
{
	int countTails;
	boolean flagChange;


	if(G == (CFG*)NULL)
		return;

	#ifdef DEbug
	printf("\nIn liveVariableAnalysis\n");
	#endif

	//The following invocation initializes the data structures of CFG_NODE
	computeDefUse_CFG(G);

	//The following loop iterates as many times as the number
	//of tail (final) states
	countTails = 0;
	while(G->tail[countTails] != (CFG_NODE*)NULL)
	{
		flagChange = TRUE;

		//The following loop is executed as long as at least one change occurs
		//while computing live_s for some path (i.e., until a fix-point is reached)
		//Backward BFS traversal of the CFG may help in reaching this fixpoint earlier
		while(flagChange)
		{
			updateLivenessInfo( G, G->tail[countTails], &flagChange );
		}

		countTails++;
	}

	#ifdef DEbug
	printf("\nOut of liveVariableAnalysis\n");
	#endif
}


//Updated the LVs of paths in the path cover
void updateLVofPaths( CFG *G )
{
	int i, j;
	LV *templv;

	for(i = 0; i < G->countNodes; i++)
	{
		//The list of variables live at the end of a path is equal to
		//the list of variables in live_f of the corresponding CFG_NODE
		G->node[i]->path->live = G->node[i]->live_f;
	}

	//For a terminal path p, live_f should be set to Def(p)
	//since there is no further scope of attaining identical values for
	//mismatched variables, if any
	//For OUTPUT_ONLY, Def(p) contains variables which are output
	for(i = 0; G->tail[i] != (CFG_NODE*)NULL; i++)
	{
		templv = (LV*) malloc (sizeof(LV));
		templv->stateIndex = G->node[i]->path->end;
		templv->countLiveVar = G->tail[i]->def->countVars;

		for(j = 0; j < templv->countLiveVar; j++)
		{
			templv->liveVar[j] = G->tail[i]->def->variable[j];
		}//end for j

		G->tail[i]->path->live = templv;
	}//end for i
}


//This function is invoked from containmentChecker to perform LVA
void performLiveVariableAnalysis( PATHS_LIST *P )
{
	CFG *G;

	G = constructCFG( P );

	liveVariableAnalysis( G );

	#ifdef DETAILS
	printCFG( G );
	#endif

	updateLVofPaths( G );
}



//Copies LV
LV* copyLV( LV* lv )
{
	int i;
	LV *newlv;

	if(lv == (LV*)NULL)
		return (LV*)NULL;

	newlv = (LV*) malloc (sizeof(LV));
	newlv->countLiveVar = lv->countLiveVar;
	newlv->stateIndex = lv->stateIndex;

	for(i = 0; i < lv->countLiveVar; i++)
	{
		newlv->liveVar[i] = lv->liveVar[i];
	}

	return newlv;
}


//Checks two LVs for equality, returns TRUE if equal
//                             returns FALSE otherwise
boolean equalLV( LV* lv1, LV* lv2 )
{
	int i;

	if(lv1 == (LV*)NULL && lv2 == (LV*)NULL)
		return TRUE;

	if(lv1 == (LV*)NULL || lv2 == (LV*)NULL)
		return FALSE;

	if(lv1->countLiveVar != lv2->countLiveVar)
		return FALSE;

	if(lv1->stateIndex != lv2->stateIndex)
		return FALSE;

	for(i = 0; i < lv1->countLiveVar; i++)
	{
		if(lv1->liveVar[i] != lv2->liveVar[i])
			return FALSE;
	}

	return TRUE;
}



//Input:  A variable x, list of output variables OUTPUT
//Output: TRUE, if x belongs to OUTPUT; FALSE, otherwise
boolean isOutputVar( int x, var_list OUTPUT )
{
	int i;

	for(i = 0; i < OUTPUT.no_of_elements; i++)
	{
		if(x == OUTPUT.var_val[i])
			return TRUE;
	}

	return FALSE;
}



#ifdef OUTPUT_ONLY
//To compute Def-Use of a node (path) in a CFG
//Modified for output variables
void computeDefUse_CFG_Node( CFG_NODE *node )
{
	int i;
	r_alpha *trans;
	boolean flagPresent;

	if(node == (CFG_NODE*)NULL)
		return;

	if(node->path == (PATH_ST*)NULL)
		return;

	node->def = (VARIABLE_LIST*) malloc (sizeof(VARIABLE_LIST));
	node->def->countVars = 0;
	node->use = (VARIABLE_LIST*) malloc (sizeof(VARIABLE_LIST));
	node->use->countVars = 0;

	//All variables appearing in the condition of the path occur in "Use"
	markVariables(node->path->condition, node->use);

	trans = node->path->transformations;

	while(trans != (r_alpha*)NULL)
	{
		//Put the lhs in Def
		if(trans->Assign.rhs->type != 'w')
		{
			//For arrays we take a conservative approach and do not include them in Def
			//The following loop is executed for scalars only

			flagPresent = FALSE;
			for(i = 0; i < node->def->countVars; i++)
			{
				if(node->def->variable[i] == trans->Assign.lhs)
				{
					flagPresent = TRUE;
					break;
				}
			}

			if(!(flagPresent))
			{
				node->def->variable[ node->def->countVars ] = trans->Assign.lhs;
				(node->def->countVars)++;
			}
		}

		//Put the variables in rhs into Use only if the lhs appears
		//in the list of output variables
		if(!(flagVar_List))
			flagPresent = isOutputVar(trans->Assign.lhs, outputVar0);
		else
			flagPresent = isOutputVar(trans->Assign.lhs, outputVar1);

		if(flagPresent)
			markVariables(trans->Assign.rhs, node->use);

		trans = trans->next;
	}//end while r_alpha
}



//Updates Live_S of a CFG_NODE
//Modified for output variables
void updateLiveS( CFG_NODE *node )
{
	int i, j;
	boolean flagPresent;
	r_alpha *trans;

	//Live_S( node ) = Use( node ) Union ( Live_F( node ) - Def( node ) )

	for(i = 0; i < node->live_f->countLiveVar; i++)
	{
		//Check if this variable (x, say) gets defined in the path or not
		//If defined, add the variables that appear on its rhs to Use( node )
		trans = node->path->transformations;
		while(trans != (r_alpha*)NULL)
		{
			if(trans->Assign.lhs == node->live_f->liveVar[i])
				markVariables(trans->Assign.rhs, node->use);

			trans = trans->next;
		}
	}


	//Add Use( node ) to Live_S( node )
	for(i = 0; i < node->use->countVars; i++)
	{
		//Check for already present in Live_S or not is avoided for speed-up
		node->live_s->liveVar[i] = node->use->variable[i];
	}
	node->live_s->countLiveVar = node->use->countVars;


	for(i = 0; i < node->live_f->countLiveVar; i++)
	{
		//Add this variable from live_f to live_s if it does not occur in Def
		//and is not already present

		flagPresent = FALSE;

		for(j = 0; j < node->def->countVars; j++)
		{
			if(node->live_f->liveVar[i] == node->def->variable[j])
			{
				flagPresent = TRUE;
				break;
			}
		}//end for j

		if( !(flagPresent) )
		{
			for(j = 0; j < node->live_s->countLiveVar; j++)
			{
				if(node->live_f->liveVar[i] == node->live_s->liveVar[j])
				{
					flagPresent = TRUE;
					break;
				}
			}//end for j
		}

		if( !(flagPresent) )
		{
			node->live_s->liveVar[ node->live_s->countLiveVar ] = node->live_f->liveVar[i];
			(node->live_s->countLiveVar)++;
		}
	}//end for i
}
#endif
