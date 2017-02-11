#ifndef FSMDA_HEADER
#define FSMDA_HEADER

/******************************************************************************************/

/*      This is the header file which contain the definitions
 *      and the prototypes of the functions which were used for
 *      the verification of the scheduling                       */

/******************************************************************************************/

#define MAXSTATES 1000
#define STACKSIZE 1000
#define SYMTABSIZE 200
#define MAXOP 400
#define QUEUESIZE 1000
#define CHAR_SIZE 1000

#define boolean int
#define TRUE 1
#define FALSE 0

//Uncomment the following definition to get details
//#define DETAILS

typedef struct normalized_cell NC;

struct normalized_cell
{
   NC *list;
   char type;
   int inc;
   NC *link;
};


typedef NC N_sum, N_term, N_primary;




// This structure is used to hold the information regarding the variables of the FSMD's
typedef struct variables{

  int var_val[SYMTABSIZE];      // integer value of the variable
  char *var_string[SYMTABSIZE]; // variable
  int no_of_elements;           // number of elements in the list
}var_list;




/* This structure is used to hold the information of an assignment operation */


typedef struct Assign
{

   int lhs;   // left hand side (lhs) variable's symbol table index is stored here
              //arrays treated similarly
   NC *rhs;   // right hand side of the assignment is stored as an expression tree
}DATA_TRANS;


//Data structure for array handling
#define MAX_ARRAY_VALUES 20

typedef struct sub_propagated_vector
{
	NC *arrayValue[MAX_ARRAY_VALUES];
	int countValues;
}SUB_PROPAGATED_VECTOR;
//End of array data structures


// Declaration for Value Propagation //

typedef struct path_struct PATH_ST;

//Data structure for propagated vector
typedef struct propagated_vector
{
   NC *cond; // condition
   //we can make the lhs implicit, which is equal to the index of the variable in the symbol tabble
   //index corresponds to sym_tab and Not val_tab (because val_tab is not assigned values consecutively)
   //rhs is stored in value (given below)
   NC *value[SYMTABSIZE];

   //array start
   SUB_PROPAGATED_VECTOR subVector[SYMTABSIZE];
   //array end
}PROPAGATED_VECTOR;
// End of Declaration for Value Propagation //



//  This structure hold the information of each transition of the FSMD
typedef struct trans_struct
{
	NC  *condition;          // transition condition

	DATA_TRANS action[100];  // action to be perfomed during transition

	int outtrans;
	//  this is the pointer to the resulting state after transition

    struct trans_struct  *next;
	//  this is the pointer to the next transition of the same state

}TRANSITION_ST;
// TRANSITION_ST is the name of the structure for transition



//  This holds the complete infomation of a state in the FSMD
typedef struct state_struct
{
	char  state_id[100];  // this is the name of the structure

	int   node_type;

	//  this is the type of the state,it may be start state or
	//  final state or any cutpoint or any other state

	int   numtrans; // this holds the number of transitions of that state

    TRANSITION_ST  *translist;
	// this is the list of all transitions from this state


	//value propagation code start
	boolean VAPFLAG; //flag to track VAP (Visited Along a Path)

	PROPAGATED_VECTOR propVector;
	//value propagation code end


}NODE_ST;  // This is the structure for a state

// NODE_ST is the name of the structure holding the state information


typedef struct fsmd_struct
{
	char name[150];

	int  numstates;

	NODE_ST  states[MAXSTATES];
}FSMD;



//  this stores the infomation about the nodes along a particular path

typedef struct path_nodes_struct
{
	int state;            // This is the pointer to the state along the path
    NC *condition;
    DATA_TRANS *actions;
    struct path_nodes_struct   *next;  // Pointer to the next (successor) node along the path
    struct path_nodes_struct   *left;  // Pointer to the previous node of the path

}PATH_NODE;


typedef struct ralpha_node
{
	DATA_TRANS  Assign;
	struct ralpha_node *next;
}r_alpha;


//The following declaration is required for LVA
typedef struct live_variables_list LV;

//This holds all the information about a particular path

struct path_struct
{
  int start;     // This is the start state of the path

  int end;       // This is the end state of the path

  int status;

  int extendible; // 0 means the path is not extendible , 1 means the path is extendible
  int concateble; // 0 means the path is not concateble , 1 means the path is concateble

  NC *condition;
  // This is to hold the conditional expression for traversing the path

  r_alpha *transformations;
  // This holds the data transfomations along the path

  PATH_NODE   *nodeslist;
  // This is the pointer to the list of states along the path
  struct path_struct  *cp;
  struct path_struct  *next;
  // This is the pointer to  the next path structure in the paths list

  //The following stores the list of variables that are LIVE at the END of this path
  LV* live;
  //The following boolean variable stores TRUE if it is the final path in the FSMD,
  //stores FALSE otherwise; this vairable is required for LVA (without OUTPUT_ONLY)
  boolean finalPath;

}; //PATH_ST;
// PATH_ST is the name of the  structure which holds information about a path

typedef struct pathlist_struct
{
	int numpaths;

	//value propagation code start
	//the following variable keeps a count
	//of the null paths inserted
	unsigned int offset_null_path;
	//value propagation code end

	PATH_ST paths[1000];
	FSMD    *fsmd;
}PATHS_LIST;


// It is for a queue of paths
typedef struct pathqueqe_struct
{
    int front;
    int end;
    PATH_ST paths[QUEUESIZE];
}PATH_Q;


typedef struct stack_struct
{
    DATA_TRANS *actions[STACKSIZE];
    NC *condition[STACKSIZE];
	int state[STACKSIZE];
	int top;
}STACK;


typedef struct sym_table
{
	char *sym_tab[SYMTABSIZE];
	int val_tab[SYMTABSIZE];
	int numsymbols;
}SYMTAB;

SYMTAB  stab;

typedef struct CST
{
	int s0;
	int s1;
	struct CST *next;
}CPQ;

// Structure for corresponding state
typedef struct CPState
{
    int s0;
    int s1;
}CPS;

// It is a queue of corresponding state
typedef struct CSQ
{
    int front;
    int end;
    CPS cstate[QUEUESIZE];
}CPS_Q;


// This function is used to find the index of a particular variable in the symbol table
typedef struct node_list
{
	NC *node;
	struct node_list *next;
}NCL;


//Definitions copied from fsmdCode

/* V0 is the list of variables which are present in FSMD M0, V1 is the list of
 * variables which are present in FSMD M1, and V0_V1 is the list of variables
 * which are present both in V0 and V1.
 */
var_list V0, V1, V0_V1, V0_kripke, V1_minus_V0_V1;

//The following is required for live variable analysis
var_list outputVar0, outputVar1;
//End of definitions


/************************/


// This function returns the integer value for the string S
int indexof_symtab( char* );

// This function retruns the string representation of the integer( or integer value of the variable )
void symbol_for_index( int , char* );

int indexof_varlist( char*, var_list* );

int isconstant( char* );

int constval( char* );

NC* create_expr( char*, char, char* );

NC* create_term( char*, int );

// This function is used to create a data structure for particular state
int create_state( char[], FSMD*, int );

// This function is used to read the FSMD information from a file
int read_fsmd( char*, FSMD*, var_list* );

// This function is used to print the given FSMD
int print_fsmd( FSMD* );

// This is used to pop a state from the stack during the DFT to find all paths
void stack_pop( STACK* );

// This function is used to push a particular state on to the stack
void stack_push( STACK*, int, NC*, DATA_TRANS* );


// from the depth first traversal
void printpath( FSMD*, STACK* );

// This function is used to find whether a particular state is a cutpoint
int cutpoint( NODE_ST );

// These two functions are used to find all possible paths in a FSMD
void dividepaths( FSMD*, int, PATHS_LIST*, STACK* );
void findpaths( FSMD*, PATHS_LIST* );

// This function is used to store the current path into memory
void  storepath( FSMD*, PATHS_LIST*, STACK* );

// This function is used to display all possible possible paths in a FSMD
void displayallpaths( PATHS_LIST* );
void displayallpaths_withcp( PATHS_LIST*, FSMD* );
NC* parsecondition( char*, NC* );
void parseassignments( char*, DATA_TRANS*, var_list* );
void parseA( char*, DATA_TRANS*, var_list* );

NC*  parseclause( char*, int, int, NC* );
NC* create_condexpr( char*, char*, int, int, int, NC* );

int isfunction( char* );

void find_Ralpha( PATH_ST* );
void find_ralpha( PATH_ST*, FSMD* );

CPQ* path_cover( PATHS_LIST*, FSMD* );
int findcp( FSMD*, PATH_ST*, int, PATHS_LIST, FSMD*, FSMD* );
int is_in_CPQ( CPQ*, int );
int r_alpha_equal( r_alpha*, r_alpha*, int*, int*, int*, int* );
void Associate_R_and_r_alpha( PATHS_LIST* );

int negateoperator(int op);

//added functions -***
void initP( CPS_Q* );
void initF( PATH_Q* );
CPQ* initEta();
int emptyP( CPS_Q* );
int emptyF( PATH_Q* );
void enQF( PATH_Q*, PATH_ST* );
void enQP( CPS_Q*, int, int );
void deQP( CPS_Q*, CPS* );
PATH_ST deQF( PATH_Q* );
void findEquivalent( PATH_ST*, FSMD*, FSMD*, CPQ*, PATHS_LIST, FSMD* );
void unionEta( CPQ*, int, int );
void equivalenceChecking_m0_to_m1( FSMD, FSMD, PATHS_LIST, PATHS_LIST, CPS_Q, PATH_Q, CPQ*, FSMD );
void extendedPaths( PATHS_LIST*, PATH_ST*, FSMD*,PATH_Q*, PATHS_LIST* ) ;
int checkCondition( PATH_ST*, PATH_ST* );
int addPath( PATH_ST*, PATH_ST*, PATHS_LIST, FSMD* );
PATH_ST* concatPaths( PATH_ST*, PATH_ST* );
PATH_ST* copyPath( PATH_ST* );
void displayApath( PATH_ST* );
void find_Ralpha1( PATH_ST* );

//new functions
int generate_kripke_st( FSMD* );
void generate_smv_code( FSMD*, int, int, int*, int );


NC* copylist( NC* );
int compare_trees( NC*, NC* );
void write_lists( NC* );

/************************/

N_sum *simplify_sum( N_sum*, N_sum* );
N_sum *simplify_sum_1( N_sum*, N_sum*, int );
int Compare_Sums( N_sum*, N_sum* );
int Check_c1_c2_And_R1_R2( int, int, int, int );
int A_implies_B( NC*, NC* );
int Compare_Conditions( NC*, NC* );
int Search_cond_in_expr( NC*, NC* );
NC *Delete_cond_from_expr( NC*, NC*, int, NC* );
NC *Remove_mult_occurence_cond_in_expr( NC*, NC*, NC* );
NC *Remove_all_mult_occurence_in_expr_1( NC*, NC*, NC* );
NC *Remove_all_mult_occurence_in_expr( NC*, NC* );
N_sum *SubExpr_simplify_sum( N_sum*, N_sum* );
N_sum *SubExpr_simplify_terms( N_sum*, int, N_sum* );
NC *SubExpr_simplify_condition( NC*, NC* );

/****************/

N_sum * Add_Sums( N_sum*, N_sum*, N_sum* );
N_sum * Add_Sums_1( N_sum*, N_sum*, N_sum* );
int Compare_Primaries( N_primary*, N_primary* );
int Compare_Terms( N_term*, N_term* );
void symbol_for_index( int, char* );
N_term * Mult_Term_With_Term_1( N_term*, N_term*, N_term* );
N_term * Mult_Term_With_Term( N_term*, N_term*, N_term* );
N_sum * Mult_Sum_With_Term_1( N_sum*, N_term*, N_sum* );
N_sum * Mult_Sum_With_Term( N_sum*, N_term*, N_sum* );
N_sum * Mult_Sum_With_Sum( N_sum*, N_sum*, N_sum* );
N_sum * Mult_Sum_With_Sum_1( N_sum*, N_sum*, N_sum* );
N_sum * Mult_sum_with_constant( N_sum*, int, N_sum* );
N_sum * Mult_sum_with_constant_1( N_sum*, int, N_sum* );
N_term * Mult_constant_with_term( int, N_term*, N_term* );
N_term * Mult_primary_with_term( N_primary*, N_term*, N_term* );
N_term * Mult_primary_with_term_1( N_primary*, N_term*, N_term* );
N_sum *Replace_Primary_1( N_term*, N_primary*, N_sum*, N_term*, N_term*, N_sum*, N_sum*, int*, int*, int* );
N_sum *Replace_Primary( N_term*, N_primary*, N_sum*, N_term*, N_term*, N_sum*, N_sum*, int*, int*, int* );
N_sum *Substitute_In_Sum( N_sum*, N_primary*, N_sum*, N_sum*, int*, int*, int* );
N_sum *Substitute_In_Sum_1( N_sum*, N_primary*, N_sum*, N_sum*, int*, int*, int* );
NC *Substitute_in_MOD_DIV( NC*, r_alpha*, NC* );

void cal_V0_intersection_V1( var_list* );
void cal_V1_minus_V0_intersection_V1( var_list* );
int search_primary_in_primary( NC*, NC*, int );

/**************************************************************/



//NB: For LVA, backward traversal of the CFG of the FSMD is required


//Data structures for CFG


//The following data structure contains a list of variables
typedef struct list_of_variables
{
	int variable[SYMTABSIZE];
	int countVars;
}VARIABLE_LIST;


//Data structures for LVA
struct live_variables_list
{
	int countLiveVar;
	int liveVar[SYMTABSIZE];
	//the values stored in liveVar tally with that of stab.sym_tab

	//the following variable stores the state index (cutpoint) to which
	//the LV corresponds to in the respective FSMD
	int stateIndex;
}; //LV;
//end of Data structures for LVA


//All the paths are enlisted in PATHS_LIST (see header.h)
//For LVA, the CFG must be available
//The following data structures are required to construct the
//CFG from PATHS_LIST

typedef struct path_graph_node CFG_NODE;

typedef struct list_of_CFG_nodes
{
	CFG_NODE *nlist;
	struct list_of_CFG_nodes *next;
}CFG_NODE_LIST;


struct path_graph_node
{
	PATH_ST *path;
	CFG_NODE_LIST *pred;
	CFG_NODE_LIST *succ;
	int indexAtPathCover;

	VARIABLE_LIST *def;
	VARIABLE_LIST *use;

	LV *live_s;
	LV *live_f;
}; //CFG_NODE

#define MAX_TAIL 10

typedef struct path_graph
{
	int countNodes;
	CFG_NODE *node[1000];
	CFG_NODE *tail[MAX_TAIL];
	//Ideally, there should be only one final (tail) state
	//coinciding with the reset state
	//However, there can be more than one "tail" states since in
	//the input file there can be multiple entries like the following
	//q_state 0 ;
	//We consider there is a -/- path (Condition: True, Transformation: Null)
	//from each such q_state to q_reset
	//tail [array] contains all such q_state

	FSMD *fsmd;
}CFG;

//end of Data structures for CFG



//Functions that appear in lva.c

//Functions related to control flow graph
CFG* constructCFG( PATHS_LIST* );
void printCFG( CFG* );


//Construction of data flow equations
void markVariables( NC*, VARIABLE_LIST* );
void computeDefUse_CFG_Node( CFG_NODE* );
void computeDefUse_CFG( CFG* );

void updateLiveF( CFG_NODE* );
void updateLiveS( CFG_NODE* );
void updateLivenessInfo( CFG*, CFG_NODE*, boolean* );

void liveVariableAnalysis( CFG* );
void updateLVofPaths( CFG* );
void performLiveVariableAnalysis( PATHS_LIST* );

//Functions related to LV
LV* copyLV( LV* );
boolean equalLV( LV*, LV* );


//Function to determine output variable
boolean isOutputVar( int, var_list );

#endif
