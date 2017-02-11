#ifndef PARSER_DOT_HEADER
#define PARSER_DOT_HEADER

//Data structure for a substatement
typedef struct SubStatement {
	char characteristics[1000]; //stores the characteristics (condition and data transformations)
	                            //of an edge
	char destState[20];         //stores the name of the destination state
}SUB_STMT;

typedef struct SubStatement_List {
	int numSubstmts;
	SUB_STMT *substmtList[20];
}SUB_STMT_LIST;

#endif
