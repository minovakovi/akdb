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
 * @author Dino Laktašić, abstracted by Tomislav Mikulček,updated by Nikola Miljancic, updated by Fran Turković
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

//int AK_check_arithmetic_statement(AK_list_elem el, const char *op, const char *a, const char *b){
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
	}else if(strcmp(op, "=") == 0){
	
		switch (el->type) {

				case TYPE_INT:
					return *(int *) a == *(int *) b;
				case TYPE_FLOAT:
					return *((float *) a) == *((float *) b);
				case TYPE_NUMBER:
					return *((double *) a) == *((double *) b);
				case TYPE_VARCHAR:
					return strcmp((const char *) a, (const char *) b) == 0;

			}
	}
	else if(strcmp(op, "!=") == 0){
	
		switch (el->type) {

				case TYPE_INT:
					return *(int *) a != *(int *) b;
				case TYPE_FLOAT:
					return *((float *) a) != *((float *) b);
				case TYPE_NUMBER:
					return *((double *) a) != *((double *) b);
				case TYPE_VARCHAR:
					return strcmp((const char *) a, (const char *) b) != 0;

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
	* @Author Fran Turković
	* @brief Function that puts start and end charachters (^,$) on input string
	* @param s input string
	* @result new sequence of charachters

*/
char *AK_add_start_end_regex_chars(const char *s){
	AK_PRO;

    size_t len = strlen(s);
   
    /* one for start char (^), one for end char($), one for trailing zero */
    char *str = malloc(len + 1 + 1 + 1);

	str[0] = '\0';
    strcat(str,"^");
	strcat(str,s);
	strcat(str,"$");

	strcpy(s,str);

    free(str);
    AK_EPI;
    return s;
}

/**
	* @Author Leon Palaić, updated by Fran Turković
	* @brief Function that evaluates regex expression on a given string input.
	* @param value string value that must match regex expression
	* @param expression POSIX regex expression
	* @param checkWildCard replaces SQL wildcard to correesponding POSIX regex charachter
	* @param sensitive case insensitive indicator 1-case sensitive,0- case insensitive
	* @param checkWildCard 0 if we don't need to replace wild charachters (regex case)
						   1 if we need to replace wild characters (LIKE case)
	* @result 0 if regex didnt match or sytnax of regex is incorecct 
			  1 if string matches coresponding regex expression
*/
int AK_check_regex_expression(const char * value, const char * expression, int sensitive, int checkWildCard){
	AK_PRO;
	char *matcherData = value;
	char * regexExpreesion;
	char * result;  
	regex_t regexCompiled;
	int isMatched;
	int caseSens;

	if(checkWildCard){
		regexExpreesion = AK_add_start_end_regex_chars(expression);
	}
	else{
		regexExpreesion = expression;
	}

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
 * @author Matija Šestak, updated by Dino Laktašić,Nikola Miljancic, abstracted by Tomislav Mikulček, updated by Fran Turković
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
			}else if(strcmp(el->data,"IN")==0 || strcmp(el->data,"=ANY")==0 || strcmp(el->data,"= ANY")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,">ANY")==0 || strcmp(el->data,"> ANY")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, ">", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, ">", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, ">", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,"<ANY")==0 || strcmp(el->data,"< ANY")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "<", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "<", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "<", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,"<=ANY")==0 || strcmp(el->data,"<= ANY")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "<=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "<=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "<=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,">=ANY")==0 || strcmp(el->data,">= ANY")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, ">=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, ">=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, ">=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,"!=ANY")==0 || strcmp(el->data,"!= ANY")==0 || strcmp(el->data,"<>ANY")==0 || strcmp(el->data,"<> ANY")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "!=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "!=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "!=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,">ALL")==0 || strcmp(el->data,"> ALL")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "<", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "<", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "<", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(!rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,"<ALL")==0 || strcmp(el->data,"< ALL")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, ">", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, ">", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, ">", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(!rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,">=ALL")==0 || strcmp(el->data,">= ALL")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "<=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "<=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "<=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(!rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,"<=ALL")==0 || strcmp(el->data,"<= ALL")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, ">=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, ">=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, ">=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(!rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,"!=ALL")==0 || strcmp(el->data,"!= ALL")==0 || strcmp(el->data,"<>ALL")==0 || strcmp(el->data,"<> ALL")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(!rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }
			}else if(strcmp(el->data,"=ALL")==0 || strcmp(el->data,"= ALL")==0){
            	
            	int rs;
				const char *my_str_literal = b->data;
				char *token, *str, *tofree;
				tofree = str = strdup(my_str_literal);
				int firstPassed = 0;  
				while ((token = strsep(&str, ","))){
					if(b->type==TYPE_INT){
						int token2 = atoi(token);
						rs = AK_check_arithmetic_statement(b, "!=", a->data, (char *)&token2);
					}
					else if(b->type==TYPE_FLOAT){
						float token2 = atof(token);
						rs = AK_check_arithmetic_statement(b, "!=", a->data, (char *)&token2);
					}
					else{
						rs = AK_check_arithmetic_statement(b, "!=", a->data, token);
					}
					if(rs){
						break;
					}
				}
				free(tofree);

	            if(!rs){
	            	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	            }
	            else{
	            	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	            }

            }else if(strcmp(el->data,"LIKE")==0 || strcmp(el->data,"~~")==0){

           		char like_regex[] = "([]:alpha:[!%_^]*)";
				int rs;

           		if(AK_check_regex_operator_expression(b->data,&like_regex)){

            		rs = AK_check_regex_expression(a->data,b->data,1,1);

					if(rs){
            			AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
					} else{
						AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
					}
            	}else{
            		printf("Could not compile LIKE expression, check your sintax.\n");
            	}

            }else if(strcmp(el->data,"NOT LIKE")==0){

           		char like_regex[] = "([]:alpha:[!%_^]*)";
				int rs;

           		if(AK_check_regex_operator_expression(b->data,&like_regex)){

            		rs = AK_check_regex_expression(a->data,b->data,1,1);

					if(!rs){
            			AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
					} else{
						AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
					}
            	}else{
            		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
            	}

            }else if(strcmp(el->data,"ILIKE")==0 || strcmp(el->data,"~~*")==0){

            	char like_regex[] = "([]:alpha:[!%_^]*)";
            	int rs;

            	if(AK_check_regex_operator_expression(b->data,&like_regex)){

            		rs = AK_check_regex_expression(a->data,b->data,0,1);

					if(rs){
            			AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
					} else {
						AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
					}
            	}else{
            		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
            	}
            }else if(strcmp(el->data,"NOT ILIKE")==0){

            	char like_regex[] = "([]:alpha:[!%_^]*)";
            	int rs;

            	if(AK_check_regex_operator_expression(b->data,&like_regex)){

            		rs = AK_check_regex_expression(a->data,b->data,0,1);

					if(!rs){
            			AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
					} else {
						AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
					}
            	}else{
            		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
            	}
            }else if(strcmp(el->data,"SIMILAR TO")==0){
            	char similar_regex[] = "([]:alpha:[!%_^|*+()!]*)";
            	int rs;

            	if(AK_check_regex_operator_expression(b->data,similar_regex)){
            		rs = AK_check_regex_expression(a->data,b->data,1,1);
            		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
            	}else{
            		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
            	}
            	
            }else if(strcmp(el->data,"~")==0){
            	//regex match implementation case sensitive
            	int rs;
            	rs = AK_check_regex_expression(a->data,b->data,1,0);
				if(rs){
            		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
				} else {
					AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
				}
            }else if(strcmp(el->data,"!~")==0){
            	//regex match implementation case sensitive
            	int rs;
            	rs = AK_check_regex_expression(a->data,b->data,1,0);
				if(!rs){
            		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
				} else {
					AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
				}
            }else if(strcmp(el->data,"~*")==0){
            	//regex match implementation case sensitive insensitive
            	int rs;
            	rs = AK_check_regex_expression(a->data,b->data,0,0);
				if(rs){
            		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
				} else {
					AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
				}
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
	printf("EXPRESSION CHECK TEST STARTS\n");
    AK_PRO;

	printf("EXPRESSION CHECK TEST 1:\n");
    struct list_node *elem = (struct list_node *) AK_malloc(sizeof(struct list_node));
    elem->type = TYPE_INT;
    const char *op = "+";
    const char *a = "800";
    const char *b = "400";
    int outcome;
    int outcome2;
    int outcome3;
    int successful = 0;
	int failed = 0;
	char *srcTable="student";
	char *destTable="select_result";

    int likeOutcome1,likeOutcome2,likeOutcome3,likeOutcome4,likeOutcome5;



 	char value [200] = "abc";
 	char value2 [200] = "thomas";
 	char expression [200] = "abc";
    char expression2 [200] = "a%";
 	char expression3 [200] = "_b_";
 	char expression4 [200] = "%Thomas%";
 	char expression5 [200] = "%thomas%";

 	if(AK_check_regex_expression(value,expression,1,1)){
		successful++;
	}else failed++;
	if(AK_check_regex_expression(value,expression2,1,1)){
		successful++;
	} else failed++;
 	if(AK_check_regex_expression(value,expression3,1,1)){
		successful++;
	} else failed++;
 	if(AK_check_regex_expression(value2, expression4,0,1)){
		successful++;
	} else failed++;
 	if(AK_check_regex_expression(value2, expression5,1,1)){
		successful++;
	} else failed++;

 	printf("Test for like,Ilike with wildcards \n");
    printf("abc - expression 'abc' outcome is: %d\n", likeOutcome1);
    printf("abc - expression 'a.*'  outcome is: %d\n", likeOutcome2);
    printf("abc - expression  _b_ outcome is: %d\n", likeOutcome3);
    printf("ILike   thomas - expression .*Thomas.* outcome is: %d\n", likeOutcome4);
    printf("Like thomas - expression .*thomas.* outcome is %d\n", likeOutcome5);
	
    AK_free(elem);
	
    printf("EXPRESSION CHECK TEST 2 (test relation operation IN):");
	
	// list of attributes which will be in the result of selection
	struct list_node *attributes = (struct list_node *) AK_malloc(sizeof (struct list_node));

	// list of elements which represent the condition for selection
	struct list_node *condition = (struct list_node *) AK_malloc(sizeof (struct list_node));


	printf("TEST ZA OPERATOR IN ZAPOČINJE!\n");

	AK_Init_L3(&attributes);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes);

	AK_Init_L3(&condition);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition);
    char conditionAtributes[] = "Marina,Dino";

	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes, sizeof(conditionAtributes), condition);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "IN", sizeof("IN"), condition);



    if (AK_select(srcTable, destTable, attributes, condition) == EXIT_SUCCESS)
    {
        successful++;
    }   
    else
    {
        failed++;
    }
	
	AK_DeleteAll_L3(&attributes);
	AK_DeleteAll_L3(&condition);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname IN ('Marina','Dino');\n\n");
    AK_print_table(destTable);
	
	AK_free(attributes);
	AK_free(condition);

	printf("TEST ZA OPERATOR >ANY ZAPOČINJE!\n");

	struct list_node *attributes2 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition2 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable2="select_result2";
	AK_Init_L3(&attributes2);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes2);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes2);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes2);

	AK_Init_L3(&condition2);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition2);
    char conditionAtributes2[] = "2008,2009,2010,2011";

	AK_InsertAtEnd_L3(TYPE_INT, conditionAtributes2, sizeof(conditionAtributes2), condition2);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, ">ANY", sizeof(">ANY"), condition2);



    if (AK_select(srcTable, destTable2, attributes2, condition2) == EXIT_SUCCESS)
    {
        successful++;
    }   
    else
    {
        failed++;
    }
	
	AK_DeleteAll_L3(&attributes2);
	AK_DeleteAll_L3(&condition2);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year >ANY (2008,2009,2010,2011);\n\n");
    AK_print_table(destTable2);
	
	AK_free(attributes2);
	AK_free(condition2);

	printf("TEST ZA OPERATOR <=ALL ZAPOČINJE!\n");

	struct list_node *attributes3 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition3 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable3="select_result3";
	AK_Init_L3(&attributes3);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes3);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes3);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes3);

	AK_Init_L3(&condition3);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition3);
    char conditionAtributes3[] = "2012,2013,2014";

	AK_InsertAtEnd_L3(TYPE_INT, conditionAtributes3, sizeof(conditionAtributes3), condition3);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<=ALL", sizeof("<=ALL"), condition3);



    if (AK_select(srcTable, destTable3, attributes3, condition3) == EXIT_SUCCESS)
    {
        successful++;
    }   
    else
    {
        failed++;
    }
	
	AK_DeleteAll_L3(&attributes3);
	AK_DeleteAll_L3(&condition3);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year <=ALL (2012,2013,2014);\n\n");
    AK_print_table(destTable3);
	
	AK_free(attributes3);
	AK_free(condition3);

	printf("TEST ZA OPERATOR LIKE\n");

	struct list_node *attributes4 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition4 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable4="select_result4";
	AK_Init_L3(&attributes4);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes4);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes4);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes4);

	AK_Init_L3(&condition4);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition4);
    char conditionAtributes4[] = "%a";

	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes4, sizeof(conditionAtributes4), condition4);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "LIKE", sizeof("LIKE"), condition4);



    if (AK_select(srcTable, destTable4, attributes4, condition4) == EXIT_SUCCESS)
    {
        successful++;
    }   
    else
    {
        failed++;
    }
	
	AK_DeleteAll_L3(&attributes4);
	AK_DeleteAll_L3(&condition4);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname LIKE '%a';\n\n");
    AK_print_table(destTable4);
	
	AK_free(attributes4);
	AK_free(condition4);

	printf("TEST ZA OPERATOR NOT ILIKE\n");

	struct list_node *attributes5 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition5 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable5="select_result5";
	AK_Init_L3(&attributes5);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes5);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes5);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes5);

	AK_Init_L3(&condition5);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition5);
    char conditionAtributes5[] = "m%";

	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes5, sizeof(conditionAtributes5), condition5);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "NOT ILIKE", sizeof("NOT ILIKE"), condition5);



    if (AK_select(srcTable, destTable5, attributes5, condition5) == EXIT_SUCCESS)
    {
        successful++;
    }   
    else
    {
        failed++;
    }
	
	AK_DeleteAll_L3(&attributes5);
	AK_DeleteAll_L3(&condition5);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname NOT ILIKE 'm%';\n\n");
    AK_print_table(destTable5);
	
	AK_free(attributes5);
	AK_free(condition5);

	printf("TEST ZA OPERATOR ~\n");

	struct list_node *attributes6 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition6 = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable6="select_result6";
	AK_Init_L3(&attributes6);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes6);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes6);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes6);

	AK_Init_L3(&condition6);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition6);
    char conditionAtributes6[] = "^Ma";

	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes6, sizeof(conditionAtributes6), condition6);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "~", sizeof("~"), condition6);



    if (AK_select(srcTable, destTable6, attributes6, condition6) == EXIT_SUCCESS)
    {
        successful++;
    }   
    else
    {
        failed++;
    }
	
	AK_DeleteAll_L3(&attributes6);
	AK_DeleteAll_L3(&condition6);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname ~ '^Ma';\n\n");
    AK_print_table(destTable6);
	
	AK_free(attributes6);
	AK_free(condition6);


    AK_EPI;
    return TEST_result(successful, failed);
}