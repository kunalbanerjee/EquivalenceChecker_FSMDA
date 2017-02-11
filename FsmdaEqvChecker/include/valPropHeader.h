#ifndef VALPROP_HEADER
#define VALPROP_HEADER

//The following variable can be replaced with stab.numsymbols
//Its value is assigned in main() in valPropCode.c
int NO_OF_VARIABLES;

#define INDEX_OF_NULL_PATH 1000
#define INDEX_OF_OMEGA_PATH 2000
#define INDEX_OF_DUMMY_STATE 3000

//Data structure for storing (un)conditionally equivalent path pairs
//along with respective propagated vectors
//Similar to CPQ
typedef struct equivalent_path_pair
{
  int p0; //stores indices from PATHS_LIST P0
  int p1; //stores indices from PATHS_LIST P1
  PROPAGATED_VECTOR *pv0; 
  PROPAGATED_VECTOR *pv1;
  struct equivalent_path_pair *next;
}PATH_PAIR;


//The following global variables are used
//to obtain experimental results

//For storing maximum depth of recursion, and
//For storing current depth of recursion
unsigned int MAX_RECURSION_DEPTH, CUR_RECURSION_DEPTH;

//For storing maximum number of mismatches between 2 value vectors
unsigned int MAX_MISMATCH;

//For storing the approximate number of code motions performed
//while obtaining the transformed behaviour from the original one
unsigned int COUNT_CODE_MOTIONS;

//For storing the number of times a corresponding state pair is visited
unsigned int corresponding_state_pair_visited[QUEUESIZE];

//For storing the number of times a corresponding state pair is visited
unsigned int C_corresponding_state_pair_visited[QUEUESIZE];
//Not able to calculate it properly due to recursion

//For storing the number of times a particular function is called
unsigned int NUM_CORRES; //can also be determined from the summation of 
                         //corresponding_state_pair_visited and C_corresponding_state_pair_visited
                         //(if they can be calculated correctly)
unsigned int NUM_FINDEQV;
unsigned int NUM_LOOPINV;


//Functions for Propagated Vector
void resetPropagatedVector( PROPAGATED_VECTOR* );
void copyPropagatedVector( PROPAGATED_VECTOR*, PROPAGATED_VECTOR* );
boolean equivalentPropagatedVector( PROPAGATED_VECTOR*, PROPAGATED_VECTOR* );
void printPropagatedVector( PROPAGATED_VECTOR* );
int countMismatch( PROPAGATED_VECTOR*, PROPAGATED_VECTOR* );


//Functions for E_u, E_c, LIST
PATH_PAIR* initE();
void unionE( PATH_PAIR*, int, int, PROPAGATED_VECTOR*, PROPAGATED_VECTOR*, PATHS_LIST*, PATHS_LIST* );
void printE( PATH_PAIR*, PATHS_LIST*, PATHS_LIST*, FSMD*, FSMD* );
//Functions for LIST
boolean already_present_LIST( PATH_PAIR*, int, int, PATHS_LIST*, PATHS_LIST*, FSMD*, FSMD* );
int removeLastPathPairFromLIST( PATH_PAIR*, PATHS_LIST*, PATHS_LIST*, FSMD*, FSMD* );


//Functions for eta, C_eta
boolean already_present_queue( CPS_Q*, int, int, unsigned int[] );
void printEta( CPS_Q*, FSMD*, FSMD*, unsigned int[] );
void initializeStatePairVisited( unsigned int[] );


//Functions related to value propagation
PATH_NODE* createDummyNode( NC*, int );
PATH_ST* createDummyPath( PATH_ST*, PROPAGATED_VECTOR* );
void createNullPath( int, PATHS_LIST* );
NC* removeIdenticalRelations( NC* );
void patchRalpha( PATH_ST*, NC* );
int findSymbolTableIndex( int );
void modifyFlags( NC*, int[] );


//Miscellaneous Functions
boolean checkFeasibilityOfCondition( NC* );
boolean compatibleConditions( NC*, NC* );
void printPath( PATH_ST*, PATHS_LIST*, int );
boolean isVariableLive( int, LV* );
boolean conflictInOutputs( r_alpha*, r_alpha* );


//MAJOR Functions
void valuePropagation( PATH_ST*, PROPAGATED_VECTOR*, PATH_ST*, PROPAGATED_VECTOR*, PROPAGATED_VECTOR*, PROPAGATED_VECTOR* );
boolean loopInvariant( PATH_ST*, PROPAGATED_VECTOR*, FSMD* );
void findEquivalentPath( PATH_ST*, PROPAGATED_VECTOR*, NODE_ST*, PROPAGATED_VECTOR*, PATHS_LIST*, PATHS_LIST*, 
			                                FSMD*, FSMD*, PROPAGATED_VECTOR*, PROPAGATED_VECTOR*, int*, int* );
boolean correspondenceChecker( NODE_ST*, NODE_ST*, PATHS_LIST*, PATHS_LIST*, FSMD*, FSMD*, 
                                      CPS_Q*, CPS_Q*, PATH_PAIR*, PATH_PAIR*, PATH_PAIR* );
boolean containmentChecker( FSMD*, FSMD* );

#endif
