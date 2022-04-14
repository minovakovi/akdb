/**
 *
@file expression_check.c Provides functions for constraint checking used in selection and theta-join
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

/**
 * @author Dino Laktašić, abstracted by Tomislav Mikulček,updated by Nikola Miljancic
 * @brief  Function that compares values according to their data type, checks arithmetic statement which is part of expression given in 
 *   	  the function below. For every type of arithmetic operator, there is switch-case statement which examines type of el and
           casts void operands to this type.
 * @param el list element, last element put in list temp which holds elements of row ordered according to expression and results of their evaluation
 * @param *op comparison operator
 * @param *a left operand
 * @param *b right operand
 * @return 0 if arithmetic statement is false, 1 if arithmetic statement is true
 */

#include "expression_check.h"

//int AK_check_arithmetic_statement(AK_list_elem el, const char *op, const char *a, const char *b) {
int AK_check_arithmetic_statement(struct list_node *el, const char *op, const char *a, const char *b) {  
  AK_PRO;
  char **numericStringEnd=NULL; //A pointer to the location where the numeric part of the string ends


	if(strcmp(op, "<") == 0){
		switch (el->type) {

			case TYPE_INT:
				AK_EPI;
				return *(int *) a < *(int *) b;
			case TYPE_FLOAT:
				AK_EPI;
				return *((float *) a) < *((float *) b);
			case TYPE_NUMBER:
				AK_EPI;
				return *((double *) a) < *((double *) b);
			case TYPE_VARCHAR:
				AK_EPI;
				return strcmp((const char *) a, (const char *) b) < 0;

		}
	}else if(strcmp(op, ">") == 0){

		switch (el->type) {

			case TYPE_INT:
				return *(int *) a > *(int *) b;
			case TYPE_FLOAT:
				return *((float *) a) > *((float *) b);
			case TYPE_NUMBER:
				return *((double *) a) > *((double *) b);
			case TYPE_VARCHAR:
				return strcmp((const char *) a, (const char *) b) > 0;

		}


	}else if(strcmp(op, "<=") == 0){		
		switch (el->type) {

			case TYPE_INT:
				return *(int *) a <= *(int *) b;
			case TYPE_FLOAT:
				return *((float *) a) <= *((float *) b);
			case TYPE_NUMBER:
				return *((double *) a) <= *((double *) b);
			case TYPE_VARCHAR:
				return strcmp((const char *) a, (const char *) b) <= 0;

		}
		
	}else if(strcmp(op, ">=") == 0){
	
		switch (el->type) {

				case TYPE_INT:
					return *(int *) a >= *(int *) b;
				case TYPE_FLOAT:
					return *((float *) a) >= *((float *) b);
				case TYPE_NUMBER:
					return *((double *) a) >= *((double *) b);
				case TYPE_VARCHAR:
					return strcmp((const char *) a, (const char *) b) >= 0;

			}

	}else if(strcmp(op, "+") == 0){

		switch (el->type) {

			case TYPE_INT:
				AK_EPI;
				return strtol(a, numericStringEnd, 10) + strtol(b, numericStringEnd, 10);
			case TYPE_FLOAT:
				AK_EPI;
				return strtod(a, numericStringEnd) + strtod(b, numericStringEnd);
			case TYPE_NUMBER:
				AK_EPI;
				return strtod(a, numericStringEnd) + strtod(b, numericStringEnd);
			case TYPE_VARCHAR:
				AK_EPI;
				return EXIT_ERROR;

		}
	}else if(strcmp(op, "-") == 0){

		switch (el->type) {

			case TYPE_INT:
				AK_EPI;
				return strtol(a, numericStringEnd, 10) - strtol(b, numericStringEnd, 10);
			case TYPE_FLOAT:
				AK_EPI;
				return strtod(a, numericStringEnd) - strtod(b, numericStringEnd);
			case TYPE_NUMBER:
				AK_EPI;
				return strtod(a, numericStringEnd) - strtod(b, numericStringEnd);
			case TYPE_VARCHAR:
				AK_EPI;
				return EXIT_ERROR;
		}
	}else if(strcmp(op, "*") == 0){

		switch (el->type) {

			case TYPE_INT:
				AK_EPI;
				return strtol(a, numericStringEnd, 10) * strtol(b, numericStringEnd, 10);
			case TYPE_FLOAT:
				AK_EPI;
				return strtod(a, numericStringEnd) * strtod(b, numericStringEnd);
			case TYPE_NUMBER:
				AK_EPI;
				return strtod(a, numericStringEnd) * strtod(b, numericStringEnd);
			case TYPE_VARCHAR:
				AK_EPI;
				return EXIT_ERROR;
		}
	}else if(strcmp(op, "/") == 0){

		switch (el->type) {

			case TYPE_INT:
				AK_EPI;
				return strtol(a, numericStringEnd, 10) / strtol(b, numericStringEnd, 10);
			case TYPE_FLOAT:
				//strtod(a, numericStringEnd) / strtod(b, numericStringEnd);
			case TYPE_NUMBER:
				//strtod(a, numericStringEnd) / strtod(b, numericStringEnd);
			case TYPE_VARCHAR:
				AK_EPI;
				return EXIT_ERROR;
		}
	}
	return 0;
}
/**
	* @Author Leon Palaić
	* @brief Function that replaces charachter wildcard (%,_) ch in string s with repl characters.
	* @param s input string
	* @param ch charachter to be replaced
	* @result new sequence of charachters

*/
char *AK_replace_wild_card(const char *s,char ch,const char *repl){
	AK_PRO;
    int count = 0;
    const char *t;
    for(t=s; *t; t++)
        count += (*t == ch);

    size_t rlen = strlen(repl);
    char *res = AK_malloc(strlen(s) + (rlen-1)*count + 1);
    char *ptr = res;
    for(t=s; *t; t++) {
        if(*t == ch) {
            memcpy(ptr, repl, rlen);
            ptr += rlen;
        } else {
            *ptr++ = *t;
        }
    }
    *ptr = 0;
    AK_EPI;
    return res;
}

/**
	* @Author Leon Palaić
	* @brief Function that evaluates regex expression on a given string input.
	* @param value string value that must match regex expression
	* @param expression POSIX regex expression
	* @param checkWildCard replaces SQL wildcard to correesponding POSIX regex charachter
	* @param sensitive case insensitive indicator 1-case sensitive,0- case insensitive
	* @result 0 if regex didnt match or sytnax of regex is incorecct 
			  1 if string matches coresponding regex expression
*/
int AK_check_regex_expression(const char * value, const char * expression, int sensitive, int checkWildCard){
	AK_PRO;
	char *matcherData = value;
	char * regexExpreesion = expression;
	char * result;
	regex_t regexCompiled;
	int isMatched;
	int caseSens;

	if(!sensitive){
		
		caseSens = REG_ICASE;
	}
	else{
		caseSens = REG_EXTENDED;
	}
	if(checkWildCard){
		result = AK_replace_wild_card(regexExpreesion,'%',".*");
		regexExpreesion = AK_replace_wild_card(result,'_',".");
		AK_free(result);
	}
	if (regcomp(&regexCompiled, regexExpreesion, caseSens)){
      printf("Could not compile regular expression, check your sintax.\n");
      isMatched = 0;
    }
    if(checkWildCard)
		AK_free(regexExpreesion);
    if (regexec(&regexCompiled, matcherData, 0, NULL, 0) != REG_NOMATCH){
    	isMatched = 1;
    }
    else{
    	
    	isMatched = 0;
    }
    regfree(&regexCompiled);
    AK_EPI;
    return isMatched;
}

/**
	* @Author Leon Palaić
	* @brief Function that evaluates regex expression on a given string input.
	* @param value string value that must match regex expression
	* @param expression POSIX regex expression
	* @result 0 if regex didnt match or sytnax of regex is incorecct 
			  1 if string matches coresponding regex expression
*/
int AK_check_regex_operator_expression(const char * value, const char * expression){
	AK_PRO;
	char *matcherData = value;
	char * regexExpreesion = expression;
	regex_t regexCompiled;
	int isMatched ;
	if (regcomp(&regexCompiled, regexExpreesion, REG_EXTENDED)){
      printf("Could not compile regular expression, check your sintax.\n");
      isMatched = 0;
    }
    
    if (regexec(&regexCompiled, matcherData, 0, NULL, 0) != REG_NOMATCH){
    	
    	isMatched = 1;
    }
    else{
    	
    	isMatched = 0;
    }
    regfree(&regexCompiled);
    AK_EPI;
    return isMatched;
}
/**
 * @author Matija Šestak, updated by Dino Laktašić,Nikola Miljancic, abstracted by Tomislav Mikulček
 * @brief  Function that evaluates whether one record (row) satisfies logical expression. It goes through
           given row. If it comes to logical operator, it evaluates by itself. For arithmetic operators
           function AK_check_arithmetic_statement() is called.
 * @param row_root beginning of the row that is to be evaluated
 * @param *expr list with the logical expression in postfix notation
 * @result 0 if row does not satisfy, 1 if row satisfies expression
 */
//int AK_check_if_row_satisfies_expression(AK_list_elem row_root, AK_list *expr) {
int AK_check_if_row_satisfies_expression(struct list_node *row_root, struct list_node *expr) {

    AK_PRO;
    if (expr == 0) {
    	AK_EPI;
    	return 1;
    }

    char true = 1, false = 0;
    int found;//, result;
    char result = 0;
   
    //list of values
    struct list_node *temp = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&temp);
    //list of results (0,1)
    struct list_node *temp_result = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&temp_result);

    struct list_node *el = AK_First_L2(expr);
    struct list_node *row;
    struct list_node *a, *b,*last,*previous,*c;

    char data[MAX_VARCHAR_LENGTH];
    int i = 0;
    while (el) {

        if (el->type == TYPE_ATTRIBS) {

            found = 0;
            row = row_root;

           

            while (row) {

    
                if (strcmp(el->data, row->attribute_name) == 0) {
                	
                	
                    found = 1;
                    break;
                }
                row = row->next;
            }

            if (!found) {
            	AK_dbg_messg(MIDDLE, REL_OP, "Expression ckeck was not able to find column: %s\n", el->data);
				AK_EPI;
                return 0;

            } else {

                int type = row->type;
                memset(data, 0, MAX_VARCHAR_LENGTH);
                memcpy(data, &row->data, sizeof(row->data));

     
				AK_InsertAtEnd_L3(type, data, sizeof(row->data), temp);
            }

        } else if (el->type == TYPE_OPERATOR) {

        	

	    	b = AK_End_L2(temp);
            a = AK_Previous_L2(b, temp);
            c = AK_Previous_L2(a,temp);

            if (strcmp(el->data, "=") == 0) {
                if (memcmp(a->data, b->data, sizeof(a->type)) == 0){
                	
					AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (char), temp_result);

				}else{

					AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);

				}

            } 
            else if (strcmp(el->data, "<>") == 0) {

                if (memcmp(a->data, b->data, a->size) != 0){

					AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);

				}
                else{

					AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);

				}

            } else if (strcmp(el->data, "OR") == 0) {

                char val_a, val_b;
	        	last = AK_End_L2(temp_result);
                previous = AK_Previous_L2(last, temp_result);
                memcpy(&val_a, last->data, sizeof (char));
                memcpy(&val_b, previous->data, sizeof (char));

                if (val_a || val_b){

					AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
				}
                else{

					AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);

				}

            } else if (strcmp(el->data, "AND") == 0) {
                char val_a, val_b;
				last = AK_End_L2(temp_result);
                previous = AK_Previous_L2(last, temp_result);
                memcpy(&val_a, last->data, sizeof (char));
                memcpy(&val_b, previous->data, sizeof (char));

                if (val_a && val_b){
                	
					AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
				}else{
					AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
				}

            } else if(strcmp(el->data,"BETWEEN")==0){
            	
            	int rs;
            	int rs2;
            	
                rs = AK_check_arithmetic_statement(a, ">=", c->data, a->data);
                rs2 = AK_check_arithmetic_statement(b,"<=", c->data, b->data);
                

	            if(rs && rs2){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }

            }else if(strcmp(el->data,"LIKE")==0 || strcmp(el->data,"~~")==0){

           		char like_regex[] = "([]:alpha:[!%_^]*)";	

           		if(AK_check_regex_operator_expression(b->data,&like_regex)){
            	int rs;
            	rs = AK_check_regex_expression(a->data,b->data,1,1);
            	AK_InsertAtEnd_L3(TYPE_INT, &rs, sizeof (int), temp_result);
            	}else{
            		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
            	}

            }else if(strcmp(el->data,"ILIKE")==0 || strcmp(el->data,"~~*")==0){

            	char like_regex[] = "([]:alpha:[!%_^]*)";
            	int rs;
            	if(AK_check_regex_operator_expression(b->data,&like_regex)){

            		rs = AK_check_regex_expression(a->data,b->data,0,1);
            		AK_InsertAtEnd_L3(TYPE_INT, &rs, sizeof (int), temp_result);

            	}else{
            		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
            	}
            	

            }else if(strcmp(el->data,"SIMILAR TO")==0){
            	char similar_regex[] = "([]:alpha:[!%_^|*+()!]*)";
            	int rs;

            	if(AK_check_regex_operator_expression(b->data,similar_regex)){
            		rs = AK_check_regex_expression(a->data,b->data,1,1);
            		AK_InsertAtEnd_L3(TYPE_INT, &rs, sizeof (int), temp_result);
            	}else{
            		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
            	}
            	
            }else if(strcmp(el->data,"~")==0){
            	//regex match implementation case sensitive
            	int rs;
            	rs = AK_check_regex_expression(a->data,b->data,1,0);
            	AK_InsertAtEnd_L3(TYPE_INT, &rs, sizeof (int), temp_result);


            }else if(strcmp(el->data,"~*")==0){
            	//regex match implementation case sensitive insensitive
            	int rs;
            	rs = AK_check_regex_expression(a->data,b->data,0,0);
            	AK_InsertAtEnd_L3(TYPE_INT, &rs, sizeof (int), temp_result);
            }else{

            		char rs;

                	rs = AK_check_arithmetic_statement(b, el->data, a->data, b->data);
					AK_InsertAtEnd_L3(TYPE_INT, &rs, sizeof (int), temp_result);
            }      

        } else {
			AK_InsertAtEnd_L3(el->type, el->data, el->size, temp);
        }
        el = el->next;
    }

    memcpy(&result, ((struct list_node *) AK_End_L2(temp_result))->data, sizeof (char));
    AK_DeleteAll_L3(&temp);
    AK_free(temp);
    AK_DeleteAll_L3(&temp_result);
    AK_free(temp_result);
    AK_EPI;
    return result;
}
//TODO: Add description
TestResult AK_expression_check_test()
{
    AK_PRO;

    struct list_node *elem = (struct list_node *) AK_malloc(sizeof(struct list_node));
    elem->type = TYPE_INT;
    const char *op = "+";
    const char *a = "800";
    const char *b = "400";
    int outcome;
    int outcome2;
    int outcome3;
    int successful = 0;

    int likeOutcome1,likeOutcome2,likeOutcome3,likeOutcome4,likeOutcome5;



 	const char * value="abc";
 	const char * value2 ="thomas";
 	const char * expression = "abc";
 	const char * expression2 = "a%";
 	const char * expression3 = "_b_";
 	const char * expression4 = "%Thomas%";
 	const char * expression5 = "%thomas%";

 	successful += AK_check_regex_expression(value,expression,1,1);
 	successful += AK_check_regex_expression(value,expression2,1,1);
 	successful += AK_check_regex_expression(value,expression3,1,1);
 	successful += AK_check_regex_expression(value2, expression4,0,1);
 	successful += AK_check_regex_expression(value2, expression5,1,1);

 	printf("Test for like,Ilike with wildcards \n");
    printf("abc - expression 'abc' outcome is: %d\n", likeOutcome1);
    printf("abc - expression 'a.*'  outcome is: %d\n", likeOutcome2);
    printf("abc - expression  _b_ outcome is: %d\n", likeOutcome3);
    printf("ILike   thomas - expression .*Thomas.* outcome is: %d\n", likeOutcome4);
    printf("Like thomas - expression .*thomas.* outcome is %d\n", likeOutcome5);
	
    AK_free(elem);
    AK_EPI;
    return TEST_result(successful, 5-successful);
}
