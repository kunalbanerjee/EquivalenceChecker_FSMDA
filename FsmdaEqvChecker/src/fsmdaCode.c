
/*   This file contains all the routines required to create an FSMD
 *   and dividing the FSMD into a set of paths from an INPUT file
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "FsmdaHeader.h"

int count_extend = 0, count_iteration = 0;
int nmodelCheck = 0;
int nWeak=0;


/*  This function calculates the list of variables which are present in both
 *  var_list 'V0' and var_list 'V1' in var_list 'V0_V1'.
 */

void
cal_V0_intersection_V1( var_list *V0_V1 )
{
  int i, j;
  for( i = 0; i < V0.no_of_elements; i++ )
  {
     for( j = 0; j < V1.no_of_elements; j++ )
     {
        if( V0.var_val[i] == V1.var_val[j] )
	{
	   V0_V1->var_val[V0_V1->no_of_elements] = V0.var_val[i];
	   V0_V1->var_string[V0_V1->no_of_elements] = ( char * ) malloc( strlen( V0.var_string[i] ) +1 );
	   strcpy( V0_V1->var_string[V0_V1->no_of_elements], V0.var_string[i] );
	   V0_V1->no_of_elements++;
	 }
     }
  }
}



/* This function checks if a variable 'var' is present in var_list 'V0_V1'.
 * It returns '1' if the variable is present , else returns '-1'.
 */
int
var_in_V0_V1( int var )
{

 int i;
 for( i = 0; i < V0_V1.no_of_elements; i++ )
 {
    if( var == V0_V1.var_val[i] )
       return 1;
 }
 return -1;
}


/* This function checks if a variable 'var' is present in var_list 'V1_minus_V0_intersection_V1'.
 * It returns '1' if the variable is present , else returns '-1'.
 */
int
var_in_V1_minus_V0_intersection_V1( int var )
{

 int i;
 for( i = 0; i < V1_minus_V0_V1.no_of_elements; i++ )
 {
    if( var == V1_minus_V0_V1.var_val[i] )
       return 1;
 }
 return -1;
}


void
cal_V1_minus_V0_intersection_V1( var_list *V1_minus_V0_V1 )
{
	int i, j, flag;
	for( i = 0; i < V1.no_of_elements; i++ )
	{
		flag = 0;
		for( j = 0; j < V0_V1.no_of_elements; j++ )
		{
			if( V1.var_val[i] == V0_V1.var_val[j] )
			{
				flag = 1;
				break;
			}
		}
		if( flag == 0 )
		{
			V1_minus_V0_V1->var_val[V1_minus_V0_V1->no_of_elements] = V1.var_val[i];
			V1_minus_V0_V1->var_string[V1_minus_V0_V1->no_of_elements] = ( char * ) malloc( strlen( V1.var_string[i] ) +1 );
			strcpy( V1_minus_V0_V1->var_string[V1_minus_V0_V1->no_of_elements], V1.var_string[i] );
			V1_minus_V0_V1->no_of_elements++;
		}
	}//end for
}



/* This function generates a 'kripke structure' for FSMD M0.
 * The 'kripke structure' is a file named "file_kripke_st".
 * It returns the no. of dummy states generated in kripke structures.
 */
 int
 generate_kripke_st( FSMD *M0 )
 {

     FILE *fp_k;
     TRANSITION_ST *temp1, *temp2, *temp4, *temp5; // holds all transitions information of a fsmd state
     NC *p; // pointer to normalized cells

     char sym_value[100];
     int i, j, k , used_flag, l, define_flag, k_state_num = 0;

	fp_k = fopen( "file_kripke_st", "w" );

	for( i = 0; i < M0->numstates; i++ )
	{
	    if( M0->states[i].numtrans == 0 )
	    { // end state
	      fprintf( fp_k, "%s ", M0->states[i].state_id );

	      for( l = 0; l < V0_V1.no_of_elements; l++ )
	      {
	           fprintf( fp_k, "D_%s=0,", V0_V1.var_string[l] );
		}
	      for( l = 0; l < V0_V1.no_of_elements - 1; l++ )
	      {
	           fprintf( fp_k, "U_%s=0,", V0_V1.var_string[l] );
	        }
	      fprintf( fp_k, "U_%s=0", V0_V1.var_string[l] );

	      fprintf( fp_k, " 1 %s", M0->states[i].state_id );	 //make e loop at the end state;
								// no need to return to the reset state

	    }

	   else if( M0->states[i].numtrans == 1 )
	   {
	     fprintf( fp_k, "%s ", M0->states[i].state_id );

	     for( l = 0; l < V0_V1.no_of_elements; l++ )
	     {
	           fprintf( fp_k, "D_%s=0,", V0_V1.var_string[l] );
		}
	     for( l = 0; l < V0_V1.no_of_elements - 1; l++ )
	     {
	           fprintf( fp_k, "U_%s=0,", V0_V1.var_string[l] );
	        }
	     fprintf( fp_k, "U_%s=0", V0_V1.var_string[l] );

	     fprintf( fp_k," %d q_k_%d\n", M0->states[i].numtrans, k_state_num );


	     fprintf( fp_k,"q_k_%d ", k_state_num );

	     temp1 = M0->states[i].translist;

	     while( temp1 != NULL )
	     {
	     // operation for D_var ( or lhs )
	     for( k = 0; k < V0_V1.no_of_elements; k++ )
	     {
	         temp2 = M0->states[i].translist;
		 define_flag = 0;
		 j = 0;
	         while( temp2->action[j].rhs != NULL && define_flag == 0 )
		 {
		      symbol_for_index( temp2->action[j].lhs, sym_value );
		      if( strcmp( sym_value, V0_V1.var_string[k] ) == 0 )
		      {
		           fprintf( fp_k, "D_%s=1,", V0_V1.var_string[k] );
			   define_flag = 1;
			   break;
			 }
		      j++;
		 }
		 if( define_flag == 0 )
		    fprintf( fp_k, "D_%s=0,", V0_V1.var_string[k] );
	      }

	      // operation for U_var ( or rhs )
	      for( k = 0; k < V0_V1.no_of_elements - 1; k++ )
	      {
	           temp2 = M0->states[i].translist;
		   used_flag = 0;
		   j = 0;
		   p = ( NC * )malloc( sizeof( NC ) );
		   p->type = 'v';
		   p->inc = V0_V1.var_val[k];
		   p->link = NULL;
		   p->list = NULL;
		   while( temp2->action[j].rhs != NULL && used_flag == 0 )
		   {
			used_flag = search_primary_in_sum( temp2->action[j].rhs, p, used_flag );
			if( used_flag == 1 )
			{
			  fprintf( fp_k, "U_%s=1,", V0_V1.var_string[k] );
			  break;
			}
		      j++;
		   }
		   if( used_flag != 1 )
		      fprintf( fp_k, "U_%s=0,", V0_V1.var_string[k] );
	         }

		 // printing the last U_var to avoid the last comma ( , )
		 k = V0_V1.no_of_elements - 1;
	         temp2 = M0->states[i].translist;
		  used_flag = 0;
		  j = 0;
		  p = ( NC * )malloc( sizeof( NC ) );
		   p->type = 'v';
		   p->inc = V0_V1.var_val[k];
		   p->link = NULL;
		   p->list = NULL;
		  while( temp2->action[j].rhs != NULL && used_flag == 0 )
		  {
		      used_flag = search_primary_in_sum( temp2->action[j].rhs, p, used_flag );
		      if( used_flag == 1 )
		      {
			  fprintf( fp_k, "U_%s=1", V0_V1.var_string[k] );
			  break;
		      }
		      j++;
		   }
		   if( used_flag != 1 )
		      fprintf( fp_k, "U_%s=0", V0_V1.var_string[k] );



	     //next state
	      fprintf( fp_k, " 1 %s\n", M0->states[temp1->outtrans].state_id );
	      temp1 = temp1->next;
	      }
	    k_state_num++;
	  } // end of "if( M0->states[i].numtrans < 2 )"

	  else
	  { // if( M0->states[i].numtrans > 1

	       fprintf( fp_k, "%s ", M0->states[i].state_id );

	       for( l = 0; l < V0_V1.no_of_elements; l++ )
	       {
	           fprintf( fp_k, "D_%s=0,", V0_V1.var_string[l] );
		}
	       for( l = 0; l < V0_V1.no_of_elements - 1; l++ )
	       {
	           fprintf( fp_k, "U_%s=0,", V0_V1.var_string[l] );
	        }
	       fprintf( fp_k, "U_%s=0", V0_V1.var_string[l] );

	       fprintf( fp_k," %d ", M0->states[i].numtrans );

	       for( l = k_state_num; l < (k_state_num + M0->states[i].numtrans) - 1; l++ )
	       {
	           fprintf( fp_k, "q_k_%d,", l );
	       }
	       fprintf( fp_k,"q_k_%d\n", l );

	       temp4 = M0->states[i].translist;

	       while( temp4 != NULL )
	       {
	          fprintf( fp_k, "q_k_%d ", k_state_num );
		  k_state_num++;

	       // operation for D_var ( or lhs )
	       for( k = 0; k < V0_V1.no_of_elements; k++ )
	       {
	         temp5 = temp4;
		 define_flag = 0;
		 j = 0;
	         while( temp5->action[j].rhs != NULL && define_flag == 0 )
		 {
		      symbol_for_index( temp5->action[j].lhs, sym_value );
		      if( strcmp( sym_value, V0_V1.var_string[k] ) == 0 )
		      {
		           fprintf( fp_k, "D_%s=1,", V0_V1.var_string[k] );
			   define_flag = 1;
			   break;
			 }
		      j++;
		   }
		 if( define_flag == 0 )
		    fprintf( fp_k, "D_%s=0,", V0_V1.var_string[k] );
	      }


	      // operation for U_var ( or rhs )
	      for( k = 0; k < V0_V1.no_of_elements - 1; k++ )
	      {
	           temp2 = temp4;
		   used_flag = 0;
		   j = 0;
		   p = ( NC * )malloc( sizeof( NC ) );
		   p->type = 'v';
		   p->inc = V0_V1.var_val[k];
		   p->link = NULL;
		   p->list = NULL;
		   while( temp2->action[j].rhs != NULL && used_flag == 0 )
		   {
		      used_flag = search_primary_in_sum( temp2->action[j].rhs, p, used_flag );
		      if( used_flag == 1 )
		      {
			  fprintf( fp_k, "U_%s=1,", V0_V1.var_string[k] );
			  break;
		      }
		      j++;
		   }
		   if( used_flag != 1 )
		      fprintf( fp_k, "U_%s=0,", V0_V1.var_string[k] );
	         }

		 // printing the last U_var to avoid the last comma ( ',' )
		   k = V0_V1.no_of_elements - 1;
		   temp2 = temp4;
		   used_flag = 0;
		   j = 0;
		   p = ( NC * )malloc( sizeof( NC ) );
		   p->type = 'v';
		   p->inc = V0_V1.var_val[k];
		   p->link = NULL;
		   p->list = NULL;
		   while( temp2->action[j].rhs != NULL && used_flag == 0 )
		   {
		      used_flag = search_primary_in_sum( temp2->action[j].rhs, p, used_flag );
		      if( used_flag == 1 )
		      {
			  fprintf( fp_k, "U_%s=1", V0_V1.var_string[k] );
			  break;
		      }
		      j++;
		   }
		   if( used_flag != 1 )
		      fprintf( fp_k, "U_%s=0", V0_V1.var_string[k] );

		// next state
	       fprintf( fp_k, " 1 %s\n", M0->states[temp4->outtrans].state_id );
	       temp4 = temp4->next;
	       }
	   }

	}
      fclose( fp_k );
      return k_state_num;
 }



 /* This function generates a 'smv code' named "kripke.smv" for the 'kripke structure'
  * named "file_kripke_st".
  */

 void
 generate_smv_code( FSMD *M0, int k_state_num, int init_state, int *var_kripke, int no_of_kripke_vars )
 {

  FILE *fp_k, *fp_smv, *temp_fp;
  int no_of_states, no_of_trans, temp_no_of_trans, i, j, k, no_of_vars, var_flag = 0, l, m, m1;
  char state_id[1000], transitions[1000], next_state_id[1000], temp_state_id[1000], temp_transitions[1000], temp_next_state_id[200], sym_value[200];
  char **state_array, **var_array, temp_var[1000], temp_var1[1000], temp_var2[100], **next_state_id1, temp_char;
  /* variables are :
   *  1) state , and
   *  2) var_array[i] // it contains all the variables present in V0_V1 var list.
   */

  no_of_states = M0->numstates + k_state_num;
  //no_of_states = total no. of states in the kripke structure

  no_of_vars = 2 * V0_V1.no_of_elements ;
  //total no. of variables  is equal to :  " 2 * V0_V1.no_of_elements ".

  state_array = ( char ** )malloc( no_of_states * sizeof( char * ) );
  for( i = 0; i < no_of_states; i++ )
     state_array[i] = ( char * )malloc( 1000 * sizeof( char ) );

  var_array = ( char ** )malloc( no_of_vars * sizeof( char * ) );
  for( i = 0; i < no_of_vars; i++ )
     var_array[i] = ( char * )malloc( 1000 * sizeof( char ) );

  // storing the list of states from kripke structure in array 'state_array'.
  i = 0;
  fp_k = fopen( "file_kripke_st", "r" );
  while( !feof( fp_k ) )
  {
     fscanf( fp_k, "%s %s %d %s", state_id, transitions, &no_of_trans, next_state_id );
     strcpy( state_array[i], state_id );
     i++;
   }
  fclose( fp_k );


  // storing the variables in array 'var_array' from the transformations
  // of the kripke structure
  i = 0;
  j = 0;
  k = 0;
  while( i < strlen( transitions ) - 2 )
  {
    if( transitions[i] == '=' && transitions[i+2] == ',' )
    {
         var_array[k][j] = '\0';
	 k++;
	 j = 0;
	 i = i + 3;
       }
    else
    {
         var_array[k][j] = transitions[i];
	 i++;
	 j++;
       }
  }
  var_array[k][j] = '\0';


 // generation of "kripke.smv" code starts here
  fp_smv = fopen( "kripke.smv", "w" );
  fprintf( fp_smv, "MODULE main\n" );

  // variable definition
  fprintf( fp_smv, " VAR\n" );

  // defining states as the set of states from kripke structure
  fprintf( fp_smv, "  state : {" );
  for( i = 0; i < no_of_states - 1; i++ )
  {
     fprintf( fp_smv, "  %s,", state_array[i] );
  }
  fprintf( fp_smv, " %s };\n", state_array[i] );

  // defining all the variables in V0_V1 var list as boolean.
  for( i = 0; i < no_of_vars; i++ )
  {
     fprintf( fp_smv, "  %s : boolean;\n", var_array[i] );
  }


  //variables assignments
  fprintf( fp_smv, " ASSIGN\n" );

  // initializin state with the state passed as parameter.
  fprintf( fp_smv, "  init( state ) := %s;\n", M0->states[init_state].state_id );

  // initializing all the variables to the value which they contain
  // when they are present at state which is passed as parameter ( initial state )
  for( i = 0; i < no_of_vars; i++ )
  {
     fprintf( fp_smv, "  init( %s ) := 0;\n", var_array[i] );
  }


  // this generates the next value for a given state
  //next( state)
  fprintf( fp_smv, "\n  next( state ) := \n" );
  fprintf( fp_smv, "    case\n" );
  fp_k = fopen( "file_kripke_st", "r" );
  while( !feof( fp_k ) )
  {
      fscanf( fp_k, "%s %s %d %s", state_id, transitions, &no_of_trans, next_state_id );
      if( no_of_trans < 2 )
	  fprintf( fp_smv, "      state = %s : %s;\n", state_id, next_state_id );
      else if( no_of_trans > 1 )
	  fprintf( fp_smv, "      state = %s : { %s };\n", state_id, next_state_id );
  }
  fclose( fp_k );
  fprintf( fp_smv, "      1 : state;\n");
  fprintf( fp_smv, "    esac;\n\n");


  // this generates the next value of the variable
  // next( variable)
  for( l = 0; l < no_of_vars; l++ )
  {
    fprintf( fp_smv, "  next( %s ) := \n", var_array[l] );
    fprintf( fp_smv, "    case\n" );

    fp_k = fopen( "file_kripke_st", "r" );
    while( !feof( fp_k ) )
    {
      fscanf( fp_k,"%s %s %d %s", state_id, transitions, &no_of_trans, next_state_id );

	// extract the 'variable' and search the 'variable value' from transition
         i = 0;
         j = 0;
         var_flag = 0;
         while( i < strlen( transitions )  && var_flag == 0 )
	 {
            if( transitions[i] == '='  )
	    {
               temp_var[j] = '\0';
               if( strcmp( temp_var, var_array[l] ) == 0 )
	       {
                  temp_var[j] = transitions[i];
                  temp_var[j+1] = transitions[i+1];
                  temp_var[j+2] = '\0';
                  var_flag = 1;
		  break;
               }
	       j = 0;
	       i = i + 3;

             }
            else
	    {
               temp_var[j] = transitions[i];
	       i++;
	       j++;
             }
	 }
         if( var_flag == 0 )
	 {
	      temp_var[j] = '\0';
              if( strcmp( temp_var, var_array[l] ) == 0 )
	      {
                  temp_var[j] = transitions[i];
                  temp_var[j+1] = transitions[i+1];
                  temp_var[j+2] = '\0';
                  var_flag = 1;
               }
	 }

	 for( i = 0; i < strlen( temp_var) - 2; i++ )
	    temp_var2[i] = temp_var[i];
	 // 'temp_var' contains the value of the variable


     if( no_of_trans < 2 )
     { // no of transitions  < 2
	 fprintf( fp_smv, "      state = %s & %s = %c : ", state_id, temp_var2, temp_var[strlen(temp_var) - 1] );

	 // using the next state and finding the next value of vars
	 temp_fp = fopen( "file_kripke_st", "r" );
         while( !feof( temp_fp ) )
	 {
               fscanf( temp_fp,"%s %s %d %s", temp_state_id, temp_transitions, &temp_no_of_trans, temp_next_state_id );
               if( strcmp( next_state_id, temp_state_id ) == 0 )
	       {
	       	 // extract the var and its value in temp_transitions
                     i = 0;
		     j = 0;
		     var_flag = 0;
		     while( i < strlen( temp_transitions ) && var_flag == 0 )
		     {
		       if( temp_transitions[i] == '=' )
		       {
			 temp_var1[j] = '\0';
			 if( strcmp( temp_var1, temp_var2 ) == 0 )
			 {
			   temp_var1[j] = temp_transitions[i];
			   temp_var1[j+1] = temp_transitions[i+1];
			   temp_var1[j+2] = '\0';
			   var_flag = 1;
			   break;
			 }
			 j = 0;
			 i = i + 3;
		       }
		       else
		       {
			 temp_var1[j] = temp_transitions[i];
			 i++;
			 j++;
		       }
		     }
		     if( var_flag == 0 )
		     {
		       temp_var1[j] = '\0';
		       if( strcmp( temp_var1, temp_var2 ) == 0 )
		       {
			 temp_var1[j] = temp_transitions[i];
			 temp_var1[j+1] = temp_transitions[i+1];
			 temp_var1[j+2] = '\0';
			 var_flag = 1;
		       }
		     }

		     // 'temp_var1' contains the next value of the var
		     fprintf( fp_smv, " %c;\n", temp_var1[strlen(temp_var1) - 1] );
	       } // end of "if( strcmp( next_state_id, temp_state_id ) == 0"
	 }
	 fclose( temp_fp );
	}
     else
     { // no of transitions  >= 2

	 fprintf( fp_smv, "      state = %s & %s = %c : { ", state_id, temp_var2, temp_var[strlen(temp_var) - 1] );

	 // storing the next states in an array.
	 next_state_id1 = ( char ** )malloc( no_of_trans * sizeof( char * ) );
	 for( m = 0; m < no_of_trans; m++)
	     next_state_id1[m] = ( char * )malloc( 1000 * sizeof( char ) );

	 m1 = 0;
	 m = 0;
	 j = 0;
	 while( next_state_id[m1] != '\0' )
	 {
           if( next_state_id[m1] == ',' )
	   {
              next_state_id1[m][j] = '\0';
	      m++;
	      j = 0;
	      m1++;
           }
          else
	  {
             next_state_id1[m][j] = next_state_id[m1];
	     m1++;
	     j++;
          }
        }
        next_state_id1[m][j] = '\0';


	for( m = 0; m < no_of_trans ; m++)
	{
	 // using the next state and finding the next value of vars
	  temp_fp = fopen( "file_kripke_st", "r" );
          while( !feof( temp_fp ) ) {
               fscanf( temp_fp,"%s %s %d %s", temp_state_id, temp_transitions, &temp_no_of_trans, temp_next_state_id );
               if( strcmp( next_state_id1[m], temp_state_id ) == 0 )
	       {
	       	 // extract the var and its value in temp_transitions
                     i = 0;
		     j = 0;
		     var_flag = 0;
		     while( i < strlen( temp_transitions ) && var_flag == 0 )
		     {
		       if( temp_transitions[i] == '=' )
		       {
			 temp_var1[j] = '\0';
			 if( strcmp( temp_var1, temp_var2 ) == 0 )
			 {
			   temp_var1[j] = temp_transitions[i];
			   temp_var1[j+1] = temp_transitions[i+1];
			   temp_char = temp_transitions[i+1];
			   temp_var1[j+2] = '\0';
			   var_flag = 1;
			   break;
			 }
			 j = 0;
			 i = i + 3;
		       }
		       else
		       {
			 temp_var1[j] = temp_transitions[i];
			 i++;
			 j++;
		       }
		     }
		     if( var_flag == 0 )
		     {
		       temp_var1[j] = '\0';
		       if( strcmp( temp_var1, temp_var2 ) == 0 )
		       {
			 temp_var1[j] = temp_transitions[i];
			 temp_var1[j+1] = temp_transitions[i+1];
			 temp_char = temp_transitions[i+1];
			 temp_var1[j+2] = '\0';
			 var_flag = 1;
		       }
		     }

		     // 'temp_var1' contains the next value of the var
		    if( m == no_of_trans - 1 )
		    {
		        fprintf( fp_smv, " %c", temp_char );
		    }
		    else
		    {
		        fprintf( fp_smv, " %c,", temp_char );
		    }
	       } // end of "if( strcmp( next_state_id, temp_state_id ) == 0"
	 }
	 fclose( temp_fp );
        }
	fprintf( fp_smv, " };\n" );
      }
    }
    fclose( fp_k );
    fprintf( fp_smv, "      1 : %s;\n", var_array[l] );
    fprintf( fp_smv, "    esac;\n");
  }

  // SPEC : A [! U_d U ! (! D_d | U_d)]
  for( i = 0; i < no_of_kripke_vars; i++ )
  {
     fprintf( fp_smv,"SPEC\n");
     symbol_for_index( var_kripke[i], sym_value );
     fprintf(fp_smv, "    !E [ !D_%s U U_%s ]\n\n", sym_value, sym_value );

  //   fprintf(fp_smv, "    A [ ! U_%s U ! ( ! D_%s | U_%s ) ]\n\n", sym_value, sym_value, sym_value );

  }

  fclose( fp_smv );
}






/*****************************************************************************/

/*      This function reads a FSMD from the given input file     */

/*****************************************************************************/

int
read_fsmd( char arg[], FSMD *M, var_list *V )
{
	FILE *f1;
	char ch[5], statename[10], condition[500], action[500], nextstate[10];
	int numtrans, i, j, k, current_state, temp;
	TRANSITION_ST *trans;

	current_state = 0;
	f1 = fopen( arg, "r" );  /* open the file for reading the FSMD  */
	if( f1 == NULL )
	{
		printf("   THE FILE COULDN'T BE OPENED, TRY AGAIN \n");
		exit(0);
	}
	fscanf( f1, "%s",M->name );
    while( !feof(f1) )
	{
		fscanf( f1, "%s%d", statename, &numtrans );

		if( feof(f1) ) break;
		temp = create_state( statename, M, current_state );

		M->states[temp].numtrans = numtrans;


		//value propagation code start
		M->states[temp].VAPFLAG = FALSE;

		M->states[temp].propVector.cond = (NC*)NULL;
		for( j=0; j < SYMTABSIZE; j++ )
		{
		  M->states[temp].propVector.value[j] = (NC*)NULL;

			//array start
			M->states[temp].propVector.subVector[j].countValues = 0;
			for(k = 0; k < MAX_ARRAY_VALUES; k++)
			{
				M->states[temp].propVector.subVector[j].arrayValue[k] = (NC*)NULL;
			}
			//array end
		}
		//value propagation code ended


		if( numtrans == 0 )
		{
			M->states[temp].node_type = 1;
			/* state with no outward transition. This is the last state of the fsmd.
			 * its has a dummy transition from itself to the start state of the fmsd,
			 * with condition = '-' and action = '-'.
 			 */
			//fscanf(f1,"%s%s%s%s",condition,ch,action,nextstate);
		}
		else
			M->states[temp].node_type = 2;

		if( current_state == 0 )
		{ /*  This is the start state of the FSMD   */
			M->states[temp].node_type = 0; // start state
		}
		if( temp == current_state )
		{
			current_state++;
			strcpy(M->states[temp].state_id,statename);
		}

		M->states[temp].numtrans=numtrans;
		M->states[temp].translist=NULL;
		for( i = 0; i < numtrans; i++ )
		{
		/* This loop will read the transtions of each state  */

			trans=(TRANSITION_ST *) malloc (sizeof(TRANSITION_ST));
			trans->next=M->states[temp].translist;
			M->states[temp].translist=trans;
			fscanf(f1,"%s%s%s%s",condition,ch,action,nextstate);
			trans->condition=parsecondition(condition,trans->condition);

		    parseassignments( action, trans->action, V );
			trans->outtrans=create_state(nextstate,M,current_state);
			if(trans->outtrans==current_state)
			{
			    current_state++;
				M->states[trans->outtrans].translist=NULL;
				M->states[trans->outtrans].numtrans=0;
				M->states[trans->outtrans].node_type=1;
				strcpy(M->states[trans->outtrans].state_id,nextstate);
			}
		    }

	}
    M->numstates=current_state;
    return 0;
}





NC*
parseclause(char *clause,int operator,int not,NC* root) {
//operator = 1 for &&, =0 for ||, -1 for last
//conditions;   not=1 for !cond. and =0 for cond.

    int flag;
    char op1[30],op2[30];
    int i=0,j=0,relop;
    flag=0;

    while(clause[i]!='\0')
    {
	    while(!flag)
	    {
		     switch(clause[i])
		     {
			      case '!':
				     if(clause[i+1]=='=')
				     {
					     op1[j]='\0';
					     relop=5;
					     i++;
				     }
				     else
					     printf(" INVALID RELATIONAL OPERATOR  \n");
			             i++;
				     flag=1;
				     break;

			     case '=':
				     if(clause[i+1]=='=')
				     {
					     op1[j]='\0';
					     relop=4;
					     i++;
				     }
				     else
					    printf(" INVALID RELATIONAL OPERATOR \n");
				     i++;
				     flag=1;
				     break;

			    case '>':
				     if(clause[i+1]=='=')
			             {
					    relop=0;
					    i++;
				     }
				     else
					    relop=1;
				     i++;
				     op1[j]='\0';
				     flag=1;
				     break;

			    case '<':
				     if(clause[i+1]=='=')
				     {
					    relop=2;
					    i++;
				     }
				     else
					    relop=3;
				     i++;
				     op1[j]='\0';
				     flag=1;
				     break;
			    case 0:
				     op1[j]='\0';
				     op2[0]='\0';
				     flag=2;
				     relop=-1;
	                 break;
		       default :
			         op1[j++]=clause[i++];
		     }
        }
	if(flag==1)
	{
	    j=0;
	    while(clause[i]!='\0')
		    op2[j++]=clause[i++];
	    op2[j]='\0';
	}
    }
    root=create_condexpr(op1,op2,relop,operator,not,root);
    return root;
}




int
isfunction(char *s) {
	 int i=0;
	 while(s[i]!='\0'&&s[i]!='(')
		i++;
 	 if(s[i]!='\0')
		 return 1;
	 return 0;
}




int
negateoperator(int op) {
 	switch(op)
 	{
		case 0:
			return 3;
		case 1:
			return 2;
		case 2:
			return 1;
		case 3:
			return 0;
		case 4:
			return 5;
		case 5:
			return 4;
	}
}



int
getfunname(char *s,char *name) {
	int i=0,count=0;
	while(s[i]!='\0'&&s[i]!='(')
	{
		name[i]=s[i];
		i++;
	}
	name[i]='\0';
	if(s[i]!='\0')
	{
		while(s[i]!='\0')
		{
			if(s[i]==',') count++;
			i++;
		}
		count++;
	}
  return count;
}




void
getarguments(char *s,char *args[10]) {
    int i = 0, count=0,j;
    char name[20];
	while(s[i]!='\0'&&s[i]!='(')
	{
		i++;
	}
	if(s[i]!='\0')
	{
        i++;
        j=0;
		while(s[i]!='\0')
		{
			if(s[i]==','||s[i]==')')
            {
                name[j]='\0';
                args[count]=(char *)malloc(strlen(name)+1);
                strcpy(args[count],name);
                count++;
                j=0;
            }
            else
                name[j++]=s[i];
			i++;
		}
	}
}





NC*
create_condexpr(char *op1,char *op2,int relop,int logicalop,int not,NC *root)
{
    int i=0,numarg=0;
	char *args[10];
	char fun[20];
	int c1,c2,f=1,val,v1,v2;
	NC *A,*temp,*S,*T,*P,*R,*O,*S1;
    	if(not)
		relop=negateoperator(relop);
	O=(NC*) malloc(sizeof(NC));
	O->list=NULL;
	O->type='O';
	O->inc=0;

	R=(NC*) malloc(sizeof(NC));
	R->list=NULL;
	R->type='R';
	R->inc=relop;
	O->link=R;
        if(root==NULL)
	{
		A=(NC *)malloc(sizeof(NC));
		A->type='A';
		A->list=NULL;
		A->inc=0;
		A->link=O;
	    root=A;
	}
	else {
            temp = root->link;
	    while( temp->list != NULL )
		     temp = temp->list;
            temp->list = O;
	}

	if( op2[0] == '\0' ) { // single variable or funcation with boolean status
        	f = isfunction(op1);
		if( f ) {
             	     if(not) R->inc=4;
		     else R->inc=5;
                     numarg=getfunname(op1,fun);
		     getarguments(op1,args);
		     S=(NC*)malloc(sizeof(NC));
		     T=(NC*)malloc(sizeof(NC));
		     S->list=NULL;
		     S->link=T;
		     S->type='S';
		     S->inc=0;
		     T->list=NULL;
		     T->type='T';
		     T->inc=1;
		     temp=NULL;
		     P=(NC*)malloc(sizeof(NC));
		     P->list=NULL;
		     P->type='f';
		     P->inc=indexof_symtab(fun);
		     T->link=P;
		     for(i=0;i<numarg;i++)
		     {
			       S1=(NC*)malloc(sizeof(NC));
			       S1->type='S';
			       S1->inc=0;
			       S1->list=NULL;
			       S1->link=create_term(args[i],1);
			       if(temp!=NULL)
				       temp->list=S1;
			       else
			           P->link=S1;
			       temp=S1;
		     }
		}
		else
		{
		     S=(NC *)malloc(sizeof(NC));
		     S->list=NULL;
		     S->link=NULL;
		     if(isconstant(op1))
		     	S->inc=constval(op1);
		     else
		     {
			S->inc=0;
			T=(NC*)malloc(sizeof(NC));
			P=(NC*)malloc(sizeof(NC));
			T->list=NULL;
			T->link=P;
			P->list=NULL;
			P->link=NULL;
			P->type='v';
			T->type='T';
			T->inc=1;
			T->inc=indexof_symtab(op1);
			S->link=T;
		     }

		     S->type='S';
		}
		R->link=S;
    	}
        else // op2 is not NULL
	{
	        c1=isconstant(op1);
		c2=isconstant(op2);
        	if(c1&&c2)  //both are constants
                {
	       	        val=constval(op1)-constval(op2);
			S=(NC *)malloc(sizeof(NC));
			S->list=NULL;
			S->link=NULL;
			S->inc=val;
			S->type='S';
	   	}
                else if(c1||c2) //only one of the operands of the relational operators are constants
		{
			S=(NC*) malloc(sizeof(NC));
			S->list=NULL;
			S->type='S';
			if(c2)
			{
			      S->inc=-1*constval(op2);
			      S->link=create_term(op1,1);
			}
			else
			{
			      S->inc=constval(op1);
			      S->link=create_term(op2,-1);
			}

		}
		else // both the operands are either variables or functions
		{
	 		 S=(NC*)malloc(sizeof(NC));
	                 S->type='S';
		         S->list=NULL;
		 	 S->inc=0;
                         v1=indexof_symtab(op1);
	                 v2=indexof_symtab(op2);
			 if(v2 > v1)
			 {
				S->link=create_term(op1,1);
				if(S->link!=NULL)
					S->link->list=create_term(op2,-1);
                         }
			else
			{
			        S->link=create_term(op2,-1);
				if(S->link!=NULL)
					 S->link->list=create_term(op1,1);
			}
		}
		R->link=S;
	}

    // write_lists(root);
    return root;
}




NC*
parsecondition(char *condition,NC *root) {

   int i=0,flag=0,j;
   char clause[30];
   root=NULL;
   if(!strcmp(condition,"-")) return NULL;
   while(condition[i]!='\0')
   {
		clause[0]='\0';
		if(condition[i]=='-')
		{
			root=NULL;
			return (NC*)NULL;
		}
		j=0;
		flag=0;
	  	if(condition[i]=='!')
		{
			i++;
			flag=1;
		}

		while(condition[i]!='\0'&&condition[i]!='&'&&condition[i]!='|')
			  clause[j++]=condition[i++];

		if(condition[i]!='\0')
		{

			i++;
			if(condition[i]=='&'||condition[i]=='|')
		  	{
				clause[j]='\0';
				j=0;
				if(condition[i]=='&')
					root=parseclause(clause,1,flag,root); //flag=1 for !cond. and =0 for cond.
				else
					root=parseclause(clause,0,flag,root);
				i++;
			 	continue;
			}
			else
	        	{
			    clause[j++]=condition[i-1];
			    while(condition[i]!='\0'&&(condition[i]!='&'||condition[i]!='|'))
			    	clause[j++]=condition[i++];
			    if(condition[i]!='\0')
		            {
					    i++;
						if(condition[i]=='&'||condition[i]=='|')
		  				{
							clause[j]='\0';
							j=0;
							if(condition[i]=='&')
						        root=parseclause(clause,1,flag,root);
							else
								root=parseclause(clause,0,flag,root);
							i++;
						}
						continue;
		             }
			    else
			    {
				    clause[j]='\0';
				    j=0;
				    root=parseclause(clause,-1,flag,root);
			    }
			}
		}
		else
		{
		    clause[j]='\0';
			j=0;
			root=parseclause(clause,-1,flag,root);
		}
   }
   return root;
}



void
parseassignments( char *s, DATA_TRANS *actions, var_list *V ) {

   int i=0,j=0,k=0;
   char A[200];
   A[0]='\0';
	while(s[i]!='\0')
	{
		if(s[i]==',')
		{
			A[j]='\0';
			j=0;
		    //	printf("%s\n",A);
			parseA( A, &actions[k], V );
			k++;
		}
		else
		{   A[j]=s[i];
			j++;
		}
		i++;
	}
    A[j]='\0';
	parseA( A, &actions[k], V );
	k++;
	actions[k].rhs=NULL;
	//printf("%s\n",A);
}


/***********************************************************************************************/

/*   This function is used to parse the given three adress code statements                     */


/***********************************************************************************************/


void
parseA( char *A, DATA_TRANS *D, var_list *V ) {

	char op1[20],op2[20],op3[20],op;
	int i=0,j=0,k=0,m;
	op1[0]='\0';op2[0]='\0';op3[0]='\0';
	op='\0';
	while(A[i]!='\0'&&A[i]!='=')
	{
		op1[j]=A[i];
		j++;
		i++;
	}
	op1[j]='\0';
	D->lhs=indexof_symtab(op1);
	m = indexof_varlist( op1, V );

	if(!strcmp(op1,"-")) return;
	i++;
	if(A[i]!='\0'&&A[i]=='-')
	{
	//	if(A[i+1]!='0'&&A[i+1]>='0'&&A[i+1]<='9')
		op2[k++]=A[i++];
	}
	while(A[i]!='+'&&A[i]!='-'&&A[i]!='*'&&A[i]!='/'&&A[i]!='%'&&A[i]!='\0')
		op2[k++]=A[i++];
	op2[k]='\0';
	if(A[i]!='\0')
	{
		op=A[i];
		j=0;i++;
		while(A[i]!='\0')
		{
			op3[j]=A[i];
			j++;i++;
		}
		op3[j]='\0';
	}
	else
	{
    	op='\0';
	    op3[0]='\0';
	}
    //printf("\n\n  %s   :=    %s   %c    %s  \n",op1,op2,op,op3);
    m = indexof_varlist( op2, V );
	m = indexof_varlist( op3, V );
	D->rhs=create_expr(op2,op,op3);
	//printf("%d  <=   ",D->lhs);
	//write_lists(D->rhs);
}

/**********************************************************************************************/

/*     This function is used to create a copy of the list structure                           */


/**********************************************************************************************/


NC*
copylist(NC *source)
{
   NC *temp;
   if(source != NULL)
   {
   		temp=(NC *)malloc(sizeof(NC));
		/*if(temp==NULL)
    		{
	    		printf(" Memory allocation error\n");
			exit(0);
	    	}*/
   		temp->type = source->type;
   		temp->inc = source->inc;
		temp->link = (NC *)malloc(sizeof(NC));
   		temp->link = copylist(source->link);
		temp->list = (NC *)malloc(sizeof(NC));
   		temp->list = copylist(source->list);
        return temp;
   }
   return NULL;
}


/**********************************************************************************************/

/*   This function is used to print the normalized expressions                                */
/*   It will take the root of the binary tree as input and print the corresponding expression */

/**********************************************************************************************/
void
write_lists( NC *root ) //WRITE_LISTS <- for finding fast
{
  char *sym_value;
  sym_value = (char * ) malloc( 1000*sizeof( char ) );

  if( root != NULL )
  {
	//KB start
	if(root->type == 0) root->type = 'S';
	//KB end
    if( root->type == 'R' || root->type == 'O' )
    {
       if( root->type == 'R' )
          printf( " ( " );
       write_lists( root->link );
    }

    switch( root->type )
    {
      case 'f':
        symbol_for_index( root->inc, sym_value );
        printf( "* %s( ", sym_value );
        break;
      case 'v':
        symbol_for_index( root->inc, sym_value );
        printf( "* %s ", sym_value );
        break;
      case 'T':
        printf( "%c %d ", ( root->inc >= 0 )?'+':'-',
                abs( root->inc ) );
        break;
      case 'S':
        printf( "%d ", root->inc );
        break;
      case 'R':
        switch( root->inc )
          {
          case 0: printf( ">= 0" );
            break;
          case 1: printf( "> 0" );
            break;
          case 2: printf( "<= 0" );
            break;
          case 3: printf( "< 0" );
            break;
          case 4: printf( "== 0" );
            break;
          case 5: printf( "!= 0" );
            break;
          }; // switch( root->inc )
        printf( " ) " );
        if( root->list != NULL )
          printf( " OR " );
        break;
      case 'A':
        break;
      case 'O':
        if( root->list != NULL )
          printf( " AND " );
        break;
      case 'D':
        printf( " * ( /   " );
        break;
      case 'M':
        printf( " * (%%   " ); //printf( "* ( %c %d   ",  root->type, root->inc );
        break;
      //KB: array start
      case 'w':
		printf( "write ( " );
		break;
	  case 'y':
	    symbol_for_index(root->inc, sym_value);
	    printf( "%s, ", sym_value );
	    break;
	  case 'a':
	    symbol_for_index(root->inc, sym_value);
	    printf( "* read ( %s, ", sym_value );
	    break;
      //KB: array end
      default:
        break;
    }; // switch( root->type )

    if( root->type != 'R' && root->type != 'O' )
      write_lists( root->link );
    if( root->type == 'f' || root->type == 'w') //2nd clause is for arrays
      printf( ")" );

    //KB: array start
    if(root->type == 'y')
      printf(", ");
    //A closing bracket has to be put explicitly in case the arrays
    //are of only one dimension
    if(root->type == 'a' && root->link->list == NULL)
      printf( ")" );
    //KB: array end

    if( root->type == 'S' && root->list != NULL )
    {
      printf( ", " );
      write_lists( root->list );
      printf(")");
      return;
    }

    write_lists( root->list );
  }
  return;
}





/**********************************************************************************************/

/* This function is used to create the exression tree in normalized form                     */
/* It takes the three operands and the operator of the three address code as input           */

/**********************************************************************************************/


NC*
create_expr( char *op2, char op, char *op3 ) {
   int c1,c2,v1,v2, i, j;
   NC *S,*T,*P,*S1,*S2;
   char op2_temp[100];
   c1=isconstant(op2); // c1 = 1 indicates op2 is constant; = 0 otherwise
   c2=isconstant(op3);
   if(op3[0]=='\0')   // handling unary operators
   {
	S=(NC*) malloc(sizeof(NC));
        S->list=NULL;
        S->type='S';
	if(c1)
	{
	   S->link=NULL;
	   S->inc=constval(op2);
	}
        else    // handling simple assignments
  	{
	  if(op2[0] == '-')
	  {
		i=1;
		j=0;
		while(op2[i] != '\0')
		{
			op2_temp[j++]=op2[i++];
		}
		op2_temp[j]='\0';
	  	S->link=create_term(op2_temp,-1);
	  }
	  else
		S->link=create_term(op2,1);
	  S->inc=0;
	}
   }
   else if(c1&&c2)  { // handling constants in which both operands are constants
	   int val;
	   switch(op) {
		   case '+':
			   val=constval(op2)+constval(op3);
			   break;
		   case '-':
			   val=constval(op2)-constval(op3);
		           break;
		   case '*':
			   val=constval(op2)*constval(op3);
			   break;
		   case '/':
			   val=constval(op2)/constval(op3);
			   break;
		   case '%':
			   val=constval(op2)%constval(op3);
			   break;
	   }
	   S=(NC *)malloc(sizeof(NC));
	   S->list=NULL;
	   S->type='S';
	   S->link=NULL;
	   S->inc=val;
   }
   else if( c1 || c2 )  { // handling single constant expressions
	  S=(NC *)malloc(sizeof(NC));
	  S->list=NULL;
	  S->type='S';
	  switch( op ) {
		 case '+':
			    if(c2) {
				     S->inc=constval(op3);
				     S->link=create_term(op2,1);
			    }
			    else {
				    S->inc=constval(op2);
				    S->link=create_term(op3,1);
			    }
			    break;
	         case '-':

			    if(c2) {
				    S->inc=-1*constval(op3);
				    S->link=create_term(op2,1);
                	    }
			    else {
				    S->inc=constval(op2);
				    S->link=create_term(op3,-1);
			    }
			   break;
		 case '*':
			   if(c2)
			        S->link=create_term(op2,constval(op3));
			   else
			    	S->link=create_term(op3,constval(op2));
			   S->inc=0;
			   break;
		 case '/':
		 case '%':
			  T=(NC*)malloc(sizeof(NC));
			  P=(NC*)malloc(sizeof(NC));
			  T->type='T';
			  S->link=T;
			  T->link=P;
			  T->list=NULL;
			  P->list=NULL;
			  S->inc = 0;
			  if(op=='/')
			      P->type='D';
			  else
			      P->type='M';

			  S1=(NC*)malloc(sizeof(NC));
			  S2=(NC*)malloc(sizeof(NC));
			  S2->type='S';
			  S1->type='S';
			  S1->list=S2;
			  S2->list=NULL;
			  P->link=S1;
			  P->inc = 0;
			  T->inc = 1;
			  if(c2) {
				  S2->inc=constval(op3);
				  S2->link=NULL;
				  S1->link=create_term(op2,1);
				  S1->inc=0;
			  }
			  else {
				  S1->inc=constval(op2);
				  S1->link=NULL;
				  S2->link=create_term(op3,1);
				  S2->inc=0;
		          }
			  break;
	  }
   }
   else {  // handling expressions involving both variables
          S=(NC*)malloc(sizeof(NC));
	  S->type='S';
	  S->list=NULL;
	  S->inc=0;
          v1=indexof_symtab(op2);
	  v2=indexof_symtab(op3);
	  switch(op)
	  {
		  case '+':
		  case '-':
			if(v2 > v1)
			{
			    S->link=create_term(op2,1);
			    if(S->link!=NULL)
			    {
			           if(op=='-')
				        S->link->list=create_term(op3,-1);
			           else
				    	S->link->list=create_term(op3,1);
			    }
			}
			else
			{
				if(op=='-')
				        S->link=create_term(op3,-1);
				else
					S->link=create_term(op3,1);
			        if(S->link!=NULL)
				      S->link->list=create_term(op2,1);
			}
			break;
		  case 	'*':

			S1=(NC*)malloc(sizeof(NC));
			S2=(NC*)malloc(sizeof(NC));
			S1->link=NULL;
			S2->link=NULL;
			S1->type='v';
			S2->type='v';
			if(v2>v1)
			{
				S1->inc=v1;
				S2->inc=v2;
			}
			else
			{
				S1->inc=v2;
				S2->inc=v1;
			}
			S1->list=S2;
			S2->list=NULL;
			T=(NC *)malloc(sizeof(NC));
			T->list=NULL;
			T->type='T';
			T->inc=1;
			T->link=S1;
			S->link=T;
			break;
		  case  '/':
		  case  '%':
		    S1=(NC *)malloc(sizeof(NC));
			S2=(NC*)malloc(sizeof(NC));
			S1->list=S2;
			S2->list=NULL;
			S1->type='S';
			S2->type='S';
			S1->inc=0;
			S2->inc=0;
			S1->link=create_term(op2,1);
			S2->link=create_term(op3,1);
			T=(NC*)malloc(sizeof(NC));
			P=(NC *)malloc(sizeof(NC));
			P->list=NULL;
			T->list=NULL;
			T->type='T';
			if(op=='/')
				P->type='D';
			else
				P->type='M';
			T->inc=1;
			P->inc=0;
			T->link=P;
			P->link=S1;
			S->link=T;
			break;
	  }
   }
   return S;
}

/******************************************************************************************/

/*  This function is used to create a term for a particular normalized term               */
/*  It takes the symbolic constant and the integer value as input and returns the pointer */
/*  to the normalized binary tree                                                         */

/******************************************************************************************/


NC*
create_term(char *op,int val) {
	NC *T,*P;
	int i=indexof_symtab(op);  //get the index of the symbol
	if( i >= 0 )
	{
		T=(NC *)malloc(sizeof(NC));
		P=(NC*) malloc(sizeof(NC));
		T->link=P;
		P->list=NULL;
		T->list=NULL;
		P->link=NULL;
		P->inc=i;
		P->type='v';
		T->type='T';
		T->inc=val;
		return T;
	}
	else
		return NULL;
}

/****************************************************************************************/

/*    This function is used to find the integer value represented by a string           */

/****************************************************************************************/



int
constval( char *s ) {
  int val = 0, i;

  if( s[0] != '-' ){
    for( i = 0; s[i] != '\0'; i++ )
      val = val * 10 + s[i] - 48;
  }       // calculating constant value
  else {
    for( i = 1; s[i] != '\0'; i++ )
      val = val * 10 + s[i] - 48;
    val = -1 * val;
  }


  return val;
}




/*****************************************************************************************/

/* This function is used to check whether the given string is an integer constant or not */
/* It returns 1 if it is a constant otherwise it will return 0                           */

/*****************************************************************************************/

int isconstant( char *s ) {
   int i = 0;
   if( s[0] == '\0' )
	   return 0;
   if( s[0] == '-' || s[0] == '+' )  {  //check for the sign of the constant
	   i=1;
	   while(s[i]!='\0') {
		   if(s[i]>='0'&&s[i]<='9')
		      i++;
		   else
		      return 0;
	   }
	   if(i==strlen(s))
	      return 1;
   }
   else  { // this handles unsigned numbers

   	  while(s[i]!='\0') {
		  if(s[i]>='0'&&s[i]<='9')
		     i++;
		  else
		     return 0;
   	  }
   	 if(i==strlen(s))
	    return 1;
   }
}

/*********************************************************************************/

/*   This function is used to find the index value of a symbol in a symbol table */
/*   If the symbol is already in the table then it will return the index  value  */
/*   otherwise it will add the symbol into the symbol table                      */
/*   if the string is null then it will return -1                                */

/*********************************************************************************/

int
indexof_symtab( char *symbol )
{
   int i;
   if( strcmp( symbol, "\0" ) )  // if the symbol is null string then return -1
     {


       for( i = 0; i < stab.numsymbols; i++ )
	 if( !strcmp( stab.sym_tab[i], symbol ) ){


	    return stab.val_tab[i];
         }

   i = stab.numsymbols++; // symbol not found in the symbol table
   stab.sym_tab[i] = ( char * ) malloc( strlen( symbol ) +1 );
   strcpy( stab.sym_tab[i], symbol );//copy the symbol into the symbol table
   stab.val_tab[i] = constval( symbol );
   return stab.val_tab[i];
     }
   return -1;
}


 /*   This function is used to find the index value of a symbol in a var_list.
  *   var_list is the list of variable which are present in the FSMD's.
  *   If the symbol is already in the var_list then it will return the index  value
  *   otherwise it will add the symbol into the var_list
  *   if the string is null then it will return -1
  */
int
indexof_varlist( char *symbol, var_list *V ) {
   int i;
   if( strcmp( symbol, "\0" ) )  { // if the symbol is null string then return -1


       for( i = 0; i < V->no_of_elements; i++ )
	 if( !strcmp( V->var_string[i], symbol ) ) {
 	    return 1;
         }

     i = V->no_of_elements++; // symbol not found in the symbol table
     V->var_string[i] = ( char * ) malloc( strlen( symbol ) +1 );
     strcpy( V->var_string[i], symbol );//copy the symbol into the symbol table
     V->var_val[i] = constval( symbol );
     return 2;
    }
   return -1;
}



/*  This function returns the string value of an index.
 *  This index is the integer value of a variable present in the FSMD.
 */

void
symbol_for_index( int n , char *sym_value ){
  int i;
  for( i = 0; i < stab.numsymbols; i++ )
    if( stab.val_tab[i] == n ){
      strcpy( sym_value, stab.sym_tab[i] );
    }
}




/*****************************************************************************/

/*          This function is used to create a new state
 *          if that is not created earlier It takes the state Id as
 *          input and returns a pointer to the state                         */

/*****************************************************************************/

int
create_state(char* stateid,FSMD *M,int current_state)  {
    int i;
	for(i=0;i<current_state;i++)
		if(!strcmp(stateid,M->states[i].state_id)) return i;
	return current_state;

}


/*****************************************************************************/

/*       This function will print the FSMD on the screen                     */

/*****************************************************************************/

int
print_fsmd(FSMD  *M)
{
    char *sym_value;
	TRANSITION_ST *temp;
	int i, j, k, l;

	sym_value = (char * ) malloc( 100*sizeof( char ) );

	printf("\n\n  %s FSMD states and its transitions are displayed below  \n\n", M->name);

	if(M!=NULL) {
	  for(i=0;i<M->numstates;i++) {
	  // This loop will display all states informations of the FSMD

	   printf("\n\n State name is  %s \n\n", M->states[i].state_id);

	   //value propagation code start

	   //printf("Node id: %d\t Node type: %d\t Number of transitions: %d\n", i, M->states[i].node_type, M->states[i].numtrans);

	   printf("Status of VAPFLAG: ");
	   if(M->states[i].VAPFLAG == FALSE)
		 printf("False\n");
	   else
	     printf("True\n");

	   if(M->states[i].propVector.cond != (NC*)NULL)
	   {
	     printf("Condition: \n");
	     write_lists(M->states[i].propVector.cond);
	   }
	   for( k=0; k < SYMTABSIZE; k++ )
	   {
		 //The second disjunct is added to handle arrays; for arrays, value[k] may be NULL but the subVector may be non-empty
	     if(M->states[i].propVector.value[k] != (NC*)NULL || M->states[i].propVector.subVector[k].countValues > 0)
	     {
	       symbol_for_index(k, sym_value);
	       printf("Propagated Vector for variable '%s' with index %d :\n", sym_value, k);
	       write_lists(M->states[i].propVector.value[k]);

	       //array start
	       for(l = 0; l < M->states[i].propVector.subVector[k].countValues; l++)
	       {
			   if(M->states[i].propVector.subVector[k].arrayValue[l] != (NC*)NULL)
			   {
				  write_lists( M->states[i].propVector.subVector[k].arrayValue[l] );
				  printf("\n");
			   }
		   }
	       //array end
	     }
	   }
	   //value propagation code end

	   temp=M->states[i].translist;
	   while(temp!=NULL) {
	          //this loop will display all the transitions from a particular state
              printf("\n   %s   to   -->%s\n",M->states[i].state_id,M->states[temp->outtrans].state_id);
              j=0;
	      write_lists( temp->condition );

				while( temp->action[j].rhs != NULL ) {
				     	symbol_for_index( temp->action[j].lhs, sym_value );
					printf("\n%s  :=  ", sym_value );

					write_lists( temp->action[j].rhs );
					j++;
				}
				temp=temp->next;
                printf("\n");
			}

		}
    }
	return 0;
}


/*****************************************************************************/

/*       This function will push the given state on to a stack               */

/*****************************************************************************/


void
stack_push( STACK *S,int state,NC *cond, DATA_TRANS *action) {

	if(S!=NULL) {
		if(S->top>=STACKSIZE) { // checking for stack over flow
			printf("\nStack over flow \n ");
			exit(0);
		}
		else {
			S->state[++S->top]=state;
                        S->condition[S->top]=cond;
                        S->actions[S->top]=action;
		}
	}

}

/*****************************************************************************/

/*        This function will pop the top state from the stack                */

/*****************************************************************************/

void
stack_pop(STACK *S) {
   if(S!=NULL&&S->top!=-1) { // checking for stack underflow
	   S->top--;
   }
   return;
}

/*****************************************************************************/

/*         This is a recursive function which is used
 *         to find all possible paths from a state  */

/*****************************************************************************/

void
dividepaths(FSMD *fsmd,int state,PATHS_LIST *paths,STACK *S) {

	TRANSITION_ST *temp;
	if(cutpoint(fsmd->states[state]))      /* if it is a cutpoint then path was ended  */
	{
	    //	printpath(fsmd,S);
		storepath(fsmd,paths,S);
		stack_pop(S);
		return;
	}
	else
	{
	    temp=fsmd->states[state].translist;
	    while(temp!=NULL)
		{
			stack_push(S,temp->outtrans,temp->condition,&temp->action[0]);
			dividepaths(fsmd,temp->outtrans,paths,S);
			temp=temp->next;
		}
	    stack_pop(S);
	    return;
	}
}

/*****************************************************************************/

/*        This function will find all the path segments in a FSMD            */

/*****************************************************************************/

void
findpaths(FSMD *fsmd,PATHS_LIST *paths) {
	TRANSITION_ST *trans;
	STACK *S;
	int i=0;
	paths->numpaths=0;

	//value propagation code start
	paths->offset_null_path = 0;

	//value propagation code end

	paths->fsmd=fsmd;
	S=(STACK *) malloc (sizeof(STACK));
	S->top=-1;
	for(i=0;i<fsmd->numstates;i++)
	{
		if(cutpoint(fsmd->states[i]))
		{
			stack_push(S,i,NULL,NULL);
			trans=fsmd->states[i].translist;
			while(trans!=NULL)
			{
				stack_push(S,trans->outtrans,trans->condition,trans->action);
				dividepaths(fsmd,trans->outtrans,paths,S);
				trans=trans->next;
			}
			stack_pop(S);
		}
	}

	for(i=0; i<paths->numpaths; i++) //initially all path are extendible
	{
		paths->paths[i].extendible=1;
	}
}

/*****************************************************************************/

/*    This function is used to display a particular path in the FSMD         */

/*****************************************************************************/

void
printpath(FSMD *fsmd,STACK *S) {
	int i;
	for(i=0;i<=S->top;i++) {
		printf(" %s --> ",fsmd->states[S->state[i]].state_id);
	}
	printf(" \n");
}

/*****************************************************************************/

/*      This function will find whether a state is a cutpoint or not         */

/*****************************************************************************/

int
cutpoint( NODE_ST state ) {
	if( state.node_type == 0 || state.node_type == 1 || state.numtrans > 1 )
	//if( state.node_type == 0 || state.numtrans > 1 )
		return 1;
	else
		return 0;
}

/*****************************************************************************/

/*      This function will store all the path segments in the memory         */

/*****************************************************************************/

void
storepath(FSMD *fsmd,PATHS_LIST *paths,STACK *S) {
	PATH_NODE *t,*end;
	int i;
	paths->paths[paths->numpaths].start=S->state[0];
	paths->paths[paths->numpaths].end=S->state[S->top];
	end=NULL;
	paths->paths[paths->numpaths].status=0;
	//paths->paths[paths->numpaths].extendible=1;
	paths->paths[paths->numpaths].condition=NULL;
	paths->paths[paths->numpaths].transformations=NULL;
	paths->paths[paths->numpaths].nodeslist=NULL;
	t=(PATH_NODE *) malloc (sizeof(PATH_NODE)*(S->top+1));

    if( t == NULL ) {
	    printf(" Memory allocation error\n");
		exit(0);
    }
/*  +++
	if(paths->numpaths == 0)
		paths->paths[paths->numpaths].next = NULL;
	else
		paths->paths[paths->numpaths].next = &paths->paths[(paths->numpaths - 1)];
*/
    for(i=0;i<=S->top;i++)
    {
    	t[i].state=S->state[i];
        t[i].condition=S->condition[i];
        t[i].actions=S->actions[i];
        t[i].next=NULL;
        t[i].left=end;
		if(end==NULL)
		    paths->paths[paths->numpaths].nodeslist=&t[i];
		else
		    end->next=&t[i];
        end=&t[i];
    }
    paths->numpaths++;
}

/*****************************************************************************/

/*      This function dispalys all possible paths of an FSMD                 */

/*****************************************************************************/

void
displayallpaths(PATHS_LIST *pl) {

    char *sym_value;
    sym_value = (char * ) malloc( 1000*sizeof( char ) );

	PATH_NODE *t;
	int i;
	r_alpha *temp1;

	printf( "\n\n Displaying all the paths of the path list \n" );

	// start from the header of the paths list
    for(i=0;i<pl->numpaths;i++)
	{
		  printf("\n  %d  ",i);
		  t=pl->paths[i].nodeslist;
		  while(t->next!=NULL) // this prints each path in the list
	      {
	           	printf("%d %s --> ",t->state,pl->fsmd->states[t->state].state_id);
                t=t->next;
		  }
		  printf("%d %s \n\n",t->state,pl->fsmd->states[t->state].state_id);
          write_lists(pl->paths[i].condition);
		  printf("\n");
		  temp1=pl->paths[i].transformations;
		  while(temp1!=NULL)
		  {
		      symbol_for_index( temp1->Assign.lhs, sym_value );
			  printf("\n %s := ", sym_value );

			  write_lists(temp1->Assign.rhs);
			  temp1=temp1->next;
		  }
		  printf("\n\n");
		  //printf("\n Extend status of path %d: %d \n", i, pl->paths[i].extendible);
	}
}


void
displayallpaths_withcp(PATHS_LIST *pl,FSMD *fsmd) {

        char *sym_value;
        sym_value = (char * ) malloc( 1000*sizeof( char ) );

	PATH_NODE *t;
        int i;
		r_alpha *temp1;
		/*  start from the header of the paths list  */
		for(i=0;i<pl->numpaths;i++) {
			printf("\n");
			t=pl->paths[i].nodeslist;
			while( t->next != NULL ) { /* this prints each path in the list    */
				printf("%d %s --> ",t->state,pl->fsmd->states[t->state].state_id);
				//    write_lists(t->condition);
				t=t->next;
			}
			printf("%d %s \n\n",t->state,pl->fsmd->states[t->state].state_id);
			t=pl->paths[i].cp->nodeslist;
			while(t->next!=NULL) {
				 printf("%d %s --> ",t->state,fsmd->states[t->state].state_id);
				 t=t->next;
		    }
		    printf("%d %s \n\n",t->state,fsmd->states[t->state].state_id);
		    write_lists(pl->paths[i].condition);
		    printf("\n\n");
		    temp1=pl->paths[i].transformations;

		    while(temp1!=NULL) {

		      symbol_for_index( temp1->Assign.lhs, sym_value );
			  printf("\n %s := ", sym_value );
			  write_lists(temp1->Assign.rhs);
			  temp1=temp1->next;

		    }
      }
}


void //DISPLAY_A_PATH <- for finding fast
displayApath(PATH_ST *p) {

    char *sym_value;
	r_alpha *temp1;

    sym_value = (char * ) malloc( 1000*sizeof( char ) );

	write_lists(p->condition);
	temp1=p->transformations;
	while(temp1!=NULL) {

	    symbol_for_index( temp1->Assign.lhs, sym_value );
		printf("\n %s := ", sym_value );
		write_lists(temp1->Assign.rhs);
		temp1=temp1->next;
	}
}



void
Associate_R_and_r_alpha(PATHS_LIST *pl)
{
   int i;
   // start from the header of the paths list
   for(i=0;i<pl->numpaths;i++)
      find_Ralpha(&pl->paths[i]);
   return;
}

/*
//Removes the primary p (if present) from the term t
//and returns an integer
//Eg: Inputs: t = x*y*y,   p = z   Outputs: t = x*y*y,  FALSE
//Eg: Inputs: t = x*y*y*z, p = y   Outputs: t = x*z,    1 - present
//Eg: Inputs: t = y*y*y,   p = y   Outputs: t = NULL,   2 - present,
//                               and the term t has reduced to NULL
N_term* reconstructTerm( N_term *t, N_primary *p, boolean *retValue )
{
	//int retValue;
	N_term *new_t, *traverse;

	if(t == NULL)
	{
		*retValue = 2;
		return NULL;
	}

	if(p == NULL)
	{
		*retValue = 0;
		return t;
	}

	printf("\nIn Reconstruct\n");
	write_lists(t);
	printf("\n");
	write_lists(p);
	printf("\nOut Reconstruct12\n");

	*retValue = 0;

	//The following loop finds the Head of t
	while(t != NULL && t->inc == p->inc)
	{
		*retValue = 1;
		t = t->list;
	}
	if(t == NULL)
	{
		printf("return NULL\n");
		*retValue = 2;
		return NULL;
	}

	new_t = (N_term*) malloc (sizeof(N_term));
	new_t = copylist( t );
	traverse = new_t;
	while(traverse->list != NULL)
	{
		if(traverse->list->inc == p->inc)
		{
			*retValue = 1;
			traverse->list = traverse->list->list;
		}
		traverse = traverse->list;
		if(traverse == NULL)
			break;
	}

	printf("\n");
	write_lists(new_t);
	printf("\nOut Reconstruct\n");

	return new_t;
}
*/


void
find_Ralpha( PATH_ST *path )
{
	int i, flag, flag_1, flag_2, break_flag = 0;
	PATH_NODE *pn, *next_pn;
	r_alpha *p_trans, *p_trans1, *p_trans2, *p_trans3, *cond_trans, *trans;
	NC *p, *p1, *sum, *c1, *c3, *condition, *s , *s1, *s2, *s3, *s4, *M_z, *result;

	//KB array start
	r_alpha *trav_trans, *new_assignment;
	NC *old, *new, *traverseList, *M_t, *old_s;
	boolean flagLink, flagNewAssgn;
	//KB array end

	//the following assignment is required for LVA (without OUTPUT_ONLY)
	path->finalPath = FALSE;

	pn = path->nodeslist;
	if( pn->next != NULL )
		next_pn = pn->next;
	else
		return;

	while( next_pn != NULL )
	{
		// conditions
		//NB: 'w' cannot occur in condition (below 'R'), only 'S' can
		//'a' can occur at the same level as 'v' or 'D'/'M'
		if( path->condition == NULL )
		{
			if( next_pn->condition != NULL )
			{
				c1 = copylist( next_pn->condition );
				cond_trans = path->transformations;
				condition = c1;

				if( condition->link != NULL )
				{
					condition = condition->link; //condition is now at 'O' (OR) level

					while( condition != NULL )
					{
						if( condition->link != NULL )
						{
							if( condition->link->link != NULL )
							{
								p_trans1 = path->transformations;
								s = ( NC * )malloc( sizeof( NC ) );
								s = copylist( condition->link->link );
								s->type = 'S';

								s1 = ( NC * )malloc( sizeof( NC ) );
								s1 = copylist( condition->link->link );
								s1->link = NULL;
								s1->list = NULL;

								if( s->link != NULL )
								{
									s = s->link;
									//s is now at 'T'
									while( s != NULL )
									{
										s2 = ( NC * )malloc( sizeof( NC ) );
										s2 = copylist( s );
										s2->list = NULL;

										if( s2->link != NULL )
										{
											//s3 can be at 'v', 'D'/'M', 'a' level
											s3 = ( NC * )malloc( sizeof( NC ) );
											s3 = copylist( s2->link );

											M_z = ( NC * )malloc( sizeof( NC ) );
											M_z->type = 'S';
											M_z->inc = 1;
											M_z->link = NULL;
											M_z->list = NULL;

											while( s3!= NULL )
											{
												if( s3->type == 'v' )
												{
													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( s3 );
													s4->link->list = NULL;
													s4->link->link->list = NULL;

													p_trans = path->transformations;
													while( p_trans != NULL )
													{
														p = ( NC * )malloc( sizeof( NC ) );
														p->type = 'v';
														p->inc = p_trans->Assign.lhs;
														p->link = NULL;
														p->list = NULL;

														sum =  ( NC * )malloc( sizeof( NC ) );
														sum->type = 'S';
														sum = copylist( p_trans->Assign.rhs );

														break_flag = 0;
														flag_1 = 0;
														flag_2 = 0;

														s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

														if( break_flag == 1 )
														{
															break;
														}
														p_trans = p_trans->next;
													}
												} // if( s3->type == 'v' )

												//KB array start
												else if( s3->type == 'a' )
												{
													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1; //s2->inc;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( s3 );
													s4->link->list = NULL;
													s4->link->link->list = NULL;

													p_trans = path->transformations;
													while( p_trans != NULL )
													{
														if( p_trans->Assign.lhs != s3->inc ||
															(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' &&
																compare_trees(s3->link, p_trans->Assign.rhs->link->link)==0) )
														{
															p_trans = p_trans->next;
															continue;
														}

														p = ( NC * )malloc( sizeof( NC ) );
														p = copylist( s3 );

														p1 = ( NC * )malloc( sizeof( NC ) );
														p1->inc = next_pn->actions[i].lhs;
														p1->link = NULL;
														p1->list = NULL;

														if(next_pn->actions[i].rhs != NULL && next_pn->actions[i].rhs->type == 'w')
															p1->type = 'w';
														else
															p1->type = 'v';


														sum =  ( NC * )malloc( sizeof( NC ) );
														sum->type = 'S';

														if(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' )
														{
															sum = copylist( p_trans->Assign.rhs->link->list );
														}
														else
														{
															sum = copylist( p_trans->Assign.rhs );
														}

														break_flag = 0;
														flag_1 = 0;
														flag_2 = 0;

														/*
														printf( "\n ##### \nbefore Substitute_In_Sum : \n ");
														write_lists( p1 );
														printf( " = ");
														write_lists( s4 );
														printf("\n wrt \n");
														write_lists(p);
														printf( " = ");
														write_lists( sum );
														printf("\n"); */

														s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

														/*
														printf( "\n after Substitute_In_Sum : s4 : ");
														write_lists( p1 );
														printf( " = ");
														write_lists( s4 );
														printf("\n ####\n"); */

														if( break_flag == 1 )
														{
															break;
														}

														p_trans = p_trans->next;
													} //end while
												}
												//KB array end

												else
												{
													trans = path->transformations;
													result = (NC *)malloc(sizeof(NC));
													result = Substitute_in_MOD_DIV( s3, trans, result );
													//KB array - Substitute_in_MOD_DIV has been modified to handle arrays

													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1; //s2->inc;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( result );
													s4->link->list = NULL;
												}

												M_z = Mult_Sum_With_Sum( M_z, s4, M_z );
												s3 = s3->list;

											} // while( s3!= NULL )

											M_t = ( NC * )malloc( sizeof( NC ) );
											M_t->type = 'S';
											M_t->inc = s2->inc;
											M_t->link = NULL;
											M_t->list = NULL;

											M_z = Mult_Sum_With_Sum( M_z, M_t, M_z );

											s1 = Add_Sums( s1, M_z, s1 );
										} // if( s2->link != NULL )

										s = s->list;
									} // while( s != NULL )

									condition->link->link = copylist( s1 );
								} // if( s->link != NULL )
							} //if( condition->link->link != NULL )
						} // if( condition->link != NULL )

						condition = condition->list;
					} // while( condition != NULL )
				} // if( condition->link != NULL )

				path->condition = copylist( c1 );
			}
		} // if( path->condition == NULL )
		else
		{
			if( next_pn->condition != NULL )
			{
				c1 = copylist( next_pn->condition );
				cond_trans = path->transformations;
				condition = c1;

				if( condition->link != NULL )
				{
					condition = condition->link;

					while( condition != NULL )
					{
						if( condition->link != NULL )
						{
							if( condition->link->link != NULL )
							{
								p_trans1 = path->transformations;
								s = ( NC * )malloc( sizeof( NC ) );
								s = copylist( condition->link->link );
								s->type = 'S';

								s1 = ( NC * )malloc( sizeof( NC ) );
								s1 = copylist( condition->link->link );

								s1->link = NULL;
								s1->list = NULL;

								if( s->link != NULL )
								{
									s = s->link;
									while( s != NULL )
									{
										s2 = ( NC * )malloc( sizeof( NC ) );
										s2 = copylist( s );
										s2->list = NULL;

										if( s2->link != NULL )
										{
											s3 = ( NC * )malloc( sizeof( NC ) );
											s3 = copylist( s2->link );

											M_z = ( NC * )malloc( sizeof( NC ) );
											M_z->type = 'S';
											M_z->inc = 1;
											M_z->link = NULL;
											M_z->list = NULL;

											while( s3!= NULL )
											{
												if( s3->type == 'v' )
												{
													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( s3 );
													s4->link->list = NULL;
													s4->link->link->list = NULL;

													p_trans = path->transformations;

													while( p_trans != NULL )
													{
														p = ( NC * )malloc( sizeof( NC ) );
														p->type = 'v';
														p->inc = p_trans->Assign.lhs;
														p->link = NULL;
														p->list = NULL;

														sum =  ( NC * )malloc( sizeof( NC ) );
														sum->type = 'S';
														sum = copylist( p_trans->Assign.rhs );

														break_flag = 0;
														flag_1 = 0;
														flag_2 = 0;

														s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

														if( break_flag == 1 )
														{
															break;
														}

														p_trans = p_trans->next;
													}
												} // if( s3->type == 'v' )

												//KB array start
												else if( s3->type == 'a' )
												{
													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1; //s2->inc;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( s3 );
													s4->link->list = NULL;
													s4->link->link->list = NULL;

													p_trans = path->transformations;
													while( p_trans != NULL )
													{
														if( p_trans->Assign.lhs != s3->inc ||
															(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' &&
																compare_trees(s3->link, p_trans->Assign.rhs->link->link)==0) )
														{
															p_trans = p_trans->next;
															continue;
														}

														p = ( NC * )malloc( sizeof( NC ) );
														p = copylist( s3 );

														p1 = ( NC * )malloc( sizeof( NC ) );
														p1->inc = next_pn->actions[i].lhs;
														p1->link = NULL;
														p1->list = NULL;

														if(next_pn->actions[i].rhs != NULL && next_pn->actions[i].rhs->type == 'w')
															p1->type = 'w';
														else
															p1->type = 'v';

														sum =  ( NC * )malloc( sizeof( NC ) );
														sum->type = 'S';

														if(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' )
														{
															sum = copylist( p_trans->Assign.rhs->link->list );
														}
														else
														{
															sum = copylist( p_trans->Assign.rhs );
														}

														break_flag = 0;
														flag_1 = 0;
														flag_2 = 0;

														/*
														printf( "\n ##### \nbefore Substitute_In_Sum : \n ");
														write_lists( p1 );
														printf( " = ");
														write_lists( s4 );
														printf("\n wrt \n");
														write_lists(p);
														printf( " = ");
														write_lists( sum );
														printf("\n"); */

														s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

														/*
														printf( "\n after Substitute_In_Sum : s4 : ");
														write_lists( p1 );
														printf( " = ");
														write_lists( s4 );
														printf("\n ####\n"); */

														if( break_flag == 1 )
														{
															break;
														}

														p_trans = p_trans->next;
													} //end while
												}
												//KB array end

												else
												{
													trans = path->transformations;
													result = (NC *)malloc(sizeof(NC));
													result = Substitute_in_MOD_DIV( s3, trans, result );
													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1; //s2->inc;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( result );
													s4->link->list = NULL;
												}

												M_z = Mult_Sum_With_Sum( M_z, s4, M_z );
												s3 = s3->list;
											} // while( s3!= NULL )

											M_t = ( NC * )malloc( sizeof( NC ) );
											M_t->type = 'S';
											M_t->inc = s2->inc;
											M_t->link = NULL;
											M_t->list = NULL;

											M_z = Mult_Sum_With_Sum( M_z, M_t, M_z );

											s1 = Add_Sums( s1, M_z, s1 );
										} // if( s2->link != NULL )

										s = s->list;
									} // while( s != NULL )

									condition->link->link = copylist( s1 );

								} // if( s->link != NULL )

							} //if( condition->link->link != NULL )

						} // if( condition->link != NULL )

						condition = condition->list;

					}// while( condition != NULL )

				} // if( condition->link != NULL )

				c3 = path->condition;
				c3 = c3->link;
				while( c3->list != NULL )
				{
					c3 = c3->list;
				}
				c3->list = c1->link;
			}
		}

		// transformations
		//In transformations, other than 'S', 'w' can also occur at the top-most level //KB trans
		i = 0;
		while( next_pn->actions[i].rhs != NULL )
		{

			if( path->transformations == NULL )
			{
				if( next_pn->actions[i].rhs != NULL )
				{
					p_trans = ( r_alpha * )malloc( sizeof( r_alpha ) );
					p_trans->Assign.lhs = next_pn->actions[i].lhs;
					p_trans->Assign.rhs = copylist( next_pn->actions[i].rhs );
					p_trans->next = NULL;
					path->transformations = p_trans;
				}
			}
			else
			{
				if( next_pn->actions[i].rhs != NULL )
				{
					p_trans1 = path->transformations;
					flag = 0;
					while( p_trans1 != NULL && flag == 0 )
					{
						if( next_pn->actions[i].lhs == p_trans1->Assign.lhs && flag == 0 )
							//&& next_pn->actions[i].rhs != NULL && next_pn->actions[i].rhs->type == 'w'
							//&& compare_trees(next_pn->actions[i].rhs->link->link, p_trans1->Assign.rhs->link->link) == 1 )
						{
							flag = 1;

							s = ( NC * )malloc( sizeof( NC ) );
							s = copylist( next_pn->actions[i].rhs );

							//KB array start
							if(s->type == 'w')
							{
								//The old_s needs to be stored for determining whether
								//there is an output dependency between the array assignments
								old_s = (NC *)malloc(sizeof(NC));
								old_s = copylist( s );

								//s3 contains value of 'y'
								s3 = (NC *)malloc(sizeof(NC));
								s3 = copylist( s->link->list );

								trans = path->transformations;
								result = (NC *)malloc(sizeof(NC));
								result = Substitute_in_MOD_DIV( s3, trans, result );
								s->link->list = result;

								flagLink = TRUE;
								traverseList = s->link->link;

								while( traverseList != NULL )
								{
									trans = path->transformations;

									new = ( NC *)malloc( sizeof( NC));
									new->type = traverseList->type;
									new->inc = traverseList->inc;
									new = Substitute_in_MOD_DIV( traverseList, trans, new );
									new->list = NULL;

									if( flagLink )
									{
										s->link->link = new;
										flagLink = FALSE;
									}
									else
									{
										old->list = new;
									}

									old = new;
									traverseList = traverseList->list;
								}

								if(p_trans1->Assign.rhs->type == 'S')
									p_trans1->Assign.rhs = copylist( s );
								else // p_trans1->Assign.rhs == 'w'
								{
									if(s->type == 'w' && p_trans1->Assign.rhs->link->inc == s->link->inc &&
												compare_trees(p_trans1->Assign.rhs->link->link, old_s->link->link) == 1)
										p_trans1->Assign.rhs = copylist( s );
									else
									{
										flagNewAssgn = TRUE;
										trav_trans = path->transformations;
										while(trav_trans != NULL)
										{
											if(trav_trans->Assign.rhs != NULL && trav_trans->Assign.rhs->type == 'w' &&
													trav_trans->Assign.rhs->link->inc == s->link->inc &&
													compare_trees(trav_trans->Assign.rhs->link->link, old_s->link->link) == 1)
											{
												flagNewAssgn = FALSE;
											}
											trav_trans = trav_trans->next;
										}

										if(flagNewAssgn)
										{
											new_assignment = (r_alpha*) malloc (sizeof(r_alpha));
											new_assignment->Assign.lhs = s->link->inc;
											new_assignment->Assign.rhs = (NC*) malloc (sizeof(NC));
											new_assignment->Assign.rhs = copylist( s );
											new_assignment->next = p_trans1->next;
											p_trans1->next = new_assignment;

											//The following statement is added to avoid repetition for new_assignment
											p_trans1 = p_trans1->next;
										}
									}
								} //end if  p_trans1->Assign.rhs == 'w'
							}
							else
							{
							//KB array end

								s->type = 'S';

								s1 = ( NC * )malloc( sizeof( NC ) );
								s1 = copylist( next_pn->actions[i].rhs );

								//s and s1 have the same content at this moment

								s1->link = NULL;
								s1->list = NULL;

								if(s->link==NULL)
								{
									p_trans1->Assign.rhs=s;
								}								
								else
								{
									s = s->link;
									while( s != NULL )
									{
										s2 = ( NC * )malloc( sizeof( NC ) );
										s2 = copylist( s );
										s2->list = NULL;

										if( s2->link != NULL )
										{
											s3 = ( NC * )malloc( sizeof( NC ) );
											s3 = copylist( s2->link );

											M_z = ( NC * )malloc( sizeof( NC ) );
											M_z->type = 'S';
											M_z->inc = 1;
											M_z->link = NULL;
											M_z->list = NULL;

											while( s3 != NULL )
											{
												if( s3->type == 'v' )
												{
													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( s3 );
													s4->link->list = NULL;
													s4->link->link->list = NULL;

													p_trans = path->transformations;
													while( p_trans != NULL )
													{
														p = ( NC * )malloc( sizeof( NC ) );
														p->type = 'v';
														p->inc = p_trans->Assign.lhs;
														p->link = NULL;
														p->list = NULL;

														sum =  ( NC * )malloc( sizeof( NC ) );
														sum->type = 'S';
														sum = copylist( p_trans->Assign.rhs );
														break_flag = 0;
														flag_1 = 0;
														flag_2 = 0;

														s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

														if( break_flag == 1 )
														{
															break;
														}

														p_trans = p_trans->next;
													}
												} // if( s3->type == 'v' )

												//KB array start
												else if( s3->type == 'a' )
												{
													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1; //s2->inc;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( s3 );
													s4->link->list = NULL;
													s4->link->link->list = NULL;

													p_trans = path->transformations;
													while( p_trans != NULL )
													{
														if( p_trans->Assign.lhs != s3->inc ||
															(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' &&
																compare_trees(s3->link, p_trans->Assign.rhs->link->link)==0) )
														{
															p_trans = p_trans->next;
															continue;
														}

														p = ( NC * )malloc( sizeof( NC ) );
														p = copylist( s3 );

														p1 = ( NC * )malloc( sizeof( NC ) );
														p1->inc = next_pn->actions[i].lhs;
														p1->link = NULL;
														p1->list = NULL;

														if(next_pn->actions[i].rhs != NULL && next_pn->actions[i].rhs->type == 'w')
															p1->type = 'w';
														else
															p1->type = 'v';


														sum =  ( NC * )malloc( sizeof( NC ) );
														sum->type = 'S';

														if(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' )
														{
															sum = copylist( p_trans->Assign.rhs->link->list );
														}
														else
														{
															sum = copylist( p_trans->Assign.rhs );
														}

														break_flag = 0;
														flag_1 = 0;
														flag_2 = 0;

														/*
														printf( "\n ##### \nbefore Substitute_In_Sum : \n ");
														write_lists( p1 );
														printf( " = ");
														write_lists( s4 );
														printf("\n wrt \n");
														write_lists(p);
														printf( " = ");
														write_lists( sum );
														printf("\n"); */

														s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

														/*
														printf( "\n after Substitute_In_Sum : s4 : ");
														write_lists( p1 );
														printf( " = ");
														write_lists( s4 );
														printf("\n ####\n"); */

														if( break_flag == 1 )
														{
															break;
														}

														p_trans = p_trans->next;
													} //end while
												}
												//KB array end

												else
												{
													trans = path->transformations;
													result = (NC *)malloc(sizeof(NC));
													result = Substitute_in_MOD_DIV( s3, trans, result );

													s4 = ( NC * )malloc( sizeof( NC ) );
													s4->type = 'S';
													s4->inc = 0;

													s4->link = ( NC * )malloc( sizeof( NC ) );
													s4->list = NULL;
													s4->link->type = 'T';
													s4->link->inc = 1; //s2->inc;

													s4->link->link = ( NC * )malloc( sizeof( NC ) );
													s4->link->link = copylist( result );
													s4->link->list = NULL;
												}


												M_z = Mult_Sum_With_Sum( M_z, s4, M_z );
												/*printf( "\n after Mult_Sum_With_Su : M_z : ");
												write_lists( p1 );
												printf( " = ");
												write_lists( M_z );
												printf("\n");
												getchar( );*/
												s3 = s3->list;
											} // while( s3!= NULL )

											M_t = ( NC * )malloc( sizeof( NC ) );
											M_t->type = 'S';
											M_t->inc = s2->inc;
											M_t->link = NULL;
											M_t->list = NULL;

											M_z = Mult_Sum_With_Sum( M_z, M_t, M_z );

											s1 = Add_Sums( s1, M_z, s1 );

											/* printf( "\n after add_sums : s1 : ");
											write_lists( p1 );
											printf( " = ");
											write_lists( s1 );
											printf("\n");
											getchar( );*/

										} // if( s2->link != NULL )

										s = s->list;

									} // while( s != NULL )

									if(p_trans1->Assign.rhs->type == 'S')
										p_trans1->Assign.rhs = copylist( s1 );
									else // p_trans1->Assign.rhs == 'w'
									{
										if(s1->type == 'w' && p_trans1->Assign.rhs->link->inc == s1->link->inc &&
												compare_trees(p_trans1->Assign.rhs->link->link, s1->link->link) == 1)
											p_trans1->Assign.rhs = copylist( s1 );
										else
										{
											flagNewAssgn = TRUE;
											trav_trans = path->transformations;
											while(trav_trans != NULL)
											{
												if(trav_trans->Assign.rhs != NULL && trav_trans->Assign.rhs->type == 'w' &&
														trav_trans->Assign.rhs->link->inc == s1->link->inc &&
														compare_trees(trav_trans->Assign.rhs->link->link, s1->link->link) == 0)
												{
													flagNewAssgn = FALSE;
												}
												trav_trans = trav_trans->next;
											}

											if(flagNewAssgn)
											{
												new_assignment = (r_alpha*) malloc (sizeof(r_alpha));
												new_assignment->Assign.lhs = s1->link->inc;
												new_assignment->Assign.rhs = (NC*) malloc (sizeof(NC));
												new_assignment->Assign.rhs = copylist( s1 );
												new_assignment->next = p_trans1->next;
												p_trans1->next = new_assignment;

												p_trans1 = p_trans1->next;
											}
										}
									}
									//p_trans1->Assign.rhs = NULL;
									//p_trans1->Assign.rhs = copylist( s1 );
									break;
								} // if( s->link != NULL )

							//KB array start
							}
							//KB array end

						}
						p_trans1 = p_trans1->next;
					}

					if( flag == 0 )
					{
						s = ( NC * )malloc( sizeof( NC ) );
						s = copylist( next_pn->actions[i].rhs );


						//KB array start
						if(s->type == 'w')
						{
							//s3 contains value of 'y'
							s3 = (NC *)malloc(sizeof(NC));
							s3 = copylist( s->link->list );

							trans = path->transformations;
							result = (NC *)malloc(sizeof(NC));
							result = Substitute_in_MOD_DIV( s3, trans, result );
							s->link->list = result;

							flagLink = TRUE;
							traverseList = s->link->link;

							while( traverseList != NULL )
							{
								trans = path->transformations;

								new = ( NC *)malloc( sizeof( NC));
								new->type = traverseList->type;
								new->inc = traverseList->inc;
								new = Substitute_in_MOD_DIV( traverseList, trans, new );
								new->list = NULL;

								if( flagLink )
								{
									s->link->link = new;
									flagLink = FALSE;
								}
								else
								{
									old->list = new;
								}

								old = new;
								traverseList = traverseList->list;
							}

							p_trans3 = path->transformations;
							p_trans2 = ( r_alpha * )malloc( sizeof( r_alpha ) );
							p_trans2->Assign.lhs = next_pn->actions[i].lhs;
							p_trans2->Assign.rhs =  ( NC * )malloc( sizeof( NC ) );
							p_trans2->Assign.rhs = copylist( s );
							p_trans2->next = NULL;

							while( p_trans3->next != NULL )
							{
								p_trans3 = p_trans3->next;
							}
							p_trans3->next = p_trans2;
						}
						else
						{
						//KB array end

							s->type = 'S';

							s1 = ( NC * )malloc( sizeof( NC ) );
							s1 = copylist( next_pn->actions[i].rhs );
							s1->link = NULL;
							s1->list = NULL;

							if( s->link != NULL )
							{
								s = s->link;
								while( s != NULL )
								{
									s2 = ( NC * )malloc( sizeof( NC ) );
									s2 = copylist( s );
									s2->list = NULL;

									if( s2->link != NULL )
									{
										s3 = ( NC * )malloc( sizeof( NC ) );
										s3 = copylist( s2->link );

										M_z = ( NC * )malloc( sizeof( NC ) );
										M_z->type = 'S';
										M_z->inc = 1;
										M_z->link = NULL;
										M_z->list = NULL;

										while( s3 != NULL )
										{
											if( s3->type == 'v' )
											{
												s4 = ( NC * )malloc( sizeof( NC ) );
												s4->type = 'S';
												s4->inc = 0;

												s4->link = ( NC * )malloc( sizeof( NC ) );
												s4->list = NULL;
												s4->link->type = 'T';
												s4->link->inc = 1;

												s4->link->link = ( NC * )malloc( sizeof( NC ) );
												s4->link->link = copylist( s3 );
												s4->link->list = NULL;
												s4->link->link->list = NULL;

												p_trans = path->transformations;
												while( p_trans != NULL )
												{
													p = ( NC * )malloc( sizeof( NC ) );
													p->type = 'v';
													p->inc = p_trans->Assign.lhs;
													p->link = NULL;
													p->list = NULL;

													p1 = ( NC * )malloc( sizeof( NC ) );
													p1->type = 'v';
													p1->inc = next_pn->actions[i].lhs;
													p1->link = NULL;
													p1->list = NULL;

													sum =  ( NC * )malloc( sizeof( NC ) );
													sum->type = 'S';
													sum = copylist( p_trans->Assign.rhs );

													break_flag = 0;
													flag_1 = 0;
													flag_2 = 0;

													/*
													printf( "\n ##### \nbefore Substitute_In_Sum : \n ");
													write_lists( p1 );
													printf( " = ");
													write_lists( s4 );
													printf("\n wrt \n");
													write_lists(p);
													printf( " = ");
													write_lists( sum );
													printf("\n"); */

													s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

													/*
													printf( "\n after Substitute_In_Sum : s4 : ");
													write_lists( p1 );
													printf( " = ");
													write_lists( s4 );
													printf("\n ####\n"); */
													//getchar( );

													if( break_flag == 1 )
													{
														break;
													}

													p_trans = p_trans->next;
												}
											} // if( s3 ->type == 'v' )

											//KB array start
											else if( s3->type == 'a' )
											{
												s4 = ( NC * )malloc( sizeof( NC ) );
												s4->type = 'S';
												s4->inc = 0;

												s4->link = ( NC * )malloc( sizeof( NC ) );
												s4->list = NULL;
												s4->link->type = 'T';
												s4->link->inc = 1; //s2->inc;

												s4->link->link = ( NC * )malloc( sizeof( NC ) );
												s4->link->link = copylist( s3 );
												s4->link->list = NULL;
												s4->link->link->list = NULL;

												p_trans = path->transformations;
												while( p_trans != NULL )
												{
													if( p_trans->Assign.lhs != s3->inc ||
														(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' &&
															compare_trees(s3->link, p_trans->Assign.rhs->link->link)==0) )
													{
														p_trans = p_trans->next;
														continue;
													}

													p = ( NC * )malloc( sizeof( NC ) );
													p = copylist( s3 );

													p1 = ( NC * )malloc( sizeof( NC ) );
													p1->inc = next_pn->actions[i].lhs;
													p1->link = NULL;
													p1->list = NULL;

													if(next_pn->actions[i].rhs != NULL && next_pn->actions[i].rhs->type == 'w')
														p1->type = 'w';
													else
														p1->type = 'v';


													sum =  ( NC * )malloc( sizeof( NC ) );
													sum->type = 'S';

													if(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' )
													{
														sum = copylist( p_trans->Assign.rhs->link->list );
													}
													else
													{
														sum = copylist( p_trans->Assign.rhs );
													}

													break_flag = 0;
													flag_1 = 0;
													flag_2 = 0;

													/*
													printf( "\n ##### \nbefore Substitute_In_Sum : \n ");
													write_lists( p1 );
													printf( " = ");
													write_lists( s4 );
													printf("\n wrt \n");
													write_lists(p);
													printf( " = ");
													write_lists( sum );
													printf("\n"); */

													s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

													/*
													printf( "\n after Substitute_In_Sum : s4 : ");
													write_lists( p1 );
													printf( " = ");
													write_lists( s4 );
													printf("\n ####\n"); */

													if( break_flag == 1 )
													{
														break;
													}

													p_trans = p_trans->next;
												} //end while
											}
											//KB array end

											else
											{
												result = (NC *)malloc(sizeof(NC));
												result = Substitute_in_MOD_DIV( s3, path->transformations, result );

												s4 = ( NC * )malloc( sizeof( NC ) );
												s4->type = 'S';
												s4->inc = 0;

												s4->link = ( NC * )malloc( sizeof( NC ) );
												s4->list = NULL;
												s4->link->type = 'T';
												s4->link->inc = 1; //s2->inc;

												s4->link->link = ( NC * )malloc( sizeof( NC ) );
												s4->link->link = copylist( result );
												s4->link->list = NULL;
											}

											//For the "else" part, M_z will have the value 1;
											//therefore after the following statement M_z will have
											//the same value as s4 - neverhteless, the following statement
											//is necessary because after the loop s1 acts on M_z

											/*
											printf( "\n **** \nafter Mult_Sum_With_Sum : M_z : ");
											write_lists( p1 );
											printf( " = ");
											write_lists( M_z );
											printf("\n"); */

											M_z = Mult_Sum_With_Sum( M_z, s4, M_z );

											/*
											printf( "\n after Mult_Sum_With_Sum : M_z : ");
											write_lists( p1 );
											printf( " = ");
											write_lists( M_z );
											printf("\n **** \n"); */
											//getchar( );


											s3 = s3->list;

										} // while( s3!= NULL )

										M_t = ( NC * )malloc( sizeof( NC ) );
										M_t->type = 'S';
										M_t->inc = s2->inc;
										M_t->link = NULL;
										M_t->list = NULL;

										M_z = Mult_Sum_With_Sum( M_z, M_t, M_z );

										s1 = Add_Sums( s1, M_z, s1 );

										/*
										printf( "\n after add_sums : s1 : ");
										write_lists( p1 );
										printf( " = ");
										write_lists( s1 );
										printf("\n ^^^^ \n"); */
										//getchar( );

									} // if( s2->link != NULL )

									s = s->list;
								}	// while( s != NULL )
							} // if( s->link != NULL )

							p_trans3 = path->transformations;
							p_trans2 = ( r_alpha * )malloc( sizeof( r_alpha ) );
							p_trans2->Assign.lhs = next_pn->actions[i].lhs;
							p_trans2->Assign.rhs =  ( NC * )malloc( sizeof( NC ) );
							p_trans2->Assign.rhs = copylist( s1 );
							p_trans2->next = NULL;

							while( p_trans3->next != NULL )
							{
								p_trans3 = p_trans3->next;
							}
							p_trans3->next = p_trans2;

						//KB array start
						} //else
						//KB array end

					} // if ( flag == 0 )
				} // if( next_pn->actions[i].rhs != NULL )
			}
			i++;
		}
		next_pn = next_pn->next;
	}
}



int
is_in_CPQ(CPQ *cpl,int state)
{
     CPQ *temp;
     temp=cpl;
     while(temp!=NULL)
     {
	     if(temp->s0==state)
            {
		       return temp->s1;
            }
	     temp=temp->next;
     }
     return -1;
}



/*****************************************************************************/

/*     Arguments are here two r_alpha ( first one of outward transitions(head)
       and  second one for path transformation(path->transformations)).
       This function compares them.
       Returns 1 if they equal; otherwise returns 0          				 */

/*****************************************************************************/

int
r_alpha_equal( r_alpha *r1, r_alpha *r2, int *var_kripke, int *length, int *var_spec, int *no_of_spec_vars )
{
  r_alpha *temp1, *temp2, *temp3, *temp4, *temp5, *temp6;
  NC *p;
  int flag = 0, m1 = -1, m2 = -1, value = 1, i, j, t_flag = 0, used_flag, define_flag, k;


  temp1 = r1;
  temp2 = r2;
  if ( temp2 == NULL && temp1 == NULL )
  {
    printf("\n both r_alpha NULL");
    return 1;
  }
  else if (temp2 == NULL)
        return 0;
  else if(temp1 == NULL)
        return 0;


   /** Following loop gurrentees that r1 does not have any extra operation **/

  temp1 = r1;
  while( temp1 != NULL )
  {
     m2 =  var_in_V0_V1( temp1->Assign.lhs );
     /* m2 == -1 if 'temp1->Assign.lhs' is not present in the list
	     * V0_V1.
	     */
	    if( m2 != -1 )
	    {
	        temp2 = r2;
	        flag = 0;
		    t_flag = 0;
       		while( temp2 != NULL )
		    {
		      if( temp2->Assign.lhs == temp1->Assign.lhs )
		      {
				if( compare_trees( temp1->Assign.rhs, temp2->Assign.rhs ) )
				{
				    flag = 1;
				    break;
				}
			  }
			  temp2 = temp2->next;
		    }

		    if( flag == 0 )
		    {
			   value = 0;
			   break;
		    }
	    }
	    temp1=temp1->next;
   }

   if( value == 0 ) // for some variable x (let), its rhs not same in r1 and r2.
	 {
           /* finding varables which have been defined in r1 but not in r2. For these variable, we need to do model checking */
	   temp3 = r1;
	   temp4 = r2;

	   while( temp3 != NULL )
	   {

	        flag = 0;
		m2 =  var_in_V0_V1( temp3->Assign.lhs );
	    	/* m2 == -1 if 'temp3->Assign.lhs' is not present in the list V0_V1.
	     	 */
	    	if( m2 != -1 )
		{

		   while( temp4 != NULL )
		   {

                        if( temp3->Assign.lhs == temp4->Assign.lhs )
			{
			   flag = 1;
			   break;
			}
			temp4 = temp4->next;
		    }

		   if( flag == 0 ) // means the variable is defined in r1 but not in r2
		   {
		   	t_flag=0;
			for( i = 0; i < *length; i++ )
			{
				//printf( "\n in for loop : length = %d \n", *length);
			      	if( var_kripke[i] == temp1->Assign.lhs)
				{
			        	t_flag = 1;
					break;
				}
			}

			if( t_flag != 1 ) // this variable is a candidate for model checking, store in var_kripke list
			{
				var_kripke[*length] = temp1->Assign.lhs;
			       	(*length)++;
				t_flag = 0;
			}
                    }

		  }
		   temp3 = temp3->next;
		}


    // finding variable for speculation
    for( i = 0; i < V1.no_of_elements; i++ )
    {
     k = 0;
     k = var_in_V1_minus_V0_intersection_V1( V1.var_val[i] ); // this function checks if a
						// variable present in V1-(V0 intersection V1). if present k=1 else -1
     if( k == 1 )
     {
      temp5 = r2;
      define_flag = 0;
      while( temp5 != NULL ) // we are checking whether the variale i of V1 is defined in r2 or not.
      {
	 if( temp5->Assign.lhs == V1.var_val[i] )
	 {
	    define_flag = 1;
	    break;
	 }
	 temp5 = temp5->next;
      }
      if( define_flag == 0 ) // variable is not defined in r2. Next, we check if it is present in any rhs of r2
      {
         temp6 = r2;
	 used_flag = 0;
	 p = ( NC * )malloc( sizeof( NC ) );
	 p->type = 'v';
	 p->inc = V1.var_val[i];
	 p->link = NULL;
	 p->list = NULL;
	 while( temp6 != NULL && used_flag == 0 )
         {
	   used_flag = search_primary_in_sum( temp6->Assign.rhs, p, used_flag );
	   if( used_flag == 1 ) //if present, store in var_spec list for speculation checking later.
	   {
	      t_flag = 0;
	      for( j = 0; j < *no_of_spec_vars; j++ )
	      {
		 //printf( "\n in for loop : length = %d \n", *length);
		 if(  var_spec[j] == V1.var_val[i] )
		 {
		    t_flag = 1;
		    break;
		 }
	      }
	      if( t_flag != 1 )
	      {
		  var_spec[*no_of_spec_vars] = V1.var_val[i];
		  (*no_of_spec_vars)++;
		  t_flag = 0;
	      }
	   }
	   temp6 = temp6->next;
         }
      }
      } // if( k == 1 )
    }
 return 0;
 }


	/*** successful completetion of this loop gurrentees that all the
	operation of r2 have an equivalent one in r1; **/
        temp1 = r1;
	temp2 = r2;
	while(temp2!=NULL)
	{
	    m1 =  var_in_V0_V1( temp2->Assign.lhs );
	    /* m1 == -1 if 'temp2->Assign.lhs' is not present in the list
	     * V0_V1.
	     */
	    if( m1 != -1 )
	    {

	        temp1=r1;
                flag=0;
       		while(temp1!=NULL)
		{
			if(temp2->Assign.lhs == temp1->Assign.lhs  )
			{
				if(compare_trees(temp1->Assign.rhs,temp2->Assign.rhs))
				{
				  	flag=1;
					break;
				}
			}
			temp1=temp1->next;
		}
		if(flag==0)
			return 0;
	    }
	    temp2=temp2->next;
   	}
     return 1;
}



/*****************************************************************************/
/*        This function try to find an equivalent path of beta (of M0) in M1
		  if succeeds then returns that equivalent path (alpha); otherwise
		  returns NULL                                                       */

/*****************************************************************************/

void findEquivalent(PATH_ST *beta1,FSMD *M1, FSMD *M0, CPQ *eta, PATHS_LIST tempP1, FSMD *M0_kripke )
{
  int k=-1,cs;


  PATH_ST *alpha, *beta;
  CPQ *head,*prev,*temp;
  prev=eta;
  beta=beta1;

  // printf("pathck %d -- %d\n",beta->start,beta->end);
  printf("\npath is now checking: %s --> %s\n", M0->states[beta->start].state_id, M0->states[beta->end].state_id);
  displayApath(beta);
  head=prev;
  if(beta->status==0)
    k = is_in_CPQ(head, beta->start);
  if(k<0)
    {
      printf("\nCould not find corresponding state: exiting system");
      exit(0);
    }
  else
    {

      cs=findcp( M1, beta, k, tempP1, M0, M0_kripke );

      if(cs >= 0) //equivalent path found
	{
	  printf("\n Equivalent path found\n");
	  beta->status=1;
	  head=prev;
	  if(is_in_CPQ(head,beta->end)<0)
	    {
	      temp=(CPQ*) malloc (sizeof(CPQ));
	      temp->next=NULL;
	      temp->s0=beta->end;
	      temp->s1=cs;
	      temp->next=prev;
	      prev=temp;
	      //  printf("\ncorresponding st : <%d, %d>",beta->end,cs);
	    }
	}
      else
	{
	  printf(" failed to find the corresponding path of path in the scheduled FSMD \n  now checking for extended path \n ");
	  alpha=NULL;
	}

    }
  // return alpha;
}





/*****************************************************************************/

/*   This function check the conditions of two path passing as arguments.
     It returns 0   if the conditions are not matched.
                1   if conditions are matched.
                2   if conditions of tempPath matched with that of path but
                       path has some other conditions also. In this case this
                       path needs to add to its successor.                   */

/*****************************************************************************/

int
checkCondition(PATH_ST *path, PATH_ST *tempPath) {
     NC *t, *temp;
     int canProceed=0, flag=0;
	 // displayApath(path);
     t=path->condition; // R_\beta
     if(t==NULL)  {// no condition of execution of the path beta
	   if(tempPath->condition == NULL)
	   {  // for this path emanates from k
	      // there is no condition of execution
	      canProceed=1; // canProceed=1 indicates both the path and the outward transition have
                        // no condition of execution; so we can proceed for checking the r_\alpha s.

       }
	   else {
	     canProceed=0; // can not proceed in this path as condition in this outward paths does
		               //not mathched
	   }
	 }

     else { // beta have some condition of execution
	   if(tempPath->condition == NULL) {
		  canProceed=2; //canProceed=2 indicates beta have R_\alpha but this outward path
		                //has no R_\alpha; still we can proceed as condition may matched in
			            //the path that follows this outward path.
	   }
	   else {  // outward path has condition of execution
	        canProceed=0;
		    temp=tempPath->condition->link;
            while(temp!=NULL) {
		       t=path->condition->link;
			   flag=0;
			   while(t!=NULL) {
			     if(t->inc==0) {
				   if(compare_trees(t->link,temp->link)) {
		               flag=1;
   					   break;
				   }
			     }
				 t=t->list;
			   }
			   if(flag == 0)
			   {
				 canProceed=0;
				 break;
			   }
			   temp=temp->list;
			}
			if(temp == NULL)
				canProceed=3; //all components of the condition of this outward path
 							  // have an equivalent in that of the path.

	    } // end else-if(tempPath->condition == NULL)
	}//end else-if(t=NULL)

	if(canProceed == 3) //lets check in other ways. It means check both conditions are totally
						// matched or not.
	{
		temp=path->condition->link;
        while(temp!=NULL)
	    {
		  t=tempPath->condition->link;
		  flag=0;
		  while(t!=NULL)
		  {
			 if(t->inc==0)
			 {
			    if(compare_trees(t->link,temp->link))
		     	{
				  flag=1;
   				  break;
				}
			 }
			 t=t->list;
		   }
		   if(flag == 0)
		   {
			 canProceed=2; //condition of the outward paths partially matched with the
			               // condition of the path. So its required to extend this path
			 break;
		   }
		   temp=temp->list;
		}
		if(temp == NULL)
			canProceed=1; //conditions are totally matched. Need not to extend.
	}

	return canProceed;
}


/*****************************************************************************/

/*     This function concats two paths passing as arguments then returns that
       concatenated path                                                     */
// MODIFIED for value propagation
/*****************************************************************************/
PATH_ST*
concatPaths(PATH_ST *path1, PATH_ST *path2)
{
	PATH_NODE *head, *prev, *nodelist, *temp;
	PATH_ST *path;
	char *sym_value = (char*)malloc(100*sizeof(char));

	int i,j;

    path=(PATH_ST *)malloc(sizeof(PATH_ST));
	path->start = path1->start;
	path->end = path2->end;
	path->status = 0;
	path->cp=NULL;
	path->next=NULL;
	path->transformations=NULL;
	path->condition=NULL;

    head=(PATH_NODE*)malloc(sizeof(PATH_NODE));
	head->state=path1->nodeslist->state;
	head->condition=NULL; //copylist(path1->nodeslist->condition);
	head->actions=NULL;
	head->next=NULL;
	head->left=NULL;

    prev=head;
    nodelist=path1->nodeslist->next;
    while(nodelist !=NULL) // copy node information of path1
    {
        prev->next=(PATH_NODE*)malloc(sizeof(PATH_NODE));
        temp=prev;
        prev=prev->next;
        prev->state=nodelist->state;
		prev->condition=copylist(nodelist->condition);
		j=0;
		while(nodelist->actions[j].rhs != NULL)
		{
			j++;
		}
		prev->actions=(DATA_TRANS *)malloc(sizeof(DATA_TRANS)*j);
		i=0;
		while(i<j)
		{
			prev->actions[i].lhs=nodelist->actions[i].lhs;
    		prev->actions[i].rhs=copylist(nodelist->actions[i].rhs);
			i++;
		}
		prev->actions[i].rhs=NULL;
		prev->next=NULL;
		prev->left=temp;
        nodelist=nodelist->next;
	} //end while

    nodelist=path2->nodeslist->next;
    while(nodelist !=NULL) // copy information from path2
    {
        prev->next=(PATH_NODE*)malloc(sizeof(PATH_NODE));
        temp=prev;
        prev=prev->next;
        prev->state=nodelist->state;
	 	prev->condition=copylist(nodelist->condition);
		j=0;
		while(nodelist->actions[j].rhs != NULL)
		{
			j++;
		}
		prev->actions=(DATA_TRANS *)malloc(sizeof(DATA_TRANS)*j);
		i=0;
		while(i<j)
		{
			prev->actions[i].lhs=nodelist->actions[i].lhs;
    		prev->actions[i].rhs=copylist(nodelist->actions[i].rhs);
			i++;
		}
		prev->actions[i].rhs = (NC*)NULL;
		prev->next=NULL;
		prev->left=temp;
        nodelist=nodelist->next;
    }
	path->nodeslist=head;
	nodelist=path->nodeslist;

	/*printf( "\n\n in concate paths : displaying nodes information\n" );
	 nodelist = path->nodeslist;
	 nodelist=nodelist->next;
	 while( nodelist != NULL )
	   {
             printf( "\n");
	     write_lists( nodelist->condition);
	     printf( "\n");
	     j = 0;
	     while( nodelist->actions[j].rhs != NULL )
	       {

		 symbol_for_index( nodelist->actions[j].lhs, sym_value );
		 printf("\n%s  :=  ", sym_value );
		 write_lists( nodelist->actions[j].rhs );
		 printf("\n");
                 j++;
	       }
	     nodelist = nodelist->next;
	   }
	printf( "\n associating R_alpha and r_alpha for contated path \n" );*/

	find_Ralpha( path );
	//displayApath( path );

	return path;
}



/*****************************************************************************/

/*    This function recursively concat the successor paths of path1 from tempP1
      with path1 and then checks whether that concatenated path is equivalent
      with path or not. It returns 1 if it finds equivalent path otherwise
      returns -1                                                             */

/*****************************************************************************/


int
addPath(PATH_ST *path, PATH_ST *path1, PATHS_LIST tempP1, FSMD *M0_kripke )
{
   FILE *fp1;
   int flag = -1, canProceed = 0, j, no_of_kripke_vars, vars, var_kripke[500], k_state_num, pid, flag_concat = 1, var_spec[500], no_of_spec_vars, flag_1, flag_2, break_flag, start, i, spec_flag, temp_lhs, j1, c_sum;
   char sym_value[200], fp_string[1000];
   PATH_ST path2, *concatedPath, *temp_path, *temp_path1, *temp_path3;
   NC *s, *s1, *s2, *s3, *s4, *p, *sum, *M_z, *temp_rhs, *temp_rhs1, *result;
   r_alpha *p_trans, *p_trans1, *p_trans3;

   for(j=0; j<tempP1.numpaths; j++)
   {
      path2 = tempP1.paths[j];
      flag_concat = 1;

     // if(path1->end == path2.start && path1->concateble == 1 )
     if(path1->end == path2.start )
      {// paths that start from end node of the ith path.

         /*if( path2.end == path1->start || path2.end == path1->end )
	 {
            flag_concat = 0;
	 }
	 else
	 {
	   temp_nodeslist = path1->nodeslist;
	   while( temp_nodeslist != NULL )
	   {
	     if( path2.end == temp_nodeslist->state )
	     {
	        flag_concat = 0;
		break;
	     }
	   }
	 }*/

	 /*printf( "\n in addpath : before concatpath :\n" );
	 printf( "path1\n" );
	 displayApath( path1 );
	 printf( "\n&tempP1.paths[j] \n" );
	 displayApath( &tempP1.paths[j] );
	 printf( "\n");*/
   	 concatedPath = concatPaths( path1, &tempP1.paths[j] );
	 /*printf( "\n in addpath : after concatpath :\n" );
	 printf( "\nconcatedpath\n" );
	 displayApath( concatedPath );
	 printf( "\n");
	 printf( "path\n" );
	 displayApath( path );
	 printf( "\n");*/
	// if( flag_concat == 0 )
	// {
	   concatedPath->concateble = 0;
	// }

         canProceed = checkCondition( path, concatedPath );
	 /*printf( "\n condition of path :" );
	 write_lists( path->condition );
	 printf( "\n");
	 printf( "\n condition of path1 :" );
	 write_lists( concatedPath->condition );
	 printf( "\n");
	 printf( "\n\n canproceed = %d\n\n", canProceed );*/
	 if(canProceed == 0)
	 {
	    continue; //conditions mismatched
	 }
	 else if(canProceed == 1)
	 {
	    no_of_kripke_vars = 0;
	    no_of_spec_vars = 0;
	    if(r_alpha_equal( path->transformations, concatedPath->transformations, var_kripke, &no_of_kripke_vars,
               var_spec, &no_of_spec_vars ) )
	    {
	       path->cp=concatedPath;
	       return concatedPath->end;  // equivalent path found
	    }
	   else
	   {
	      if( no_of_kripke_vars > 0 )
	      {
		 nmodelCheck++;
		//if( list 'var_kripke' is not NULL ) we do the model checking tasks;
		 //and conclude whether this particular path 'tempP1.paths[i]' is equivalent to
		 //'path'.
		 printf( "\n in addpaths : the variables for which model checking need to apply are : ");
		 for( vars = 0; vars < no_of_kripke_vars; vars++)
		 {
             	    symbol_for_index( var_kripke[vars], sym_value );
	     	    printf( "\t %s ", sym_value );
		 }
		 printf( "\n" );

		 //  This function generates a kripke structurte for FSMD M0.
		 // It is a file named "file_kripke_st".
		 k_state_num = generate_kripke_st( M0_kripke );

		 // This function generates a 'smv code' named "kripke.smv" for the 'kripke structure'
  		 // named "file_kripke_st".
                 generate_smv_code( M0_kripke, k_state_num, path->end, var_kripke, no_of_kripke_vars );

		 // the Nusmv model checker is invoked and the equivalence of this path is checked
		 if ((pid = fork()) == 0)
		 {
                     FILE *fp = fopen("Nusmv.kripke.output", "w");
                     int fd = fileno(fp);
                     dup2(fd, STDOUT_FILENO);
                     close(fd);
                     execl("/home/chandan/myworkInPhD/tools/NuSMV/NuSMV-2.4.3/nusmv/NuSMV",
                            "/home/chandan/myworkInPhD/tools/NuSMV/NuSMV-2.4.3/nusmv/NuSMV", "kripke.smv", NULL);
                     exit(0);
                 }
		 wait( pid );
		 // if the equivalence is found then end state of the corresponding
		 // equivalent path is returned, else, no_of_kripke_vars = 0 and continue.
		 flag = 0;
		 fp1 = fopen( "Nusmv.kripke.output", "r" );
		 while( !feof( fp1 ) ) {
		    fscanf( fp1, "%s", fp_string );
		    if( strcmp( fp_string, "false" ) == 0 )
		    {
		       flag = 1;
		       no_of_kripke_vars = 0;
		       printf("\n Equivalent path not found by model checking\n");
		  //  exit(0);
		       break;
		    }
		 }
		 fclose( fp1);
		 printf("\n flag = %d\n", flag);
	//	 getchar();
		 if( flag == 0 )
		 {
		    nWeak++;
		    path->cp= &tempP1.paths[j];
		    printf("\n Equivalent path found by model checking\n");
	            return tempP1.paths[j].end;  // equivalent path found
		 }

	        }
		else if( no_of_spec_vars > 0 ) // for speculation
	        {
	         nmodelCheck++;
		 for( vars = 0; vars < no_of_spec_vars; vars++)
		 {
		   // spec_flag = -1;
             	   // k = var_in_V1_minus_V0_intersection_V1( var_spec[vars] ); // this function checks if a
										// variable present in V1-(V0 intersection V1). if present k=1 else -1
		   // if( k == 1 )
		   // {
		       symbol_for_index( var_spec[vars], sym_value );
		       printf( "\n in findcp : the variables for which speculation will be used are : ");
	     	       printf( "\t %s \n", sym_value );
		       start = tempP1.paths[j].start; //start = tempP1.paths[i].start; //KB replaced i with j
		       /* in the two loops, we are trying to find the path which ends in the start node of tempP1.paths[i].start and try to find the value of the varaible var_spec[vars] in this path; if that variable does not modify in this path, then return -1 as a indication of speculation fails*/
		       for(j = 0; j < tempP1.numpaths; j++)
               		{
		          if( start == tempP1.paths[j].end )
			  {
			    temp_path = &tempP1.paths[j];
			    p_trans = temp_path->transformations;
			    spec_flag = -1;
			    while( p_trans != NULL )
			    {
			       if( p_trans->Assign.lhs == var_spec[vars] )
			       {
			          temp_lhs = p_trans->Assign.lhs;
			          temp_rhs = copylist( p_trans->Assign.rhs );
				  spec_flag = 1;
				  break;
			       }
			       p_trans = p_trans->next;
			    }
			    if( spec_flag == 1 )
			       break;
			    else
			       return -1;
			  }
			}
		//	displayApath( &tempP1.paths[j] );
		//	printf("\n j = %d, j+1 = %d\n", j, j+1 );
			if( spec_flag != 1 )
			           return -1;
			/* for the rest of the path which end in the start node of tempP1.paths[i].end, we check whether the variable  var_spec[vars] has the same value in all of the paths.*/
			for(j1 = j+1; j1 < tempP1.numpaths; j1++)
                	{
			       	    if( start == tempP1.paths[j1].end )
			  	    {
				    	spec_flag = -1;
				        temp_path3 = &tempP1.paths[j1];
			                p_trans3 = temp_path3->transformations;
			                while( p_trans3 != NULL )
			    		{
			       		   if( p_trans3->Assign.lhs == var_spec[vars] )
			       		   {
				              temp_rhs1 = copylist( p_trans->Assign.rhs );
					      c_sum = Compare_Sums( temp_rhs->link, temp_rhs1->link );
					      if( c_sum == 0 && temp_rhs->inc == temp_rhs1->inc )
					      {
					        spec_flag = 1;
						break;
					      }	// if(c_sum == 0 && temp_rhs->inc == temp_rhs1->inc)
				           }//if(p_trans3->Assign.lhs == var_spec[vars] )
					   p_trans3 = p_trans3->next;
				        }//while( p_trans3 != NULL )

					if( spec_flag == -1 )
					   return -1;
				      }//if(start == tempP1.paths[j1].end )
			  } //for(j1 = j+1; j1 < tempP1.numpaths; j1++)

			   	/* here, we are updating the r_alpha for the path tempP1.paths[i] with the vlaue of the variable var_spec[vars]. The value of this variable has been found in the paths that ends in the start node of the path  tempP1.paths[i]*/
				if( spec_flag == 1 )
				{
				  temp_path1 = &tempP1.paths[j]; // temp_path1 = &tempP1.paths[i]; //KB replaced i with j
			      p_trans1 = temp_path1->transformations;
				  while( p_trans1 != NULL )
				  {
				     s = ( NC * )malloc( sizeof( NC ) );
	                 s = copylist( p_trans1->Assign.rhs );
	     			 s->type = 'S';
	     			 s1 = ( NC * )malloc( sizeof( NC ) );
	     			 s1 = copylist( s );
	     			 s1->link = NULL;
	     			 s1->list = NULL;
	     			 if( s->link != NULL )
	     			 {
	     				s = s->link;
	     				while( s != NULL )
	     				{
	        			   s2 = ( NC * )malloc( sizeof( NC ) );
					       s2 = copylist( s );
					       s2->list = NULL;
	        			   if( s2->link != NULL )
					       {
		   			       s3 = ( NC * )malloc( sizeof( NC ) );
		   			       s3 = copylist( s2->link );
		   			       s3->list = NULL;
		   			       M_z = ( NC * )malloc( sizeof( NC ) );
		   			       M_z->type = 'S';
		   			       M_z->inc = 1;
		      			   M_z->link = NULL;
		   			       M_z->list = NULL;
		   			       while( s3!= NULL )
		   				   {
						     if( s3->type == 'v' )
						     {
		      				   s4 = ( NC * )malloc( sizeof( NC ) );
		      				   s4->type = 'S';
		      				   s4->inc = 0;
		      				   s4->link = ( NC * )malloc( sizeof( NC ) );
		      				   s4->list = NULL;
		      			       s4->link->type = 'T';
		      				   s4->link->inc = s2->inc;
		                       s4->link->link = ( NC * )malloc( sizeof( NC ) );
		      				   s4->link->link = copylist( s3 );
		      				   s4->link->list = NULL;

	                 		   p = ( NC * )malloc( sizeof( NC ) );
	        	 			   p->type = 'v';
	        	 			   p->inc = temp_lhs;
	        	 			   p->link = NULL;
	        	 			   p->list = NULL;
	        	 			   sum =  ( NC * )malloc( sizeof( NC ) );
	        	 			   sum->type = 'S';
	        	 			   sum = copylist( temp_rhs );

	                 		   break_flag = 0;
 			 			       flag_1 = 0;
	   		 			       flag_2 = 0;

	        	 			   s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

						   } // if( s3->type == 'v' )
						   else
						   {
						        result = (NC *)malloc(sizeof(NC));
		        			    result=Substitute_in_MOD_DIV( s3, temp_path1->transformations, result );
							    s4 = ( NC * )malloc( sizeof( NC ) );
		        				s4->type = 'S';
		        				s4->inc = 0;

		        				s4->link = ( NC * )malloc( sizeof( NC ) );
		        				s4->list = NULL;
		        				s4->link->type = 'T';
		        				s4->link->inc = s2->inc;

		        				s4->link->link = ( NC * )malloc( sizeof( NC ) );
		        				s4->link->link = copylist( result );
		        				s4->link->list = NULL;

						   }

		         			   M_z = Mult_Sum_With_Sum( M_z, s4, M_z );
		         			   s3 = s3->list;
		   				} // while( s3!= NULL )
		      				s1 = Add_Sums( s1, M_z, s1 );
					    } // if( s2->link != NULL )
	        			    s = s->list;
	     				 } // while( s != NULL )
	        			 p_trans1->Assign.rhs = NULL;
					 p_trans1->Assign.rhs = copylist( s1 );

	     			      } // if( s->link != NULL )
				     p_trans1 = p_trans1->next;
				     displayApath( temp_path1 );
			          getchar( );
				  } //while( p_trans1 != NULL )
				  displayApath( temp_path1 );
			          getchar( );

				} // if( spec_flag == 1 )
				else
				 return -1;

		    //}
		    printf( "\n end of %d iteration \n", vars );
		    printf( "spec_flag = %d\n", spec_flag );
		    getchar( );
		 }
		 no_of_kripke_vars = 0;
	              no_of_spec_vars = 0;
	    	      if( r_alpha_equal(path->transformations, tempP1.paths[j].transformations, var_kripke, &no_of_kripke_vars, var_spec, &no_of_spec_vars ) )
	    	      { //tempP1.paths[j].transformations // KB replaced i with j
	      			     path->cp= &tempP1.paths[j];// KB replaced i with j
	      			     return tempP1.paths[j].end;  // equivalent path found // KB replaced i with j
	    	      }

	     } // else if( no_of_spec_vars > 0 )



	      }
	   }
	   else if( canProceed == 2 )
	   {
	        flag = addPath(path, concatedPath, tempP1, M0_kripke );
	   }

	 }
	if(flag >= 0)
           break;
   }
   return flag;
}



/***************************************************************************************/

 /* Copy path1 to path                                                                */

/**************************************************************************************/
PATH_ST*
copyPath( PATH_ST *path1 ) {

    r_alpha *temp,*temp1, *head;
    PATH_NODE *node1, *node2, *node3;
    PATH_ST *path;
    path=(PATH_ST *)malloc(sizeof(PATH_ST));
    path->start=path1->start;
    path->end=path1->end;
    path->status=0;
    path->condition=copylist(path1->condition);
    path->cp=NULL;
    path->next=NULL;
    head=(r_alpha *)malloc(sizeof(r_alpha));
    temp=path1->transformations;
    head->Assign.lhs=temp->Assign.lhs;
    head->Assign.rhs=copylist(temp->Assign.rhs);
    path->transformations=head;
    temp=path1->transformations->next;

    while( temp != NULL )
    {
		temp1=(r_alpha *)malloc(sizeof(r_alpha));
		temp1->Assign.lhs=temp->Assign.lhs;
		temp1->Assign.rhs=copylist(temp->Assign.rhs);
		temp1->next=NULL;
		head->next=temp1;
		head=head->next;
		temp1=temp1->next;
		temp=temp->next;
	}
	node1=path1->nodeslist;
	node2=(PATH_NODE *)malloc(sizeof(PATH_NODE));
	node2->state=node1->state;
	node2->condition=copylist(node1->condition);
	node2->actions=node1->actions; //may required copy the actions individuallly.
	path->nodeslist=node2;
	node1=node1->next;

	while( node1 != NULL )
	{
		node3=(PATH_NODE *)malloc(sizeof(PATH_NODE));
		node3->state=node1->state;
		node3->condition=copylist(node1->condition);
		node3->actions=node1->actions;
		node3->next=NULL;
		node2->next=node3;
		node3=node3->next;
		node2=node2->next;
		node1=node1->next;
	}
	return path;
}



/**************************************************************************************/

/*  This function tries to find an equivalent path of "path" in "fsmd". It starts
    searching from corresponding state k(index). tempP1 is the set of all paths
    from one cutpoint to another without having an intermeadiary cutpoint.
    This set will help to find actual equivalent path.
    This function returns -1 if it fails to find equivalent path otherwise return
    some value greater than equal to 0                                         */

/**************************************************************************************/

int
findcp(FSMD *fsmd, PATH_ST *path, int k, PATHS_LIST tempP1, FSMD *M0, FSMD *M0_kripke )
{
   FILE *fp1;
   PATH_ST path1, *temp_path, *temp_path1, *temp_path3;
   r_alpha *p_trans, *p_trans1, *p_trans3;
   NC *s, *s1, *s2, *s3, *s4, *M_z, *p, *sum, *temp_rhs, *temp_rhs1, *result;
   int i = 0, j, canProceed, found = 0, var_kripke[500],no_of_kripke_vars, k_state_num, vars, pid,
       flag, var_spec[500], no_of_spec_vars, start, break_flag, flag_1, flag_2, j1, spec_flag, c_sum, temp_lhs;
   char sym_value[200], fp_string[1000];

   for(i=0; i< tempP1.numpaths; i++)
   {
      if(tempP1.paths[i].start == k)
      {	//path starts from the corresponding state
         canProceed = checkCondition(path, &tempP1.paths[i]);
	 /*printf( "\n condition of path :" );
	 write_lists( path->condition );
	 printf( "\n");
	 printf( "\n condition of path1 :" );
	 write_lists( tempP1.paths[i].condition );
	 printf( "\n");
	 printf( "\n\n canproceed = %d\n\n", canProceed );*/
         if(canProceed == 0)
	 {  //conditions mismatched
	    continue;
	 }
	 else if(canProceed == 1) //conditions matched totally; check the data transformations
	 {
	    no_of_kripke_vars = 0;
	    no_of_spec_vars = 0;
	    if( r_alpha_equal(path->transformations, tempP1.paths[i].transformations, var_kripke,
                &no_of_kripke_vars, var_spec, &no_of_spec_vars ) )
	    {
	      path->cp= &tempP1.paths[i];
	      return tempP1.paths[i].end;  // equivalent path found
	    }
	    else
	    {
	      if( no_of_kripke_vars > 0 ) // for reverse speculation
	      {
		 nmodelCheck++;
		//if( list 'var_kripke' is not NULL ) we do the model checking tasks;
		//and conclude whether this particular path 'tempP1.paths[i]' is equivalent to
		//'path'.
		printf( "\n in findcp : the variables for which kripke model will be used are : ");
		for( vars = 0; vars < no_of_kripke_vars; vars++)
		{
             	   symbol_for_index( var_kripke[vars], sym_value );
	     	   printf( "\t %s ", sym_value );
		 }
		 printf( "\n" );

		 //  This function generates a kripke structurte for FSMD M0.
		 // It is a file named "file_kripke_st".
		 k_state_num = generate_kripke_st( M0_kripke );

		 //  This function generates a 'smv code' named "kripke.smv" for the 'kripke structure'
  		 // named "file_kripke_st".

		 generate_smv_code( M0_kripke, k_state_num, path->end, var_kripke, no_of_kripke_vars );

		 // the Nusmv model checker is invoked and the equivalence of this path is checked
		if ((pid = fork()) == 0)
		 {
                     FILE *fp = fopen("Nusmv.kripke.output", "w");
                     int fd = fileno(fp);
                     dup2(fd, STDOUT_FILENO);
                     close(fd);
                     execl("/home/chandan/myworkInPhD/tools/NuSMV/NuSMV-2.4.3/nusmv/NuSMV", "/home/chandan/myworkInPhD/tools/NuSMV/NuSMV-2.4.3/nusmv/NuSMV", "kripke.smv", NULL);
                     exit(0);
                 }
		 wait( pid );
		// if the equivalence is found then end state of the corresponding
		// equivalent path is returned, else, no_of_kripke_vars = 0 and continue.
		fp1 = fopen( "Nusmv.kripke.output", "r" );
		flag = 0;
		while( !feof( fp1 ) ) // In output file, if any 'false' statement is present, we conclude paths are not
                              // equivalent
		{
		    fscanf( fp1, "%s", fp_string );
		    if( strcmp( fp_string, "false" ) == 0 )
		    {
		       flag = 1;
		       no_of_kripke_vars = 0;
		       printf("\n Equivalent path not found by model checking\n");
		       break;
		    }
		}
		fclose( fp1);
		if( flag == 0 )
		{
		  path->cp= &tempP1.paths[i];
		  printf("\n Equivalent path found by model checking\n");
	          return tempP1.paths[i].end;  // equivalent path found and return
		}

	      }
	      else if( no_of_spec_vars > 0 ) // for speculation
	      {
	         nmodelCheck++;
		 for( vars = 0; vars < no_of_spec_vars; vars++)
		 {
		   // spec_flag = -1;
             	   // k = var_in_V1_minus_V0_intersection_V1( var_spec[vars] ); // this function checks if a
										// variable present in V1-(V0 intersection V1). if present k=1 else -1
		    //if( k == 1 )
		    //{
		       symbol_for_index( var_spec[vars], sym_value );
		       printf( "\n in findcp : the variables for which speculation will be used are : ");
	     	       printf( "\t %s \n", sym_value );
		       start = tempP1.paths[i].start;
		       /* in the two loops, we are trying to find the path which ends in the start node of tempP1.paths[i].start and try to find the value of the varaible var_spec[vars] in this path; if that variable does not modify in this path, then return -1 as a indication of speculation fails*/
		       for(j = 0; j < tempP1.numpaths; j++)
               		{
		          if( start == tempP1.paths[j].end )
			  {
			    temp_path = &tempP1.paths[j];
			    p_trans = temp_path->transformations;
			    spec_flag = -1;
			    while( p_trans != NULL )
			    {
			       if( p_trans->Assign.lhs == var_spec[vars] )
			       {
			          temp_lhs = p_trans->Assign.lhs;
			          temp_rhs = copylist( p_trans->Assign.rhs );
				  spec_flag = 1;
				  break;
			       }
			       p_trans = p_trans->next;
			    }
			    if( spec_flag == 1 )
			       break;
			    else
			       return -1;
			  }
			}
		//	displayApath( &tempP1.paths[j] );
		//	printf("\n j = %d, j+1 = %d\n", j, j+1 );
			if( spec_flag != 1 )
			           return -1;
			/* for the rest of the path which end in the start node of tempP1.paths[i].end, we check whether the variable  var_spec[vars] has the same value in all of the paths.*/
			for(j1 = j+1; j1 < tempP1.numpaths; j1++)
                	{
			       	    if( start == tempP1.paths[j1].end )
			  	    {
				    	spec_flag = -1;
				        temp_path3 = &tempP1.paths[j1];
			                p_trans3 = temp_path3->transformations;
			                while( p_trans3 != NULL )
			    		{
			       		   if( p_trans3->Assign.lhs == var_spec[vars] )
			       		   {
				              temp_rhs1 = copylist( p_trans->Assign.rhs );
					      c_sum = Compare_Sums( temp_rhs->link, temp_rhs1->link );
					      if( c_sum == 0 && temp_rhs->inc == temp_rhs1->inc )
					      {
					        spec_flag = 1;
						break;
					      }	// if(c_sum == 0 && temp_rhs->inc == temp_rhs1->inc)
				           }//if(p_trans3->Assign.lhs == var_spec[vars] )
					   p_trans3 = p_trans3->next;
				        }//while( p_trans3 != NULL )

					if( spec_flag == -1 )
					   return -1;
				      }//if(start == tempP1.paths[j1].end )
			  } //for(j1 = j+1; j1 < tempP1.numpaths; j1++)

			   	/* here, we are updating the r_alpha for the path tempP1.paths[i] with the vlaue of the variable var_spec[vars]. The value of this variable has been found in the paths that ends in the start node of the path  tempP1.paths[i]*/

				if( spec_flag == 1 )
				{
				  temp_path1 = &tempP1.paths[i];
			          p_trans1 = temp_path1->transformations;
				  while( p_trans1 != NULL )
				  {
				     s = ( NC * )malloc( sizeof( NC ) );
	                             s = copylist( p_trans1->Assign.rhs );
	     			     s->type = 'S';
	     			     s1 = ( NC * )malloc( sizeof( NC ) );
	     			     s1 = copylist( s );
	     			     s1->link = NULL;
	     			     s1->list = NULL;
	     			     if( s->link != NULL )
	     			     {
	     				s = s->link;
	     				while( s != NULL )
	     				{
	        			   s2 = ( NC * )malloc( sizeof( NC ) );
					   s2 = copylist( s );
					   s2->list = NULL;
	        			   if( s2->link != NULL )
					   {
		   			       s3 = ( NC * )malloc( sizeof( NC ) );
		   			       s3 = copylist( s2->link );
		   			       s3->list = NULL;
		   			       M_z = ( NC * )malloc( sizeof( NC ) );
		   			       M_z->type = 'S';
		   			       M_z->inc = 1;
		      			       M_z->link = NULL;
		   			       M_z->list = NULL;
		   			       while( s3!= NULL )
		   				{
						  if( s3->type == 'v' )
						  {
		      				   s4 = ( NC * )malloc( sizeof( NC ) );
		      				   s4->type = 'S';
		      				   s4->inc = 0;
		      				   s4->link = ( NC * )malloc( sizeof( NC ) );
		      				   s4->list = NULL;
		      			           s4->link->type = 'T';
		      				   s4->link->inc = s2->inc;
		      				   s4->link->link = ( NC * )malloc( sizeof( NC ) );
		      				   s4->link->link = copylist( s3 );
		      				   s4->link->list = NULL;

	                 		   p = ( NC * )malloc( sizeof( NC ) );
	        	 			   p->type = 'v';
	        	 			   p->inc = temp_lhs;
	        	 			   p->link = NULL;
	        	 			   p->list = NULL;
	        	 			   sum =  ( NC * )malloc( sizeof( NC ) );
	        	 			   sum->type = 'S';
	        	 			   sum = copylist( temp_rhs );

	                 		   break_flag = 0;
 			 			       flag_1 = 0;
	   		 			       flag_2 = 0;

	        	 			   s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

						   } // if( s3->type == 'v' )
						   else
						   {
						        result = (NC *)malloc(sizeof(NC));
								result=Substitute_in_MOD_DIV(s3,temp_path1->transformations, result );
								s4 = ( NC * )malloc( sizeof( NC ) );
								s4->type = 'S';
		        				s4->inc = 0;

		        				s4->link = ( NC * )malloc( sizeof( NC ) );
		        				s4->list = NULL;
		        				s4->link->type = 'T';
		        				s4->link->inc = s2->inc;

		        				s4->link->link = ( NC * )malloc( sizeof( NC ) );
		        				s4->link->link = copylist( result );
		        				s4->link->list = NULL;

						   }

		         			   M_z = Mult_Sum_With_Sum( M_z, s4, M_z );
		         			   s3 = s3->list;
		   				} // while( s3!= NULL )
		      				s1 = Add_Sums( s1, M_z, s1 );
					    } // if( s2->link != NULL )
	        			    s = s->list;
	     				 } // while( s != NULL )
	        			 p_trans1->Assign.rhs = NULL;
					 p_trans1->Assign.rhs = copylist( s1 );

	     			      } // if( s->link != NULL )
				     p_trans1 = p_trans1->next;
				     displayApath( temp_path1 );
			          getchar( );
				  } //while( p_trans1 != NULL )
				  displayApath( temp_path1 );
			          getchar( );
				} // if( spec_flag == 1 )
				else
				 return -1;

		    //}
		    printf( "\n end of %d iteration \n", vars );
		    printf( "spec_flag = %d\n", spec_flag );
		    getchar( );
		 }
		 no_of_kripke_vars = 0;
	              no_of_spec_vars = 0;
	    	      if( r_alpha_equal(path->transformations, tempP1.paths[i].transformations, var_kripke, &no_of_kripke_vars, var_spec, &no_of_spec_vars ) )
	    	      {
	      			     path->cp= &tempP1.paths[i];
	      			     return tempP1.paths[i].end;  // equivalent path found
	    	      }

	     } // else if( no_of_spec_vars > 0 )
	  }
	}
	else  // canProceed == 2
	{
	   path1=tempP1.paths[i];
	   found = addPath( path, &path1, tempP1, M0_kripke );
	   if(found >= 0)
	   {
	      return found;
	   }
        }

     }// end if(tempP1.paths[i].start == k)

   } // end for(i=0; i< tempP1.numpaths; i++)

  if(i == tempP1.numpaths)
  {
     path->cp=NULL;
     return -1;
  }

}



/****************************************************************************************/

/*        This function concat the path temppath with all its successor
          (endnode of temppath=startnode of the successor) paths in p0 of M0 and
          put them all in queue F */

/****************************************************************************************/

void
extendedPaths(PATHS_LIST *p0,PATH_ST *temppath,FSMD *M0,PATH_Q *F, PATHS_LIST *tF)
{
  PATH_NODE *temp,*nodelist,*head,*prev,*temp1;
  PATH_ST *beta;
  int endstate, i;


  tF->numpaths=0;
  tF->fsmd=M0;
  beta=temppath;
  endstate=beta->end;
//  write_lists(temppath->condition);

  for(i=0;i<p0->numpaths;i++) //for every paths of p0, this loop checks which one is the successor of
    //beta(temppath)
    {
      if(endstate == p0->paths[i].start) // if successor then create the concatenated path and put in
	//the list tF.
	{
	  //tF.paths[tF.numpaths]=(PATH_ST*)malloc(sizeof(PATH_ST));
	  /* create a new concatenated path */
	  tF->paths[tF->numpaths].start=beta->start;
	  tF->paths[tF->numpaths].end=p0->paths[i].end;
	  tF->paths[tF->numpaths].status=0;
	  tF->paths[tF->numpaths].cp=NULL;
	  tF->paths[tF->numpaths].next=NULL;
	  tF->paths[tF->numpaths].condition=NULL;
	  tF->paths[tF->numpaths].transformations=NULL;

	  temp1=(PATH_NODE*)malloc(sizeof(PATH_NODE));
	  temp1->state=beta->nodeslist->state;
	 // write_lists(beta->nodeslist->condition);
	  temp1->condition=NULL; //beta->nodeslist->condition;
	  temp1->actions=NULL; //beta->nodeslist->actions;
	  temp1->next=NULL;
	  temp1->left=NULL;

	  prev=temp1;
	  head=prev;
	//  write_lists(prev->condition);
	  nodelist=beta->nodeslist->next;

	  while(nodelist !=NULL) // copy node information of temppath(beta)
            {
              prev->next=(PATH_NODE*)malloc(sizeof(PATH_NODE));
              temp=prev;
              prev=prev->next;
              prev->state=nodelist->state;
	      prev->condition=copylist(nodelist->condition);

	      prev->actions=nodelist->actions;
	      prev->next=NULL;
	      prev->left=temp;
              nodelist=nodelist->next;
            }

	  tF->paths[tF->numpaths].nodeslist=head;

	  nodelist=p0->paths[i].nodeslist->next;
	  while(nodelist !=NULL) // copy information from the successor path
            {
              prev->next=(PATH_NODE*)malloc(sizeof(PATH_NODE));
              temp=prev;
              prev=prev->next;
              prev->state=nodelist->state;
	      prev->condition=copylist(nodelist->condition);

	      prev->actions=nodelist->actions;
	      prev->next=NULL;
	      prev->left=temp;
              nodelist=nodelist->next;
            }

	if( tF->paths[tF->numpaths].end==beta->start || tF->paths[tF->numpaths].end==p0->paths[0].start )
	      tF->paths[tF->numpaths].extendible=0;

	 tF->numpaths+=1;
	}
    }



     for(i=0;i<tF->numpaths;i++)
     {
	 nodelist=tF->paths[i].nodeslist;
	 nodelist=nodelist->next;
	 while(nodelist != NULL)
	 {
	     if(tF->paths[i].end == nodelist->state)
	     {
		tF->paths[i].extendible=0;
		break;
	     }

	     nodelist=nodelist->next;
	 }

	 /*printf( "\n in extended paths : \n");
	 printf( "\n display node information of the path : \n");
         nodelist=tF->paths[i].nodeslist;
	 nodelist=nodelist->next;
	 while( nodelist != NULL )
	   {
             printf( "\n");
	     write_lists( nodelist->condition);
	     printf( "\n");
	     j = 0;
	     while( nodelist->actions[j].rhs != NULL )
	       {

		 symbol_for_index( nodelist->actions[j].lhs, sym_value );
		 printf("\n%s  :=  ", sym_value );
		 write_lists( nodelist->actions[j].rhs );
		 printf("\n");
                 j++;
	       }
	     nodelist = nodelist->next;
	   }
	 displayApath( &tF->paths[i] );
	 printf( "\n");
	 getchar( );*/
      }

     // compute R_\alpha and r_\alpha of each concatenated path in tF.
     Associate_R_and_r_alpha( tF );
     //displayallpaths( tF );
     //getchar( );
}


/****************************************************************************************/

/*        This function finds the path cover of M0 and try to find an equivalent path in
          M1 for every path of the path cover. Finding the paths of the path cover as well
          finding the equivalent path is going on simultaneously  */

/****************************************************************************************/

void
equivalenceChecking_m0_to_m1(FSMD M0, FSMD M1, PATHS_LIST p0, PATHS_LIST p1, CPS_Q P, PATH_Q F, CPQ *eta, FSMD M0_kripke )
{
        CPS tempcs;
        CPQ *eta1;
        eta1=eta;
        PATH_ST beta;
	    PATHS_LIST tF;
        int i;

        while( !emptyF(&F) || !emptyP(&P) )
	{ //loop until both F and P is empty

	    if( emptyF(&F) )
	    {
	    /* if no path remains in F then consider the next corresponding pair in P.
	     * Put all paths starting from  s0 (let <s0, s1> is the corresponding state) in F
	     */


		printf( "\n F is empty \n" );
		deQP( &P, &tempcs );
		for( i = 0; i < p0.numpaths; i++ )
		{
		    if( p0.paths[i].start == tempcs.s0 )
		    {
			enQF( &F, &p0.paths[i] );
		     }
		  }
	      }
	    else
	    { //F is not empty

		beta = deQF( &F ); // beta is the next path for which we have to find an equivalent path
		count_iteration++;

		findEquivalent( &beta, &M1, &M0, eta1, p1, &M0_kripke );
		/* function for searching an equivalent path of beta in M1*/


		if( beta.cp != NULL )
		{ // equivalent path found

		    if( is_in_CPQ( eta1, beta.end ) < 0 )
		    {
		    /* Check whether endpathNode of beta is already in  eta.
		     * If not ( is_in_end returns value less than 0),
		     * put <beta.end, alpha.end> in P. otherwise corresponding pair
		     * contains endpathNode of beta is already considered or
		     * waiting for its turn in F
		     */
            enQP( &P, beta.end, beta.cp->end );
			unionEta( eta, beta.end, beta.cp->end );
                      }
		  }
		else
		{ // no equivalent path found

		      count_extend++;
		      extendedPaths( &p0, &beta, &M0,&F, &tF );
		      //extend beta with its all successors in p0

                      if( tF.numpaths > 0 )
                      {
	 	          for(i=0;i<tF.numpaths;i++)
			  {
		      	    enQF(&F,&tF.paths[i]);
		          }
		      }
		      else
		      {
		          printf( "\n No extended paths found \n" );
			//  printf( "\n FSMD's are NOT EQUIVALENT \n" );
			  break;
		      }

		  }
	      }  // end else(empty(&F)

	  }  // end while(!emptyF(&F) || !emptyP(&P))
	eta1 = eta;
	if(emptyF(&F) && emptyP(&P))
	{
		while( eta1 != NULL )
		{
	    		printf( "   ( %s, %s )  ", M0.states[eta1->s0].state_id, M1.states[eta1->s1].state_id );
	    		eta1 = eta1->next;
	 	}
		printf("\n\n\n\n\n\n FSMDs ARE Equivalent \n");
	}
	else
		printf("\n\n\n\n\n FSMDs ARE NOT Equivalent \n\n");
}




/* for P and F if front>end then empty. during enQ, increase(check overflow) end then enter; during deQ,
   increase enter(first check Q is not empty); Intially, front=0,end=-1*/

/****************************************************************************************/

/*        This function will initialize the front and end value of the queue of paths   */

/****************************************************************************************/

void initF(PATH_Q *F)
{
        F->front=0;
        F->end=-1;
}

/****************************************************************************************/

/*        This function will initialize the front and end value of the queue of
          Corresponding state pairs */

/****************************************************************************************/

void
initP(CPS_Q *P)
{
        P->front=0;
        P->end=-1;
}

/****************************************************************************************/

/*        This function will put the reset state of two FSMD in first node of the linklist
          of corresponding states. Returns this node */

/****************************************************************************************/

CPQ*
initEta()
{
        CPQ *eta;
        eta=(CPQ*)malloc(sizeof(CPQ));
        eta->s0=0;
    	eta->s1=0;
	eta->next=NULL;
        return eta;
}

/****************************************************************************************/

/*        This function checks whether queue of corresponding state is empty or not.
          Returns 1 if empty otherwise 0    */

/****************************************************************************************/

int
emptyP(CPS_Q *P) {
  if(P->front > P->end)
       return 1;
  else
       return 0;
}

/****************************************************************************************/

/*        This function checks whether queue of paths is empty or not.
          Returns 1 if empty otherwise 0                            */

/****************************************************************************************/

int
emptyF(PATH_Q *F)
{
  if(F->front > F->end)
     return 1;
  else
     return 0;
}


/****************************************************************************************/

/*       This function put path p0 in the queue F                        */

/****************************************************************************************/

void
enQF(PATH_Q *F,PATH_ST *p0)
{
   if(F->end==(QUEUESIZE-1))
   {
          printf("\n\n Queue FULL(F): Exiting system");
          exit(0);
   }
   else
   {
       F->end+=1;
       F->paths[F->end].start=p0->start;
       F->paths[F->end].end=p0->end;
       F->paths[F->end].status=p0->status;
       F->paths[F->end].extendible=p0->extendible;
       F->paths[F->end].condition=copylist(p0->condition);
       F->paths[F->end].transformations=p0->transformations;
       F->paths[F->end].nodeslist=p0->nodeslist;
       F->paths[F->end].cp=p0->cp;
       F->paths[F->end].next=p0->next;
     }
}

/****************************************************************************************/

/*       This function put corresponding state pair <s0, s1> in the queue P             */

/****************************************************************************************/

void
enQP(CPS_Q *P,int s0,int s1)
{
   if(P->end==(QUEUESIZE-1))
   {
          printf("\n\n Queue FULL(P): Exiting system");
          exit(0);
     }
   else
   {
       P->end+=1;
       P->cstate[P->end].s0=s0;
       P->cstate[P->end].s1=s1;
     }
}

/****************************************************************************************/

/*       This function delete the front element from queue of corresponding state P.
         and put it in temp          */

/****************************************************************************************/

void
deQP( CPS_Q *P,CPS *temp )
{
   if(emptyP(P))
   {
        printf("\n Queue(P) underflow : Exiting system\n");
        exit(0);
     }
   else
   {
       temp->s0=P->cstate[P->front].s0;
       temp->s1=P->cstate[P->front].s1;
       P->front=P->front+1;
     }
}


/****************************************************************************************/

/*       This function delete the front element from queue of paths F.
         and returns this path          */

/****************************************************************************************/

PATH_ST
deQF(PATH_Q *F) {
  PATH_ST f;
  if(emptyF(F)) {
        printf("\n Queue(F) underflow : Exiting system\n");
        exit(0);
     }
  else {
	 f=F->paths[F->front];
         F->front=F->front + 1;
	 return f;
     }
}



/****************************************************************************************/

/*       This function checks whether pair <s0, s1> is in eta or not. if it is not present
         in eta then add the pair at the end of it    */

/****************************************************************************************/

void
unionEta(CPQ *eta, int s0, int s1) {
   CPQ *temp,*temp1,*temp2;
   int found=0;
   temp=eta;
   while(temp !=NULL) { // checks for <s0, s1> in eta
     if( temp->s0 == s0 && temp->s1 == s1 ) {
         found=1;
         break;
     }
     temp2=temp;
     temp=temp->next;
  }
  if(!found)  { // found=0 indicates <s0, s1> is not in eta
     temp1=(CPQ*)malloc(sizeof(CPQ));
     temp1->s0=s0;
     temp1->s1=s1;
     temp1->next=NULL;
     temp2->next=temp1;
  }

}



//// normal.c

int
compare_trees(NC *t1,NC *t2)
{
   if(t1!=NULL&&t2!=NULL)
   {
	  //KB start
	  if(t1->type == 0) t1->type = 'S';
	  if(t2->type == 0) t2->type = 'S';
	  //KB end
      if(t1->type==t2->type&&t1->inc==t2->inc)
		return compare_trees(t1->link,t2->link)&compare_trees(t1->list,t2->list);
	  //KB start
	  /*else
	    printf("%c %d <--> %c %d", t1->type, t1->inc, t2->type, t2->inc);*/
	  //KB end
   }
   else if(t1==NULL&&t2==NULL)
      return 1;

   return 0;
}





/* This function adds two normalized sums and returns a normalized sum
 */
N_sum *
Add_Sums( N_sum *sum1, N_sum *sum2, N_sum *add_result)
{
  if( sum1 == NULL && sum2 == NULL )
    return NULL;
  if( sum1 == NULL && sum2 != NULL )
  {
    add_result = copylist( sum2 );
    add_result->list = NULL;
  }
  if( sum1 != NULL && sum2 == NULL )
  {
    add_result = copylist( sum1 );
    add_result->list = NULL;
  }
  if( sum1 != NULL && sum2 != NULL )
  {
	   add_result->type = 'S'; //KB Jun 7, 2013
       add_result->inc = sum1->inc + sum2->inc;
       add_result->list = NULL;
       add_result->link = Add_Sums_1( sum1->link, sum2->link, add_result->link );
  }
  return add_result;
}




/* This function works on the suffixes s1 and s2 of two  normalized sums
 * to return their resultant normalized sum.
 * This function adds the terms of the normalized sum s1 with the
 * terms of the normalized sum s2 and returns the resultant normalized sum.
 */

N_sum *
Add_Sums_1( N_sum *sum1, N_sum *sum2, N_sum *add_result )
{
  int m;

  if( sum1 == NULL && sum2 == NULL )
    return NULL;
  if( sum1 == NULL && sum2 != NULL )
  {
    add_result = copylist( sum2 );
    add_result->type = 'T';
    add_result->list = Add_Sums_1( NULL, sum2->list, add_result->list );
  }
  if( sum1 != NULL && sum2 == NULL )
  {
    add_result = copylist( sum1 );
    add_result->type = 'T';
    add_result->list = Add_Sums_1( sum1->list, NULL, add_result->list );
  }

  if( sum1 != NULL && sum2 != NULL )
  {
    m = Compare_Terms( sum1->link, sum2->link );

    if( m == 0 )
    {
       if( sum1->inc + sum2->inc != 0 )
       {
          add_result = copylist( sum2 );
          add_result->type = 'T';
          add_result->inc = sum1->inc + sum2->inc;
          add_result->list = Add_Sums_1( sum1->list, sum2->list, add_result->list );
       }
       else
       {
          add_result = copylist( sum2 );
	      add_result->type = 'T';
	      add_result->link = NULL;
	      add_result->list = NULL;
          add_result = Add_Sums_1( sum1->list, sum2->list, add_result );
       }
    }
    else if( m < 0 )
    {
      add_result = copylist( sum1 );
      add_result->type = 'T';
      add_result->list = Add_Sums_1( sum1->list, sum2, add_result->list );
    }
    else
    {//if( m > 0 )
      add_result = copylist( sum2 );
      add_result->type = 'T';
      add_result->list = Add_Sums_1( sum1, sum2->list, add_result->list );
    }
  }
  return add_result;
}



/* This function compares the two terms in the style of string compare
 * Here t1 is the pointer to the first primary and t2 is the pointer to the
 * first primary of the term t1 and t2 respectively
 */

int
Compare_Terms( N_term *t1, N_term *t2 )
{

 int v;

 if( t1 == NULL && t2 == NULL )
   return 0;
 if( t1 == NULL && t2 != NULL )
   return -1;
 if( t1 != NULL && t2 == NULL )
   return 1;

 v = Compare_Primaries( t1, t2 );
 if( v == 0 )
   v = Compare_Terms( t1->list, t2->list );
 else if( v < 0 )
   return -1;
 else
   return 1;

 return v;

}




/* This function compares the two primaries in the style of string compare
 */
int
Compare_Primaries( N_primary *primary1, N_primary *primary2 )
{
  int m, m1, m2;

  //updating Compare_Primaries
  //with precedence order D,M > w,a > v

  if( ( primary1->type == 'D' || primary1->type == 'M' ) && ( primary2->type == 'D' || primary2->type == 'M' ) )
  {   if( primary1->type == primary2->type )
      {
      m1 = Compare_Sums( primary1->link->link, primary2->link->link );
      m2 = Compare_Sums( primary1->link->list->link, primary2->link->list->link );
      if( m1 == 0 && ( primary1->link->inc ==  primary2->link->inc ) )
        m1 = 0;
      else if( m1 == 0 &&  ( primary1->link->inc <  primary2->link->inc ) )
        m1 = -1;
      else if( m1 == 0 && ( primary1->link->inc >  primary2->link->inc ) )
        m1 =  1;
      else if( m1 < 0 )
        m1 = -1;
      else if( m1 > 0 )
        m1 = 1;

      if( m2 == 0 && ( primary1->link->list->inc ==  primary2->link->list->inc ) )
        m2 = 0;
      else if( m2 == 0 &&  ( primary1->link->list->inc <  primary2->link->list->inc ) )
        m2 = -1;
      else if( m2 == 0 && ( primary1->link->list->inc >  primary2->link->list->inc ) )
        m2 =  1;
      else if( m2 < 0 )
        m2 = -1;
      else if( m2 > 0 )
        m2 = 1;

      switch( m1 )
      {
        case 0:
           if( m2 == 0 )
	         m = 0;
	       else if( m2 == -1 )
	         m = -1;
	       else if( m2 == 1 )
	         m = 1;
           break;
	    case -1:
	       m = -1;
	       break;
	    case 1:
	       m = 1;
	       break;
      }
      return 1;

      } //
  }
  //KB: array start
  else if( ( primary1->type == 'D' || primary1->type == 'M' ) )
  {
      return 1;
  }
  else if( ( primary1->type == 'w' ) && ( primary2->type == 'D' || primary2->type == 'M' ) )
  {
      return -1;
  }
  else if( primary1->type == 'w' && primary2->type == 'w' )
  {
      m1 = Compare_Sums( primary1->link->link, primary2->link->link );
      m2 = Compare_Sums( primary1->link->list->link, primary2->link->list->link );
      if( m1 == 0 && ( primary1->link->inc ==  primary2->link->inc ) )
        m1 = 0;
      else if( m1 == 0 && ( primary1->link->inc <  primary2->link->inc ) )
        m1 = -1;
      else if( m1 == 0 && ( primary1->link->inc >  primary2->link->inc ) )
        m1 =  1;
      else if( m1 < 0 )
        m1 = -1;
      else if( m1 > 0 )
        m1 = 1;

      if( m2 == 0 && ( primary1->link->list->inc ==  primary2->link->list->inc ) )
        m2 = 0;
      else if( m2 == 0 && ( primary1->link->list->inc <  primary2->link->list->inc ) )
        m2 = -1;
      else if( m2 == 0 && ( primary1->link->list->inc >  primary2->link->list->inc ) )
        m2 =  1;
      else if( m2 < 0 )
        m2 = -1;
      else if( m2 > 0 )
        m2 = 1;

      switch( m1 )
      {
        case 0:
           if( m2 == 0 )
	         m = 0;
	       else if( m2 == -1 )
	         m = -1;
	       else if( m2 == 1 )
	         m = 1;
           break;
	    case -1:
	       m = -1;
	       break;
	    case 1:
	       m = 1;
	       break;
      }
      return m;
  }
  else if( primary1->type == 'a' && primary2->type == 'a' )
  {
	  if ( primary1->inc < primary2->inc )
       return -1;
      else if( primary1->inc > primary2->inc )
       return 1;

	  m = Compare_Sums( primary1->link, primary2->link );

      return m;
  }
  else if( primary1->type == 'a' && primary2->type == 'w' )
  {
      if ( primary1->inc < primary2->link->inc )
       return -1;
      else if( primary1->inc > primary2->link->inc )
       return 1;

	  m = Compare_Sums( primary1->link, primary2->link->link );

      return m;
  }
  else if( primary1->type == 'w' && primary2->type == 'a' )
  {
      if ( primary1->link->inc < primary2->inc )
       return -1;
      else if( primary1->link->inc > primary2->inc )
       return 1;

	  m = Compare_Sums( primary1->link->link, primary2->link );

      return m;
  }
  else if( primary1->type == 'w' && primary2->type == 'v' )
  {
	  return 1;
  }
  else if( primary1->type == 'v' && primary2->type == 'w' )
  {
	  return -1;
  }
  else if( primary1->type == 'a' && primary2->type == 'v' )
  {
	  return 1;
  }
  else if( primary1->type == 'v' && primary2->type == 'a' )
  {
	  return -1;
  }
  //KB: array end
  else //  ( primary1->type == 'v' ) && ( primary2->type == 'v' )
  {
    if( primary1->inc == primary2->inc )
       return 0;
    else if ( primary1->inc < primary2->inc )
       return -1;
    else
       return 1;
  }
}



/* This function multiplies a normalized term 't1' with another normalized
 * term 't2' and returns the resultant normalized term.
 */

N_term *
Mult_Term_With_Term( N_term *t1, N_term *t2, N_term *mult_TT )
{
  if( t1 == NULL && t2 == NULL )
    return NULL;
  if( t1 == NULL && t2 != NULL )
  {
    mult_TT->inc = t2->inc;
    mult_TT->link = ( NC * )malloc( sizeof( NC ) );
    mult_TT->link = Mult_Term_With_Term_1( NULL, t2->link, mult_TT->link);
  }
  if( t1 != NULL && t2 == NULL )
  {
    mult_TT->inc = t1->inc;
    mult_TT->link = ( NC * )malloc( sizeof( NC ) );
    mult_TT->link = Mult_Term_With_Term_1( t1->link, NULL, mult_TT->link);
  }
  if( t1 != NULL && t2 != NULL )
  {
    mult_TT->inc = t1->inc * t2->inc;
    mult_TT->link = ( NC * )malloc( sizeof( NC ) );
    mult_TT->link = Mult_Term_With_Term_1( t1->link, t2->link, mult_TT->link);
  }
  return mult_TT;
}



/* This function multiplies all the primaries of a normalized term 't1'
 * with all the primaries of another normalized  term 't2' and returns
 * the resultant normalized term.
 */
N_term *
Mult_Term_With_Term_1( N_term *t1, N_term *t2, N_term *mult_TT )
{
 int v;

 if( t1 == NULL && t2 == NULL )
   return NULL;
 if( t1 == NULL && t2 != NULL )
 {
    mult_TT = copylist( t2 );
    mult_TT->list = ( NC * )malloc( sizeof( NC ) );
    mult_TT->list = Mult_Term_With_Term_1( NULL, t2->list, mult_TT->list);
  }
 if( t1 != NULL && t2 == NULL )
 {
    mult_TT = copylist( t1 );
    mult_TT->list = ( NC * )malloc( sizeof( NC ) );
    mult_TT->list = Mult_Term_With_Term_1( t1->list, NULL, mult_TT->list);
  }
 if( t1 != NULL && t2 != NULL )
 {
   v = Compare_Primaries( t1, t2 );
   if( v <= 0 )
   {
     mult_TT = copylist( t1 );
     mult_TT->list = ( NC * )malloc( sizeof( NC ) );
     mult_TT->list = Mult_Term_With_Term_1( t1->list, t2, mult_TT->list );
   }
   else
   {
     mult_TT = copylist( t2 );
     mult_TT->list = ( NC * )malloc( sizeof( NC ) );
     mult_TT->list = Mult_Term_With_Term_1( t1, t2->list, mult_TT->list );
   }
 }
 return mult_TT;
}



/*  This function multiplies a normalized sum with a normalized term and
 *  returns  the resultant normalized sum.
 */

N_sum *
Mult_Sum_With_Term( N_sum *sum, N_term *t, N_sum *result )
{

  N_term *term1;
  N_sum *sum2;

  if( sum == NULL && t == NULL )
    return NULL;
  if( sum == NULL && t != NULL )
    return NULL;
  if( sum != NULL && t == NULL )
    return NULL;

  if( sum != NULL && t != NULL )
  {
    result = ( NC * )malloc( sizeof( NC ) );
    result->type = 'S';
    result->inc = 0;
    result->list = NULL;

    term1 = ( NC * )malloc( sizeof( NC ) );
    term1 = copylist( t );

    term1->inc = sum->inc * t->inc;
    sum2 = ( NC * )malloc( sizeof( NC ) );
    sum2->type = 'T';
    sum2->list = NULL;

    if( sum->link != NULL )
    {
       sum2 = Mult_Sum_With_Term_1( sum->link, t, sum2 );

    }
    else
    {
      result->link = copylist( term1 );
      return result;
    }
    if( sum->inc != 0 )
    {
      result->link = Add_Sums_1( term1, sum2, result->link );
      return result;
    }
    else
    {
	result->link = copylist( sum2 );
	return result;
    }
  }
  return result;
}



/*  This function  multiplies all the terms of a normalized sum with a
 *  normalized term , and returns the resultant normalized sum.
 */

N_sum *
Mult_Sum_With_Term_1( N_sum *sum, N_term *t, N_sum *result )
{
  N_term *term1, *term2;


  if( sum == NULL && t == NULL )
    return NULL;
  if( sum == NULL && t != NULL )
    return NULL;
  if( sum != NULL && t == NULL )
    return NULL;
  if( sum != NULL && t != NULL )
  {
    term1 = ( NC * )malloc( sizeof( NC ) );
    term1->type = 'T';

    term2 = ( NC * )malloc( sizeof( NC ) );
    term2->type = 'T';
    term2 = copylist( sum );
    term2->list = NULL;

    term1 = Mult_Term_With_Term( term2, t, term1 );

    result= ( NC * )malloc( sizeof( NC ) );
    result->type = 'T';
    result = Add_Sums_1( term1, Mult_Sum_With_Term_1( sum->list, t, result->list ), result );
    return result;
  }
}



/*  This function multiplies a normalized sum 'sum1' with another normalized
 *  sum 'sum2' and returns  the resultant normalized sum.
 *
 */
N_sum *
Mult_Sum_With_Sum( N_sum *sum1, N_sum *sum2, N_sum *result )
{
  N_sum *temp1, *temp2;

  if( sum1 == NULL && sum2 == NULL )
    return NULL;
  if( sum1 != NULL && sum2 == NULL )
    return NULL;
  if( sum1 == NULL && sum2 != NULL )
    return NULL;

  if( sum1 != NULL && sum2 != NULL )
  {


     if( sum1->link == NULL )
     {
        //Mult_sum_with_constant
        result = ( NC *) malloc( sizeof( NC ) );
        result->type = 'S';
        result->list = NULL;
        result = Mult_sum_with_constant( sum2, sum1->inc, result );
        return result;
     }
    else
    {
       if( sum2->inc != 0 )
       {
          temp1 = ( NC *) malloc( sizeof( NC ) );
          temp1->type = 'S';
          temp1->list = NULL;
          temp1 = Mult_Sum_With_Sum_1( sum1, sum2->link, temp1 );

          temp2 = ( NC *) malloc( sizeof( NC ) );
          temp2->type = 'S';
          temp2->list = NULL;
          temp2 = Mult_sum_with_constant( sum1, sum2->inc, temp2 );

          result = ( NC *) malloc( sizeof( NC ) );
          result->type = 'S';
          result->list = NULL;
          result = Add_Sums( temp1, temp2, result );
      }
      else
      {
         result = ( NC *) malloc( sizeof( NC ) );
         result->type = 'S';
         result->list = NULL;
         result = Mult_Sum_With_Sum_1( sum1, sum2->link, result );
     }
   }
  }

  return result;
}




/*  This function multiplies all the terms of the normalized sum 'sum2'
 *  with the normalized sum 'sum1' and returns the resultant normalized sum.
 */
N_sum *
Mult_Sum_With_Sum_1( N_sum *sum1, N_sum *sum2, N_sum *result ){

NC * term1, *temp1;

 if( sum1 == NULL && sum2 == NULL )
    return NULL;
  if( sum1 != NULL && sum2 == NULL )
    return NULL;
  if( sum1 == NULL && sum2 != NULL )
    return NULL;

  if( sum1 != NULL && sum2 != NULL ){

    term1 = ( NC *) malloc( sizeof( NC ) );
    term1 = copylist( sum2 );
    term1->list = NULL;

    temp1 = ( NC *) malloc( sizeof( NC ) );
    temp1->type = 'S';
    temp1 = Mult_Sum_With_Term( sum1, term1, temp1 );


    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'S';

    result = Add_Sums( temp1, Mult_Sum_With_Sum_1( sum1, sum2->list, result->list ), result );

  }

   return result;
}




/*  This function multiplies a normalized sum with a constant and returns
 *  the resultant  normalized sum.
 */

N_sum *
Mult_sum_with_constant( N_sum *sum, int n, N_sum *result ){

  if( sum == NULL )
    return NULL;

  if( sum->inc != 0 ){

    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'S';
    result->inc = sum->inc * n;

    result->link = ( NC *) malloc( sizeof( NC ) );
    result->link->type = 'T';
    result->link = Mult_sum_with_constant_1( sum->link, n, result->link );

  }
  else {

    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'S';
    result->inc = 0;

    result->link = ( NC *) malloc( sizeof( NC ) );
    result->link->type = 'T';
    result->list = NULL;
    result->link = Mult_sum_with_constant_1( sum->link, n, result->link );
  }
  return result;
}




/*  This function multiplies all the terms of a normalized sum with a
 *  constant and returns the resultant sum of terms
 */
N_sum *
Mult_sum_with_constant_1( N_sum *sum, int n, N_sum  *result )
{
  if( sum == NULL )
    return NULL;
  else
  {
    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'T';
    result = copylist( sum );
    result->inc = sum->inc * n ;
    result->list = ( NC *) malloc( sizeof( NC ) );
    result->list->type = 'T';
    result->list = Mult_sum_with_constant_1( sum->list, n, result->list );
  }
  return result;
}




/*  This function multiplies a constant with a normalized term, and returns
 *  the resultant normalized term.
 */
N_term *
Mult_constant_with_term( int n, N_term *t, N_term *result )
{
  result = ( NC *) malloc( sizeof( NC ) );
  result->type = 'T';
  result = copylist( t );
  result->inc = t->inc * n;
  return result;
}



/*  This function multiplies a primary with a normalized term and returns
 *  the resultant normalized term.
 */
N_term *
Mult_primary_with_term( N_primary *p, N_term *t, N_term *result )
{

  if( p == NULL && t == NULL )
    return NULL;

  if( p != NULL && t == NULL )
  {
    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'T';
    result->list = NULL;
    result->inc = 1;
    result->link = ( NC *) malloc( sizeof( NC ) );
    result->link = Mult_primary_with_term_1( p, NULL, result->link );
  }

  if( p != NULL && t != NULL )
  {
    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'T';
    result->list = NULL;
    result->inc = t->inc;
    result->link = Mult_primary_with_term_1( p, t->link, result->link );
  }
  return result;
}





/*  This function multiplies all the primaries of the term 't'  with
 *  the primary 'p' and retuns the resultant normalized term.
 */
N_term *
Mult_primary_with_term_1( N_primary *p, N_term *t, N_term *result ){

  int v;

  if( p == NULL && t == NULL )
    return NULL;


  if( p == NULL && t != NULL ){

    result = ( NC *) malloc( sizeof( NC ) );
    result = copylist( t );

    result->list = ( NC *) malloc( sizeof( NC ) );
    result->list = Mult_primary_with_term_1( NULL, t->list, result->list );
  }


 if( p != NULL && t == NULL ){

   result = ( NC *) malloc( sizeof( NC ) );
   result = copylist( p );


   result->list = ( NC *) malloc( sizeof( NC ) );
   result->list = Mult_primary_with_term_1( NULL, NULL, result->list );
  }

 if( p != NULL && t != NULL ){

    v = Compare_Primaries( p, t );

    if( v <= 0 )
    {
      result = ( NC *) malloc( sizeof( NC ) );
      result = copylist( p );

      result->list = ( NC *) malloc( sizeof( NC ) );
      result->list = Mult_primary_with_term_1( NULL, t, result->list );
    }
    else
    {
      result = ( NC *) malloc( sizeof( NC ) );
      result = copylist( t );

      result->list = ( NC *) malloc( sizeof( NC ) );
      result->list = Mult_primary_with_term_1( p, t->list, result->list );
    }
  }
 return result;
}



/*  This function works on the term 't' , a primary 'x' , a normalized sum
 * 's', a normalized term 'p' , a normalized term 'q' , and another
 * normalized sum 'z' .
 * Here 'x' is to be replaced for by 's' in 't' , 'p' is formed by multiplying
 * the constant and the other primaries that do not match 'x' , and 'q' is
 * formed by multiplying all the primaries from 't' that match 'x' ; for
 * every primary in 'q' , 'z' is multiplied by 's' and then 'p' is multiplied
 * to 'z' and returned .
 * Initially it should be invoked with { p = NULL , q = NULL , z = 1 }
 */

N_sum *
Replace_Primary( N_term *t, N_primary *x, N_sum *s, N_term *p, N_term *q, N_sum *z, N_sum *result, int *break_flag, int *flag_1, int *flag_2  )
{

  NC *p1;

  if( t == NULL )
    return NULL;

  if( t != NULL )
  {
    p1 = ( NC *) malloc( sizeof( NC ) );
    p1->type = 'T';
    p1->inc = t->inc;
    p1->list = NULL;
    p1->link = NULL;

    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'S';
    result->list = NULL;
    result = Replace_Primary_1( t->link, x, s, p1, q, z, result, break_flag, flag_1, flag_2  );
    return result;
  }
  return NULL;
}



/* This function works on all the primaries of the term 't' , a primary 'x' ,
 * a normalized sum  's', a normalized term 'p' , a normalized term 'q' ,
 * and another normalized sum 'z' .
 */

N_sum *
Replace_Primary_1( N_term *t, N_primary *x, N_sum *s, N_term *p, N_term *q, N_sum *z, N_sum *result , int *break_flag, int *flag_1, int *flag_2  )
{
  int v;
  N_sum *z1;
  N_term *q1, *p1, *term;
  N_primary *pr;

  if( t == NULL && q == NULL )
  {
    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'S';
    result->list = NULL;

    term = ( NC *) malloc( sizeof( NC ) );
    term->type = 'T';
    term->inc = p->inc ;
    term->list = NULL;
    term->link = copylist( p->link );

    if( term->link != NULL )
    {
        result = Mult_Sum_With_Term( z, term, result );
    }
    else
    {
        result = Mult_sum_with_constant( z, term->inc, result );
    }
    return result;
  }

 if( t == NULL && q != NULL )
 {
   z1 = ( NC *) malloc( sizeof( NC ) );
   z1->type = 'S';
   z1->list = NULL;
   z1 = Mult_Sum_With_Sum( z, s, z1 );

   result = ( NC *) malloc( sizeof( NC ) );
   result->type = 'S';
   result->list = NULL;
   result = Replace_Primary_1( NULL, x, s, p, q->list, z1, result , break_flag, flag_1, flag_2  );
 /*
   printf( "\n in replace primaries_1 : z = ");
   write_lists( z );
   printf( "\n");
   printf( "\n in replace primaries_1 : s = ");
   write_lists( s );
   printf( "\n");
   printf( "\n in replace primaries_1 : z1 = ");
   write_lists( z1 );
   printf( "\n");
   printf( "\n in replace primaries_1 : result = ");
   write_lists( result );
   printf( "\n");*/
 }

 if( t != NULL )
 {
  if( t->type == 'M' || t->type == 'D' )
  {
    /*result = ( NC *) malloc( sizeof( NC ) );
    result->type = t->type;
    result->inc = 0;
    result->list = NULL;
    if( t->link->link != NULL )
    {
      result->link = ( NC *) malloc( sizeof( NC ) );
      result->link->type = 'S';
      result->link = Substitute_In_Sum( t->link, x, s, result->link , break_flag, flag_1, flag_2  );
    }
    else //if( t->link->link == NULL )
    {
       result->link = ( NC *) malloc( sizeof( NC ) );
       result->link->type = 'S';
       result->link->link = NULL;
       result->link->inc = t->link->inc;

    }

    if( t->link->list->link != NULL )
    {
       result->link->list = ( NC *) malloc( sizeof( NC ) );
       result->link->list->type = 'S';
       result->link->list->list = NULL;
       result->link->list = Substitute_In_Sum( t->link->list, x, s, result->link->list , break_flag, flag_1, flag_2  );
    }
    else
    {
        result->link->list = ( NC *) malloc( sizeof( NC ) );
        result->link->list->type = 'S';
		result->link->list->inc = t->link->list->inc;
        result->link->list->list = NULL;
		result->link->list->link = NULL;
    }
    return result;*/
  }
  else
  {
	v = Compare_Primaries( t, x );

    if( v == 0 )
    {

     *break_flag = 1;

     q1 = ( NC *) malloc( sizeof( NC ) );
     q1->type = 'T';
     q1->inc = 1;
     q1->list = NULL;

     if(t->type == 'v')
     {
		pr = ( NC *) malloc( sizeof( NC ) );
		pr->type = 'v';
		pr->inc = t->inc;
		pr->list = NULL;
		pr->link = NULL;
	 }
	 else
	 {
		pr = ( NC *) malloc( sizeof( NC ) );
		pr = copylist( t );
		pr->list = NULL;
	 }

     q1->link = Mult_primary_with_term_1( pr, q, q1->link );

     result = ( NC *) malloc( sizeof( NC ) );
     result->type = 'S';
     result->list = NULL;
     result = Replace_Primary_1( t->list, x, s, p, q1->link, z, result , break_flag, flag_1, flag_2  );
    }

    else
    {
     p1 = ( NC *) malloc( sizeof( NC ) );
     p1->type = 'T';
     p1->list = NULL;
     p1->link = NULL;

     if(t->type == 'v')
     {
		pr = ( NC *) malloc( sizeof( NC ) );
		pr->type = 'v';
		pr->inc = t->inc;
		pr->list = NULL;
	 }
	 else
	 {
		pr = ( NC *) malloc( sizeof( NC ) );
		pr = copylist( t );
		pr->list = NULL;
	 }

     p1 = Mult_primary_with_term( pr, p, p1 );
     result = ( NC *) malloc( sizeof( NC ) );
     result->type = 'S';
     result->list = NULL;
     result = Replace_Primary_1( t->list, x, s, p1, q, z, result , break_flag, flag_1, flag_2  );
    }
   }
 }
 return result;
}




/* This function works on the normalized sum 's' to substitute each
 * occurrence of primary 'x' in 's' with the normalized sum 'z'
 */
N_sum *
Substitute_In_Sum( N_sum *s, N_primary *x, N_sum *z, N_sum *result , int *break_flag, int *flag_1, int *flag_2 )
{

  N_sum *sum, *result1;

  if( s == NULL )
    return NULL;

  if( s != NULL ){

    sum = ( NC *) malloc( sizeof( NC ) );
    sum->type = 'S';
    sum->inc = s->inc;
    sum->link = NULL;
    sum->list = NULL;

    result = ( NC *) malloc( sizeof( NC ) );
    result->type = 'S';
    result->list = NULL;

    result1 = ( NC *) malloc( sizeof( NC ) );
    result1->type = 'S';
    result1->list = NULL;
    /*
    printf( "\n in Substitute_In_Sum : s->link = ");
    write_lists( s->link );
    printf( "\n" );
    printf( "\n in Substitute_In_Sum : x = ");
    write_lists( x );
    printf( "\n" );
    printf( "\n in Substitute_In_Sum : z = ");
    write_lists( z );
    printf( "\n" );*/

    result1 = Substitute_In_Sum_1( s->link, x, z, result1 , break_flag , flag_1, flag_2 );
    /*
    printf( "\n in Substitute_In_Sum : sum = ");
    write_lists( sum );
    printf( "\n" );
    printf( "\n in Substitute_In_Sum : result1 = ");
    write_lists( result1 );
    printf( "\n" ); */

    result = Add_Sums( sum, result1, result );
    /*
    printf( "\n in Substitute_In_Sum : result = ");
    write_lists( result );
    printf( "\n" ); */

    return result;
  }
  //return NULL;
}



/* This function works on all the terms of the normalized sum 's' to
 * substitute each occurrence of primary 'x'  in all the terms of 's' with
 * the normalized sum 'z'.
 */

N_sum *
Substitute_In_Sum_1( N_sum *s, N_primary *x, N_sum *z, N_sum *result , int *break_flag, int *flag_1, int *flag_2  )
{

  N_sum *z_sum, *temp1;
  N_term *term;


  if( s == NULL )
    return NULL;

  if( s != NULL )
  {
    if( s->link != NULL )
    {
      z_sum = ( NC *) malloc( sizeof( NC ) );
      z_sum->type = 'S';
      z_sum->inc = 1;
      z_sum->list = NULL;
      z_sum->link = NULL;

      term = ( NC *) malloc( sizeof( NC ) );
      term = copylist( s );
      term->type = 'T';
      term->list = NULL;

      result = ( NC *) malloc( sizeof( NC ) );
      result->type = 'S';

      temp1 = ( NC *) malloc( sizeof( NC ) );
      temp1->type = 'S';
      temp1->list = NULL;
      temp1 = Replace_Primary( term, x, z, NULL, NULL, z_sum, temp1 , break_flag, flag_1, flag_2  );

      result = Add_Sums( temp1, Substitute_In_Sum_1( s->list, x, z, result->list , break_flag, flag_1, flag_2  ), result );

      return result;
    }
  }
}




/*  This funtion does the arithmetic simplification at of a normalized sum
 *  The common constant factors are extracted from the normalized sum.
 */

N_sum *
simplify_sum( N_sum *sum, N_sum *result) {

int min;
N_sum *sum1, *sum2;

sum1 = ( NC * )malloc( sizeof( NC ) );
sum1 = copylist( sum );
sum2 = ( NC * )malloc( sizeof( NC ) );
sum2 = copylist( sum );

min = abs( sum->inc );
sum1 = sum1->link;

while( sum1 != NULL ) {
   if( abs( sum1->inc ) < min )
     min = abs( sum1->inc );
   else
     min = min;

   sum1 = sum1->list;
 }


  if( min != 1 && min != 0 ) {
     result->inc = sum2->inc / min;
     result->link = simplify_sum_1( sum2->link,result->link, min );
     return result;
   }

  return result;
}


/*  This funtion does the arithmetic simplification at of a normalized sum
 *  The common constant factors are extracted from the normalized sum.
 *  The common constant factor is 'min' which is calculated from the function
 *  "simplify_sum".
 */

N_sum *
simplify_sum_1( N_sum *sum, N_sum *result, int min ) {
  N_sum *sum1;

  sum1 = ( NC * )malloc( sizeof( NC ) );
  sum1 = copylist( sum );

   if( sum == NULL)
      return NULL;

   if( sum != NULL ) {
      result->inc = sum->inc / min;
      result->list = simplify_sum_1( sum->list, result->list, min );;
    }

   return result;
}



/*  This funtion does the arithmetic simplification at of a normalized conditional expression
 *  The common constant factors are extracted from the normalized conditional expression.
 */
NC *
simplify_condition( NC * condition, NC * result ) {

 NC *condition1;

 if( condition == NULL )
   return NULL;

 if( condition != NULL) {

    condition1 = copylist( condition );
    result->link->link = simplify_sum( condition1->link->link, result->link->link );

    result->list = simplify_condition( condition->list, result->list );
 }
 return result;
}


/* This function compares two sums  'sum1 and 'sum2' without including constant factor.
 * All the terms of the normalized sums  'sum1' and 'sum2' and returns
 * '0' if sum1 == sum2 , returns -1 if sum1 < sum2 else returns +1.
 */

int
Compare_Sums( N_sum *sum1, N_sum *sum2 ) {
 N_term *t1, *t2;
 int m;

 if( sum1 == NULL && sum2 == NULL )
    return 0;
 if( sum1 == NULL && sum2 != NULL )
    return -1;
 if( sum1 != NULL && sum2 == NULL )
    return 1;
 if( sum1 != NULL && sum2 != NULL ) {

  t1 = ( NC * )malloc( sizeof( NC ) );
  t2 = ( NC * )malloc( sizeof( NC ) );

  t1 = copylist( sum1 );
  t1->list = NULL;

  t2 = copylist( sum2 );
  t2->list = NULL;


  m = Compare_Terms( t1->link, t2->link );

  if( m == 0 && t1->inc == t2->inc ) {
     m = Compare_Sums( sum1->list, sum2->list );

   }
  else if( m == 0 && t1->inc < t2->inc )
     return -1;
  else if( m == 0 && t1->inc > t2->inc )
     return 1;
  else if( m < 0 )
     return -1;
  else // if m > 0
    return 1;

 }
 return m;

}



/*  This function compares constants c1 and c2 depending upon the relations R1 and R2,
 *  which must be satisfied for A to imply B ( A -> B ) .
 * It returns '1' if there is success else returns -1.
 */

int
Check_c1_c2_And_R1_R2( int c1, int c2, int R1, int R2 ) {

 // case 1
 if( R2 == 4 ) {
  if( R1 == 4 && c1 == c2 )
    return 1;
  else
    return -1;
 }

 // case 2
 if( R2 == 0 ) {
   if( R1 == 4 && c2 >= c1 )
     return 1;
   if( R1 == 0 && c2 >= c1 )
     return 1;
   else
     return -1;
  }

 //case 3
 if( R2 == 5 ){
   if( R1 == 0 && c2 > c1 )
     return 1;
   if( R1 == 5 && c1 == c2 )
     return 1;
   if( R1 == 2 && c2 < c1 )
     return 1;
   if( R1 == 4 && c1 != c2 )
     return 1;
   else
     return -1;
  }

  //case 4
 if( R2 == 2 ) {
   if( R1 == 4 && c2 <= c1 )
     return 1;
   if( R1 == 2 && c2 <= c1 )
     return 1;
   else
     return -1;
 }

}




/*  This function checks if condition A implies condition B.
 *  It returns '1' if A implies B  else returns -1.
 */

int
A_implies_B( NC *condition_A, NC *condition_B ) {

N_sum *sum1, *sum2;
NC *condition1, *condition2;
int R1, R2, c1, c2, s, c;


 if( condition_A == NULL || condition_B == NULL )
   return -1;
 if( condition_A != NULL && condition_B != NULL ) {

  condition1 = ( NC * )malloc( sizeof( NC ) );
  condition2 = ( NC * )malloc( sizeof( NC ) );

  sum1 = ( NC * )malloc( sizeof( NC ) );
  sum2 = ( NC * )malloc( sizeof( NC ) );

    if( condition_A->type == 'A' ) {

     condition1 = copylist( condition_A );
     condition2 = copylist( condition_B );

     R1 = condition1->link->link->inc;
     R2 = condition2->link->link->inc;

     c1 = condition1->link->link->link->inc;
     c2 = condition2->link->link->link->inc;

     sum1 = condition1->link->link->link->link;
     sum2 = condition2->link->link->link->link;
    }

    else if( condition_A->type == 'O' ) {
      condition1 = copylist( condition_A );
      condition2 = copylist( condition_B );

      R1 = condition1->link->inc;
      R2 = condition2->link->inc;

      c1 = condition1->link->link->inc;
      c2 = condition2->link->link->inc;

      sum1 = condition1->link->link->link;
      sum2 = condition2->link->link->link;
    }

    s = Compare_Sums( sum1, sum2 );

    c = Check_c1_c2_And_R1_R2( c1, c2, R1, R2 );

    if( s == 0 && c == 1 )
      return 1;
    else
      return -1;

 }

}


/*  This function compares two conditions and returns '0' if both are equal else returns '-1'.
 */

int
Compare_Conditions( NC *condition1, NC *condition2 ) {

int number;

 if( condition1 == NULL && condition2 == NULL )
   return 1;
 if( condition1 == NULL && condition2 != NULL )
   return -1;
 if( condition1 != NULL && condition2 == NULL )
   return -1;
 if( condition1 != NULL && condition2 != NULL ) {

   if( condition1->type == 'A' ) {
     if( condition1->inc == condition2->inc )
       number = Compare_Conditions( condition1->link, condition2->link );
     else
       return -1;
   }

   if( condition1->type == 'O' ) {
     if( condition1->inc == condition2->inc )
       number = Compare_Conditions( condition1->link, condition2->link );
     else
       return -1;
   }

   if( condition1->type == 'R' ) {
     if( condition1->inc == condition2->inc )
       number = Compare_Conditions( condition1->link, condition2->link );
     else
       return -1;
   }

   if( condition1->type == 'S' ) {

        number = Compare_Sums( condition1->link, condition2->link );

	if( number == 0 && condition1->inc == condition2->inc )
	   return 0;
	if( number == 0 && condition1->inc < condition2->inc )
	   return -1;
	if( number == 0 && condition1->inc > condition2->inc )
	   return 1;

	if( number < 0 )
	   return -1;
	else
	   return 1;

   }

 }

 return number;
}



/*  This function calculates the number of occurrences of a condition in conditional
 *  expression and returns
 *  the count / number.
 */
int
Search_cond_in_expr( NC *expression, NC *condition ) {

NC *expression1, *temp_cond, *condition1;
int count = 0, number;

if( expression == NULL )
  return 0;
if( expression != NULL ) {
  expression1 = ( NC * )malloc( sizeof( NC ) );
  expression1 = copylist( expression );

  condition1 = ( NC * )malloc( sizeof( NC ) );
  condition1 = copylist( condition );

  if( expression1->type == 'A' && condition1->type == 'O' )
    expression1 = expression1->link;
  if( expression1->type == 'O' && condition1->type == 'A' )
    condition1 = condition1->link;
  if( expression1->type == 'A' && condition1->type == 'A' ) {
     expression1 = expression1->link;
     condition1 = condition1->link;
   }

  temp_cond = ( NC * )malloc( sizeof( NC ) );
  while( expression1 != NULL ) {

       temp_cond = copylist( expression1 );
       temp_cond->list = NULL;

       number = Compare_Conditions( temp_cond, condition1 );

       if( number == 0 )
          count = count + 1;
       else
          count = count;

       expression1 = expression1->list;
   }
 }
 return count ;
}




/*  This function deletes 'count' number of occurrences of a condition ,from the
 *  conditional expression and
 *  returns the resultant conditional expression.
 *  The number of occurrences of a condition in a conditional expression can be found
 *  by using the function
 *  "Search_cond_in_expr( )" .
 *  Both the expression and the condtion must be at the 'O' level .
 */

NC *
Delete_cond_from_expr( NC *expression, NC *condition, int count, NC *result ) {

 NC *temp_condition;
 int number;

 temp_condition = ( NC* ) malloc(sizeof( NC ) );

 if( expression == NULL && count == 0 )
    return NULL;
 if( count == 0 && expression != NULL ) {

   result = copylist( expression );
   return result;
 }

 if( count != 0 && expression != NULL ) {

   temp_condition = copylist( expression );
   temp_condition->list = NULL;

   number = Compare_Conditions( temp_condition, condition );

   if( number == 0 )
   {
     count--;
     result = Delete_cond_from_expr( expression->list, condition, count, result );
   }

   else
   {

       result = copylist( temp_condition );
       result->list = Delete_cond_from_expr( expression->list, condition, count, result->list );

     }

   return result;
 }
 return result;


}




/*  This function removes the multiple occurrences of a condition ( eg. a>b ) from the
 *  conditional expression ( eg. a>b&&c>2&&b==2&&a>b)and returns
 *  the resultant conditional expression.
 */

NC *
Remove_mult_occurence_cond_in_expr( NC *expression, NC* condition, NC *result ) {

 int count;
 NC *expression1;

 expression1 = copylist( expression );

 if( expression1 == NULL )
    return NULL;

 if( expression1 != NULL ) {

 if( expression1->type == 'A' && condition->type == 'O' )
   expression1 = expression1->link;
 if( expression1->type == 'O' && condition->type == 'A' )
   condition = condition->link;
 if( expression1->type == 'A' && condition->type == 'A' ) {
     expression1 = expression1->link;
     condition = condition->link;
   }

   count = Search_cond_in_expr( expression1, condition );

   if( count > 1 ) {
     result = Delete_cond_from_expr( expression1, condition, count-1, result );
     return result;
   }

   else
     return expression1;
 }

}




/*  This function removes all the multiple occurrences of all the conditions
 * ( in conditional expression2 ) from
 *  conditional expression1 and returns the resultant conditional expression.
 */

NC *
Remove_all_mult_occurence_in_expr_1( NC *expression1, NC *expression2, NC *result ) {

 NC *condition;

 if( expression2 == NULL )
    return expression1;
 if( expression2 != NULL ) {

  condition = ( NC * )malloc( sizeof( NC ) );
  condition = copylist( expression2 );
  condition->list = NULL;

  result = Remove_mult_occurence_cond_in_expr( expression1, condition, result );

  result = Remove_all_mult_occurence_in_expr_1( result, expression2->list, result );
 }

 return result;

}



/*  This function removes all the multiple occurrences of all the conditions  from
 *  conditional expression and returns the resultant conditional expression.
 */

NC *
Remove_all_mult_occurence_in_expr( NC *expression, NC *result ) {

NC *expression1;


 if( expression == NULL )
    return NULL;

 if( expression != NULL ) {

 expression1 = ( NC * )malloc( sizeof( NC ) );
 expression1 = copylist( expression );


   result = copylist( expression );
   result->list = NULL;

   result->link = Remove_all_mult_occurence_in_expr_1( expression1->link, expression1->link, result->link );

 }
 return result;

}



/*  This function does simplification at the formula level of a conditional expression.
 *  Some literals of the formula can be deleted by the rule "if( A->B ) then ( A && B => A )".
 *  It takes a conditional expression and returns the resultant simplified conditional expression.
 */

NC *
simplify_literals( NC *expr, NC *result ) {

 NC *expr1, *expr2, *expr3, *expr4;

 if( expr == NULL )
    return NULL;

 if( expr != NULL ) {
      result = copylist( expr );
      expr1 = copylist( expr );

      while( expr1 != NULL ) {
           expr2 = copylist( expr1->list );

	   while( expr2 != NULL ) {
	        expr3 = copylist( expr1 );
                expr3->list = NULL;

                expr4 = copylist( expr2 );
                expr4->list = NULL;

		if( A_implies_B( expr3, expr4 ) == 1 ) {
		   result = Delete_cond_from_expr( expr, expr4, 1, result );
		   expr = copylist( result );
		   expr1 = copylist( result );
		   expr2 = copylist( expr1 );
		}
		expr2 = expr2->list;
	  }

          expr1 = expr1->list;
      }
  }

  return result;

}


/*

 //  This function does the sub expression simplification of a normalized
  //  conditional expression.The commmon subexpresssions are collected together.
  //


NC *
SubExpr_simplify_condition( NC *condition, NC *result ) {

NC *condition1;

if( condition == NULL )
   return NULL;

if( condition != NULL ) {

     condition1 = copylist( condition );
     condition1->list = NULL;
     result = copylist( condition1 );
     result->link->link = SubExpr_simplify_sum( condition1->link->link, result->link->link );
     result->list = SubExpr_simplify_condition( condition->list, result->list );
     return result;
  }

}


 //  This function does the sub expression simplification of a normalized
  //  sum.The commmon subexpresssions are collected together.It returns the
 //  resultant normalized sum.
  //

N_sum *
SubExpr_simplify_sum( N_sum *sum, N_sum *result ) {

int term;
result = copylist( sum );

if( sum == NULL )
   return NULL;

 if( sum != NULL ) {

    if( sum->link != NULL ) {
        term = sum->link->inc;

        if( sum->link->list != NULL ) {

          result->link = SubExpr_simplify_terms( sum->link, term, result->link );
          return result;
        }

        else {
             result = copylist( sum );
             return result;
        }
    }

    else
         return sum;

  }

}



//  This function takes a normalized sum and it collects the commmon subexpresssions
 //  and simplifies it.It returns the resultant normalized sum of terms of a normalized sum.
 //


N_sum *
SubExpr_simplify_terms( N_sum *sum, int term, N_sum *result ) {

N_sum *sum1, *sum2;
N_term *t1, *t2;
int m;

 if( sum == NULL ) {
      return NULL;
  }

if( sum != NULL ) {

  if( sum->list == NULL ) {
        result = copylist( sum );
      return result;
   }


  if( sum->list != NULL ) {
     sum1 = ( NC * )malloc( sizeof( NC ) );
     sum2 = ( NC * )malloc( sizeof( NC ) );
     t1 = ( NC * )malloc( sizeof( NC ) );
     t2 = ( NC * )malloc( sizeof( NC ) );

     sum1 = copylist( sum );
     sum2 = copylist( sum->list );

     t1 = copylist( sum1 );
     t1->list = NULL;

     t2 = copylist( sum2 );
     t2->list = NULL;

     m = Compare_Terms( t1->link, t2->link );

     if( m == 0 ) {

      if( term + t2->inc != 0 )
      {
        result = copylist( t1 );
        result->inc = term + t2->inc;
        term = result->inc;
        result->list = SubExpr_simplify_terms( sum2->list, term, result->list );
        return result;
       }

       if( term + t2->inc == 0 )
       {
            result = SubExpr_simplify_terms( sum2->list, term, result );
	    return result;
       }
     }

     if( m != 0 )
     {
        result = copylist( t1 );
	term = t2->inc;
	result->list = SubExpr_simplify_terms( sum2, term, result->list );
	return result;
     }
   }
  }
  return result;
}
*/



/*  This function searches the primary 'p' in normlaized primary 'primary'. If primary 'p' is present
 *  it returns '1' else returns '-1'.
 */
int
search_primary_in_primary( NC *primary, NC *p, int flag )
{
  if( flag == 1 )
    return 1;
  if( primary == NULL )
     return -1;
  else
  { // if( primary != NULL )
    if( primary->type == 'v' )
    {
      if( primary->inc == p->inc )
      {
        flag = 1;
	    return flag;
      }
      else
      {
         flag = search_primary_in_primary( primary->list, p, flag );
	     return flag;
      }
    }
    else if( primary->type == 'M' || primary->type == 'D' )
    {
       flag = search_primary_in_sum( primary->link, p, flag );
       if( flag == 1 )
          return flag;
       flag = search_primary_in_sum( primary->link->list, p, flag );
       if( flag == 1 )
          return flag;
    }
    //KB: array start
    else if( primary->type == 'w' )
    {
        //for 'write' (and also for 'read'), we cannot replace the
        //entire array - we can only replace its index variables
		flag = search_primary_in_sum( primary->link->link, p, flag );
        if( flag == 1 )
          return flag;
		flag = search_primary_in_sum( primary->link->list, p, flag );
		if(flag == 1)
		  return flag;
	}
	else if( primary->type == 'a' )
    {
		flag = search_primary_in_sum( primary->link, p, flag );
        if( flag == 1 )
          return flag;
	}
    //KB: array end
  }
  return flag;
}



/*  This function searches the primary 'p' in normlaized term 'sum'. If primary 'p' is present
 *  it returns '1' else returns '-1'.
 */
int
search_primary_in_term( NC *term, NC *p, int flag )
{
  if( flag == 1 )
    return 1;
  if( term == NULL )
     return -1;
  else
  {  // if( term != NULL )
     flag = search_primary_in_primary( term->link, p, flag );
     if( flag == 1 )
       return 1;
     else
     {  // if( flag != 1 )
        flag = search_primary_in_term( term->list, p, flag );
	return flag;
     }
  }
  return flag;
}


/*  This function searches the primary 'p' in normlaized sum 'sum'. If primary 'p' is present
 *  it returns '1' else returns '-1'.
 */
int
search_primary_in_sum( NC *sum, NC *p, int flag )
{
  if( flag == 1 )
    return 1;
  if( sum == NULL )
     return -1;
  else
  {  // if( sum != NULL )
     flag = search_primary_in_term( sum->link, p, flag );
     return flag;
  }
}



//The function is enhanced to handle (Read and Write operations of) arrays
//However, its name remains unmodified
NC *
Substitute_in_MOD_DIV( NC *M_D_sum, r_alpha *transformations, NC *result )
{
	r_alpha *trans, *trans1, *p_trans;
	NC *s, *s1, *s2, *s3, *s4, *M_z, *p, *sum, *M_t;
	int break_flag, flag_1, flag_2;

	//KB array start
	NC *old, *new, *traverseList, *chk;
	boolean flagLink;
	//KB array end

	if( M_D_sum->type == 'M' || M_D_sum->type == 'D' )
	{
		//write_lists( M_D_sum );
		trans = transformations;
		result = ( NC *)malloc( sizeof( NC));
		result->type = M_D_sum->type;
		result->inc = M_D_sum->inc;
		result->list = NULL;
		//write_lists( result );

		result->link = ( NC *)malloc( sizeof( NC));
		result->link->type = M_D_sum->link->type;
		result->link->inc = M_D_sum->link->inc;
		result->link = Substitute_in_MOD_DIV( M_D_sum->link, trans, result->link );

		trans1 = transformations;
		result->link->list = ( NC *)malloc( sizeof( NC));
		result->link->list->type = M_D_sum->type;
		result->link->list->inc = M_D_sum->inc;
		result->link->list = Substitute_in_MOD_DIV( M_D_sum->link->list, trans1, result->link->list );
		result->link->list->list = NULL;
		return result;
	}

	//KB array start
	if( M_D_sum->type == 'a' || M_D_sum->type == 'y' )
	{
		//write_lists( M_D_sum ); printf("\n");
		result = ( NC *)malloc( sizeof( NC));
		result->type = M_D_sum->type;
		result->inc = M_D_sum->inc;
		result->list = NULL;

		if(M_D_sum->type == 'y')
		{
			trans = transformations;

			result->list = ( NC *)malloc( sizeof( NC));
			result->list->type = M_D_sum->list->type;
			result->list->inc = M_D_sum->list->inc;
			result->list = Substitute_in_MOD_DIV( M_D_sum->list, trans, result->list );
			result->list->list = NULL;
		}

		flagLink = TRUE;
		traverseList = M_D_sum->link;

		while( traverseList != NULL )
		{
			trans = transformations;

			new = ( NC *)malloc( sizeof( NC));
			new->type = traverseList->type;
			new->inc = traverseList->inc;
			new = Substitute_in_MOD_DIV( traverseList, trans, new );
			new->list = NULL;

			if( flagLink )
			{
				result->link = new;
				flagLink = FALSE;
			}
			else
			{
				old->list = new;
			}

			old = new;
			traverseList = traverseList->list;
		}

		return result;
	}
	//KB array end

	if( M_D_sum->type == 'S')
	{
		trans = transformations;
		s = ( NC * )malloc( sizeof( NC ) );
		s = copylist( M_D_sum );
	    s->type = 'S';

		s1 = ( NC * )malloc( sizeof( NC ) );
		s1 = copylist( M_D_sum );

		s1->link = NULL;
		s1->list = NULL;

		if( s->link != NULL )
		{
			s = s->link;
			while( s != NULL )
			{
				s2 = ( NC * )malloc( sizeof( NC ) );
				s2 = copylist( s );
				s2->list = NULL;

				if( s2->link != NULL )
				{
					s3 = ( NC * )malloc( sizeof( NC ) );
					s3 = copylist( s2->link );
					//s3->list = NULL; //This assignment makes the following "while" loop redundant //KBss

					M_z = ( NC * )malloc( sizeof( NC ) );
					M_z->type = 'S';
					M_z->inc = 1;
					M_z->link = NULL;
					M_z->list = NULL;

					while( s3!= NULL )
					{
						if( s3->type == 'v' )
						{
							s4 = ( NC * )malloc( sizeof( NC ) );
							s4->type = 'S';
							s4->inc = 0;

							s4->link = ( NC * )malloc( sizeof( NC ) );
							s4->list = NULL;
							s4->link->type = 'T';
							s4->link->inc = 1; //s2->inc;

							s4->link->link = ( NC * )malloc( sizeof( NC ) );
							s4->link->link = copylist( s3 );
							s4->link->list = NULL;
							s4->link->link->list = NULL;

							p_trans = transformations;
							while( p_trans != NULL )
							{
								p = ( NC * )malloc( sizeof( NC ) );
								p->type = 'v';
								p->inc = p_trans->Assign.lhs;
								p->link = NULL;
								p->list = NULL;

								sum =  ( NC * )malloc( sizeof( NC ) );
								sum->type = 'S';
								sum = copylist( p_trans->Assign.rhs );

								break_flag = 0;
								flag_1 = 0;
								flag_2 = 0;

								s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

								if( break_flag == 1 )
								{
									break;
								}

								p_trans = p_trans->next;
							} //end while

						} // if( s3 ->type == 'v' )

						//KB array start
						else if( s3->type == 'a' )
						{
							s4 = ( NC * )malloc( sizeof( NC ) );
							s4->type = 'S';
							s4->inc = 0;

							s4->link = ( NC * )malloc( sizeof( NC ) );
							s4->list = NULL;
							s4->link->type = 'T';
							s4->link->inc = 1; //s2->inc;

							s4->link->link = ( NC * )malloc( sizeof( NC ) );
							s4->link->link = copylist( s3 );
							s4->link->list = NULL;
							s4->link->link->list = NULL;

							p_trans = transformations;
							while( p_trans != NULL )
							{
								if( p_trans->Assign.lhs != s3->inc ||
									(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w' &&
										compare_trees(s3->link, p_trans->Assign.rhs->link->link)==0) )
								{
									p_trans = p_trans->next;
									continue;
								}

								p = ( NC * )malloc( sizeof( NC ) );
								p = copylist( s3 );

								sum =  ( NC * )malloc( sizeof( NC ) );
								sum->type = 'S';

								if(p_trans->Assign.rhs != NULL && p_trans->Assign.rhs->type == 'w')
								{
									sum = copylist( p_trans->Assign.rhs->link->list );
								}
								else
								{
									sum = copylist( p_trans->Assign.rhs );
								}

								break_flag = 0;
								flag_1 = 0;
								flag_2 = 0;

								/*
								printf( "\n ##### \nbefore Substitute_In_Sum : \n ");
								write_lists( s4 );
								printf("\n wrt \n");
								write_lists(p);
								printf( " = ");
								write_lists( sum );
								printf("\n"); */

								s4 = Substitute_In_Sum( s4, p, sum,  s4 , &break_flag, &flag_1, &flag_2 );

								/*
								printf( "\n after Substitute_In_Sum : s4 : ");
								printf( " = ");
								write_lists( s4 );
								printf("\n ####\n"); */

								if( break_flag == 1 )
								{
									break;
								}

								p_trans = p_trans->next;
							} //end while
						}
						//KB array end

						else
						{
							trans = transformations;
							result = (NC *)malloc(sizeof(NC));
							result = Substitute_in_MOD_DIV( s3, trans, result );
							s4 = ( NC * )malloc( sizeof( NC ) );
							s4->type = 'S';
							s4->inc = 0;

							s4->link = ( NC * )malloc( sizeof( NC ) );
							s4->list = NULL;
							s4->link->type = 'T';
							s4->link->inc = 1; //s2->inc;

							s4->link->link = ( NC * )malloc( sizeof( NC ) );
							s4->link->link = copylist( result );
							s4->link->list = NULL;
						}

						M_z = Mult_Sum_With_Sum( M_z, s4, M_z );

						s3 = s3->list;
					} // while( s3!= NULL )

					M_t = ( NC * )malloc( sizeof( NC ) );
					M_t->type = 'S';
					M_t->inc = s2->inc;
					M_t->link = NULL;
					M_t->list = NULL;

					M_z = Mult_Sum_With_Sum( M_z, M_t, M_z );

					s1 = Add_Sums( s1, M_z, s1 );
				} // if( s2->link != NULL )

				s = s->list;
			} // while( s != NULL )
		} // if( s->link != NULL )

		result = copylist( s1 );
		return result;
	}

	else //for uninterpreted functions (of type 'f') //KB Aug 27, 2013
	{
		return result;
	}
}


// normal.c

/****************************************************************************************/

/*                           THE MAIN FUNCTION                                          */

/****************************************************************************************/
/*
int main( int argc, char* argv[] ) {

    //****************** Variable declaration ***********************
    FSMD  M0, M1, M0_kripke;
    // M0 -- original FSMD , M1 -- Scheduled FSMD

    PATHS_LIST p0, p1;
    // p0 is path cover of M0 and p1 is path cover of M1

    CPQ *eta;
    // eta = set corresponding state Pair

    PATH_Q F;
    // list of paths of M0 starting with nodes having corresponding nodes but
    // ending with nodes whose  corresponding nodes have not yet been found

    CPS_Q P;
    // Working list of corresponding states from which paths will be examined
    int i;
	struct timeval tv1;
	struct timezone tz1;
	struct timeval tv2;
	struct timezone tz2;

    gettimeofday(&tv1, &tz1);

	//************** Read two FSMD structure ****
	read_fsmd( argv[1], &M0, &V0 );
	read_fsmd( argv[1], &M0_kripke, &V0_kripke );
        //print_fsmd( &M0 );
	//getchar();



	read_fsmd( argv[2], &M1, &V1 );
	//print_fsmd( &M1 );
	//getchar();

	cal_V0_intersection_V1( &V0_V1 );

        printf( "\n vars of : V0_V1  \n");
        for( i = 0; i < V0_V1.no_of_elements; i++ )
	{
             printf( " %s \n", V0_V1.var_string[i] );
	}

	cal_V1_minus_V0_intersection_V1( &V1_minus_V0_V1 );

        printf( "\n vars of : V1_minus_V0_V1 \n");
        for( i = 0; i < V1_minus_V0_V1.no_of_elements; i++ )
	{
             printf( " %s \n", V1_minus_V0_V1.var_string[i] );
	}
	getchar( );

    // Data structure initialization
    initF(&F);
    initP(&P);
    eta=initEta();
    enQP(&P,0,0);    // place the reset state of two fsmd as corresponding state in P

    findpaths(&M0,&p0);
    findpaths(&M1,&p1);

    Associate_R_and_r_alpha(&p0); // computes R_alpha & r_alpha
    displayallpaths(&p0);
    getchar();

    Associate_R_and_r_alpha(&p1);
    displayallpaths(&p1);
    getchar();

       equivalenceChecking_m0_to_m1( M0, M1, p0, p1, P, F, eta, M0_kripke );

	printf("\n\n\n\n###################### Verification Report #######################\n\n");

	printf("\n No. of states in M0: %d and No. of states in M1: %d\n", M0.numstates, M1.numstates);

	printf("\n No. of paths in initial path cover of M0: %d and No. of paths in actual path cover of M0: %d\n",
        p0.numpaths, p1.numpaths);

	printf("\nProgram iterates %d times and %d times path extensions required\n", count_iteration, count_extend);

	printf("\n Number of Weak Equivalence path found: %d\n", nWeak);

        printf("\n Model Checker has called %d times", nmodelCheck);
	gettimeofday(&tv2, &tz2);
	printf("\n Exec time is %ld sec and %ld microsecs\n", tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec);

	printf("\n\n####################################################################\n\n");


  }
*/

