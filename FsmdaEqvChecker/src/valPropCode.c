#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <stdlib.h>
#include "FsmdaHeader.h"
#include "parser.h"
#include "valPropHeader.h"



//NB: Since the purposes for PV0 and PV1 are achieved by modifying
//the data structure for state (NODE_ST)
//We pass M0 and M1 as arguments in place of PV0 and PV1 respectively.

//All parameters appended with "_out", for each function, is supposed
//to be returned to the caller function

/************* Functions for Propagated Vector *************/

//Resets the condition and the values of a propagated vector
void resetPropagatedVector( PROPAGATED_VECTOR *pv )
{
	int i, j;

	pv->cond = (NC*)NULL;
	for(i = 0; i < SYMTABSIZE; i++)
	{
		pv->value[i] = (NC*)NULL;

		//array start
		pv->subVector[i].countValues = 0;
		for(j = 0; j < MAX_ARRAY_VALUES; j++)
		{
			pv->subVector[i].arrayValue[j] = (NC*)NULL;
		}
		//array end
	}
}



//Used to copy a propagated vector src to another propagated vector dest
void copyPropagatedVector( PROPAGATED_VECTOR *dest, PROPAGATED_VECTOR *src )
{
	int i, j;

	if(dest==(PROPAGATED_VECTOR*)NULL || src==(PROPAGATED_VECTOR*)NULL)
	{
		printf("\n In copyPropagatedVector() with a NULL vector\nERROR: Exiting System\n");
		exit(0);
	}

	dest->cond = copylist(src->cond);

	for( i=0; i < NO_OF_VARIABLES; i++ )
	{
		dest->value[i] = copylist(src->value[i]);

		//array start
		dest->subVector[i].countValues = src->subVector[i].countValues;
		for(j = 0; j < MAX_ARRAY_VALUES; j++)
		{
			dest->subVector[i].arrayValue[j] = copylist(src->subVector[i].arrayValue[j]);
		}
		//array end
	}
}



//Used to check whether two propagated vectors are equivalent or not
//Returns TRUE if equivalent, otherwise returns FALSE
boolean equivalentPropagatedVector( PROPAGATED_VECTOR *pv1, PROPAGATED_VECTOR *pv2 )
{
  int i, j, k;

  //Note that the first two checks are redundant
  //but have been included for the purpose of debugging
  if(pv1 == (PROPAGATED_VECTOR*)NULL && pv2 == (PROPAGATED_VECTOR*)NULL)
  {
    #ifdef DEbug
    printf("\nOut of equivalentPropagatedVector() with TRUE because both vectors are NULL\n");
    #endif

    return TRUE;
  }

  if(pv1 == (PROPAGATED_VECTOR*)NULL || pv2 == (PROPAGATED_VECTOR*)NULL)
  {
    #ifdef DEbug
    printf("\nOut of equivalentPropagatedVector() with FALSE because exactly one of the vectors is NULL\n");
    #endif

    return FALSE;
  }

  if(compare_trees(pv1->cond, pv2->cond) == 0)
  {
    #ifdef DEbug
    printf("\nOut of equivalentPropagatedVector() with FALSE because of mismatch in conditions\n");
    #endif

    return FALSE;
  }

  for( i=0; i < NO_OF_VARIABLES; i++ )
  {
	//The following check prevents uselesss calculation for - (minus)
	if(stab.val_tab[i] == 0)
		continue;

    if(compare_trees(pv1->value[i], pv2->value[i]) == 0)
    {
	  for(j = 0; j < V0_V1.no_of_elements; j++)
	  {
		if(strcmp(V0_V1.var_string[j],stab.sym_tab[i]) == 0)
		{
			//#ifdef DEbug
			  printf("\nOut of equivalentPropagatedVector() with FALSE because of mismatch in values for common variable: %s\n", stab.sym_tab[i]);
			//#endif

			return FALSE;
		}
	  }

      #ifdef LVA

      //For LVA, mismatch in an uncommon variable also implies C-equivalence
      //It has been ensured in valueProapagation (for LVA) that "dead" uncommon variables
      //are reset to NULL before returning the control to findEquivalentPath

      #ifdef DEbug
		printf("\nMismatch found for uncommon variable: %s\n", stab.sym_tab[i]);
	  #endif

	  return FALSE;

      #else

      #ifdef DEbug
      printf("\nMismatch found for uncommon variable: %s\n", stab.sym_tab[i]);
      #endif

      #endif
    }

    //array start
    if(pv1->subVector[i].countValues != pv2->subVector[i].countValues)
    {
		for(j = 0; j < V0_V1.no_of_elements; j++)
	    {
		  if(strcmp(V0_V1.var_string[j],stab.sym_tab[i]) == 0)
		  {
			//#ifdef DEbug //KB
			printf("\nOut of equivalentPropagatedVector() with FALSE because of mismatch in count of array values for common variable: %s\n", stab.sym_tab[i]);
			//#endif

			return FALSE;
		  }
	    }
	    #ifdef LVA

        #ifdef DEbug
		  printf("\nMismatch found in count of array values for uncommon variable: %s\n", stab.sym_tab[i]);
		#endif

	    return FALSE;

        #else

        #ifdef DEbug
		printf("\nMismatch found in count of array values for uncommon variable: %s\n", stab.sym_tab[i]);
        #endif

        #endif
	}

    for(k = 0; k < pv1->subVector[i].countValues; k++)
    {
	  if(compare_trees(pv1->subVector[i].arrayValue[k], pv2->subVector[i].arrayValue[k]) == 0)
      {
	    for(j = 0; j < V0_V1.no_of_elements; j++)
	    {
		  if(strcmp(V0_V1.var_string[j],stab.sym_tab[i]) == 0)
		  {
			//#ifdef DEbug //KB
			printf("\nOut of equivalentPropagatedVector() with FALSE because of mismatch in array value for common variable: %s\n", stab.sym_tab[i]);
			printf("No value corresponding to the following is found.\n"); write_lists(pv1->subVector[i].arrayValue[k]);
			//#endif

			return FALSE;
		  }
	    }
	    #ifdef LVA

	    #ifdef DEbug
	      printf("\nMismatch found in array value for uncommon variable: %s\n", stab.sym_tab[i]);
	    #endif

	    return FALSE;

	    #else

        #ifdef DEbug
        printf("\nMismatch found in array value for uncommon variable: %s\n", stab.sym_tab[i]);
        #endif

        #endif
	  } //end if compare_trees
	}
    //array end
  }

  #ifdef DEbug
  printf("\nOut of equivalentPropagatedVector() with TRUE indicating Case 1.1\n");
  #endif

  return TRUE;
}



//Used to print a propagated vector
void printPropagatedVector( PROPAGATED_VECTOR *pv )
{
  int i, j;

  if(pv==(PROPAGATED_VECTOR*)NULL)
  {
	#ifdef DEbug
    printf("\nWARNING: In printPropagatedVector() with a NULL vector\n");
    #endif
    return;
  }

  printf("\nPropagated Vector\n Condition: \n");
  if(pv->cond == (NC*)NULL)
    printf("\t True\n");
  else
    write_lists(pv->cond);

  for( i=0; i < NO_OF_VARIABLES; i++ )
  {
	//the following check is for handling - (minus)
	if(stab.val_tab[i] == 0)
	  continue;

    if(pv->value[i] != (NC*)NULL && pv->subVector[i].countValues == 0)
    //the second conjunct is introduced to differentiate scalars from arrays
    {
      printf("\n %s := ", stab.sym_tab[i]);
      write_lists(pv->value[i]);
    }

    //array start
    if(pv->subVector[i].countValues > 0)
    {
		for(j = 0; j < pv->subVector[i].countValues; j++)
		{
			if(pv->subVector[i].arrayValue[j] != (NC*)NULL)
			{
				printf("\n %s := ", stab.sym_tab[i]);
				write_lists(pv->subVector[i].arrayValue[j]);
			}
		} //end for
	}
	//array end
  }
}



//To count the number of mismatches between 2 Value Vectors
//NB: countMismatch and equivalentPropVector are created with
//different intentions and hence both are maintained
int countMismatch( PROPAGATED_VECTOR *pv1, PROPAGATED_VECTOR *pv2 )
{
  int i, j, k, moreArrayValues;
  //For storing current number of mismatches between 2 value vectors
  unsigned int cur_mismatch = 0;

  if(pv1==(PROPAGATED_VECTOR*)NULL || pv2==(PROPAGATED_VECTOR*)NULL)
  {
    #ifdef DEbug
    printf("\n In countMismatch(), atleast one of vectors is NULL, returning -1\n");
    #endif

    return -1;
  }

  for(i = 0; i < NO_OF_VARIABLES; i++)
  {
    if(compare_trees(pv1->value[i], pv2->value[i]) == 0)
    {
      cur_mismatch++;

      //incrementing the count for code motions
      for(j = 0; j < V0_V1.no_of_elements; j++)
	    if(stab.val_tab[i] == V0_V1.var_val[j])
	    {
	      COUNT_CODE_MOTIONS++;
	      break;
	    }
    }

    //array start
    moreArrayValues = (pv1->subVector[i].countValues >= pv2->subVector[i].countValues) ? pv1->subVector[i].countValues : pv2->subVector[i].countValues;
    for(k = 0; k < moreArrayValues; k++)
    {
	  if(compare_trees(pv1->value[i], pv2->value[i]) == 0)
      {
        cur_mismatch++;

        //incrementing the count for code motions
        COUNT_CODE_MOTIONS++;
        //the check for common variable is omitted to avoid repetitive
        //checks for the same array
      }
	}
    //array end
  }

  if(cur_mismatch > MAX_MISMATCH)
    MAX_MISMATCH = cur_mismatch;

  #ifdef DEbug
  printf("\n Out of countMismatch, #mismatches = %d\n", cur_mismatch);
  #endif

  return cur_mismatch;
}


/************* Functions for E_u, E_c, LIST *************/

/******************************/
//Used to initialize E
//Also returns the same address
/******************************/
PATH_PAIR* initE()
{
  PATH_PAIR *E = (PATH_PAIR*)malloc(sizeof(PATH_PAIR));

  //E is initialized to some garbage value
  E->p0 = -1;
  E->p1 = -1;
  E->pv0 = (PROPAGATED_VECTOR*)NULL;
  E->pv1 = (PROPAGATED_VECTOR*)NULL;
  E->next = (PATH_PAIR*)NULL;

  return E;
}


/****************************************************************************************/
/*       This function checks whether pair <p0, p1> is in E or not. if it is not present
         then adds the pair at the end of E    */
//Similar to unionEta
/****************************************************************************************/

void unionE( PATH_PAIR *E, int p0, int p1, PROPAGATED_VECTOR *pv0, PROPAGATED_VECTOR *pv1, PATHS_LIST *P0, PATHS_LIST *P1 )
{
   PATH_PAIR *temp,*temp1,*temp2;

   if(E==(PATH_PAIR*)NULL)
   {
     printf("\n In unionE() with NULL E\nERROR: Exiting System\n");
     exit(0);
   }

   //The following checks are required to prevent duplicate entries for null paths in E_c
   //If one is not bothered about duplicate null paths in E_c, then only the code in the
   //body of the first "if" would suffice
   if( p0 < P0->numpaths && p1 < P1->numpaths )
   {
		temp = E;
		while(temp != (PATH_PAIR*)NULL)
		{
		//checks for <p0, p1> in E
		if( temp->p0 == p0 && temp->p1 == p1 && equivalentPropagatedVector(temp->pv0, pv0) && equivalentPropagatedVector(temp->pv1, pv1) )
			return;

		//did not invoke already_present_LIST because we need to stop
		//before the NULL entry, achieved by the following 2 lines
		temp2 = temp;
		temp = temp->next;
		}//end while
	}
	else if( p0 >= P0->numpaths)
	{
		temp = E;
		while(temp != (PATH_PAIR*)NULL)
		{
		//checks for <p0, p1> in E
		//temp->p0 >= P0->numpaths ensures it is a null path AND
		//P0->paths[temp->p0].start == P0->paths[p0].start ensures that both are null paths at the same state
		if( (temp->p0 >= P0->numpaths && P0->paths[temp->p0].start == P0->paths[p0].start)
		    && temp->p1 == p1 && equivalentPropagatedVector(temp->pv0, pv0) && equivalentPropagatedVector(temp->pv1, pv1) )
			return;

		temp2 = temp;
		temp = temp->next;
		}//end while
	}
	else
	{
		temp = E;
		while(temp != (PATH_PAIR*)NULL)
		{
		//checks for <p0, p1> in E
		if( (temp->p1 >= P1->numpaths && P1->paths[temp->p1].start == P1->paths[p1].start)
		    && temp->p0 == p0 && equivalentPropagatedVector(temp->pv0, pv0) && equivalentPropagatedVector(temp->pv1, pv1) )
			return;

		temp2 = temp;
		temp = temp->next;
		}//end while
	}

    //<p0, p1> is not in E
    temp1 = (PATH_PAIR*)malloc(sizeof(PATH_PAIR));
    temp1->p0 = p0;
    temp1->p1 = p1;
    temp1->pv0 = (PROPAGATED_VECTOR*)malloc(sizeof(PROPAGATED_VECTOR));
    copyPropagatedVector( temp1->pv0, pv0 );
    temp1->pv1 = (PROPAGATED_VECTOR*)malloc(sizeof(PROPAGATED_VECTOR));
    copyPropagatedVector( temp1->pv1, pv1 );
    temp1->next = (PATH_PAIR*)NULL;
    temp2->next = temp1;
}


//Prints E which may store U-equivalent, C-equivalent or
//(potential) C-equivalent (i.e., LIST) path pairs
void printE( PATH_PAIR *E, PATHS_LIST *P0, PATHS_LIST *P1, FSMD *M0, FSMD *M1 )
{
  unsigned int count = 0;

  while( E != (PATH_PAIR*)NULL )
  {
    //The following check is used to prevent printing
    //the initial element of E (which is garbage)
    if(E->p0 != -1)
    {

		printf("\n\n%d)\nPath of M0:\n",++count);

		if( E->p0 >= P0->numpaths )
			printf("\n\tNull Path at %s\n", M0->states[P0->paths[E->p0].start].state_id);
		else
			printPath( &(P0->paths[E->p0]), P0, E->p0 );
		printPropagatedVector( E->pv0 );

		printf("\n\t is equivalent to\nPath of M1:\n");

		if( E->p1 >= P1->numpaths )
			printf("\n\tNull Path at %s\n", M1->states[P1->paths[E->p1].start].state_id);
		else
			printPath( &(P1->paths[E->p1]), P1, E->p1 );
		printPropagatedVector( E->pv1 );

    }//end if E->p0 != -1

    E = E->next;
  }//end while
}


/************* Functions for LIST (specifically) *************/

/****************************************************************************************/
/*       This function checks whether the pair <endState(p0), endState(p1)> in the
 *       latest entry of LIST is already present in the previous entries of LIST or not.
 *       If it is present, then returns TRUE
                     otherwise returns FALSE    */
//       This check is required to prevent recursions of correspondenceChecker with states
//       already present in the same recurion tree (which will lead to an infinite loop).
//       Note that this function is invoked only when neither p0 nor p1 is a null path
/****************************************************************************************/

boolean already_present_LIST( PATH_PAIR *LIST, int p0, int p1, PATHS_LIST *P0, PATHS_LIST *P1, FSMD *M0, FSMD *M1 )
{
	PATH_PAIR *tempList;

	if(LIST == (PATH_PAIR*)NULL)
	{
		printf("\n In already_present_LIST() with NULL E\nERROR: Exiting System\n");
        exit(0);
	}

	#ifdef DEbug
	printf("\n In already_present_LIST() with path ids (%d, %d)", p0, p1);
	#endif

	tempList = LIST;

	while( tempList != (PATH_PAIR*)NULL )
	{
		if(tempList->p0 != -1 || tempList->p1 != -1)
		{
			if( strcmp(M0->states[P0->paths[tempList->p0].start].state_id, M0->states[P0->paths[p0].end].state_id)==0 &&
				strcmp(M1->states[P1->paths[tempList->p1].start].state_id, M1->states[P1->paths[p1].end].state_id)==0 )
			{
				#ifdef DEbug
				printf("\n Out of already_present_LIST() with TRUE - Start\n");
				#endif

				return TRUE;
			}
			if( strcmp(M0->states[P0->paths[tempList->p0].end].state_id, M0->states[P0->paths[p0].end].state_id)==0 &&
				strcmp(M1->states[P1->paths[tempList->p1].end].state_id, M1->states[P1->paths[p1].end].state_id)==0 )
			{
				#ifdef DEbug
				printf("\n Out of already_present_LIST() with TRUE - End\n");
				#endif

				return TRUE;
			}
		}
		tempList = tempList->next;
	}

	//Check for propagated vectors has not been done because
	//revisiting the same state with different propagated vectors
	//would automatically fail the loop-invariant test

	#ifdef DEbug
	printf("\n Out of already_present_LIST() with FALSE\n");
	#endif

	return FALSE;
}



//This function removes the last path-pair from LIST
//It is invoked on returning from a recursive call to correspondenceChecker
//The function returns 0 if LIST is empty
//                     1 if LIST has only 1 entry
//                     2 otherwise
int removeLastPathPairFromLIST( PATH_PAIR *LIST, PATHS_LIST *P0, PATHS_LIST *P1, FSMD *M0, FSMD *M1 )
{
	PATH_PAIR *tempList1, *tempList2;

	if(LIST==(PATH_PAIR*)NULL)
    {
		printf("\n In removeLastPathPairFromLIST() with NULL E\nERROR: Exiting System\n");
		exit(0);
    }

	//The following check is necessary because when this function is called
	//after discovery of U-equivalent path-pair, there may be no path-pair to
	//remove from LIST
	if(LIST->next != (PATH_PAIR*)NULL)
	{
		if(LIST->next->next != (PATH_PAIR*)NULL)
		{
			tempList1 = LIST->next;
			tempList2 = LIST->next->next;
			while(tempList2->next != (PATH_PAIR*)NULL)
			{
				tempList1 = tempList1->next;
				tempList2 = tempList2->next;
			}

			#ifdef DEbug
			printf("\n Removing the following path-pair from LIST:\n");
			printE(tempList2, P0, P1, M0, M1);
			#endif

			tempList1->next = (PATH_PAIR*)NULL;
			return 2;
		}
		else
		{
			#ifdef DEbug
			printf("\n Removing the following path-pair from LIST:\n");
			printE(LIST->next->next, P0, P1, M0, M1);
			#endif

			LIST->next = (PATH_PAIR*)NULL;
			return 1;
		}
	}
	else
	{
		#ifdef DEbug
		printf("\n removeLastPathPairFromLIST called with empty LIST\n");
		#endif

		return 0;
	}
}


/************* Functions for eta, C_eta *************/

//To check whether a corresponding state pair is already
//present in the queue (returns TRUE) or not (returns FALSE)
//Note that since "eta" stores corresponding state pairs
//(and not C-corresponding state pairs), this check saves
//execution time and ensures Termination
boolean already_present_queue( CPS_Q *queue, int s0, int s1, unsigned int state_pair_visited[] )
{
  int i;

  if(queue==(CPS_Q*)NULL)
  {
    printf("\n In already_present_queue() with a NUll queue\nERROR: Exiting System\n");
    exit(0);
  }

  for( i=0; i <= queue->end; i++ )
  {
    if(queue->cstate[i].s0 == s0 && queue->cstate[i].s1 == s1)
    {
      #ifdef DEbug
      printf("\n Out of already_present_queue with return value TRUE because match is found for entry number: %d\n",i);
      #endif

      state_pair_visited[i] = state_pair_visited[i] + 1;
      return TRUE;
    }
  }

  #ifdef DEbug
  printf("\n Out of already_present_queue with return value FALSE\n");
  #endif

  return FALSE;
}



//Prints eta which stores corresponding state pairs
void printEta( CPS_Q *eta, FSMD *M0, FSMD *M1, unsigned int state_pair_visited[] )
{
  int i;

  if(eta==(CPS_Q*)NULL)
  {
    printf("\nWARNING: In printEta() with NULL eta\n");
    return;
  }
  if(M0==(FSMD*)NULL || M1==(FSMD*)NULL)
  {
    printf("\nWARNING: In printEta() with a NULL FSMD\n");
    return;
  }

  for( i=0; i <= eta->end; i++ )
  {
    printf("\t%d. ( %s, %s )\t", i+1, M0->states[eta->cstate[i].s0].state_id, M1->states[eta->cstate[i].s1].state_id);

    #ifdef DEbug
    printf("#visited: %d", state_pair_visited[i] + 1);
    //the additional (+1) is a compensate for initializing with 0 (instead of 1)
    #endif
  }
}



//To initialize corresponding_state_pair_visited[]
void initializeStatePairVisited( unsigned int state_pair_visited[] )
{
  int i;

  for( i=0; i < QUEUESIZE; i++ )
    state_pair_visited[i] = 0;
}



/************* Functions related to value propagation *************/

//Creates a dummy path node
//Inputs: A normalized expression and a flag
//If the flag is -1, then the expression is stored as node's condition
//If the flag is positive, then the expression is stored as node's
//actions corresponding to variable with index [flag]
PATH_NODE* createDummyNode( NC *expression, int flagIndex )
{
  PATH_NODE *node = (PATH_NODE*)malloc(sizeof(PATH_NODE));

  if(expression==(NC*)NULL)
  {
    #ifdef DEbug
    printf("\nWARNING: In createDummyNode() with NULL expression\n");
    //exit(0);
    #endif
  }

  node->state = INDEX_OF_DUMMY_STATE;
  if(flagIndex == -1)
  {
    node->condition = copylist(expression);
    node->actions = (DATA_TRANS*)malloc(sizeof(DATA_TRANS));
    node->actions[0].lhs = 0;
    node->actions[0].rhs = (NC*)NULL;
  }
  else
  {
    node->condition = (NC*)NULL;
    node->actions = (DATA_TRANS*)malloc(2*sizeof(DATA_TRANS));
    node->actions[0].lhs = stab.val_tab[flagIndex];
    node->actions[0].rhs = copylist(expression);
    node->actions[1].lhs = 0;
    node->actions[1].rhs = (NC*)NULL;
  }
  node->next = (PATH_NODE*)NULL;
  node->left = (PATH_NODE*)NULL;

  return node;
}



//Creates a dummy path with (R = pv_gamma.cond) & (s = pv_gamma.value)
//Appends gamma to this dummy path and finds out the path characteristics
//of the resulting concatenated path
//Used to find the modified R and s for gamma based on pv_gamma
PATH_ST* createDummyPath( PATH_ST *gamma, PROPAGATED_VECTOR *pv_gamma )
{
  int i, j, flagHead = 1;
  r_alpha *r_alpha_temp, *r_alpha_temp2;
  PATH_ST *gammaDummy, *gammaModified; //dummy path and updated path
  PATH_NODE *headState, *tempState;

  if(gamma==(PATH_ST*)NULL)
  {
    printf("\n In createDummyPath() with NULL path\nERROR: Exiting System\n");
    exit(0);
  }
  if(pv_gamma==(PROPAGATED_VECTOR*)NULL)
  {
    printf("\n In createDummyPath() with NULL vector\nERROR: Exiting System\n");
    exit(0);
  }

  #ifdef DEbug
  printf("\n In createDummyPath()\n");
  if(gamma->condition == (NC*)NULL && gamma->transformations == (r_alpha*)NULL)
    printf("\n Null path is passed with state index %d\n", gamma->start);
  else
    displayApath(gamma);
  printPropagatedVector(pv_gamma);
  #endif

  gammaDummy = (PATH_ST*)malloc(sizeof(PATH_ST));

  gammaDummy->start = gamma->start;
  gammaDummy->end = gamma->start; //starts and ends in the same state
  gammaDummy->status = gamma->status;
  gammaDummy->extendible = gamma->extendible;
  gammaDummy->concateble = gamma->concateble;

  gammaDummy->condition = copylist(pv_gamma->cond);

  headState = createDummyNode(pv_gamma->cond, -1);
  gammaDummy->nodeslist = headState;

  for( i=0; i< NO_OF_VARIABLES; i++ )
  {
	//the following check prevents values for - (minus)
	//which occurs in input file to indicate absence of expression
	if(stab.val_tab[i] == 0)
		continue;

    if(pv_gamma->value[i] != (NC*)NULL)
    {
	  r_alpha_temp = (r_alpha*)malloc(sizeof(r_alpha));
	  r_alpha_temp->Assign.lhs = stab.val_tab[i];
	  r_alpha_temp->Assign.rhs = copylist(pv_gamma->value[i]);
	  r_alpha_temp->next = (r_alpha*)NULL;

      tempState = createDummyNode(pv_gamma->value[i], i);
      headState->next = tempState;
      tempState->left = headState;
      headState = headState->next;

      if(flagHead == 0)
      {
	    r_alpha_temp2->next = r_alpha_temp;
	    r_alpha_temp2 = r_alpha_temp2->next;
      } //end if flagHead = 0

      if(flagHead == 1)
      {
	    gammaDummy->transformations = r_alpha_temp;
	    r_alpha_temp2 = r_alpha_temp;
	    flagHead = 0;
      } //end if flagHead = 1

    }

    //array start
    //NB for an array, value[i] MUST be NULL but arrayValue(s) may be non-empty
    if(pv_gamma->subVector[i].countValues > 0)
    {
		for(j = 0; j < pv_gamma->subVector[i].countValues; j++)
		{
			if(pv_gamma->subVector[i].arrayValue[j] != (NC*)NULL)
			{
				r_alpha_temp = (r_alpha*)malloc(sizeof(r_alpha));
				r_alpha_temp->Assign.lhs = stab.val_tab[i];
				r_alpha_temp->Assign.rhs = copylist(pv_gamma->subVector[i].arrayValue[j]);
				r_alpha_temp->next = (r_alpha*)NULL;

				tempState = createDummyNode(pv_gamma->subVector[i].arrayValue[j], i);
				headState->next = tempState;
				tempState->left = headState;
				headState = headState->next;

				if(flagHead == 0)
				{
					r_alpha_temp2->next = r_alpha_temp;
					r_alpha_temp2 = r_alpha_temp2->next;
				} //end if flagHead = 0

				if(flagHead == 1)
				{
					gammaDummy->transformations = r_alpha_temp;
					r_alpha_temp2 = r_alpha_temp;
					flagHead = 0;
				} //end if flagHead = 1
			 }
		} //end for j
	}
    //array end

  } //end for i

  gammaDummy->cp = (PATH_ST*)NULL;
  gammaDummy->next = (PATH_ST*)NULL;

  #ifdef DEbug
  printf("\n Displaying the dummy path created\n");
  displayApath(gammaDummy);
  printf("\n End of displaying the dummy path created\n");
  #endif

  if(gamma->condition == (NC*)NULL && gamma->transformations == (r_alpha*)NULL)
  {
    //applicable for null paths only
    return gammaDummy;
  }
  else
  {
    gammaModified = concatPaths(gammaDummy, gamma); //also finds R and r of gammaModified

    //patch for Ralpha
    if(gammaDummy->condition != (NC*)NULL)
    {
		patchRalpha(gammaModified, gammaDummy->condition);
	}

    #ifdef DEbug
    printf("\n Displaying concatenated path\n");
    displayApath(gammaModified);
    printf("\n End of displaying concatenated path\n");
    #endif
  }

  return gammaModified;
}



//Input: A state, a path cover
//Output: Updated path cover with a null path at that state,
//start state = end state for null path (similar to dummy path)
//all other elements are theoretically non-existent
void createNullPath( int stateIndex, PATHS_LIST *P )
{

	int path_id;

	//Note: A state can have multiple null paths associated with it

	path_id = P->numpaths + P->offset_null_path;

	P->paths[path_id].start = stateIndex;
	P->paths[path_id].end  = stateIndex;

	//the following definitions are useless since they are never used
	P->paths[path_id].status = 0;
	P->paths[path_id].extendible = 0;
	P->paths[path_id].concateble = 0;
	P->paths[path_id].condition = (NC*)NULL;
	P->paths[path_id].transformations = (r_alpha*)NULL;
	P->paths[path_id].nodeslist = (PATH_NODE*)NULL;
	P->paths[path_id].cp =(PATH_ST*)NULL;
	P->paths[path_id].next = (PATH_ST*)NULL;
	//care should be taken that none of the items defined above is accessed

	(P->offset_null_path)++;

	#ifdef DEbug
	printf("\n Out of createNullPath( %d )\n", stateIndex);
	#endif
}



//To remove persisting identical relations (R) within
//the same condition
NC* removeIdenticalRelations( NC *condition )
{
	NC *Tcond1, *Tcond2, *prevCond2, *retCond;
	boolean flag;

	Tcond1 = copylist(condition);
	retCond = Tcond1;


	Tcond1 = Tcond1->link;
	while( Tcond1->list != (NC*)NULL )
	{
		prevCond2 = Tcond1;
		Tcond2 = Tcond1->list;

		while( Tcond2 != (NC*)NULL )
		{
			flag = TRUE;
			if(compare_trees(Tcond1->link, Tcond2->link) == 1)
			{
				prevCond2->list = Tcond2->list;
				flag = FALSE;
			}
			Tcond2 = Tcond2->list;
			if( flag )
				prevCond2 = prevCond2->list;

		}
		Tcond1 = Tcond1->list;
	}

	return retCond;
}



//Patch for find_Ralpha
//Inputs: A path, and a condition (c1)
//Let the path's condition be c2
//Replaces the condition of the path with (c1 AND c2)
void patchRalpha( PATH_ST *path, NC *condition )
{
	NC *cond1, *cond2, *tempC;
	NC *t1, *t2, *node;
	boolean flag;

	if(path == (PATH_ST*)NULL)
	{
		printf("\n In patchRalpha with a NULL path\nERROR: Exiting system\n");
	}

	cond1 = copylist(condition);
	cond2 = copylist(path->condition);

	tempC = cond1->link;
	while( tempC->list != (NC*)NULL )
	{
		tempC = tempC->list;
	}

	//Appending (ANDing) cond1 and cond2
	//tempC->list = cond2->link;
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

		if( flag == TRUE )
		{
			node = (NC*)malloc(sizeof(NC));
			node->list = (NC*)NULL;
			node->type = 'O';
			node->inc = 0;
			node->link = copylist( t2->link );

			tempC->list = node;
			tempC = node;
		}

		t2 = t2->list;
	}

	path->condition = cond1;
	//path->condition = removeIdenticalRelations( cond1 );
}



//This function is used to find the index value of a symbol in SYMTAB stab
//Input: A value of val_tab
//Output: Corresponding sym_tab
//Note that "stab" is a global variable
//Returns -1 if the value is absent
int findSymbolTableIndex( int value )
{
  int i;

  for( i=0; i < stab.numsymbols; i++ )
  {
    if(stab.val_tab[i] == value)
      return i;
  }

  #ifdef DEbug
  printf("\n The symbol being searched for is not present in the symbol table\n");
  printf("ERROR: Exiting system\n");
  exit(0);
  #endif

  return -1;
}



//Function to find out which variables
//occur in rhs, and sets the value
//corresponding to that varible to 2 in flags[]
//Required by valuePropagation()
void modifyFlags( NC *rhs, int flags[] )
{
   int temp;

   if(rhs != (NC*)NULL)
   {
       if(rhs->type == 'v')
       {
	     temp = findSymbolTableIndex(rhs->inc);

	     if(flags[temp]!=3)
	       flags[temp] = 2;
       }

       //array start
       //This function "modifyFlags" will be redundant if all values are propagated blindly,
       //especially in the presence of arrays.
       if(rhs->type == 'a')
       {
	     temp = findSymbolTableIndex(rhs->inc);

	     if(flags[temp]!=3)
	       flags[temp] = 2;
       }
       //array end

       modifyFlags(rhs->link, flags);
       modifyFlags(rhs->list, flags);
   }
}


/************* Miscellaneous Functions *************/

//To check the feasibility of a condition
boolean checkFeasibilityOfCondition( NC *condition )
{
	NC *cond, *condR;

	if(condition == (NC*)NULL)
		return TRUE;

	cond = condition->link; //cond is at 'O' (OR) level

	while(cond != (NC*)NULL)
	{
		condR = cond->link; //condR is at 'R' level

		while(condR != (NC*)NULL)
		{
			//NB: All the feasibility checks done here is possible only if
			//there is a constant
			if(condR->link->link != (NC*)NULL)
			{
				condR = condR->list;
				continue;
			}
			//relational operators: >= 0, > 1, <= 2, < 3, == 4, != 5

			//Case 0: To avoid c >= 0 where c < 0
			if(condR->inc == 0 && condR->link->inc < 0)
			{
				#ifdef DEbug
				printf("\nCase 0: c >= 0 where c < 0\n");
				#endif

				return FALSE;
			}
			//Case 1: To avoid c > 0 where c <= 0
			if(condR->inc == 1 && condR->link->inc <= 0)
			{
				#ifdef DEbug
				printf("\nCase 1: c > 0 where c <= 0\n");
				#endif

				return FALSE;
			}
			//Case 2: To avoid c <= 0 where c > 0
			if(condR->inc == 2 && condR->link->inc > 0)
			{
				#ifdef DEbug
				printf("\nCase 2: c <= 0 where c > 0\n");
				#endif

				return FALSE;
			}
			//Case 3: To avoid c < 0 where c >= 0
			if(condR->inc == 3 && condR->link->inc >= 0)
			{
				#ifdef DEbug
				printf("\nCase 3: c < 0 where c >= 0\n");
				#endif

				return FALSE;
			}
			//Case 4: To avoid c == 0 where c != 0
			if(condR->inc == 4 && condR->link->inc != 0)
			{
				#ifdef DEbug
				printf("\nCase 4: c == 0 where c != 0\n");
				#endif

				return FALSE;
			}
			//Case 5: To avoid c != 0 where c == 0
			if(condR->inc == 5 && condR->link->inc == 0)
			{
				#ifdef DEbug
				printf("\nCase 5: c != 0 where c == 0\n");
				#endif

				return FALSE;
			}

			condR = condR->list;
		}

		cond = cond->list;
	}

	return TRUE;
}

/*******************************************************************************/
/*   This function checks whether 2 conditions are compatible or not
     It returns 0   if condition1 has some conjunct c1, and
                       condition2 has some conjunct !c1
                1   otherwise                                     */
/*******************************************************************************/
boolean compatibleConditions( NC *cond1, NC *cond2 )
{
      NC *t, *temp;

      #ifdef DEbug
      printf("\n In compatibleConditions() with arguments\n");
	  printf(" First Condition:"); write_lists(cond1);
	  printf("\n Second Condition:"); write_lists(cond2);
      #endif

      //To check whether the conditions are individually feasible or not
      //if( !(checkFeasibilityOfCondition( cond1 ) && checkFeasibilityOfCondition( cond2 )) )
		//return FALSE;

      //cond1 = removeIdenticalRelations( cond1 );
      //cond2 = removeIdenticalRelations( cond2 );

      if(cond1 == (NC*)NULL || cond2 == (NC*)NULL)
      {
		// no condition of execution in either means they are compatible
	    return TRUE;
      }

	  temp=cond2->link;
      while(temp != (NC*)NULL)
	  {
		t=cond1->link;
	    while(t != (NC*)NULL)
	    {
			//negate conditions are detected at 'R' level
	        if( temp->link->type != 'R' || t->link->type != 'R' )
	        {
				printf("\n Problem in comparing relations\nERROR: Exiting System\n");
				exit(0);
			}
	        if(compare_trees( t->link->link, temp->link->link ) == 1)
	        {
				//relational operators: >= 0, > 1, <= 2, < 3, == 4, != 5
				if( (temp->link->inc == 4 && t->link->inc == 5) || (temp->link->inc == 5 && t->link->inc == 4) ||
				    (temp->link->inc == 0 && t->link->inc == 3) || (temp->link->inc == 3 && t->link->inc == 0) ||
				    (temp->link->inc == 1 && t->link->inc == 2) || (temp->link->inc == 2 && t->link->inc == 1) )
				{
					#ifdef DEbug
					printf("\n Out of compatibleConditions() with return value 0\n");
					#endif

					return FALSE;
				}
			}
			t=t->list;
	     }//end while t
	     temp=temp->list;
	  }//end while temp

	  #ifdef DEbug
	  printf("\n Out of compatibleConditions() with return value 1\n");
	  #endif

	  return TRUE;
}



//To print a path
void printPath( PATH_ST *p, PATHS_LIST *P, int pathIndex )
{

    char *sym_value;
	r_alpha *temp1;
	PATH_NODE *t;

    sym_value = (char*)malloc(1000 * sizeof( char ));


    //Null paths are taken care of in printE itself
    //So only non-null paths are printed (showing individual states)
    if(pathIndex < P->numpaths)
    {
		t = P->paths[pathIndex].nodeslist;
		while(t->next != (PATH_NODE*)NULL)
		{
			printf("%s --> ", P->fsmd->states[t->state].state_id);
			t = t->next;
		}
		printf("%s", P->fsmd->states[t->state].state_id);
	}
	printf("\n");

	write_lists(p->condition);
	temp1=p->transformations;
	while(temp1!=NULL)
	{
		symbol_for_index( temp1->Assign.lhs, sym_value );
		printf("\n %s := ", sym_value );
		write_lists(temp1->Assign.rhs);
		temp1=temp1->next;
	}
}


//Input:  A variable x, a list L of (Live) variables
//Output: TRUE if x appears in L, FALSE otherwise
boolean isVariableLive( int x, LV* L )
{
	int i;

	if(L == (LV*)NULL)
		return FALSE;

	for(i = 0; i < L->countLiveVar; i++)
	{
		if(L->liveVar[i] == x)
			return TRUE;
	}

	return FALSE;
}


extern boolean isOutputVar( int, var_list );

//Input:  Two r_alpha's r_1 and r_2
//Output: TRUE if some output variable x that appears in both r_1 and r_2
//has different values in the two r_alpha's; FALSE otherwise
//NB: If output variable x appears in r_1 but not in r_2 then this function will
//return FALSE (assuming there is no conflict for some other output variable) -
//this is done to allow 'subsequent' computation of x in the other FSMD
boolean conflictInOutputs( r_alpha *r_1, r_alpha *r_2 )
{
	r_alpha *t1, *t2;

	if( r_1 == (r_alpha*)NULL || r_2 == (r_alpha*)NULL )
		return FALSE;

	//NB: Output variables are basically the "port" variables (which are scalars)
	//The following computations are equivalent
	//p1:  port0 = 10, p2:  port0 = 20, p3:  port1 = 30
	//p1':  (empty)  , p2': port0 = 10, p3': port0 = 20, port1 = 30
	//But when the paths p2 and p2' are compared the values for port0 will mismatch
	//leading to the conclusion that the computations are not equivalent. -- This
	//occurs beacuse port0 is used to write values multiple times.
	//Hence, using a different name for each port is advised - this is also the usual
	//scenario beacuse each signal in hardware is put on a different wire

	t1 = r_1;
	while(t1 != (r_alpha*)NULL)
	{
		if( !(isOutputVar(t1->Assign.lhs, outputVar0)) )
		{
			t1 = t1->next;
			continue;
		}

		t2 = r_2;
		while(t2 != (r_alpha*)NULL)
		{
			if(t1->Assign.lhs == t2->Assign.lhs)
			{
				if(compare_trees(t1->Assign.rhs, t2->Assign.rhs) == 1)
					break;
				else
					return TRUE; //conflict in values
			}

			t2 = t2->next;
		}

		t1 = t1->next;
	}


	t2 = r_2;
	while(t2 != (r_alpha*)NULL)
	{
		if( !(isOutputVar(t2->Assign.lhs, outputVar1)) )
		{
			t2 = t2->next;
			continue;
		}

		t1 = r_1;
		while(t1 != (r_alpha*)NULL)
		{
			if(t2->Assign.lhs == t1->Assign.lhs)
			{
				if(compare_trees(t2->Assign.rhs, t1->Assign.rhs) == 1)
					break;
				else
					return TRUE; //conflict in values
			}

			t1 = t1->next;
		}

		t2 = t2->next;
	}

	return FALSE; //No conflict in values identified
}


/************* MAJOR Functions (that appear in the paper) *************/

//Comment the following #define
//if one wants to propagate values based on the old scheme
//(i.e., without taking live variables into consideration)
#define LVA


//Un-Comment the following #define
//if one wants to propagate all values on finding
//at least 1 mismatch for a "common" variable
//(NB: does not take live variables into consideration)
//#define PROPAGATE_ALL

//Function to achieve propagation of values
//uses createDummyPath() to find modified path characteristics
//of paths beta and alpha, and calculates
//the vectors propagated to the end states of beta and alpha
void valuePropagation( PATH_ST *beta, PROPAGATED_VECTOR *pv_beta_in, PATH_ST *alpha, PROPAGATED_VECTOR *pv_alpha_in,
		       PROPAGATED_VECTOR *pv_beta_out, PROPAGATED_VECTOR *pv_alpha_out )
{
  int i, j, temp, flagAbsent;

  #ifndef PROPAGATE_ALL
  int flagSymbValue[NO_OF_VARIABLES];
  #else
  boolean flagDifferent;
  #endif

  PATH_ST *betaModified, *alphaModified;
  r_alpha *s_beta, *s_alpha;

  //array start
  int flagAbsentArray;
  //Values for flagAbsentArray -- 2 if the variable is scalar
  //                           -- 1 if the variable is array and absent
  //                           -- 0 if the variable is array and present
  //array end

  if(beta==(PATH_ST*)NULL || alpha==(PATH_ST*)NULL)
  {
    printf("\n In valuePropagation() with a NULL path\nERROR: Exiting System\n");
    exit(0);
  }
  if(pv_beta_in==(PROPAGATED_VECTOR*)NULL || pv_alpha_in==(PROPAGATED_VECTOR*)NULL)
  {
    printf("\n In valuePropagation() with a NULL input vector\nERROR: Exiting System\n");
    exit(0);
  }
  if(pv_beta_out==(PROPAGATED_VECTOR*)NULL || pv_alpha_out==(PROPAGATED_VECTOR*)NULL)
  {
    printf("\n In valuePropagation() with a NULL output vector\nERROR: Exiting System\n");
    exit(0);
  }

  #ifdef DEbug
  printf("\n In valuePropagation()\n");
  if(beta->condition == (NC*)NULL && beta->transformations == (r_alpha*)NULL)
    printf("A null path has been passed with state index %d\n", beta->start);
  else
    displayApath(beta);
  printPropagatedVector(pv_beta_in);

  if(alpha->condition == (NC*)NULL && alpha->transformations == (r_alpha*)NULL)
    printf("A null path has been passed with state index %d", alpha->start);
  else
    displayApath(alpha);
  printPropagatedVector(pv_alpha_in);
  #endif

  betaModified = createDummyPath(beta, pv_beta_in);
  alphaModified = createDummyPath(alpha, pv_alpha_in);
  //Note: These betaModified and alphaModified were already computed
  //in findEquivalentPath. However, this redundancy is not removed to
  //maintain uniformity with the Paper.

  #ifdef DEbug
  printf("Modified paths\nPath beta'\n");
  displayApath(betaModified);
  printf("\nPath alpha'\n");
  displayApath(alphaModified);
  #endif

  pv_beta_out->cond = copylist(betaModified->condition);
  pv_alpha_out->cond = copylist(alphaModified->condition);

  for(i = 0; i < NO_OF_VARIABLES; i++)
  {
    pv_beta_out->value[i] = (NC*)NULL;
    pv_alpha_out->value[i] = (NC*)NULL;

    //array start
    pv_beta_out->subVector[i].countValues = 0;
    pv_alpha_out->subVector[i].countValues = 0;
    for(j = 0; j < MAX_ARRAY_VALUES; j++)
    {
		pv_beta_out->subVector[i].arrayValue[j] = (NC*)NULL;
		pv_alpha_out->subVector[i].arrayValue[j] = (NC*)NULL;
	}
    //array end
  }

  #ifdef LVA

  //LVA -- similar to older value propagation
  //In the former, we checked for equivalence between (identical) common variables
  //Now, we check for common variables that are LIVE
  //If No mismatch for any common variable occurs, but some uncommon variable is
  //still Live, then also we propagate values (for that uncommon variable)

  //the array flagSymbValue[] is used for converting the value of a variable
  //in PROPAGATED_VECTOR back to its symbolic value
  //0 stored initially,
  //3 on finding mismatch,
  //2 if some other (mismatched) variable depends on it,
  //1 if the values match and no other (mismatched) variable depends on it
  for( i=0; i < NO_OF_VARIABLES; i++ )
    flagSymbValue[i] = 0;


  s_beta = betaModified->transformations;
  while(s_beta != (r_alpha*)NULL)
  {
	//the following check prevents uselesss calculation for - (minus)
	if(s_beta->Assign.lhs == 0)
	{
		s_beta = s_beta->next;
		continue;
	}

    s_alpha = alphaModified->transformations;
    temp = findSymbolTableIndex(s_beta->Assign.lhs);

    //array start

    if(s_beta->Assign.rhs != (NC*)NULL && s_beta->Assign.rhs->type == 'w')
	  flagAbsentArray = 1;
	else
	  flagAbsentArray = 2;

	//The lhs may be an array variable
	//So, there may be multiple entries corresponding to it
	//Therefore, the whole s_alpha has to be checked for a match
	if(flagAbsentArray == 1)
	{
		while(s_alpha != (r_alpha*)NULL)
		{
			if(s_beta->Assign.lhs == s_alpha->Assign.lhs && compare_trees(s_beta->Assign.rhs, s_alpha->Assign.rhs) == 1)
			{
				flagAbsentArray = 0;
				break;
			}
			s_alpha = s_alpha->next;
		}

		if(flagAbsentArray != 0)
		{
			//Match not found
			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_beta->Assign.rhs); printf("occurs in beta only\n");
			#endif

			if( (beta->finalPath  || isVariableLive(s_beta->Assign.lhs, beta->live)) ||
				(alpha->finalPath || isVariableLive(s_beta->Assign.lhs, alpha->live)) )
			{
				//the variable is live

				pv_beta_out->subVector[temp].arrayValue[pv_beta_out->subVector[temp].countValues] = copylist(s_beta->Assign.rhs);
				(pv_beta_out->subVector[temp].countValues)++;

				flagSymbValue[temp] = 3;

				//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
				modifyFlags(s_beta->Assign.rhs, flagSymbValue);
			}
			else
			{
				#ifdef DEbug
				printf("%s is Not Live\n", stab.sym_tab[temp]);
				#endif
			}
		}
	}//end if (flagAbsentArray == 1)

	else if(flagAbsentArray == 2)
	{
	//array end

		flagAbsent = 1;

		while(s_alpha != (r_alpha*)NULL)
		{
			//to check whether the transformations are for the same variable or not
			if( s_beta->Assign.lhs == s_alpha->Assign.lhs )
			{
				flagAbsent = 0;

				if(compare_trees(s_beta->Assign.rhs, s_alpha->Assign.rhs) == 1)
				{
					if(flagSymbValue[temp] == 0)
						flagSymbValue[temp] = 1;
						//flagSymbValue[temp] = 2; //KB - to force PA
				}
				else
				{
					#ifdef DEbug
					printf("\nMismatch for variable: %s\n First (beta) Expression\n", stab.sym_tab[temp]);
					write_lists(s_beta->Assign.rhs);
					printf("\n Second (alpha) Expression\n");
					write_lists(s_alpha->Assign.rhs);
					#endif

					if( (beta->finalPath  || isVariableLive(s_beta->Assign.lhs, beta->live)) ||
						(alpha->finalPath || isVariableLive(s_beta->Assign.lhs, alpha->live)) )
					{

						flagSymbValue[temp] = 3;

						pv_beta_out->value[temp] = copylist(s_beta->Assign.rhs);
						pv_alpha_out->value[temp] = copylist(s_alpha->Assign.rhs);

						//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
						modifyFlags(s_beta->Assign.rhs, flagSymbValue);

						//modify flagSymbValue[] for those variables on which variable with index [s_alpha->Assign.lhs] depends
						modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
					}
					else
					{
						#ifdef DEbug
						printf("%s is Not Live\n", stab.sym_tab[temp]);
						#endif
					}
				}
				break;
			}//end if (variables match)

			s_alpha = s_alpha->next;
		}//end while s_alpha

		if(flagAbsent == 1)
		{
			//The s_beta->Assign.lhs (variable) is assigned in beta but not in alpha
			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_beta->Assign.rhs); printf("occurs in beta only\n");
			#endif

			if( (beta->finalPath  || isVariableLive(s_beta->Assign.lhs, beta->live)) ||
				(alpha->finalPath || isVariableLive(s_beta->Assign.lhs, alpha->live)) )
			{

				flagSymbValue[temp] = 3;

				pv_beta_out->value[temp] = copylist(s_beta->Assign.rhs);

				//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
				modifyFlags(s_beta->Assign.rhs, flagSymbValue);
			}
			else
			{
				#ifdef DEbug
				printf("%s is Not Live\n", stab.sym_tab[temp]);
				#endif
			}
		}
	}//end if (flagAbsentArray == 1)

	s_beta = s_beta->next;
  }//end while s_beta


  s_alpha = alphaModified->transformations;
  while(s_alpha != (r_alpha*)NULL)
  {
	//the following check prevents uselesss calculation for - (minus)
	if(s_alpha->Assign.lhs == 0)
	{
		s_alpha = s_alpha->next;
		continue;
	}

	s_beta = betaModified->transformations;
	temp = findSymbolTableIndex(s_alpha->Assign.lhs);

    //array start

    if(s_alpha->Assign.rhs != (NC*)NULL && s_alpha->Assign.rhs->type == 'w')
	  flagAbsentArray = 1;
	else
	  flagAbsentArray = 2;

	//The lhs may be an array variable
	//So, there may be multiple entries corresponding to it
	//Therefore, the whole s_alpha has to be checked for a match
	if(flagAbsentArray == 1)
	{
		while(s_beta != (r_alpha*)NULL)
		{
			if(s_alpha->Assign.lhs == s_beta->Assign.lhs && compare_trees(s_alpha->Assign.rhs, s_beta->Assign.rhs) == 1)
			{
				flagAbsentArray = 0;
				break;
			}
			s_beta = s_beta->next;
		}

		if(flagAbsentArray != 0)
		{
			//Match not found

			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_alpha->Assign.rhs); printf("occurs in alpha only\n");
			#endif

			if( (alpha->finalPath || isVariableLive(s_alpha->Assign.lhs, alpha->live)) ||
				(beta->finalPath  || isVariableLive(s_alpha->Assign.lhs, beta->live)) )
			{

				pv_alpha_out->subVector[temp].arrayValue[pv_alpha_out->subVector[temp].countValues] = copylist(s_alpha->Assign.rhs);
				(pv_alpha_out->subVector[temp].countValues)++;

				flagSymbValue[temp] = 3;

				//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
				modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
			}
			else
			{
				#ifdef DEbug
				printf("%s is Not Live\n", stab.sym_tab[temp]);
				#endif
			}
		}
	}//end if (flagAbsentArray == 1)

	else if(flagAbsentArray == 2)
	{
	//array end

		flagAbsent = 1;

		while(s_beta != (r_alpha*)NULL)
		{
			//to check whether the transformations are for the same variable or not
			if( s_alpha->Assign.lhs == s_beta->Assign.lhs)
			{
				flagAbsent = 0;

				if(compare_trees(s_alpha->Assign.rhs, s_beta->Assign.rhs) == 1)
				{
					if(flagSymbValue[temp] == 0)
						flagSymbValue[temp] = 1;
						//flagSymbValue[temp] = 2; //KB - to force PA
				}
				else
				{
					#ifdef DEbug
					printf("\nMismatch for variable: %s\n First (alpha) Expression\n", stab.sym_tab[temp]);
					write_lists(s_alpha->Assign.rhs);
					printf("\nSecond (beta) Expression\n");
					write_lists(s_beta->Assign.rhs);
					#endif

					if( (alpha->finalPath || isVariableLive(s_alpha->Assign.lhs, alpha->live)) ||
						(beta->finalPath  || isVariableLive(s_alpha->Assign.lhs, beta->live)) )
					{

						flagSymbValue[temp] = 3;

						pv_beta_out->value[temp] = copylist(s_beta->Assign.rhs);
						pv_alpha_out->value[temp] = copylist(s_alpha->Assign.rhs);

						//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
						modifyFlags(s_beta->Assign.rhs, flagSymbValue);

						//modify flagSymbValue[] for those variables on which variable with index [s_alpha->Assign.lhs] depends
						modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
					}
					else
					{
						#ifdef DEbug
						printf("%s is Not Live\n", stab.sym_tab[temp]);
						#endif
					}
				}
				break;
			}//end if (variables match)

			s_beta = s_beta->next;
		}//end while s_beta

		if(flagAbsent == 1)
		{
			//The s_beta->Assign.lhs (variable) is assigned in beta but not in alpha
			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_alpha->Assign.rhs); printf("occurs in alpha only\n");
			#endif

			if( (alpha->finalPath || isVariableLive(s_alpha->Assign.lhs, alpha->live)) ||
				(beta->finalPath  || isVariableLive(s_alpha->Assign.lhs, beta->live)) )
			{
				flagSymbValue[temp] = 3;

				pv_alpha_out->value[temp] = copylist(s_alpha->Assign.rhs);

				//modify flagSymbValue[] for those variables on which variable with index [s_alpha->Assign.lhs] depends
				modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
			}
			else
			{
				#ifdef DEbug
				printf("%s is Not Live\n", stab.sym_tab[temp]);
				#endif
			}
		}
	}//end if (flagAbsentArray == 2)

  s_alpha = s_alpha->next;
  }//end while s_alpha


  //If some variables are dead, then their flagSymbValue will continue to
  //remain 0 and their values will remain (NC*)NULL, and therefore
  //will not affect the following propagated vector computation

  //converting the value of a variable in PROPAGATED_VECTOR back to its symbolic value
  //also taking care of flagSymbValue[variable] = 2
  for(i = 0; i < NO_OF_VARIABLES; i++)
  {
    if(flagSymbValue[i] == 1)
    {
      ; //nothing to do, (NC*)NULL is already stored
    }
    else if(flagSymbValue[i] == 2 && stab.val_tab[i] != 0)
    //the second conjunct prevents uselesss calculation for - (minus)
    {

      s_beta = betaModified->transformations;
      while(s_beta != (r_alpha*)NULL)
      {
	    temp = findSymbolTableIndex(s_beta->Assign.lhs);
	    if(temp == i)
	    {
		  //array start
		  if(s_beta->Assign.rhs != (NC*)NULL && s_beta->Assign.rhs->type == 'w')
		  {
			flagAbsentArray = 1;
			for(j = 0; j < pv_beta_out->subVector[i].countValues; j++)
			{
				if(compare_trees(pv_beta_out->subVector[i].arrayValue[j], s_beta->Assign.rhs) == 1)
				{
					flagAbsentArray = 0;
					break;
				}
			}
			if(flagAbsentArray == 1)
			{
				pv_beta_out->subVector[i].arrayValue[pv_beta_out->subVector[i].countValues] = copylist(s_beta->Assign.rhs);
				(pv_beta_out->subVector[i].countValues)++;
			}
		  }
		  else
		  {
		  //array end
			pv_beta_out->value[i] = copylist(s_beta->Assign.rhs);
			break;
			//This "break" is absent in case of arrays because unlike scalars,
			//an array can have multiple entries in the data transformation
		  }
	    }
	    s_beta = s_beta->next;
      }//end while s_beta

      s_alpha = alphaModified->transformations;
      while(s_alpha != (r_alpha*)NULL)
      {
	    temp = findSymbolTableIndex(s_alpha->Assign.lhs);
	    if(temp == i)
	    {
		  //array start
		  if(s_alpha->Assign.rhs != (NC*)NULL && s_alpha->Assign.rhs->type == 'w')
		  {
			flagAbsentArray = 1;
			for(j = 0; j < pv_alpha_out->subVector[i].countValues; j++)
			{
				if(compare_trees(pv_alpha_out->subVector[i].arrayValue[j], s_alpha->Assign.rhs) == 1)
				{
					flagAbsentArray = 0;
					break;
				}
			}
			if(flagAbsentArray == 1)
			{
				pv_alpha_out->subVector[i].arrayValue[pv_alpha_out->subVector[i].countValues] = copylist(s_alpha->Assign.rhs);
				(pv_alpha_out->subVector[i].countValues)++;
			}
		  }
		  else
		  {
		  //array end
			pv_alpha_out->value[i] = copylist(s_alpha->Assign.rhs);
			break;
			//This "break" is absent in case of arrays because unlike scalars,
			//an array can have multiple entries in the data transformation
		  }
	    }
	    s_alpha = s_alpha->next;
      }//end while s_alpha

    }//end else if
  }//end for


  #elif defined PROPAGATE_ALL


  flagDifferent = FALSE;
  s_beta = betaModified->transformations;

  while(s_beta != (r_alpha*)NULL && !(flagDifferent) )
  {
    s_alpha = alphaModified->transformations;
    flagAbsent = 1;
    temp = findSymbolTableIndex(s_beta->Assign.lhs);

    while(s_alpha != (r_alpha*)NULL)
    {
      //to check whether the transformations are for the same variable or not
      if( s_beta->Assign.lhs == s_alpha->Assign.lhs && s_beta->Assign.lhs != 0 )
      //the second conjunct prevents uselesss calculation for - (minus)
      {
	     flagAbsent = 0;

	     if(compare_trees(s_beta->Assign.rhs, s_alpha->Assign.rhs) == 0)
	     {
			//array start
			//On accounting different values for a variable, we can immediately declare
			//a Mismatch only if the variable is a scalar - hence, the following check
			if(s_beta->Assign.rhs != (NC*)NULL && s_beta->Assign.rhs->type != 'w')
			{
			//array end
				#ifdef DEbug
				printf("\nMismatch in values for variable: %s\n", stab.sym_tab[temp]);
				#endif

				flagDifferent = TRUE;
				break;
			}
         }
      }

	  s_alpha = s_alpha->next;
    }//end while s_alpha

    if( flagDifferent )
		break;

    if( flagAbsent == 1 && s_beta->Assign.lhs != 0 )
    {
		#ifdef DEbug
		printf("\nMismatch in values for variable: %s\n", stab.sym_tab[temp]);
		#endif

		flagDifferent = TRUE;
		break;
	}

	 s_beta = s_beta->next;
  }//end while s_beta


  s_alpha = alphaModified->transformations;

  while(s_alpha != (r_alpha*)NULL && !(flagDifferent) )
  {
    s_beta = betaModified->transformations;
    flagAbsent = 1;
    temp = findSymbolTableIndex(s_alpha->Assign.lhs);

    while(s_beta != (r_alpha*)NULL)
    {
      //to check whether the transformations are for the same variable or not
      if( s_alpha->Assign.lhs == s_beta->Assign.lhs && s_alpha->Assign.lhs != 0 )
      //the second conjunct prevents uselesss calculation for - (minus)
      {
	     flagAbsent = 0;

	     if(compare_trees(s_alpha->Assign.rhs, s_beta->Assign.rhs) == 0)
	     {
			//array start
			//On accounting different values for a variable, we can immediately declare
			//a Mismatch only if the variable is a scalar - hence, the following check
			if(s_alpha->Assign.rhs != (NC*)NULL && s_alpha->Assign.rhs->type != 'w')
			{
			//array end
				#ifdef DEbug
				printf("\nMismatch in values for variable: %s\n", stab.sym_tab[temp]);
				#endif

				flagDifferent = TRUE;
				break;
			}
         }
      }

	  s_beta = s_beta->next;
    }//end while s_beta

    if( flagDifferent )
		break;

    if( flagAbsent == 1 && s_alpha->Assign.lhs != 0 )
    {
		#ifdef DEbug
		printf("\nMismatch in values for variable: %s\n", stab.sym_tab[temp]);
		#endif

		flagDifferent = TRUE;
		break;
	}

	 s_alpha = s_alpha->next;
  }//end while s_alpha

  if( flagDifferent )
  {
	  //computing propagated vectors
	  for( i=0; i < NO_OF_VARIABLES; i++ )
      {

         s_beta = betaModified->transformations;
         while(s_beta != (r_alpha*)NULL)
         {
	        temp = findSymbolTableIndex(s_beta->Assign.lhs);
	        if(temp == i)
	        {
				//array start
				if(s_beta->Assign.rhs != (NC*)NULL && s_beta->Assign.rhs->type == 'w')
				{
					flagAbsentArray = 1;
					for(j = 0; j < pv_beta_out->subVector[i].countValues; j++)
					{
						if(compare_trees(pv_beta_out->subVector[i].arrayValue[j], s_beta->Assign.rhs) == 1)
						{
							flagAbsentArray = 0;
							break;
						}
					}
					if(flagAbsentArray == 1)
					{
						pv_beta_out->subVector[i].arrayValue[pv_beta_out->subVector[i].countValues] = copylist(s_beta->Assign.rhs);
						(pv_beta_out->subVector[i].countValues)++;
					}
				}
				else
				{
				//array end
					pv_beta_out->value[i] = copylist(s_beta->Assign.rhs);
					break;
				}
			}
			s_beta = s_beta->next;
         }//end while s_beta

         s_alpha = alphaModified->transformations;
         while(s_alpha != (r_alpha*)NULL)
         {
	        temp = findSymbolTableIndex(s_alpha->Assign.lhs);
	        if(temp == i)
	        {
				//array start
				if(s_alpha->Assign.rhs != (NC*)NULL && s_alpha->Assign.rhs->type == 'w')
				{
					flagAbsentArray = 1;
					for(j = 0; j < pv_alpha_out->subVector[i].countValues; j++)
					{
						if(compare_trees(pv_alpha_out->subVector[i].arrayValue[j], s_alpha->Assign.rhs) == 1)
						{
							flagAbsentArray = 0;
							break;
						}
					}
					if(flagAbsentArray == 1)
					{
						pv_alpha_out->subVector[i].arrayValue[pv_alpha_out->subVector[i].countValues] = copylist(s_alpha->Assign.rhs);
						(pv_alpha_out->subVector[i].countValues)++;
					}
				}
				else
				{
				//array end
					pv_alpha_out->value[i] = copylist(s_alpha->Assign.rhs);
					break;
				}
			}
	        s_alpha = s_alpha->next;
         }//end while s_alpha
      }//end for
  }//end if flagDifferent


  #else

  //old value propagation (without considering Live Variables)

  //the array flagSymbValue[] is used for converting the value of a variable
  //in PROPAGATED_VECTOR back to its symbolic value
  //0 stored initially,
  //3 on finding mismatch,
  //2 if some other (mismatched) variable depends on it,
  //1 if the values match and no other (mismatched) variable depends on it
  for( i=0; i < NO_OF_VARIABLES; i++ )
    flagSymbValue[i] = 0;


  s_beta = betaModified->transformations;
  while(s_beta != (r_alpha*)NULL)
  {
	//the following check prevents uselesss calculation for - (minus)
	if(s_beta->Assign.lhs == 0)
	{
		s_beta = s_beta->next;
		continue;
	}

    s_alpha = alphaModified->transformations;
    temp = findSymbolTableIndex(s_beta->Assign.lhs);

    //array start

    if(s_beta->Assign.rhs != (NC*)NULL && s_beta->Assign.rhs->type == 'w')
	  flagAbsentArray = 1;
	else
	  flagAbsentArray = 2;

	//The lhs may be an array variable
	//So, there may be multiple entries corresponding to it
	//Therefore, the whole s_alpha has to be checked for a match
	if(flagAbsentArray == 1)
	{
		while(s_alpha != (r_alpha*)NULL)
		{
			if(s_beta->Assign.lhs == s_alpha->Assign.lhs && compare_trees(s_beta->Assign.rhs, s_alpha->Assign.rhs) == 1)
			{
				flagAbsentArray = 0;
				break;
			}
			s_alpha = s_alpha->next;
		}

		if(flagAbsentArray != 0)
		{
			//Match not found

			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_beta->Assign.rhs); printf("occurs in beta only\n");
			#endif

			pv_beta_out->subVector[temp].arrayValue[pv_beta_out->subVector[temp].countValues] = copylist(s_beta->Assign.rhs);
			(pv_beta_out->subVector[temp].countValues)++;

			flagSymbValue[temp] = 3;

			//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
			modifyFlags(s_beta->Assign.rhs, flagSymbValue);
		}
	} //end if (flagAbsentArray == 1)

	else if(flagAbsentArray == 2)
	{
	//array end


		flagAbsent = 1;

		while(s_alpha != (r_alpha*)NULL)
		{
			//to check whether the transformations are for the same variable or not
			if( s_beta->Assign.lhs == s_alpha->Assign.lhs )
			{
				flagAbsent = 0;

				if(compare_trees(s_beta->Assign.rhs, s_alpha->Assign.rhs) == 1)
				{
					if(flagSymbValue[temp] == 0)
						flagSymbValue[temp] = 1;
						//flagSymbValue[temp] = 2; //KB - to force PA
				}
				else
				{
					#ifdef DEbug
					printf("\nMismatch for variable: %s\n First (beta) Expression\n", stab.sym_tab[temp]);
					write_lists(s_beta->Assign.rhs);
					printf("\n Second (alpha) Expression\n");
					write_lists(s_alpha->Assign.rhs);
					#endif

					flagSymbValue[temp] = 3;

					pv_beta_out->value[temp] = copylist(s_beta->Assign.rhs);
					pv_alpha_out->value[temp] = copylist(s_alpha->Assign.rhs);

					//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
					modifyFlags(s_beta->Assign.rhs, flagSymbValue);

					//modify flagSymbValue[] for those variables on which variable with index [s_alpha->Assign.lhs] depends
					modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
				}
				break;
			}//end if (variables match)

			s_alpha = s_alpha->next;
		}//end while s_alpha

		if(flagAbsent == 1)
		{
			//The s_beta->Assign.lhs (variable) is assigned in beta but not in alpha
			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_beta->Assign.rhs); printf("occurs in beta only\n");
			#endif

			flagSymbValue[temp] = 3;

			pv_beta_out->value[temp] = copylist(s_beta->Assign.rhs);

			//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
			modifyFlags(s_beta->Assign.rhs, flagSymbValue);
		}
	}//end if (flagAbsentArray == 1)

	s_beta = s_beta->next;
  }//end while s_beta


  s_alpha = alphaModified->transformations;
  while(s_alpha != (r_alpha*)NULL)
  {
	//the following check prevents uselesss calculation for - (minus)
	if(s_alpha->Assign.lhs == 0)
	{
		s_alpha = s_alpha->next;
		continue;
	}

	s_beta = betaModified->transformations;
	temp = findSymbolTableIndex(s_alpha->Assign.lhs);

    //array start

    if(s_alpha->Assign.rhs != (NC*)NULL && s_alpha->Assign.rhs->type == 'w')
	  flagAbsentArray = 1;
	else
	  flagAbsentArray = 2;

	//The lhs may be an array variable
	//So, there may be multiple entries corresponding to it
	//Therefore, the whole s_alpha has to be checked for a match
	if(flagAbsentArray == 1)
	{
		while(s_beta != (r_alpha*)NULL)
		{
			if(s_alpha->Assign.lhs == s_beta->Assign.lhs && compare_trees(s_alpha->Assign.rhs, s_beta->Assign.rhs) == 1)
			{
				flagAbsentArray = 0;
				break;
			}
			s_beta = s_beta->next;
		}

		if(flagAbsentArray != 0)
		{
			//Match not found

			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_alpha->Assign.rhs); printf("occurs in alpha only\n");
			#endif

			pv_alpha_out->subVector[temp].arrayValue[pv_alpha_out->subVector[temp].countValues] = copylist(s_alpha->Assign.rhs);
			(pv_alpha_out->subVector[temp].countValues)++;

			flagSymbValue[temp] = 3;

			//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
			modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
		}
	}//end if (flagAbsentArray == 1)

	else if(flagAbsentArray == 2)
	{
	//array end

		flagAbsent = 1;

		while(s_beta != (r_alpha*)NULL)
		{
			//to check whether the transformations are for the same variable or not
			if( s_alpha->Assign.lhs == s_beta->Assign.lhs)
			{
				flagAbsent = 0;

				if(compare_trees(s_alpha->Assign.rhs, s_beta->Assign.rhs) == 1)
				{
					if(flagSymbValue[temp] == 0)
						flagSymbValue[temp] = 1;
						//flagSymbValue[temp] = 2; //KB - to force PA
				}
				else
				{
					#ifdef DEbug
					printf("\nMismatch for variable: %s\n First (alpha) Expression\n", stab.sym_tab[temp]);
					write_lists(s_alpha->Assign.rhs);
					printf("\nSecond (beta) Expression\n");
					write_lists(s_beta->Assign.rhs);
					#endif

					flagSymbValue[temp] = 3;

					pv_beta_out->value[temp] = copylist(s_beta->Assign.rhs);
					pv_alpha_out->value[temp] = copylist(s_alpha->Assign.rhs);

					//modify flagSymbValue[] for those variables on which variable with index [s_beta->Assign.lhs] depends
					modifyFlags(s_beta->Assign.rhs, flagSymbValue);

					//modify flagSymbValue[] for those variables on which variable with index [s_alpha->Assign.lhs] depends
					modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
				}
				break;
			}//end if (variables match)

			s_beta = s_beta->next;
		}//end while s_beta

		if(flagAbsent == 1)
		{
			//The s_beta->Assign.lhs (variable) is assigned in beta but not in alpha
			#ifdef DEbug
			printf("\n%s := ", stab.sym_tab[temp]); write_lists(s_alpha->Assign.rhs); printf("occurs in alpha only\n");
			#endif

			flagSymbValue[temp] = 3;

			pv_alpha_out->value[temp] = copylist(s_alpha->Assign.rhs);

			//modify flagSymbValue[] for those variables on which variable with index [s_alpha->Assign.lhs] depends
			modifyFlags(s_alpha->Assign.rhs, flagSymbValue);
		}
	}//end if (flagAbsentArray == 2)

  s_alpha = s_alpha->next;
  }//end while s_alpha


  //converting the value of a variable in PROPAGATED_VECTOR back to its symbolic value
  //also taking care of flagSymbValue[variable] = 2
  for(i = 0; i < NO_OF_VARIABLES; i++)
  {
    if(flagSymbValue[i] == 1)
    {
      ; //nothing to do, (NC*)NULL is already stored
    }
    else if(flagSymbValue[i] == 2 && stab.val_tab[i] != 0)
    //the second conjunct prevents uselesss calculation for - (minus)
    {

      s_beta = betaModified->transformations;
      while(s_beta != (r_alpha*)NULL)
      {
	    temp = findSymbolTableIndex(s_beta->Assign.lhs);
	    if(temp == i)
	    {
		  //array start
		  if(s_beta->Assign.rhs != (NC*)NULL && s_beta->Assign.rhs->type == 'w')
		  {
			flagAbsentArray = 1;
			for(j = 0; j < pv_beta_out->subVector[i].countValues; j++)
			{
				if(compare_trees(pv_beta_out->subVector[i].arrayValue[j], s_beta->Assign.rhs) == 1)
				{
					flagAbsentArray = 0;
					break;
				}
			}
			if(flagAbsentArray == 1)
			{
				pv_beta_out->subVector[i].arrayValue[pv_beta_out->subVector[i].countValues] = copylist(s_beta->Assign.rhs);
				(pv_beta_out->subVector[i].countValues)++;
			}
		  }
		  else
		  {
		  //array end
			pv_beta_out->value[i] = copylist(s_beta->Assign.rhs);
			break;
			//This "break" is absent in case of arrays because unlike scalars,
			//an array can have multiple entries in the data transformation
		  }
	    }
	    s_beta = s_beta->next;
      }//end while s_beta

      s_alpha = alphaModified->transformations;
      while(s_alpha != (r_alpha*)NULL)
      {
	    temp = findSymbolTableIndex(s_alpha->Assign.lhs);
	    if(temp == i)
	    {
		  //array start
		  if(s_alpha->Assign.rhs != (NC*)NULL && s_alpha->Assign.rhs->type == 'w')
		  {
			flagAbsentArray = 1;
			for(j = 0; j < pv_alpha_out->subVector[i].countValues; j++)
			{
				if(compare_trees(pv_alpha_out->subVector[i].arrayValue[j], s_alpha->Assign.rhs) == 1)
				{
					flagAbsentArray = 0;
					break;
				}
			}
			if(flagAbsentArray == 1)
			{
				pv_alpha_out->subVector[i].arrayValue[pv_alpha_out->subVector[i].countValues] = copylist(s_alpha->Assign.rhs);
				(pv_alpha_out->subVector[i].countValues)++;
			}
		  }
		  else
		  {
		  //array end
			pv_alpha_out->value[i] = copylist(s_alpha->Assign.rhs);
			break;
			//This "break" is absent in case of arrays because unlike scalars,
			//an array can have multiple entries in the data transformation
		  }
	    }
	    s_alpha = s_alpha->next;
      }//end while s_alpha

    }//end else if
  }//end for

  #endif


  #ifdef DEbug
  printf("\n Vectors to be propagated\n");
  printPropagatedVector(pv_beta_out);
  printPropagatedVector(pv_alpha_out);
  #endif

  countMismatch(pv_beta_out, pv_alpha_out);
}


//Loop detection using back edge has been replaced with VAPFLAG
//Loop detection is now a responsibility of correspondenceChecker
//Checks whether the propagated vector stored in end state of gamma is same as
//pv_gamma or not
//If found equal then the (mismatched) expressions are declared to be loop invariant
boolean loopInvariant( PATH_ST *gamma, PROPAGATED_VECTOR *pv_gamma, FSMD *M )
{
  int i;

  #ifdef DEbug
  int j, flag = 0;
  #endif

  NUM_LOOPINV++;

  if(gamma==(PATH_ST*)NULL)
  {
    printf("\n In loopInvariant() with NULL path\nERROR: Exiting System\n");
    exit(0);
  }
  if(pv_gamma==(PROPAGATED_VECTOR*)NULL)
  {
    printf("\n In loopInvariant() with NULL vector\nERROR: Exiting System\n");
    exit(0);
  }
  if(M==(FSMD*)NULL)
  {
    printf("\n In loopInvariant() with NULL FSMD\nERROR: Exiting System\n");
    exit(0);
  }

  #ifdef DEbug
  printf("\n In loopInvariant()\n Arguments\n Path\n");
  displayApath(gamma);
  printPropagatedVector(pv_gamma);
  printf("\n End of arguments\n");
  printPropagatedVector( &(M->states[gamma->end].propVector) );
  #endif

  for( i=0; i < NO_OF_VARIABLES; i++ )
  {
    //check for mismatch in value for some variable
    if( stab.val_tab[i] != 0 && compare_trees(M->states[gamma->end].propVector.value[i], pv_gamma->value[i]) == 0 )
    //the first conjunct prevents uselesss calculation for - (minus)
    {
      #ifdef DEbug
      flag = 0;
      //check to find out whether the variable is a temporary variable or not
      for( j=0; j < V0_V1.no_of_elements; j++ )
      {
        if( strcmp(stab.sym_tab[i], V0_V1.var_string[j]) == 0 )
        {
          flag = 1;
          break;
        }
      }//end for j

      if(flag == 1)
      {
        if(M->states[gamma->end].numtrans != 0)
          printf("\n Out of loopInvariant() with return value FALSE for mismatch in case of common variable: %s\n", stab.sym_tab[i]);
        else
          printf("\n Out of loopInvariant() with return value FALSE for mismatch in case of common variable: %s, in the Final State\n", stab.sym_tab[i]);

        //return FALSE;	<-- No longer required just for common variables
      }
	  else
	    printf("\n Out of loopInvariant() with return value FALSE for mismatch in case of uncommon variable: %s\n", stab.sym_tab[i]);
      #endif

      //There is a need for both the common and the uncommon variables to match (and not just common variables)
      return FALSE;
    }//end if compare_trees = 0
  }//end for

  #ifdef DEbug
  printf("\n Out of loopInvariant() with return value TRUE because no mismatch for a common variable is found\n");
  #endif

  return TRUE;
}



//Finds a path alpha_out emanating from q_1j in M1, and
//conditionally equivalent equivalent to beta of M0
//PENDING: Updation of P0, P1 - Because there is no need
//indexOfP0_out is used for betaPrime, and
//indexOfP1_out is used for alpha
void findEquivalentPath( PATH_ST *beta, PROPAGATED_VECTOR *pv_beta_in, NODE_ST *q_1j, PROPAGATED_VECTOR *pv_alpha_in, PATHS_LIST *P0, PATHS_LIST *P1,
			 FSMD *M0, FSMD *M1, PROPAGATED_VECTOR *pv_beta_out, PROPAGATED_VECTOR *pv_alpha_out, int *indexOfP0_out, int *indexOfP1_out )
{
	int i, null_path_id;
	int resChkCond;
	PATH_ST *betaModified, *alphaModified;

	NUM_FINDEQV++;

	if(beta==(PATH_ST*)NULL)
	{
		printf("\n In findEquivalentPath() with a NULL path\nERROR: Exiting System\n");
		exit(0);
	}
	if(q_1j==(NODE_ST*)NULL)
	{
		printf("\n In findEquivalentPath() with a NULL state\nERROR: Exiting System\n");
		exit(0);
	}
	if(pv_beta_in==(PROPAGATED_VECTOR*)NULL || pv_alpha_in==(PROPAGATED_VECTOR*)NULL)
	{
		printf("\n In findEquivalentPath() with a NULL input vector\nERROR: Exiting System\n");
		exit(0);
	}
	if(P0==(PATHS_LIST*)NULL || P1==(PATHS_LIST*)NULL)
	{
		printf("\n In findEquivalentPath() with a NULL path list\nERROR: Exiting System\n");
		exit(0);
	}
	if(M0==(FSMD*)NULL || M1==(FSMD*)NULL)
	{
		printf("\n In findEquivalentPath() with a NULL FSMD\nERROR: Exiting System\n");
		exit(0);
	}
	if(pv_beta_out==(PROPAGATED_VECTOR*)NULL || pv_alpha_out==(PROPAGATED_VECTOR*)NULL)
	{
		printf("\n In findEquivalentPath() with a NULL output vector\nERROR: Exiting System\n");
		exit(0);
	}

	#ifdef DEbug
	printf("\n In findEquivalentPath(.., .., %s, ..)\n", q_1j->state_id);
	printf("\nBeta:"); displayApath(beta);
	printf("\nPV_Beta:"); printPropagatedVector(pv_beta_in);
	printf("\nPV_Alpha:"); printPropagatedVector(pv_alpha_in);
	#endif

	//Comnpute the modified condition of execution and the modified data transformation of beta
	betaModified = createDummyPath(beta, pv_beta_in);

	for( i=0; i < P1->numpaths; i++ )
	{
		//to determine startPtNd(alpha) = q_1j
		if( strcmp(M1->states[P1->paths[i].start].state_id, q_1j->state_id) == 0 &&
			compatibleConditions(P1->paths[i].condition,pv_alpha_in->cond) &&
			compatibleConditions(P1->paths[i].condition,betaModified->condition) &&
			compatibleConditions(pv_alpha_in->cond,betaModified->condition) )
		{
			#ifdef DEbug
			printf("\n Comparing beta with path alpha with id: %d\n", i);
			#endif

			//Comnpute the modified condition of execution and the modified data transformation of alpha
			alphaModified = createDummyPath(&(P1->paths[i]), pv_alpha_in);

			if(compatibleConditions(betaModified->condition, alphaModified->condition) != FALSE)
				resChkCond =  checkCondition(betaModified, alphaModified);

			#ifdef DEbug
			printf("\n Result of checkCondition(): %d\n", resChkCond);
			#endif

			//Case 1
			if(resChkCond == 1) //R_beta = R_alpha
			{
				//Case 1.1: s_beta = s_alpha
				//Case 1.2: s_beta != s_alpha
				//have been merged because valuePropagation() SHOULD take care of it

				//Check for conflicts in Output Variables
				if(conflictInOutputs(betaModified->transformations, alphaModified->transformations))
				{
					printf("\nMismatch in output variables detected\n");
					printf("\nBeta:"); displayApath(beta);
					printf("\nPV_Beta:"); printPropagatedVector(pv_beta_in);
					printf("\nPV_Alpha:"); printPropagatedVector(pv_alpha_in);
					printf("\nAlpha:"); displayApath(&(P1->paths[i]));
				}

				valuePropagation(beta, pv_beta_in, &(P1->paths[i]), pv_alpha_in, pv_beta_out, pv_alpha_out);
				*indexOfP1_out = i;
				return;
			}
			//Case 2
			else if(resChkCond == 2) //R_beta => R_alpha
			{
				//Check for conflicts in Output Variables
				if(conflictInOutputs(betaModified->transformations, alphaModified->transformations))
				{
					printf("\nMismatch in output variables detected\n");
					printf("\nBeta:"); displayApath(beta);
					printf("\nPV_Beta:"); printPropagatedVector(pv_beta_in);
					printf("\nPV_Alpha:"); printPropagatedVector(pv_alpha_in);
					printf("\nAlpha:"); displayApath(&(P1->paths[i]));
				}

				createNullPath(beta->start, P0);

				null_path_id = P0->numpaths + P0->offset_null_path - 1; //-1 because offset_null_path has already been increased
				valuePropagation(&(P0->paths[null_path_id]), pv_beta_in, &(P1->paths[i]), pv_alpha_in, pv_beta_out, pv_alpha_out);
				*indexOfP0_out = INDEX_OF_NULL_PATH;
				*indexOfP1_out = i;
				return;
			}
			if(compatibleConditions(alphaModified->condition, betaModified->condition) != FALSE)
				resChkCond =  checkCondition(alphaModified, betaModified);

			#ifdef DEbug
			printf("\n Result2 of checkCondition(): %d\n", resChkCond);
			#endif

			//Case 3
			if(resChkCond == 2) //R_alpha => R_beta
			{
				//Check for conflicts in Output Variables
				if(conflictInOutputs(betaModified->transformations, alphaModified->transformations))
				{
					printf("\nMismatch in output variables detected\n");
					printf("\nBeta:"); displayApath(beta);
					printf("\nPV_Beta:"); printPropagatedVector(pv_beta_in);
					printf("\nPV_Alpha:"); printPropagatedVector(pv_alpha_in);
					printf("\nAlpha:"); displayApath(&(P1->paths[i]));
				}

				createNullPath(P1->paths[i].start, P1);

				null_path_id = P1->numpaths + P1->offset_null_path - 1; //-1 because offset_null_path has already been increased
				valuePropagation(beta, pv_beta_in, &(P1->paths[null_path_id]), pv_alpha_in, pv_beta_out, pv_alpha_out);
				*indexOfP1_out = INDEX_OF_NULL_PATH;
				return;
			}

		}//end if alpha
	}//end for i

	//Case omega
	*indexOfP1_out = INDEX_OF_OMEGA_PATH;

	return;
}



//Used to find whether the states q_0i and q_1j are conditionally corresponding state pair or not
//eta, E_u, E_c, LIST are updated
boolean correspondenceChecker( NODE_ST *q_0i, NODE_ST *q_1j, PATHS_LIST *P0, PATHS_LIST *P1, FSMD *M0, FSMD *M1,
                                     CPS_Q *eta, CPS_Q *C_eta, PATH_PAIR *E_u, PATH_PAIR* E_c, PATH_PAIR* LIST )
{
  int i, *indexOfP0, *indexOfP1;
  int nullP0, nullP1;
  boolean selfRecursion, retValue = TRUE;
  PROPAGATED_VECTOR *orig_pv_beta, *orig_pv_alpha;
  PROPAGATED_VECTOR *pv_beta, *pv_alpha;
  PATH_PAIR *tempList;

  NUM_CORRES++;

  if(q_0i==(NODE_ST*)NULL || q_1j==(NODE_ST*)NULL)
  {
    printf("\n In correspondenceChecker() with a NULL state\nERROR: Exiting System\n");
    exit(0);
  }
  if(P0==(PATHS_LIST*)NULL || P1==(PATHS_LIST*)NULL)
  {
    printf("\n In correspondenceChecker() with a NULL path list\nERROR: Exiting System\n");
    exit(0);
  }
  if(M0==(FSMD*)NULL || M1==(FSMD*)NULL)
  {
    printf("\n In correspondenceChecker() with a NULL FSMD\nERROR: Exiting System\n");
    exit(0);
  }
  if(eta==(CPS_Q*)NULL)
  {
    printf("\n In correspondenceChecker() with a NULL eta\nERROR: Exiting System\n");
    exit(0);
  }
  if(E_u==(PATH_PAIR*)NULL || E_c==(PATH_PAIR*)NULL)
  {
    printf("\n In correspondenceChecker() with a NULL E\nERROR: Exiting System\n");
    exit(0);
  }
  if(LIST==(PATH_PAIR*)NULL)
  {
	printf("\n In correspondenceChecker() with a NULL E\nERROR: Exiting System\n");
    exit(0);
  }

  indexOfP0 = (int*)malloc(sizeof(int));
  indexOfP1 = (int*)malloc(sizeof(int));

  orig_pv_beta = (PROPAGATED_VECTOR*)malloc(sizeof(PROPAGATED_VECTOR));
  orig_pv_alpha = (PROPAGATED_VECTOR*)malloc(sizeof(PROPAGATED_VECTOR));

  pv_beta = (PROPAGATED_VECTOR*)malloc(sizeof(PROPAGATED_VECTOR));
  pv_alpha = (PROPAGATED_VECTOR*)malloc(sizeof(PROPAGATED_VECTOR));

  #ifdef DEbug
  printf("\n In correspondenceChecker( %s, %s )\n", q_0i->state_id, q_1j->state_id);
  #endif

  for( i=0; i < P0->numpaths; i++ )
  {
	//check to find which paths emanate from q_0i
    if( strcmp(M0->states[P0->paths[i].start].state_id, q_0i->state_id) == 0 )
    {

	//Feasibility check (for the condition) should be done here

	#ifdef DEbug
	printf("Checking condition for path with index %d\n", i);
	#endif

	////createDummyPath() is called to find (cond_0i AND R_beta{v_0i/v}
      //betaModified = createDummyPath(P0.paths[i], q_0i.propVector);
      //if(betaModified->condition != (NC*)NULL )

    //the above strategy is discarded since I don't know how to represent the condition "False"

    //though the function compatibleConditions() does not actually imitate the logical AND function
    //it serves our purpose because it additionally handles the scenario A AND B1
    //(i.e, renders the check (alpha = omega) useless)
    if( compatibleConditions(q_0i->propVector.cond, P0->paths[i].condition) && compatibleConditions(q_1j->propVector.cond, P0->paths[i].condition) )
    {
	  *indexOfP0 = i;

	  #ifdef DEbug
	  printf("\n In correspondenceChecker( %s, %s ) with path_id: %d\n", q_0i->state_id, q_1j->state_id, i);
	  #endif

	  //These vectors are passed as an argument to unionE()
	  copyPropagatedVector( orig_pv_beta, &(q_0i->propVector) );
	  copyPropagatedVector( orig_pv_alpha, &(q_1j->propVector) );

	  findEquivalentPath( &(P0->paths[i]), &(q_0i->propVector), q_1j, &(q_1j->propVector), P0, P1, M0, M1, pv_beta, pv_alpha, indexOfP0, indexOfP1);

	  if(*indexOfP0 == INDEX_OF_NULL_PATH && *indexOfP1 == INDEX_OF_NULL_PATH)
	  {
		  printf("\n findEquivalentPath returned 2 NULL paths\nERROR: Exiting system\n");
		  exit(0);
	  }

	  #ifdef DEbug
		printf("\n Back from findEquivalentPath() to correspondenceChecker()\n");
	    printf(" indexOfP0: %d, indexOfP1: %d\n", *indexOfP0, *indexOfP1);

	    printf(" Beta path:\n");
	    if(*indexOfP0 == INDEX_OF_NULL_PATH)
	      printf(" betaPrime is a null path\n");
	    else
	      printPath( &(P0->paths[*indexOfP0]), P0, *indexOfP0 );
	    printPropagatedVector(pv_beta);

	    printf("\n\n Alpha path:\n");
	    if(*indexOfP1 == INDEX_OF_OMEGA_PATH)
	      printf(" alpha is a non-existent path\n");
	    else if(*indexOfP1 == INDEX_OF_NULL_PATH)
	      printf(" alpha is a null path\n");
	    else
	      printPath( &(P1->paths[*indexOfP1]), P1, *indexOfP1 );
	    printPropagatedVector(pv_alpha);
	  #endif

	  if(*indexOfP1 == INDEX_OF_OMEGA_PATH) //(alpha = omega)
	  {
	    printf("\n Equivalent path of \n"); printPath( &(P0->paths[i]), P0, i ); printf("\n is not found \nDisplaying List\n\n");

		printPropagatedVector(pv_beta);
		printPropagatedVector(pv_alpha);

	    printE(LIST, P0, P1, M0, M1);
	    return FALSE;
	  }
	  else
	  {
		  if(equivalentPropagatedVector(pv_beta, pv_alpha) == FALSE)
		  {
			  //unequal propagated vectors obtained => C-equivalence

			  //the following check prevents self-recursion
			  if(*indexOfP0 != INDEX_OF_NULL_PATH && *indexOfP1 != INDEX_OF_NULL_PATH && already_present_LIST(LIST, *indexOfP0, *indexOfP1, P0, P1, M0, M1) == TRUE)
				selfRecursion = TRUE;
			  else
				selfRecursion = FALSE;

			  //creation of null paths in respective path cover, and
			  //update LIST (should be done AFTER selfRecursion has been defined)
			  if(*indexOfP0 == INDEX_OF_NULL_PATH)
			  {
				  //update P0
				  nullP0 = P0->numpaths + P0->offset_null_path - 1; //-1 because offset_null_path has already been increased
				  unionE( LIST, nullP0, *indexOfP1, orig_pv_beta, orig_pv_alpha, P0, P1 );
			  }
			  else if(*indexOfP1 == INDEX_OF_NULL_PATH)
			  {
				  //update P1
				  nullP1 = P1->numpaths + P1->offset_null_path - 1; //-1 because offset_null_path has already been increased
				  unionE( LIST, *indexOfP0, nullP1, orig_pv_beta, orig_pv_alpha, P0, P1 );
			  }
			  else
			  {
				  //update LIST only
				  unionE( LIST, *indexOfP0, *indexOfP1, orig_pv_beta, orig_pv_alpha, P0, P1 );
			  }


     		  //check for loop invariance, VAPFLAG = TRUE implies loop has been crossed over
			  if( (*indexOfP0 != INDEX_OF_NULL_PATH && M0->states[P0->paths[i].end].VAPFLAG == TRUE && loopInvariant(&(P0->paths[i]), pv_beta, M0) == FALSE) ||
	              (*indexOfP1 != INDEX_OF_NULL_PATH && M1->states[P1->paths[*indexOfP1].end].VAPFLAG == TRUE && loopInvariant(&(P1->paths[*indexOfP1]), pv_alpha, M1) == FALSE) )
	          {
	            printf("\n Propagated values are Not Loop Invariant\nDisplaying LIST\n\n");

				printPropagatedVector(pv_beta);
				printPropagatedVector(pv_alpha);

	            printE(LIST, P0, P1, M0, M1);
                return FALSE;
	          }
	          else
	          {
	            #ifdef DEbug
	            printf("\n No loop detected\n");
	            #endif
	          }

	          //check for whether reset state has been reached
	          //in our case, check for whether some final state has been reached
	          //i.e., state with no outgoing transitions
	          if( (*indexOfP0 != INDEX_OF_NULL_PATH) && (M0->states[P0->paths[*indexOfP0].end].numtrans == 0) )
	          {
			    printf("\n Reset (Final) state of M0 reached with C-equivalence; path cannot be extended farther\nDisplaying LIST\n\n");

				printPropagatedVector(pv_beta);
				printPropagatedVector(pv_alpha);

			    printE(LIST, P0, P1, M0, M1);
                return FALSE;
			  }
	          else if( (*indexOfP1 != INDEX_OF_NULL_PATH) && (M1->states[P1->paths[*indexOfP1].end].numtrans == 0) )
	          {
			    printf("\n Reset (Final) state of M1 reached with C-equivalence; path cannot be extended farther\nDisplaying LIST\n\n");

				printPropagatedVector(pv_beta);
				printPropagatedVector(pv_alpha);

			    printE(LIST, P0, P1, M0, M1);
       		    return FALSE;
			  }
			  //There is scope of extending the path
			  else
			  {

	            //update PV0
	            if(*indexOfP0 == INDEX_OF_NULL_PATH)
	              copyPropagatedVector( &(q_0i->propVector), pv_beta );
	            else
	              copyPropagatedVector( &(M0->states[P0->paths[i].end].propVector), pv_beta );

	            //update PV1
	            if(*indexOfP1 == INDEX_OF_NULL_PATH)
	              copyPropagatedVector( &(q_1j->propVector), pv_alpha );
	            else
	              copyPropagatedVector( &(M1->states[P1->paths[*indexOfP1].end].propVector), pv_alpha );

				//set the VAPFLAG's to TRUE
				if(*indexOfP0 != INDEX_OF_NULL_PATH)
				{
					M0->states[P0->paths[i].start].VAPFLAG = TRUE;
					M0->states[P0->paths[i].end].VAPFLAG = TRUE;
				}
				//else
				//	M0->states[P0->paths[i].start].VAPFLAG = TRUE;

				if(*indexOfP1 != INDEX_OF_NULL_PATH)
				{
					M1->states[P0->paths[*indexOfP1].start].VAPFLAG = TRUE;
					M1->states[P0->paths[*indexOfP1].end].VAPFLAG = TRUE;
				}
				//else
				//	M1->states[P0->paths[*indexOfP1].start].VAPFLAG = TRUE;
				//The "else" parts have been commented out to avoid mistakenly
				//detecting a loop when two consecutive null paths are inserted
				//in the same state; q0a --(c1 & c2 & c3)--> q0b,
				//q1a --(c1)--> q1b --(c2)--> q1c --(c3)--> q1d

	            //recursive call
	            CUR_RECURSION_DEPTH++;
	            if(CUR_RECURSION_DEPTH > MAX_RECURSION_DEPTH)
		          MAX_RECURSION_DEPTH = CUR_RECURSION_DEPTH;

	            #ifdef DEbug
	            printf("\n\nThrough Recursion: %d\n", CUR_RECURSION_DEPTH);
	            #endif

	            //bitwise AND makes TRUE & FALSE = FALSE (multiplication (*) would have done the trick as well)
	            if(*indexOfP0 == INDEX_OF_NULL_PATH && *indexOfP1 == INDEX_OF_NULL_PATH) //this case is Impossible
	            {
		          printf("ERROR: Both paths are null \nERROR: Exiting system instead of recursive call\n");
		          exit(0);
	            }
	            else if(*indexOfP0 == INDEX_OF_NULL_PATH)
	              retValue = retValue & correspondenceChecker( q_0i, &(M1->states[P1->paths[*indexOfP1].end]), P0, P1, M0, M1, eta, C_eta, E_u, E_c, LIST );
	            else if(*indexOfP1 == INDEX_OF_NULL_PATH)
	              retValue = retValue & correspondenceChecker( &(M0->states[P0->paths[i].end]), q_1j, P0, P1, M0, M1, eta, C_eta, E_u, E_c, LIST );
	            else
	            {
					if( selfRecursion == FALSE )
					{
						retValue = retValue & correspondenceChecker( &(M0->states[P0->paths[i].end]), &(M1->states[P1->paths[*indexOfP1].end]), P0, P1, M0, M1, eta, C_eta, E_u, E_c, LIST );
					}
					else
					{
						//If both beta and alpha end in cutpoints which are already present in LIST,
						//then also E_c needs to be updated, because we will not get an opportunity
						//to update E_c otherwise

					    //update E_c and C_eta
					    #ifdef DEbug
					    printf("\n Updating E_c when both beta and alpha end in a loop entry/exit cutpoint\n");
					    #endif

					    tempList = LIST;
						while( tempList != (PATH_PAIR*)NULL )
						{
							if(tempList->p0 != -1 || tempList->p1 != -1)
							{
								//Resetting the VAPFLAG back to FALSE
								M0->states[P0->paths[tempList->p0].start].VAPFLAG = FALSE;
								M0->states[P0->paths[tempList->p0].end].VAPFLAG = FALSE;
								M1->states[P1->paths[tempList->p1].start].VAPFLAG = FALSE;
								M1->states[P1->paths[tempList->p1].end].VAPFLAG = FALSE;

								unionE( E_c, tempList->p0, tempList->p1, tempList->pv0, tempList->pv1, P0, P1 );

								if( !already_present_queue(C_eta, P0->paths[tempList->p0].end, P1->paths[tempList->p1].end, C_corresponding_state_pair_visited) )
									enQP( C_eta, P0->paths[tempList->p0].end, P1->paths[tempList->p1].end );
							}
							tempList = tempList->next;
						}//end for E_c

					    //to compensate for the decrement done below
					    CUR_RECURSION_DEPTH++;
					}
	            }

	            removeLastPathPairFromLIST(LIST, P0, P1, M0, M1);
	            CUR_RECURSION_DEPTH--;
		     }
          }//end of unequal propagated vectors
          else
          {
			 //U-equivalent pair of paths found

	         //update E_u
	         #ifdef DEbug
	         printf("\n Updating E_u\n");
	         #endif

	         unionE( E_u, *indexOfP0, *indexOfP1, orig_pv_beta, orig_pv_alpha, P0, P1 );

	         //update Propagated Vectors
	         //resetPropagatedVector( &(M0->states[P0->paths[*indexOfP0].end].propVector) );
	         //resetPropagatedVector( &(M1->states[P1->paths[*indexOfP1].end].propVector) );

	         //update eta
	         if(*indexOfP0 == INDEX_OF_NULL_PATH || *indexOfP1 == INDEX_OF_NULL_PATH) //this case is Impossible
	         {
		       printf("ERROR: One of the paths is null while updating E_u\nERROR: Exiting system instead of updating eta\n");
		       exit(0);
	         }
	         //NB: The conditions of the states' propagated vectors are to be reset to NULL
	         //Otherwise, it may prevent exploration of other paths starting from q_0i and q_1j
	         M0->states[P0->paths[i].end].propVector.cond = (NC*)NULL;
		     M1->states[P1->paths[*indexOfP1].end].propVector.cond = (NC*)NULL;

		     #ifdef DEbug
		     printf("Enqueueing ( %s, %s ) in eta\n\n", M0->states[P0->paths[i].end].state_id, M1->states[P1->paths[*indexOfP1].end].state_id);
		     #endif

		     if( !already_present_queue(eta, P0->paths[i].end, P1->paths[*indexOfP1].end, corresponding_state_pair_visited) )
		         enQP( eta, P0->paths[i].end, P1->paths[*indexOfP1].end );

	         //update E_c and C_eta
	         #ifdef DEbug
	         printf("\n Updating E_c\n");
	         #endif

	         tempList = LIST;
	         while( tempList != (PATH_PAIR*)NULL )
	         {
				 if(tempList->p0 != -1 || tempList->p1 != -1)
				 {
					//Resetting the VAPFLAG back to FALSE
					M0->states[P0->paths[tempList->p0].start].VAPFLAG = FALSE;
					M0->states[P0->paths[tempList->p0].end].VAPFLAG = FALSE;
					M1->states[P1->paths[tempList->p1].start].VAPFLAG = FALSE;
					M1->states[P1->paths[tempList->p1].end].VAPFLAG = FALSE;

					unionE( E_c, tempList->p0, tempList->p1, tempList->pv0, tempList->pv1, P0, P1 );
					//do not reset the propagated vectors

					if( !already_present_queue(C_eta, P0->paths[tempList->p0].end, P1->paths[tempList->p1].end, C_corresponding_state_pair_visited) )
						enQP( C_eta, P0->paths[tempList->p0].end, P1->paths[tempList->p1].end );
				 }
			     tempList = tempList->next;
			 }//end for E_c
	      }//end equal proapagated vectors
	   }//end (alpha = omega)
   } }//end beta_m
  }//end for

  #ifdef DEbug
  printf("\n Out of correspondenceChecker( %s, %s ) with return value %d\n", q_0i->state_id, q_1j->state_id, retValue);
  #endif

  return retValue;
}



//Initializes the data structures P0, P1, eta, E
//Outputs whether M0 is contained in M1 or not
boolean containmentChecker( FSMD *M0, FSMD *M1 )
{

    //****************** Variable declaration ***********************

	PATHS_LIST P0, P1;
    // P0 is path cover of M0 and P1 is path cover of M1

    CPS_Q eta;
    // eta = set corresponding state pairs
    CPS_Q C_eta;
    // C_eta = set C_corresponding state pairs
    CPS tempEta;
    //required for dequeueing eta

    PATH_PAIR *E_u, *E_c;
    //stores unconditionally equivalent and conditionally equivalent path pairs respectively
    PATH_PAIR *LIST;
    //stores (potential) C-equivalent paths along a concatenated path
    //in case "failure" is encountered in correspondenceChecker, this data structure is
    //output to help the user of equivalenceChecker in debugging

    int result;//, err
    int i;

    struct rusage timeUsage;
    //for timing analysis


    if(M0==(FSMD*)NULL || M1==(FSMD*)NULL)
    {
		printf("\n In containmentChecker() with a NULL FSMD\nERROR: Exiting System\n");
		exit(0);
	}

	cal_V0_intersection_V1( &V0_V1 );

	#ifdef DEbug
	printf( "\n vars of : V0_V1  \n");
	for( i = 0; i < V0_V1.no_of_elements; i++ )
		printf( " %s\n", V0_V1.var_string[i] );
	#endif

	cal_V1_minus_V0_intersection_V1( &V1_minus_V0_V1 );

	#ifdef DEbug
	printf( "\n vars of : V1_minus_V0_V1 \n" );
	for( i = 0; i < V1_minus_V0_V1.no_of_elements; i++ )
		printf( " %s \n", V1_minus_V0_V1.var_string[i] );
	#endif

    //Data structure initialization
    initP(&eta);
	enQP(&eta,0,0);    // place the reset states of two fsmds as corresponding state pair in eta
    initializeStatePairVisited( corresponding_state_pair_visited );

    initP(&C_eta);
    initializeStatePairVisited( C_corresponding_state_pair_visited );

    E_u = initE();
    E_c = initE();

    findpaths(M0,&P0);
	Associate_R_and_r_alpha(&P0); // computes R_alpha & r_alpha

    #ifdef DETAILS
    printf("\n\nDisplaying all paths of M0\n\n");
	displayallpaths(&P0);
	#endif

	#ifdef LVA
    performLiveVariableAnalysis(&P0);
    #endif


    findpaths(M1,&P1);
    Associate_R_and_r_alpha(&P1);

    #ifdef DETAILS
    printf("\n\nDisplaying all paths of M1\n\n");
    displayallpaths(&P1);
    #endif

    #ifdef LVA
	performLiveVariableAnalysis(&P1);
	#endif

    #ifdef DEbug
    for(i=0; i < stab.numsymbols; i++)
		printf("Symbol %d: \t %s \t %d\n", i, stab.sym_tab[i], stab.val_tab[i]);
	#endif
    //NB: Initialization of PV0 and PV1 already done by read_fsmd (parser)

	MAX_RECURSION_DEPTH = 0;
    COUNT_CODE_MOTIONS = 0;

    NUM_CORRES = 0;
    NUM_FINDEQV = 0;
    NUM_LOOPINV = 0;

	result = TRUE;
	while( !(emptyP(&eta)) )
	{
		CUR_RECURSION_DEPTH = 0;
		LIST = initE();

		#ifdef DEbug
		printf("\n\nThrough Queue: %d\n", CUR_RECURSION_DEPTH);
		#endif

		resetPropagatedVector( &(M0->states[eta.cstate[eta.front].s0].propVector) );
		resetPropagatedVector( &(M1->states[eta.cstate[eta.front].s1].propVector) );

		if(correspondenceChecker(&(M0->states[eta.cstate[eta.front].s0]), &(M1->states[eta.cstate[eta.front].s1]), &P0, &P1, M0, M1, &eta, &C_eta, E_u, E_c, LIST) == FALSE)
		{
			printf("\n\nFailure detected in case of correspondenceChecker( %s, %s )\n\n",M0->states[eta.cstate[eta.front].s0].state_id, M1->states[eta.cstate[eta.front].s1].state_id);

			result = FALSE;
			break;
		}

		deQP(&eta, &tempEta);
	}


	printf("\n\n\n\n###################### Verification Report #######################\n\n");

	if( result == TRUE )
		printf("\n M0 (%s) and M1 (%s) are equivalent.\n", M0->name, M1->name);
	else
		printf("\n M0 (%s) and M1 (%s) may not be equivalent.\n", M0->name, M1->name);

	printf("\n No. of states in M0: %d, and No. of states in M1: %d\n", M0->numstates, M1->numstates);

	printf("\n No. of paths in the path cover of M0: %d, and No. of paths in the path cover of M1: %d\n", P0.numpaths, P1.numpaths);

	#ifdef DEbug
	printf("\n No. of null paths in the path cover of M0: %d, and No. of null paths in the path cover of M1: %d\n", P0.offset_null_path, P1.offset_null_path);
	#endif

	//The following strategy for finding number of common and uncommon variables does not work because
	//of repetition of variables in V0 and V1. Moreover, integer values (like 1, 2, etc.) are also
	//stored in the variable lists.

	//countUncommon = (V0.no_of_elements - V0_V1.no_of_elements)+ (V1.no_of_elements - V0_V1.no_of_elements);
	//printf("\n No. of common variables: %d, and No. of uncommon variables: %d\n", V0_V1.no_of_elements, countUncommon);
	//NB: The presence of - (minus) can make a difference by 1, from reality, for the above outputs

	printf("\n Maximum depth of recursion: %d\n", MAX_RECURSION_DEPTH);

	printf("\n Maximum mismatch between two value vectors: %d\n", MAX_MISMATCH);

	printf("\n List of corresponding states: \n");
	printEta(&eta, M0, M1, corresponding_state_pair_visited);

	printf("\n\n List of C-corresponding states: \n");
	printEta(&C_eta, M0, M1, C_corresponding_state_pair_visited);

	printf("\n\n\n List of unconditionally equivalent (U-equivalent) paths: \n");
	printE(E_u, &P0, &P1, M0, M1);

	printf("\n\n List of conditionally equivalent (C-equivalent) paths: \n");
	printE(E_c, &P0, &P1, M0, M1);

	//The number of code motions in reality may vary from COUNT_CODE_MOTIONS by a large amount
	//The division by 2 is for taking into consideration the fact that a variable
	//is defined at two different positions in the two behaviours to account for a mismatch
	#ifdef DEbug
	printf("\n\n Approximate number of code motions: %d\n", COUNT_CODE_MOTIONS/2);

	printf("\nNumber of times correspondenceChecker called:%d\n", NUM_CORRES);
	printf("Number of times findEquivalentPath (valuePropagation) called:%d\n", NUM_FINDEQV);
	printf("Number of times loopInvariant called:%d\n", NUM_LOOPINV);
	#endif

	//err =
	getrusage(RUSAGE_SELF, &timeUsage);

	printf("\n User CPU time used: %ld sec and %ld microsecs\n", timeUsage.ru_utime.tv_sec, timeUsage.ru_utime.tv_usec);
	printf("\n System CPU time used: %ld sec and %ld microsecs\n", timeUsage.ru_stime.tv_sec, timeUsage.ru_stime.tv_usec);

	printf("\n\n####################################################################\n\n");

	return result;
}

//The following variable is used to indicate the Parser which FSMD is to be generated
//    FALSE implies FSMD M0
//    TRUE  implies FSMD M1
boolean flagVar_List;

//FSMDs are declared globally so that they can be available to the Parser
FSMD *M0, *M1;


extern void callParser( char* );


//Main Function
//Takes FSMDs M0 and M1 as input
int main( int argc, char* argv[] )
{
	//FSMD  M0, M1;
	//M0 -- original FSMD , M1 -- Scheduled FSMD

	if(argc != 3)
	{
		printf("\nInadequate number of parameters provided\nExiting System\n");
		return 0;
	}


	//Read two FSMD structures
	flagVar_List = FALSE;

	outputVar0.no_of_elements = 0;

	callParser(argv[1]);
	//read_fsmd( argv[1], &M0, &V0 );

	#ifdef DETAILS
	print_fsmd( M0 );
	#endif

	//parsing of first file is complete now

	flagVar_List = TRUE;

	outputVar1.no_of_elements = 0;

	callParser(argv[2]);
	//read_fsmd( argv[2], &M1, &V1 );

	#ifdef DETAILS
	print_fsmd( M1 );
	#endif

	#ifdef DEbug
	printf("\n FSMDs read successfully \n");
	#endif

	NO_OF_VARIABLES = stab.numsymbols;

	containmentChecker(M0, M1); //V0 and V1 are globally defined

	//return containmentChecker(M0, M1);
	//The above line is commented out because returning any non-zero value
	//resutls in "make Error"

	return 0;
}
