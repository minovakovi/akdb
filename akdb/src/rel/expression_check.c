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

#include "expression_check.h"

typedef enum {
    OP_LT, OP_GT, OP_LE, OP_GE, OP_EQ, OP_NE, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_INVALID
} Operator;

Operator get_operator(const char *op) {
    if (strcmp(op, "<") == 0) return OP_LT;
    if (strcmp(op, ">") == 0) return OP_GT;
    if (strcmp(op, "<=") == 0) return OP_LE;
    if (strcmp(op, ">=") == 0) return OP_GE;
    if (strcmp(op, "=") == 0) return OP_EQ;
    if (strcmp(op, "!=") == 0) return OP_NE;
    if (strcmp(op, "+") == 0) return OP_ADD;
    if (strcmp(op, "-") == 0) return OP_SUB;
    if (strcmp(op, "*") == 0) return OP_MUL;
    if (strcmp(op, "/") == 0) return OP_DIV;
    return OP_INVALID;
}

int compare_lt(void *a, void *b, int type) {
    switch (type) {
        case TYPE_INT: return *(int *)a < *(int *)b;
        case TYPE_FLOAT: return *(float *)a < *(float *)b;
        case TYPE_NUMBER: return *(double *)a < *(double *)b;
        case TYPE_VARCHAR: return strcmp((const char *)a, (const char *)b) < 0;
        default: return 0;
    }
}

int compare_gt(void *a, void *b, int type) {
    switch (type) {
        case TYPE_INT: return *(int *)a > *(int *)b;
        case TYPE_FLOAT: return *(float *)a > *(float *)b;
        case TYPE_NUMBER: return *(double *)a > *(double *)b;
        case TYPE_VARCHAR: return strcmp((const char *)a, (const char *)b) > 0;
        default: return 0;
    }
}

int compare_le(void *a, void *b, int type) {
    switch (type) {
        case TYPE_INT: return *(int *)a <= *(int *)b;
        case TYPE_FLOAT: return *(float *)a <= *(float *)b;
        case TYPE_NUMBER: return *(double *)a <= *(double *)b;
        case TYPE_VARCHAR: return strcmp((const char *)a, (const char *)b) <= 0;
        default: return 0;
    }
}

int compare_ge(void *a, void *b, int type) {
    switch (type) {
        case TYPE_INT: return *(int *)a >= *(int *)b;
        case TYPE_FLOAT: return *(float *)a >= *(float *)b;
        case TYPE_NUMBER: return *(double *)a >= *(double *)b;
        case TYPE_VARCHAR: return strcmp((const char *)a, (const char *)b) >= 0;
        default: return 0;
    }
}

int compare_eq(void *a, void *b, int type) {
    switch (type) {
        case TYPE_INT: return *(int *)a == *(int *)b;
        case TYPE_FLOAT: return *(float *)a == *(float *)b;
        case TYPE_NUMBER: return *(double *)a == *(double *)b;
        case TYPE_VARCHAR: return strcmp((const char *)a, (const char *)b) == 0;
        default: return 0;
    }
}

int compare_ne(void *a, void *b, int type) {
    switch (type) {
        case TYPE_INT: return *(int *)a != *(int *)b;
        case TYPE_FLOAT: return *(float *)a != *(float *)b;
        case TYPE_NUMBER: return *(double *)a != *(double *)b;
        case TYPE_VARCHAR: return strcmp((const char *)a, (const char *)b) != 0;
        default: return 0;
    }
}

int arithmetic_add(const char *a, const char *b, char **end, int type) {
    switch (type) {
        case TYPE_INT: return strtol(a, end, 10) + strtol(b, end, 10);
        case TYPE_FLOAT: return strtod(a, end) + strtod(b, end);
        case TYPE_NUMBER: return strtod(a, end) + strtod(b, end);
        default: return EXIT_ERROR;
    }
}

int arithmetic_sub(const char *a, const char *b, char **end, int type) {
    switch (type) {
        case TYPE_INT: return strtol(a, end, 10) - strtol(b, end, 10);
        case TYPE_FLOAT: return strtod(a, end) - strtod(b, end);
        case TYPE_NUMBER: return strtod(a, end) - strtod(b, end);
        default: return EXIT_ERROR;
    }
}

int arithmetic_mul(const char *a, const char *b, char **end, int type) {
    switch (type) {
        case TYPE_INT: return strtol(a, end, 10) * strtol(b, end, 10);
        case TYPE_FLOAT: return strtod(a, end) * strtod(b, end);
        case TYPE_NUMBER: return strtod(a, end) * strtod(b, end);
        default: return EXIT_ERROR;
    }
}

int arithmetic_div(const char *a, const char *b, char **end, int type) {
    switch (type) {
        case TYPE_INT: return strtol(a, end, 10) / strtol(b, end, 10);
        case TYPE_FLOAT: return strtod(a, end) / strtod(b, end);
        case TYPE_NUMBER: return strtod(a, end) / strtod(b, end);
        default: return EXIT_ERROR;
    }
}

/**
 * @author Dino Laktašić, abstracted by Tomislav Mikulček,updated by Nikola Miljancic, updated by Fran Turković, updated by Karlo Rusovan
 * @brief  Function that compares values according to their data type, checks arithmetic statement which is part of expression given in 
 *   	  the function below. For every type of arithmetic operator, there is switch-case statement which examines type of el and
           casts void operands to this type.
 * @param el list element, last element put in list temp which holds elements of row ordered according to expression and results of their evaluation
 * @param *op comparison operator
 * @param *a left operand
 * @param *b right operand
 * @return 0 if arithmetic statement is false, 1 if arithmetic statement is true
 */

//int AK_check_arithmetic_statement(AK_list_elem el, const char *op, const char *a, const char *b){
int AK_check_arithmetic_statement(struct list_node *el, const char *op, const char *a, const char *b) {  
    AK_PRO;
    char **numericStringEnd = NULL; 

    Operator operator = get_operator(op);
    if (operator == OP_INVALID) {
        return EXIT_ERROR;
    }

    switch (operator) {
        case OP_LT: return compare_lt((void *)a, (void *)b, el->type);
        case OP_GT: return compare_gt((void *)a, (void *)b, el->type);
        case OP_LE: return compare_le((void *)a, (void *)b, el->type);
        case OP_GE: return compare_ge((void *)a, (void *)b, el->type);
        case OP_EQ: return compare_eq((void *)a, (void *)b, el->type);
        case OP_NE: return compare_ne((void *)a, (void *)b, el->type);
        case OP_ADD: return arithmetic_add(a, b, numericStringEnd, el->type);
        case OP_SUB: return arithmetic_sub(a, b, numericStringEnd, el->type);
        case OP_MUL: return arithmetic_mul(a, b, numericStringEnd, el->type);
        case OP_DIV: return arithmetic_div(a, b, numericStringEnd, el->type);
        default: return EXIT_ERROR;
    }
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
	char * regexExpression;
	char * result;  
	regex_t regexCompiled;
	int isMatched;
	int caseSens = REG_EXTENDED;

	if(checkWildCard){
		regexExpression = AK_add_start_end_regex_chars(expression);
	}
	else{
		regexExpression = expression;
	}

	if(!sensitive){
		caseSens |= REG_ICASE;
	}
	if(checkWildCard){
		result = AK_replace_wild_card(regexExpression,'%',".*");
		regexExpression = AK_replace_wild_card(result,'_',".");
		AK_free(result);
	}
	if (regcomp(&regexCompiled, regexExpression, caseSens)){
        printf("Could not compile regular expression, check your sintax.\n");
        isMatched = 0;
    }
    if(checkWildCard)
		AK_free(regexExpression);
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
	char * regexExpression = expression;
	regex_t regexCompiled;
	int isMatched ;
	if (regcomp(&regexCompiled, regexExpression, REG_EXTENDED)){
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


void evaluate_equal_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	if (strcmp((char *)a->data, (char *)b->data) == 0) {
        AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result);
    } else {
        AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
    }
}

void evaluate_not_equal_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	if (strcmp((char *)a->data, (char *)b->data) != 0) {
        AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result);
    } else {
        AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
    }
}

void evaluate_or_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	char val_a, val_b;
	struct list_node *last = AK_End_L2(temp_result);
	struct list_node *previous = AK_Previous_L2(last, temp_result);
	memcpy(&val_a, last->data, sizeof(char));
	memcpy(&val_b, previous->data, sizeof(char));
	if (val_a || val_b) {
	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result);
	} else {
	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
	}
}

void evaluate_and_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
	char val_a, val_b;
	struct list_node *last = AK_End_L2(temp_result);
	struct list_node *previous = AK_Previous_L2(last, temp_result);
	memcpy(&val_a, last->data, sizeof (char));
	memcpy(&val_b, previous->data, sizeof (char));

	if (val_a && val_b){		
		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	}else{
		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	}
}

void evaluate_between_operator(struct list_node *a, struct list_node *b, struct list_node *c, struct list_node *temp_result) {
    char true = 1, false = 0;
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
}
    
void evaluate_in_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
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
}

void evaluate_gt_any_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
    	int rs = 0;
		const char *my_str_literal = b->data;
		char *token, *str, *tofree;
		tofree = str = strdup(my_str_literal);
		int firstPassed = 0;  
		while ((token = strsep(&str, ","))){
			if(a->type==TYPE_INT){
				int token2 = atoi(token);
				rs = AK_check_arithmetic_statement(b, ">", a->data, (char *)&token2);
			}
			else if(a->type==TYPE_FLOAT){
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
}

void evaluate_lt_any_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
    int rs = 0;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			int token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, "<", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
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
}

void evaluate_lte_any_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
    int rs = 0;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	int token2;
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, "<=", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
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
}

void evaluate_gte_any_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
    int rs = 0;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			int token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, ">=", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
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
}

void evaluate_ne_any_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
    int rs = 0;
    const char *my_str_literal = b->data;
    char *token, *str, *tofree;
    tofree = str = strdup(my_str_literal);
    while ((token = strsep(&str, ","))) {
        if (a->type == TYPE_INT) {
            int token2 = atoi(token);
            int check = AK_check_arithmetic_statement(a, "!=", a->data, (char *)&token2);
            if (check) {
                rs = 1;
                break;
            }
        } else if (a->type == TYPE_FLOAT) {
            float token2 = atof(token);
            int check = AK_check_arithmetic_statement(a, "!=", a->data, (char *)&token2);
            if (check) {
                rs = 1;
                break;
            }
        } else {
            int check = AK_check_arithmetic_statement(a, "!=", a->data, token);
            if (check) {
                rs = 1;
                break;
            }
        }
    }
	free(tofree);

    if (rs) {
        AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result);
    } else {
        AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
    }
}

void evaluate_gt_all_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			int token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, ">", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
			float token2 = atof(token);
			rs = AK_check_arithmetic_statement(b, ">", a->data, (char *)&token2);
		}
		else{
			rs = AK_check_arithmetic_statement(b, ">", a->data, token);
		}
		if(!rs){
			 free(tofree);
            AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
            return;
		}
	}
	free(tofree);

	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
}

void evaluate_lt_all_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			int token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, "<", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
			float token2 = atof(token);
			rs = AK_check_arithmetic_statement(b, "<", a->data, (char *)&token2);
		}
		else{
			rs = AK_check_arithmetic_statement(b, "<", a->data, token);
		}
		if (!rs) {
            free(tofree);
            AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
            return;
        }
    }
    free(tofree);

    AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result);
}

void evaluate_gte_all_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			int token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, ">=", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
			float token2 = atof(token);
			rs = AK_check_arithmetic_statement(b, ">=", a->data, (char *)&token2);
		}
		else{
			rs = AK_check_arithmetic_statement(b, ">=", a->data, token);
		}
		if (!rs) { 
            free(tofree);
            AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
            return;
        }
    }

    free(tofree);
    AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result);
}

void evaluate_lte_all_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	
	char true = 1, false = 0;
	int rs = 1;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))) {
	    if (a->type == TYPE_INT) {
            int token2 = atoi(token);
            if (!AK_check_arithmetic_statement(b, "<=", a->data, (char *)&token2)) {
                rs = 0;
                break;
            }
        } else if (a->type == TYPE_FLOAT) {
            float token2 = atof(token);
            if (!AK_check_arithmetic_statement(b, "<=", a->data, (char *)&token2)) {
                rs = 0;
                break;
            }
        } else {
            if (!AK_check_arithmetic_statement(b, "<=", a->data, token)) {
                rs = 0;
                break;
            }
        }
    }

	free(tofree);

    if(rs){
    	AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
    }
    else{
    	AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
    }
}

void evaluate_ne_all_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			int token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
			float token2 = atof(token);
			rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
		}
		else{
			rs = AK_check_arithmetic_statement(b, "=", a->data, token);
		}
		 if (rs) {
            AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
            free(tofree);
            return;
        }
    }

    AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result); 
    free(tofree);
}

void evaluate_e_all_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	const char *my_str_literal = b->data;
	char *token, *str, *tofree;
	tofree = str = strdup(my_str_literal);
	int firstPassed = 0;  
	while ((token = strsep(&str, ","))){
		if(a->type==TYPE_INT){
			int token2 = atoi(token);
			rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
		}
		else if(a->type==TYPE_FLOAT){
			float token2 = atof(token);
			rs = AK_check_arithmetic_statement(b, "=", a->data, (char *)&token2);
		}
		else{
			rs = AK_check_arithmetic_statement(b, "=", a->data, token);
		}
		 if (!rs) {
            AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
            free(tofree);
            return;
        }
    }

    AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result); 
    free(tofree);
}

void evaluate_like_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
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
}

void evaluate_not_like_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;	
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
}

void evaluate_ilike_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;	
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
}

void evaluate_not_ilike_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;	
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
}

void evaluate_similar_to_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
    char true = 1, false = 0;
    char similar_regex[] = "([]:alpha:[!%_^|*+()!]*)";
    int rs;

    if (AK_check_regex_operator_expression(b->data, similar_regex)) {
        rs = AK_check_regex_expression(a->data, b->data, 1, 1);
        if (rs) {
            AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof(int), temp_result);
        } else {
            AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
        }
    } else {
        AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof(int), temp_result);
    }
}


void evaluate_tilde_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	rs = AK_check_regex_expression(a->data,b->data,1,0);
	if(rs){
		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	} else {
		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	}
}

void evaluate_not_tilde_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	rs = AK_check_regex_expression(a->data,b->data,1,0);
	if(!rs){
		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	} else {
		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	}
}

void evaluate_tilde_star_operator(struct list_node *a, struct list_node *b, struct list_node *temp_result) {
	char true = 1, false = 0;
	int rs;
	rs = AK_check_regex_expression(a->data,b->data,0,0);
	if(rs){
		AK_InsertAtEnd_L3(TYPE_INT, &true, sizeof (int), temp_result);
	} else {
		AK_InsertAtEnd_L3(TYPE_INT, &false, sizeof (int), temp_result);
	}		
}

/**
 * @author Matija Šestak, updated by Dino Laktašić,Nikola Miljancic, abstracted by Tomislav Mikulček, updated by Fran Turković, Karlo Rusovan
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
                evaluate_equal_operator(a, b, temp_result);
            } else if (strcmp(el->data, "<>") == 0) {
                evaluate_not_equal_operator(a, b, temp_result);
            } else if (strcmp(el->data, "OR") == 0) {
                evaluate_or_operator(a, b, temp_result);
            } else if (strcmp(el->data, "AND") == 0) {
                evaluate_and_operator(a, b, temp_result);
            } else if(strcmp(el->data,"BETWEEN")==0){
            	evaluate_between_operator(a, b, c, temp_result);
			}else if(strcmp(el->data,"IN")==0 || strcmp(el->data,"=ANY")==0 || strcmp(el->data,"= ANY")==0){
            	evaluate_in_operator(a, b, temp_result);
			}else if(strcmp(el->data,">ANY")==0 || strcmp(el->data,"> ANY")==0){
            	evaluate_gt_any_operator(a, b, temp_result);
			}else if(strcmp(el->data,"<ANY")==0 || strcmp(el->data,"< ANY")==0){
            	evaluate_lt_any_operator(a, b, temp_result);
			}else if(strcmp(el->data,"<=ANY")==0 || strcmp(el->data,"<= ANY")==0){
            	evaluate_lte_any_operator(a, b, temp_result);
			}else if(strcmp(el->data,">=ANY")==0 || strcmp(el->data,">= ANY")==0){
            	evaluate_gte_any_operator(a, b, temp_result);
			}else if(strcmp(el->data,"!=ANY")==0 || strcmp(el->data,"!= ANY")==0 || strcmp(el->data,"<>ANY")==0 || strcmp(el->data,"<> ANY")==0){
            	evaluate_ne_any_operator(a, b, temp_result);
			}else if(strcmp(el->data,">ALL")==0 || strcmp(el->data,"> ALL")==0){
            	evaluate_gt_all_operator(a, b, temp_result);
			}else if(strcmp(el->data,"<ALL")==0 || strcmp(el->data,"< ALL")==0){
            	evaluate_lt_all_operator(a, b, temp_result);
			}else if(strcmp(el->data,">=ALL")==0 || strcmp(el->data,">= ALL")==0){
            	evaluate_gte_all_operator(a, b, temp_result);
			}else if(strcmp(el->data,"<=ALL")==0 || strcmp(el->data,"<= ALL")==0){
            	evaluate_lte_all_operator(a, b, temp_result);
			}else if(strcmp(el->data,"!=ALL")==0 || strcmp(el->data,"!= ALL")==0 || strcmp(el->data,"<>ALL")==0 || strcmp(el->data,"<> ALL")==0){
            	evaluate_ne_all_operator(a, b, temp_result);
			}else if(strcmp(el->data,"=ALL")==0 || strcmp(el->data,"= ALL")==0){
            	evaluate_e_all_operator(a, b, temp_result);
            }else if(strcmp(el->data,"LIKE")==0 || strcmp(el->data,"~~")==0){
				evaluate_like_operator(a, b, temp_result);
            }else if(strcmp(el->data,"NOT LIKE")==0){
				evaluate_not_like_operator(a, b, temp_result);
            }else if(strcmp(el->data,"ILIKE")==0 || strcmp(el->data,"~~*")==0){
				evaluate_ilike_operator(a, b, temp_result);
            }else if(strcmp(el->data,"NOT ILIKE")==0){
				evaluate_not_ilike_operator(a, b, temp_result);
            }else if(strcmp(el->data,"SIMILAR TO")==0){
            	evaluate_similar_to_operator(a, b, temp_result);
            }else if(strcmp(el->data,"~")==0){
            	evaluate_tilde_operator(a, b, temp_result);
            }else if(strcmp(el->data,"!~")==0){
            	evaluate_not_tilde_operator(a, b, temp_result);
            }else if(strcmp(el->data,"~*")==0){
            	evaluate_tilde_star_operator(a, b, temp_result);
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

/**
 * @brief Function for testing expression checks.
 *
 * Function which performs tests for regex  and relational operators (LIKE, ILIKE, IN, ANY, ALL) 
 * using predefined test data. It executes selection queries with different conditions, validates results, 
 * and prints outcomes.
 *
 * @return The test result indicating the number of successful and failed tests.
 */
TestResult AK_expression_check_test()
{
    AK_PRO;
    int successful = 0;
	int failed = 0;
	char *srcTable="student";

	test_expression_check(&successful, &failed);

	test_operator_in(srcTable, &successful, &failed);

	test_operator_eq(srcTable, &successful, &failed);

	test_operator_not_eq(srcTable, &successful, &failed);

	test_operator_or(srcTable, &successful, &failed);

	test_operator_and(srcTable, &successful, &failed);

	test_operator_between(srcTable, &successful, &failed);
	
	test_operator_any(srcTable, &successful, &failed);
	
	test_operator_all(srcTable, &successful, &failed);

	test_operator_like(srcTable, &successful, &failed);

	test_operator_not_like(srcTable, &successful, &failed);

	test_operator_ilike(srcTable, &successful, &failed);

	test_operator_not_ilike(srcTable, &successful, &failed);

	test_operator_similar_to(srcTable, &successful, &failed);

	test_operator_tilde(srcTable, &successful, &failed);

	test_operator_not_tilde(srcTable, &successful, &failed);

	test_operator_tilde_star(srcTable, &successful, &failed);

    AK_EPI;
    return TEST_result(successful, failed);
}

void test_expression_check(int *successful, int *failed){
	  int likeOutcome1 = 0, likeOutcome2 = 0, likeOutcome3 = 0, likeOutcome4 = 0,likeOutcome5 = 0;

 	char value [200] = "abc";
 	char value2 [200] = "thomas";
 	char expression [200] = "abc";
    char expression2 [200] = "a%";
 	char expression3 [200] = "_b_";
 	char expression4 [200] = "%Thomas%";
 	char expression5 [200] = "%thomas%";

 	if(AK_check_regex_expression(value,expression,1,1)){
		likeOutcome1 = 1;
		(*successful)++;
	}else failed++;
	if(AK_check_regex_expression(value,expression2,1,1)){
		likeOutcome2 = 1;
		(*successful)++;
	} else failed++;
 	if(AK_check_regex_expression(value,expression3,1,1)){
		likeOutcome3 = 1;
		(*successful)++;
	} else failed++;
 	if(AK_check_regex_expression(value2, expression4,0,1)){
		likeOutcome4 = 1;
		(*successful)++;
	} else failed++;
 	if(AK_check_regex_expression(value2, expression5,1,1)){
		likeOutcome5 = 1;
		(*successful)++;
	} else failed++;

 	printf("Test for like,Ilike with wildcards \n");
    printf("abc - expression 'abc' outcome is: %d\n", likeOutcome1);
    printf("abc - expression 'a.*'  outcome is: %d\n", likeOutcome2);
    printf("abc - expression  _b_ outcome is: %d\n", likeOutcome3);
    printf("ILike   thomas - expression .*Thomas.* outcome is: %d\n", likeOutcome4);
    printf("Like thomas - expression .*thomas.* outcome is %d\n", likeOutcome5);
	
}

void test_operator_in(char *srcTable, int *successful, int *failed) {
	printf("-------------------------------------------------------------------------\n");	
	printf("TEST ZA OPERATOR IN ZAPOČINJE!\n");

	struct list_node *attributes_in = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition_in = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable_in = "select_result_in";

	AK_Init_L3(&attributes_in);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_in);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_in);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_in);

	AK_Init_L3(&condition_in);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_in);
    
	char *conditionAtributes_in = AK_malloc(strlen("Marina,Dino") + 1);
	strcpy(conditionAtributes_in, "Marina,Dino");

	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_in, strlen(conditionAtributes_in) + 1, condition_in);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "IN", sizeof("IN"), condition_in);

    if (AK_select(srcTable, destTable_in, attributes_in, condition_in, NULL) == EXIT_SUCCESS)
    {
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	
	AK_DeleteAll_L3(&attributes_in);
	AK_DeleteAll_L3(&condition_in);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname IN ('Marina','Dino');\n\n");
    AK_print_table(destTable_in);
	
	AK_free(attributes_in);
	AK_free(condition_in);
	
}

void test_operator_eq(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR = ZAPOČINJE!\n");

	struct list_node *attributes_eq = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_eq = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_eq = "select_result_eq";
	AK_Init_L3(&attributes_eq);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_eq);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_eq);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_eq);

	AK_Init_L3(&condition_eq);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_eq);
	AK_InsertAtEnd_L3(TYPE_VARCHAR, "Ivan", strlen("Ivan") + 1, condition_eq);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof("="), condition_eq);

	AK_print_table(srcTable);
	AK_print_table(destTable_eq);
	if (AK_select(srcTable, destTable_eq, attributes_eq, condition_eq, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}


	AK_DeleteAll_L3(&attributes_eq);
	AK_DeleteAll_L3(&condition_eq);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname = 'Ivan';\n\n");
	AK_print_table(destTable_eq);

	AK_free(attributes_eq);
	AK_free(condition_eq);

}

void test_operator_not_eq(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR <> ZAPOČINJE!\n");

	struct list_node *attributes_ne = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_ne = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_ne = "select_result_ne";
	AK_Init_L3(&attributes_ne);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_ne);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_ne);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_ne);

	AK_Init_L3(&condition_ne);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_ne);
	AK_InsertAtEnd_L3(TYPE_VARCHAR, "Ivan", strlen("Ivan") + 1, condition_ne);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<>", sizeof("<>"), condition_ne);

	if (AK_select(srcTable, destTable_ne, attributes_ne, condition_ne, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_ne);
	AK_DeleteAll_L3(&condition_ne);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname <> 'Ivan';\n\n");
	AK_print_table(destTable_ne);

	AK_free(attributes_ne);
	AK_free(condition_ne);
}

void test_operator_or(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR OR ZAPOČINJE!\n");

	struct list_node *attributes_or = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_or = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_or = "select_result_or";

	AK_Init_L3(&attributes_or);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_or);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_or);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_or);


	AK_Init_L3(&condition_or);

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_or);
	int year_val = 2012;
	AK_InsertAtEnd_L3(TYPE_INT, &year_val, sizeof(int), condition_or);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof("="), condition_or);

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_or);
	year_val = 2011;
	AK_InsertAtEnd_L3(TYPE_INT, &year_val, sizeof(int), condition_or);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof("="), condition_or);


	AK_InsertAtEnd_L3(TYPE_OPERATOR, "OR", sizeof("OR"), condition_or);

	if (AK_select(srcTable, destTable_or, attributes_or, condition_or, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	AK_DeleteAll_L3(&attributes_or);
	AK_DeleteAll_L3(&condition_or);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year = 2012 OR year = 2011;\n\n");
	AK_print_table(destTable_or);

	AK_free(attributes_or);
	AK_free(condition_or);

}

void test_operator_and(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR AND ZAPOČINJE!\n");

	struct list_node *attributes_and = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_and = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_and = "select_result_and";

	AK_Init_L3(&attributes_and);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_and);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_and);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_and);

	AK_Init_L3(&condition_and);

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_and);
	int year_val_and = 2000;
	AK_InsertAtEnd_L3(TYPE_INT, &year_val_and, sizeof(int), condition_and);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof("="), condition_and);

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_and);
	year_val_and = 2000;
	AK_InsertAtEnd_L3(TYPE_INT, &year_val_and, sizeof(int), condition_and);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "=", sizeof("="), condition_and);

	AK_InsertAtEnd_L3(TYPE_OPERATOR, "AND", sizeof("AND"), condition_and);

	if (AK_select(srcTable, destTable_and, attributes_and, condition_and, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_and);
	AK_DeleteAll_L3(&condition_and);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname = 'Dino' AND year = 2010;\n\n");
	AK_print_table(destTable_and);

	AK_free(attributes_and);
	AK_free(condition_and);
}

void test_operator_between(char *srcTable, int *successful, int *failed) {

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR BETWEEN ZAPOČINJE!\n");

	struct list_node *attributes_between = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_between = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_between = "select_result_between";

	AK_Init_L3(&attributes_between);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_between);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_between);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_between);

	AK_Init_L3(&condition_between);

	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_between);

	int val_start = 2000;
	int val_end = 2012;

	AK_InsertAtEnd_L3(TYPE_INT, &val_start, sizeof(int), condition_between);
	AK_InsertAtEnd_L3(TYPE_INT, &val_end, sizeof(int), condition_between);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "BETWEEN", sizeof("BETWEEN"), condition_between);


	if (AK_select(srcTable, destTable_between, attributes_between, condition_between, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_between);
	AK_DeleteAll_L3(&condition_between);

	AK_print_table(srcTable);
	printf("\n SELECT year, firstname, weight FROM student WHERE year BETWEEN 2000 AND 2012;\n\n");
	AK_print_table(destTable_between);

	AK_free(attributes_between);
	AK_free(condition_between);
}

void test_operator_any(char *srcTable, int *successful, int *failed) {

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR >ANY ZAPOČINJE!\n");

	struct list_node *attributes_gt_any = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition_gt_any = (struct list_node *) AK_malloc(sizeof (struct list_node));


	char *destTable_gt_any="select_result_gt_any";
	AK_Init_L3(&attributes_gt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_gt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_gt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_gt_any);

	AK_Init_L3(&condition_gt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_gt_any);
    char *conditionAtributes_gt_any = AK_malloc(strlen("2008,2011") + 1);
	strcpy(conditionAtributes_gt_any, "2008,2011");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_gt_any, strlen(conditionAtributes_gt_any) + 1, condition_gt_any);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, ">ANY", sizeof(">ANY"), condition_gt_any);



    if (AK_select(srcTable, destTable_gt_any, attributes_gt_any, condition_gt_any, NULL) == EXIT_SUCCESS)
    {
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	
	AK_DeleteAll_L3(&attributes_gt_any);
	AK_DeleteAll_L3(&condition_gt_any);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year >ANY (2008,2011);\n\n");
    AK_print_table(destTable_gt_any);
	
	AK_free(attributes_gt_any);
	AK_free(condition_gt_any);
	AK_free(conditionAtributes_gt_any);


	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR <ANY ZAPOČINJE!\n");

	struct list_node *attributes_lt_any = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_lt_any = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_lt_any = "select_result_lt_any";

	AK_Init_L3(&attributes_lt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_lt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_lt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_lt_any);

	AK_Init_L3(&condition_lt_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_lt_any);
	char *conditionAttributes_lt_any = AK_malloc(strlen("2008,2011") + 1);
	strcpy(conditionAttributes_lt_any, "2008,2011");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_lt_any, strlen(conditionAttributes_lt_any) + 1, condition_lt_any);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<ANY", sizeof("<ANY"), condition_lt_any);

	if (AK_select(srcTable, destTable_lt_any, attributes_lt_any, condition_lt_any, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_lt_any);
	AK_DeleteAll_L3(&condition_lt_any);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year <ANY (2008,2011);\n\n");
	AK_print_table(destTable_lt_any);

	AK_free(attributes_lt_any);
	AK_free(condition_lt_any);
	AK_free(conditionAttributes_lt_any);


	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR >=ANY ZAPOČINJE!\n");

	struct list_node *attributes_gte_any = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_gte_any = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_gte_any = "select_result_gte_any";

	AK_Init_L3(&attributes_gte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_gte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_gte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_gte_any);

	AK_Init_L3(&condition_gte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_gte_any);
	char *conditionAttributes_gte_any = AK_malloc(strlen("2008,2011") + 1);
	strcpy(conditionAttributes_gte_any, "2008,2011");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_gte_any, strlen(conditionAttributes_gte_any) + 1, condition_gte_any);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, ">=ANY", sizeof(">=ANY"), condition_gte_any);

	if (AK_select(srcTable, destTable_gte_any, attributes_gte_any, condition_gte_any, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_gte_any);
	AK_DeleteAll_L3(&condition_gte_any);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year >=ANY (2008,2011);\n\n");
	AK_print_table(destTable_gte_any);

	AK_free(attributes_gte_any);
	AK_free(condition_gte_any);
	AK_free(conditionAttributes_gte_any);

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR <=ANY ZAPOČINJE!\n");

	struct list_node *attributes_lte_any = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_lte_any = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_lte_any = "select_result_lte_any";

	AK_Init_L3(&attributes_lte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_lte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_lte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_lte_any);

	AK_Init_L3(&condition_lte_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_lte_any);
	char *conditionAttributes_lte_any = AK_malloc(strlen("2008,2011") + 1);
	strcpy(conditionAttributes_lte_any, "2008,2011");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_lte_any, strlen(conditionAttributes_lte_any) + 1, condition_lte_any);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<=ANY", sizeof("<=ANY"), condition_lte_any);

	if (AK_select(srcTable, destTable_lte_any, attributes_lte_any, condition_lte_any, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_lte_any);
	AK_DeleteAll_L3(&condition_lte_any);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year <=ANY (2008,2011);\n\n");
	AK_print_table(destTable_lte_any);

	AK_free(attributes_lte_any);
	AK_free(condition_lte_any);
	AK_free(conditionAttributes_lte_any);

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR !=ANY ZAPOČINJE!\n");

	struct list_node *attributes_neq_any = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_neq_any = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_neq_any = "select_result_neq_any";

	AK_Init_L3(&attributes_neq_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_neq_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_neq_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_neq_any);

	AK_Init_L3(&condition_neq_any);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_neq_any);
	char *conditionAttributes_neq_any = AK_malloc(strlen("2008,2011") + 1);
	strcpy(conditionAttributes_neq_any, "2008,2011");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_neq_any, strlen(conditionAttributes_neq_any) + 1, condition_neq_any);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "!=ANY", sizeof("!=ANY"), condition_neq_any);

	if (AK_select(srcTable, destTable_neq_any, attributes_neq_any, condition_neq_any, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_neq_any);
	AK_DeleteAll_L3(&condition_neq_any);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year !=ANY (2008,2011);\n\n");
	AK_print_table(destTable_neq_any);

	AK_free(attributes_neq_any);
	AK_free(condition_neq_any);
	AK_free(conditionAttributes_neq_any);

}

void test_operator_all(char *srcTable, int *successful, int *failed) {

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR >ALL ZAPOČINJE!\n");

	struct list_node *attributes_gt_all = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_gt_all = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_gt_all = "select_gt_all";
	AK_Init_L3(&attributes_gt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_gt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_gt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_gt_all);

	AK_Init_L3(&condition_gt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_gt_all);
	char *conditionAttributes_gt_all = AK_malloc(strlen("2012,2013,2014") + 1);
	strcpy(conditionAttributes_gt_all, "2012,2013,2014");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_gt_all, strlen(conditionAttributes_gt_all) + 1, condition_gt_all);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, ">ALL", sizeof(">ALL"), condition_gt_all);

	if (AK_select(srcTable, destTable_gt_all, attributes_gt_all, condition_gt_all, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	AK_DeleteAll_L3(&attributes_gt_all);
	AK_DeleteAll_L3(&condition_gt_all);

	AK_print_table(srcTable);
	printf("\n SELECT firstname, year, weight FROM student WHERE year >ALL (2012,2013,2014);\n\n");
	AK_print_table(destTable_gt_all);

	AK_free(attributes_gt_all);
	AK_free(condition_gt_all);
	AK_free(conditionAttributes_gt_all);


	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR <ALL ZAPOČINJE!\n");

	struct list_node *attributes_lt_all = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_lt_all = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_lt_all = "select_lt_all";
	AK_Init_L3(&attributes_lt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_lt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_lt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_lt_all);

	AK_Init_L3(&condition_lt_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_lt_all);
	char *conditionAttributes_lt_all = AK_malloc(strlen("2012,2013,2014") + 1);
	strcpy(conditionAttributes_lt_all, "2012,2013,2014");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_lt_all, strlen(conditionAttributes_lt_all) + 1, condition_lt_all);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<ALL", sizeof("<ALL"), condition_lt_all);

	if (AK_select(srcTable, destTable_lt_all, attributes_lt_all, condition_lt_all, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_lt_all);
	AK_DeleteAll_L3(&condition_lt_all);

	AK_print_table(srcTable);
	printf("\n SELECT firstname, year, weight FROM student WHERE year <ALL (2012,2013,2014);\n\n");
	AK_print_table(destTable_lt_all);

	AK_free(attributes_lt_all);
	AK_free(condition_lt_all);
	AK_free(conditionAttributes_lt_all);

	
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR >=ALL ZAPOČINJE!\n");

	struct list_node *attributes_gte_all = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_gte_all = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_gte_all = "select_gte_all";
	AK_Init_L3(&attributes_gte_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_gte_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_gte_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_gte_all);

	AK_Init_L3(&condition_gte_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_gte_all);
	char *conditionAttributes_gte_all = AK_malloc(strlen("2012,2013,2014") + 1);
	strcpy(conditionAttributes_gte_all, "2012,2013,2014");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_gte_all, strlen(conditionAttributes_gte_all) + 1, condition_gte_all);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, ">=ALL", sizeof(">=ALL"), condition_gte_all);

	if (AK_select(srcTable, destTable_gte_all, attributes_gte_all, condition_gte_all, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	AK_DeleteAll_L3(&attributes_gte_all);
	AK_DeleteAll_L3(&condition_gte_all);

	AK_print_table(srcTable);
	printf("\n SELECT firstname, year, weight FROM student WHERE year >=ALL (2012,2013,2014);\n\n");
	AK_print_table(destTable_gte_all);

	AK_free(attributes_gte_all);
	AK_free(condition_gte_all);
	AK_free(conditionAttributes_gte_all);

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR <=ALL ZAPOČINJE!\n");

	struct list_node *attributes_le_all = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition_le_all = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable_le_all="select_le_all";
	AK_Init_L3(&attributes_le_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_le_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_le_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_le_all);

	AK_Init_L3(&condition_le_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_le_all);
    char *conditionAtributes_in_le_all = AK_malloc(strlen("2012,2013,2014") + 1);
	strcpy(conditionAtributes_in_le_all, "2012,2013,2014");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_in_le_all, strlen(conditionAtributes_in_le_all) + 1, condition_le_all);

	AK_InsertAtEnd_L3(TYPE_OPERATOR, "<=ALL", sizeof("<=ALL"), condition_le_all);



    if (AK_select(srcTable, destTable_le_all, attributes_le_all, condition_le_all, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	
	AK_DeleteAll_L3(&attributes_le_all);
	AK_DeleteAll_L3(&condition_le_all);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE year <=ALL (2012,2013,2014);\n\n");
    AK_print_table(destTable_le_all);
	
	AK_free(attributes_le_all);
	AK_free(condition_le_all);
	AK_free(conditionAtributes_in_le_all);


	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR =ALL ZAPOČINJE!\n");

	struct list_node *attributes_eq_all = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_eq_all = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_eq_all = "select_eq_all";

	AK_Init_L3(&attributes_eq_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_eq_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_eq_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_eq_all);

	AK_Init_L3(&condition_eq_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_eq_all);

	char *conditionAttributes_eq_all = AK_malloc(strlen("2012,2013,2014") + 1);
	strcpy(conditionAttributes_eq_all, "2012,2013,2014");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_eq_all, strlen(conditionAttributes_eq_all) + 1, condition_eq_all);

	AK_InsertAtEnd_L3(TYPE_OPERATOR, "=ALL", sizeof("=ALL"), condition_eq_all);

	if (AK_select(srcTable, destTable_eq_all, attributes_eq_all, condition_eq_all, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_eq_all);
	AK_DeleteAll_L3(&condition_eq_all);

	AK_print_table(srcTable);
	printf("\nSELECT firstname, year, weight FROM student WHERE year =ALL (2012,2013,2014);\n\n");
	AK_print_table(destTable_eq_all);

	AK_free(attributes_eq_all);
	AK_free(condition_eq_all);
	AK_free(conditionAttributes_eq_all);

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR !=ALL ZAPOČINJE!\n");

	struct list_node *attributes_ne_all = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_ne_all = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_ne_all = "select_ne_all";

	AK_Init_L3(&attributes_ne_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_ne_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_ne_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_ne_all);

	AK_Init_L3(&condition_ne_all);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), condition_ne_all);

	char *conditionAttributes_ne_all = AK_malloc(strlen("2012,2013,2014") + 1);
	strcpy(conditionAttributes_ne_all, "2012,2013,2014");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAttributes_ne_all, strlen(conditionAttributes_ne_all) + 1, condition_ne_all);

	AK_InsertAtEnd_L3(TYPE_OPERATOR, "!=ALL", sizeof("!=ALL"), condition_ne_all);

	if (AK_select(srcTable, destTable_ne_all, attributes_ne_all, condition_ne_all, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_ne_all);
	AK_DeleteAll_L3(&condition_ne_all);

	AK_print_table(srcTable);
	printf("\nSELECT firstname, year, weight FROM student WHERE year !=ALL (2012,2013,2014);\n\n");
	AK_print_table(destTable_ne_all);

	AK_free(attributes_ne_all);
	AK_free(condition_ne_all);
	AK_free(conditionAttributes_ne_all);

}

void test_operator_like(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR LIKE\n");

	struct list_node *attributes_like = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition_like = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable_like="select_result_like";
	AK_Init_L3(&attributes_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_like);

	AK_Init_L3(&condition_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_like);
    char *conditionAtributes_in_in = AK_malloc(strlen("%a")+1);
	strcpy(conditionAtributes_in_in, "%a");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_in_in, sizeof(conditionAtributes_in_in), condition_like);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "LIKE", sizeof("LIKE"), condition_like);



    if (AK_select(srcTable, destTable_like, attributes_like, condition_like, NULL) == EXIT_SUCCESS)
    {
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	
	AK_DeleteAll_L3(&attributes_like);
	AK_DeleteAll_L3(&condition_like);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname LIKE '%%a';\n\n");
    AK_print_table(destTable_like);
	
	AK_free(attributes_like);
	AK_free(condition_like);
}

void test_operator_not_like(char *srcTable, int *successful, int *failed) {

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR NOT LIKE\n");

	struct list_node *attributes_not_like = (struct list_node *) AK_malloc(sizeof (struct list_node));
	struct list_node *condition_not_like = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable_not_like = "select_result_not_like";
	AK_Init_L3(&attributes_not_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_not_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_not_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_not_like);

	AK_Init_L3(&condition_not_like);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_not_like);
	char *condition_not_like_value = AK_malloc(strlen("%a") + 1);
	strcpy(condition_not_like_value, "%a");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, condition_not_like_value, sizeof(condition_not_like_value), condition_not_like);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "NOT LIKE", sizeof("NOT LIKE"), condition_not_like);

	if (AK_select(srcTable, destTable_not_like, attributes_not_like, condition_not_like, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_not_like);
	AK_DeleteAll_L3(&condition_not_like);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname NOT LIKE '%%a';\n\n");
	AK_print_table(destTable_not_like);

	AK_free(attributes_not_like);
	AK_free(condition_not_like);

}

void test_operator_ilike(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR ILIKE\n");

	struct list_node *attributes_ilike = (struct list_node *) AK_malloc(sizeof (struct list_node));
	struct list_node *condition_ilike = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable_ilike = "select_result_ilike";
	AK_Init_L3(&attributes_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_ilike);

	AK_Init_L3(&condition_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_ilike);
	char *condition_ilike_value = AK_malloc(strlen("%A") + 1); 
	strcpy(condition_ilike_value, "%A");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, condition_ilike_value, sizeof(condition_ilike_value), condition_ilike);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "ILIKE", sizeof("ILIKE"), condition_ilike);

	if (AK_select(srcTable, destTable_ilike, attributes_ilike, condition_ilike, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_ilike);
	AK_DeleteAll_L3(&condition_ilike);

	AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname ILIKE '%%A';\n\n");
	AK_print_table(destTable_ilike);

	AK_free(attributes_ilike);
	AK_free(condition_ilike);

}

void test_operator_not_ilike(char *srcTable, int *successful, int *failed) {

	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR NOT ILIKE\n");

	struct list_node *attributes_not_ilike = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition_not_ilike = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable_not_like="select_result_not_ilike";
	AK_Init_L3(&attributes_not_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_not_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_not_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_not_ilike);

	AK_Init_L3(&condition_not_ilike);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_not_ilike);
    char *conditionAtributes_not_ilike = AK_malloc(strlen("%A")+1);
	strcpy(conditionAtributes_not_ilike, "%A");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_not_ilike, sizeof(conditionAtributes_not_ilike), condition_not_ilike);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "NOT ILIKE", sizeof("NOT ILIKE"), condition_not_ilike);



    if (AK_select(srcTable, destTable_not_like, attributes_not_ilike, condition_not_ilike, NULL) == EXIT_SUCCESS)
    {
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	
	AK_DeleteAll_L3(&attributes_not_ilike);
	AK_DeleteAll_L3(&condition_not_ilike);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname NOT ILIKE '%%A';\n\n");
    AK_print_table(destTable_not_like);
	
	AK_free(attributes_not_ilike);
	AK_free(condition_not_ilike);
}

void test_operator_similar_to(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR SIMILAR TO\n");

	struct list_node *attributes_similar = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_similar = (struct list_node *) AK_malloc(sizeof(struct list_node));

	char *destTable_similar = "select_result_similar";

	AK_Init_L3(&attributes_similar);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_similar);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_similar);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_similar);

	AK_Init_L3(&condition_similar);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_similar);

	char *similar_pattern = AK_malloc(strlen("M_____") + 1);
	strcpy(similar_pattern, "M_____");

	AK_InsertAtEnd_L3(TYPE_VARCHAR, similar_pattern, strlen(similar_pattern) + 1, condition_similar);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "SIMILAR TO", sizeof("SIMILAR TO"), condition_similar);

	if (AK_select(srcTable, destTable_similar, attributes_similar, condition_similar, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_similar);
	AK_DeleteAll_L3(&condition_similar);

	AK_print_table(srcTable);
	printf("\n SELECT firstname, year, weight FROM student WHERE firstname SIMILAR TO 'M______';\n\n");
	AK_print_table(destTable_similar);

	AK_free(attributes_similar);
	AK_free(condition_similar);
	AK_free(similar_pattern);

}

void test_operator_tilde(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR ~\n");

	struct list_node *_tilda = (struct list_node *) AK_malloc(sizeof (struct list_node));

	struct list_node *condition_tilda = (struct list_node *) AK_malloc(sizeof (struct list_node));

	char *destTable_tilda="select_result_tilda";
	AK_Init_L3(&_tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), _tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), _tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), _tilda);

	AK_Init_L3(&condition_tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_tilda);
    char conditionAtributes_tilda[] = "^Ma";

	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_tilda, sizeof(conditionAtributes_tilda), condition_tilda);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "~", sizeof("~"), condition_tilda);



    if (AK_select(srcTable, destTable_tilda, _tilda, condition_tilda, NULL) == EXIT_SUCCESS)
   	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}
	
	AK_DeleteAll_L3(&_tilda);
	AK_DeleteAll_L3(&condition_tilda);
	
    AK_print_table(srcTable);
	printf("\n SELECT firstname,year,weight FROM student WHERE firstname ~ '^Ma';\n\n");
    AK_print_table(destTable_tilda);
	
	AK_free(_tilda);
	AK_free(condition_tilda);
}

void test_operator_not_tilde(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR !~\n");

	struct list_node *attributes_not_tilda = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_not_tilda = (struct list_node *) AK_malloc(sizeof(struct list_node));
	char *destTable_not_tilda = "select_result_not_tilda";

	AK_Init_L3(&attributes_not_tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_not_tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_not_tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_not_tilda);

	AK_Init_L3(&condition_not_tilda);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_not_tilda);
	char conditionAtributes_not_tilda[] = "^Ma";
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_not_tilda, sizeof(conditionAtributes_not_tilda), condition_not_tilda);
	AK_InsertAtEnd_L3(TYPE_OPERATOR, "!~", sizeof("!~"), condition_not_tilda);

	if (AK_select(srcTable, destTable_not_tilda, attributes_not_tilda, condition_not_tilda, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_not_tilda);
	AK_DeleteAll_L3(&condition_not_tilda);

	AK_print_table(srcTable);
	printf("\n SELECT firstname, year, weight FROM student WHERE firstname !~ '^Ma';\n\n");
	AK_print_table(destTable_not_tilda);

	AK_free(attributes_not_tilda);
	AK_free(condition_not_tilda);
}

void test_operator_tilde_star(char *srcTable, int *successful, int *failed) {
	printf("--------------------------------------------------------------------------\n");
	printf("TEST ZA OPERATOR ~*\n");

	struct list_node *attributes_tilda_i = (struct list_node *) AK_malloc(sizeof(struct list_node));
	struct list_node *condition_tilda_i = (struct list_node *) AK_malloc(sizeof(struct list_node));
	char *destTable_tilda_i = "select_result_tilda_star";

	AK_Init_L3(&attributes_tilda_i);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), attributes_tilda_i);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof("year"), attributes_tilda_i);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof("weight"), attributes_tilda_i);

	AK_Init_L3(&condition_tilda_i);
	AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof("firstname"), condition_tilda_i);
	char *conditionAtributes_tilda_i = AK_malloc(strlen("^ma") + 1);
	strcpy(conditionAtributes_tilda_i, "^ma");
	AK_InsertAtEnd_L3(TYPE_VARCHAR, conditionAtributes_tilda_i, strlen(conditionAtributes_tilda_i) + 1, condition_tilda_i);

	AK_InsertAtEnd_L3(TYPE_OPERATOR, "~*", sizeof("~*"), condition_tilda_i);

	if (AK_select(srcTable, destTable_tilda_i, attributes_tilda_i, condition_tilda_i, NULL) == EXIT_SUCCESS)
	{
		(*successful)++;
	}
	else
	{
		(*failed)++;
	}

	AK_DeleteAll_L3(&attributes_tilda_i);
	AK_DeleteAll_L3(&condition_tilda_i);

	AK_print_table(srcTable);
	printf("\n SELECT firstname, year, weight FROM student WHERE firstname ~* '^ma';\n\n");
	AK_print_table(destTable_tilda_i);

	AK_free(attributes_tilda_i);
	AK_free(condition_tilda_i);

}

