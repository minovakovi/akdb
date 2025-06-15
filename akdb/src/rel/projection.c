/**
@file projection.c Provides functions for relational projection operation
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
 17 */

#include "projection.h"

/**
 * @author Matija Novak, rewritten and optimized by Dino Laktašić to support AK_list  
 * @brief  Function that creates a new header for the projection table
 * @param old_block_add address of the block from which we copy headers we need
 * @param dstTable name of the new table - destination table
 * @param att list of the attributes which should the projection table contain
 * @result Newly created header
 * @return No return value
 */

void AK_create_block_header(int old_block, char *dstTable, struct list_node *att) {

    AK_PRO;	
    AK_block *temp_block;
	
    temp_block = (AK_block *) AK_read_block(old_block);
    AK_header *temp;

    struct list_node *list_elem,*list_elem2;
    struct list_node *att_operation_elem,*a,*b,*c,*d,*operator,*cached_elem;

    AK_header header[MAX_ATTRIBUTES];
    
    int head = 0;
    int new_head = 0;
    int a_type;
    int b_type;
    int header_type = NULL;

    while (strcmp(temp_block->header[head].att_name, "") != 0) {

        list_elem = AK_First_L2(att);	
        while (list_elem != NULL) {
            char *att_name = (char *) list_elem->data;

            if (strcmp(att_name, temp_block->header[head].att_name) == 0) {
                memcpy(&header[new_head], &temp_block->header[head], sizeof(temp_block->header[head]));
                new_head++;
            } else if (strstr(att_name, temp_block->header[head].att_name) != NULL) {
                char *exp = (char *) malloc(1 + strlen(att_name));
                strcpy(exp, att_name);
                AK_remove_substring(exp, temp_block->header[head].att_name);
                int cached_head = head;

                while (strcmp(temp_block->header[cached_head].att_name, "") != 0) {
                    if (strstr(exp, temp_block->header[cached_head].att_name) != NULL) {
                        int type = AK_determine_header_type(temp_block->header[head].type, temp_block->header[cached_head].type);

                        temp = (AK_header*) AK_create_header(att_name, type, FREE_INT, FREE_CHAR, FREE_CHAR);
                        memcpy(&header[new_head], temp, sizeof(AK_header));
                        new_head++;
                        break;
                    }
                    cached_head++;
                }

                AK_free(exp);
            }

            list_elem = list_elem->next;
        }

        head++;     
    }


    memset(header + new_head, 0, sizeof(AK_header) * (MAX_ATTRIBUTES - new_head));
    header[new_head].att_name[0] = '\0';

    AK_free(temp_block);
    AK_temp_create_table(dstTable, header, SEGMENT_TYPE_TABLE);
    AK_EPI;
}

/**
    * @author Leon Palaić
    * @brief Function that fetches arithmetic operator from given expression string, determinates given operator so it can be used for aritmetic operations
    * @param exp input expression string
    * @result character - aritmetic operator
    * @return character  
*/
char *AK_get_operator(char *exp){
    //iterates through given string expression and finds out which aritmetic operator is it
    if(strstr(exp,"+"))
        return "+";
    else if(strstr(exp,"-"))
        return "-";
    else if(strstr(exp,"/"))
        return "/";
    else if(strstr(exp,"%"))
        return "%";
    else if(strstr(exp,"*"))
        return "*";
    else
        return NULL;
}

/**
    * @author Leon Palaić
    * @brief Function that iterates through given string and removes specified part of that string. 
    * @param s input string 
    * @param substring string that needs to be removed
    * @result Cleaned new string
    * @return No return value
    
*/
void AK_remove_substring(char *s,const char *substring)
{
    // iterates through given string and removes specified part of that string
    while( s=strstr(s,substring) ){
        memmove(s,s+strlen(substring),1+strlen(s+strlen(substring)));
        break;
    }
}


/**
    * @author Leon Palaić
    * @brief Function that determines the new header type
    * @param firstOperand operand type
    * @param secondOperand operand type
    * @result Function returns determinated header type
    * @return Integer - type

*/
int AK_determine_header_type(int firstOperand, int secondOperand) {
    if (firstOperand == TYPE_VARCHAR || secondOperand == TYPE_VARCHAR) {
        return TYPE_VARCHAR;
    }
    if (firstOperand == TYPE_FLOAT || secondOperand == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }
    if (firstOperand == TYPE_INT || secondOperand == TYPE_INT) {
        return TYPE_INT;
    }
    if (firstOperand == TYPE_NUMBER || secondOperand == TYPE_NUMBER) {
        return TYPE_NUMBER;
    }

    return TYPE_INT;
}


/**
    * @author Leon Palaić
    * @brief  Function that creates new header name from passed operand names and operator
    * @param first operand name
    * @param second operand name
    * @param operator given operator
    * @result Function returns set of characters that represent new header name
    * @return Character - new name

*/
char *AK_create_header_name(char *first, char *operator, char *second) {
    size_t len = strlen(first) + strlen(operator) + strlen(second) + 1;
    char *finalName = (char *) malloc(len);

    strcpy(finalName, first);
    strcat(finalName, operator);
    strcat(finalName, second);
    return finalName;
}


/** 
  * @author Matija Novak, rewritten and optimized by Dino Laktašić to support AK_list
  * @brief  Function that copies the data from old table block to the new projection table
  * @param old_block block from which we copy data
  * @param dstTable name of the new table
  * @param att list of the attributes which should the projection table contain
  * @param expr given expression to check 
  * @result New projection table that contains all blocks from old table
  * @return No return value
 */
void AK_copy_block_projection(AK_block *old_block, struct list_node *att, char *dstTable,struct list_node *expr) {
    AK_PRO;
 
    struct list_node * row_root = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    struct list_node * list_elem,*a,*c,*operator;
    struct list_node * end_list;
    int i; //holds the count of tuple_dict inside the main for loop 
    int head; //head counter
    int something_to_copy; //boolean variable to indicate if there is data to copy in these set of tuple_dicts
    int size; //holds the tuple_dict data size of the current tuple_dict in the main for loop

    char data[MAX_VARCHAR_LENGTH]; //data to copy

    //data for operation
    struct AK_operand *first=(struct AK_operand *) AK_malloc(sizeof(struct AK_operand));
    struct AK_operand *second=(struct AK_operand *) AK_malloc(sizeof(struct AK_operand));
        
    //loop for iterate through all tuple_dicts in block
    for (i = 0; i < DATA_BLOCK_SIZE;) {
        head = something_to_copy = 0;

        //iterates through header attributes and determinates if header is a legit string different from empty
        while (strcmp(old_block->header[head].att_name, "") != 0) {
        
	    list_elem = (struct list_node *) AK_First_L2(att);

            while (list_elem != NULL) {
                size = old_block->tuple_dict[i].size;

                //used to check if the data is correct, it calculates the size of the data
                int overflow = old_block->tuple_dict[i].size + old_block->tuple_dict[i].address;
                
                //if the data is what we need, if the size is not null, and data is correct and less than 
                //the needed free space, sets all characters from the data string to 0 (comparable with 
                //initialization), and copies data to the curent tuple_dict address from old block into string
                if ((strcmp(list_elem->data, old_block->header[head].att_name) == 0) && (size != 0)
                        && (overflow < old_block->AK_free_space + 1) && (overflow > -1)) {
                    
                    memset(data, 0, MAX_VARCHAR_LENGTH); 
                    memcpy(data, old_block->data + old_block->tuple_dict[i].address, old_block->tuple_dict[i].size); //copy data
                   
                    //insert element to list to be inserted into new table
                    AK_Insert_New_Element(old_block->tuple_dict[i].type, data, dstTable, list_elem->data, row_root); //ForUpdate 0

                    //set indicator for existing data in touple_dict that can be copied
                    something_to_copy = 1;

                //if the data is not what we need, but the size is not null, and data is correct and less than the needed free space
                //sets all characters from the first data operation string to 0,
                //and copies data to the first operation data string from the old block 
                }else if((strstr(list_elem->data,old_block->header[head].att_name)!=NULL)&& (size != 0)
                        && (overflow < old_block->AK_free_space + 1) && (overflow > -1)){

                            memset(first->value, 0, MAX_VARCHAR_LENGTH);
                            memcpy(first->value, old_block->data + old_block->tuple_dict[i].address, old_block->tuple_dict[i].size);

                            char * exp = (char *) malloc(1 + strlen(list_elem->data));
                            strcpy(exp,list_elem->data);

                            AK_remove_substring(exp,old_block->header[head].att_name);

                            first->type = old_block->header[head].type;
                            int cached_head = head;
                            int j=i;
                            int size2 = old_block->tuple_dict[j].size;

                            //returns the first occurrence in data of the given header name (first position)
                            int positionFirst = strstr(list_elem->data,old_block->header[head].att_name) - list_elem->data;                       
                            
                            //used to check if the data is correct
                            
                            int overflow2 = old_block->tuple_dict[j].size + old_block->tuple_dict[j].address;
                            while(strcmp(old_block->header[cached_head].att_name,"")!=0){
                                if((strstr(exp,old_block->header[cached_head].att_name)!=NULL)&& (size2 != 0)
                                && (overflow2 < old_block->AK_free_space + 1) && (overflow2 > -1)){
                                
                                    memset(second->value, 0, MAX_VARCHAR_LENGTH);
                                    memcpy(second->value, old_block->data + old_block->tuple_dict[j].address, old_block->tuple_dict[j].size);
                                    second->type = old_block->header[cached_head].type;

                                    //returns the second occurrence in data of the given header based on cached_head
                                    int positionSecond = strstr(list_elem->data,old_block->header[cached_head].att_name) - list_elem->data;
                                    
                                    int type = AK_determine_header_type(first->type,second->type); // determinate type of attributes to be in table
                                    
                                    char *operation_result = NULL;
                                    void *casted_result = NULL;

                                    if (positionSecond < positionFirst) {
                                        operation_result = AK_perform_operation(AK_get_operator(list_elem->data), second, first, type);
                                    } else {
                                        operation_result = AK_perform_operation(AK_get_operator(list_elem->data), first, second, type);
                                    }

                                    // Pretvori string u binarni tip koji očekuje AK_Insert_New_Element
                                    if (type == TYPE_INT) {
                                        int *ival = (int *)AK_malloc(sizeof(int));
                                        *ival = atoi(operation_result);
                                        casted_result = ival;
                                    } else if (type == TYPE_FLOAT) {
                                        float *fval = (float *)AK_malloc(sizeof(float));
                                        *fval = atof(operation_result);
                                        casted_result = fval;
                                    } else if (type == TYPE_NUMBER) {
                                        double *dval = (double *)AK_malloc(sizeof(double));
                                        *dval = atof(operation_result);
                                        casted_result = dval;
                                    } else {
                                        casted_result = strdup(operation_result);
                                    }

                                    AK_Insert_New_Element(type, casted_result, dstTable, list_elem->data, row_root);

                                    free(operation_result);
                                    free(casted_result);

                                    break;
                                }
                                cached_head++;
                                j++;
                            }
                        free(exp);
                }                
                list_elem = list_elem->next;
            }
            
            head++;
            i++;
        }

        //if there is data to be copied write row to the projection table
        if (something_to_copy) {
            AK_dbg_messg(HIGH, REL_OP, "\nInsert row to projection table.\n");

            if(expr != NULL){ //if there is given expression - check if data satisfies it
                if(AK_check_if_row_satisfies_expression(row_root,expr)){ //check if data that is to be inserted satisfies given expression, if yes insert row
                    AK_insert_row(row_root);
                }
            }else{ //if there is not given expression to check - insert row into the table
                AK_insert_row(row_root);
            }
            	    
            AK_DeleteAll_L3(&row_root);
        }
    }
    AK_EPI;
    AK_free(first);
    AK_free(second);
    AK_free(row_root);
}


/**
    * @author Leon Palaić
    * @brief Function that performes arithmetics operation depended on given operator
    * @param firstOperand first operand
    * @param secondOperand second operand 
    * @param op aritmetic operator
    * @param type type of operand 
    * @result result of arithmetic operation
    * @return character

*/
char *AK_perform_operation(char *op, struct AK_operand *firstOperand, struct AK_operand *secondOperand, int type) {
    char *result_str = malloc(MAX_VARCHAR_LENGTH);
    if (!result_str) return NULL;

    if (type == TYPE_NUMBER) {
        double a = 0, b = 0, result = 0;

        if (firstOperand->type == TYPE_NUMBER)
            a = (*(double*)firstOperand->value);
        else if (firstOperand->type == TYPE_FLOAT)
            a = (double)(*(float*)firstOperand->value);
        else if (firstOperand->type == TYPE_INT)
            a = (double)(*(int*)firstOperand->value);

        if (secondOperand->type == TYPE_NUMBER)
            b = (*(double*)secondOperand->value);
        else if (secondOperand->type == TYPE_FLOAT)
            b = (double)(*(float*)secondOperand->value);
        else if (secondOperand->type == TYPE_INT)
            b = (double)(*(int*)secondOperand->value);

        if (strcmp(op, "+") == 0)
            result = a + b;
        else if (strcmp(op, "-") == 0)
            result = a - b;
        else if (strcmp(op, "*") == 0)
            result = a * b;
        else if (strcmp(op, "/") == 0)
            result = a / b;

        snprintf(result_str, MAX_VARCHAR_LENGTH, "%f", result);

    } else if (type == TYPE_FLOAT) {
        float a = 0, b = 0, result = 0;

        if (firstOperand->type == TYPE_NUMBER)
            a = (float)(*(double*)firstOperand->value);
        else if (firstOperand->type == TYPE_FLOAT)
            a = *(float*)firstOperand->value;
        else if (firstOperand->type == TYPE_INT)
            a = (float)(*(int*)firstOperand->value);

        if (secondOperand->type == TYPE_NUMBER)
            b = (float)(*(double*)secondOperand->value);
        else if (secondOperand->type == TYPE_FLOAT)
            b = *(float*)secondOperand->value;
        else if (secondOperand->type == TYPE_INT)
            b = (float)(*(int*)secondOperand->value);

        if (strcmp(op, "+") == 0)
            result = a + b;
        else if (strcmp(op, "-") == 0)
            result = a - b;
        else if (strcmp(op, "*") == 0)
            result = a * b;
        else if (strcmp(op, "/") == 0)
            result = a / b;

        snprintf(result_str, MAX_VARCHAR_LENGTH, "%f", result);

    } else if (type == TYPE_INT) {
        int a = 0, b = 0, result = 0;

        if (firstOperand->type == TYPE_NUMBER)
            a = (int)(*(double*)firstOperand->value);
        else if (firstOperand->type == TYPE_FLOAT)
            a = (int)(*(float*)firstOperand->value);
        else if (firstOperand->type == TYPE_INT)
            a = *(int*)firstOperand->value;

        if (secondOperand->type == TYPE_NUMBER)
            b = (int)(*(double*)secondOperand->value);
        else if (secondOperand->type == TYPE_FLOAT)
            b = (int)(*(float*)secondOperand->value);
        else if (secondOperand->type == TYPE_INT)
            b = *(int*)secondOperand->value;

        if (strcmp(op, "+") == 0)
            result = a + b;
        else if (strcmp(op, "-") == 0)
            result = a - b;
        else if (strcmp(op, "*") == 0)
            result = a * b;
        else if (strcmp(op, "/") == 0)
            result = a / b;
        else if (strcmp(op, "%") == 0)
            result = a % b;

        snprintf(result_str, MAX_VARCHAR_LENGTH, "%d", result);
    } else {
        free(result_str);
        return NULL;
    }

    return result_str;
}

/**
 * @author Matija Novak, rewritten and optimized by Dino Laktašić, now support cacheing
 * @brief  Function that makes a projection of some table on given attributes
 * @param srcTable source table - table on which projection is made
 * @param expr given expression to check while doing projection
 * @param att list of atributes on which we make projection
 * @param dstTable table name for projection table - new table - destination table
 * @result Projection table on given attributes
 * @return EXIT_SUCCESS if continues succesfuly, when not EXIT_ERROR
 */

int AK_projection(char *srcTable, char *dstTable, struct list_node *att, struct list_node *expr) {

    //geting the table addresses from table on which we make projection
    AK_PRO;
    table_addresses *src_addr = (table_addresses *) AK_get_table_addresses(srcTable);

    if (!src_addr) {
        AK_dbg_messg(LOW, REL_OP, "AK_projection: Failed to get table addresses for %s\n", srcTable);
        AK_EPI;
        return EXIT_ERROR;
    }

    if (src_addr->address_from[0] == 0) {
        AK_dbg_messg(LOW, REL_OP, "\nAK_projection: Table doesn't exist or has no data.\n");
        AK_free(src_addr);
        AK_EPI;
        return EXIT_ERROR;
    }

    if (!att) {
        AK_dbg_messg(LOW, REL_OP, "AK_projection: Attribute list is missing.\n");
        AK_free(src_addr);
        AK_EPI;
        return EXIT_ERROR;
    }

    AK_create_block_header(src_addr->address_from[0], dstTable, att);
    AK_dbg_messg(LOW, REL_OP, "TABLE %s CREATED from %s!\n", dstTable, srcTable);
    AK_dbg_messg(MIDDLE, REL_OP, "\nAK_projection: start copying data\n");

    for (int i = 0; src_addr->address_from[i] != 0; i++) {
        int startAddress = src_addr->address_from[i];
        AK_dbg_messg(MIDDLE, REL_OP, "\nAK_projection: copy extent: %d\n", i);

        for (int j = startAddress; j <= src_addr->address_to[i]; j++) {
            AK_mem_block *temp = (AK_mem_block *) AK_get_block(j);

            if (!temp || temp->block->last_tuple_dict_id == 0) {
                break; // empty or invalid block
            }

            AK_dbg_messg(MIDDLE, REL_OP, "\nAK_projection: copy block: %d\n", j);
            AK_copy_block_projection(temp->block, att, dstTable, expr);
        }
    }

    AK_free(src_addr);
    AK_dbg_messg(LOW, REL_OP, "PROJECTION_TEST_SUCCESS\n\n");
    AK_EPI;
    return EXIT_SUCCESS;
}

/**
 * @author Dino Laktašić, rewritten and optimized by Irena Ilišević to support ILIKE operator and perform usual projection 
 * @brief  Function for projection operation testing, tests usual projection functionality, projection when it is given aritmetic operation or expresson
 * @result Projection tables and number od passed tests
 * @return Test result - number of successful and unsuccessful tests
 */

TestResult AK_op_projection_test() {
    AK_PRO;
    printf("\n********** PROJECTION TEST **********\n\n");
  
    int success=0;
    int failed=0;

    // Tests projection when is given expresson and operator ILIKE
    struct list_node * att1 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    struct list_node * expr1 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&att1);   
    char expression1 []= "%Dino%";

    printf("\nSelect firstname,lastname from student where firstname ILIKE dino\n\n");

    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr1);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, expression1, strlen(expression1)+1, expr1);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "ILIKE", sizeof ("ILIKE"), expr1);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "lastname", sizeof ("lastname"), att1);
    
    int test_projection1 = AK_projection("student", "projection_test1", att1,  expr1);
    AK_print_table("projection_test1");
    AK_DeleteAll_L3(&att1);
    AK_DeleteAll_L3(&expr1);
    
    if (test_projection1 == EXIT_SUCCESS){
		printf("\n\nTest succeeded!\n");
        success++;
    }
    else{
		printf("\n\nTest failed!\n");
        failed++;
    }

    // Tests projection to work with given aritmetic operations

    struct list_node * att2 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&att2);  

    struct list_node * expr2 = (struct list_node *) AK_malloc(sizeof(struct list_node));   
    AK_Init_L3(&expr2); 

    printf("\nSelect firstname,lastname,year,weight,weight+year,weight-year,year*year,year-weight,year/weight from student\n\n");

    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "lastname", sizeof ("lastname"), att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year", sizeof ("year"), att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight", sizeof ("weight"), att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight+year", sizeof("weight+year"),att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "weight-year", sizeof("weight-year"),att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year*year", sizeof("year*year"),att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year-weight", sizeof("year-weight"),att2);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "year/weight", sizeof("year/weight"),att2);

    int test_projection2 = AK_projection("student", "projection_test2", att2,  NULL);
    AK_print_table("projection_test2");
    AK_DeleteAll_L3(&att2);
    AK_DeleteAll_L3(&expr2);
    
    if (test_projection2 == EXIT_SUCCESS){
		printf("\n\nTest succeeded!\n");
        success++;
    }
    else{
		printf("\n\nTest failed!\n");
        failed++;
    }
    
    //Tests projection when is given expresson and operator LIKE

    struct list_node * att3 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    struct list_node * expr3 = (struct list_node *) AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&att3);   
    char expression []= "%in%";

    printf("\nSelect firstname,lastname from student where firstname LIKE %%in%\n\n");

    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), expr3);
    AK_InsertAtEnd_L3(TYPE_VARCHAR, &expression, strlen(expression)+1, expr3);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "LIKE", sizeof ("LIKE"), expr3);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "lastname", sizeof ("lastname"), att3);  
    
    int test_projection3 = AK_projection("student", "projection_test3", att3, expr3);
    AK_print_table("projection_test3");
    AK_DeleteAll_L3(&att3);

    if (test_projection3 == EXIT_SUCCESS){
            printf("\n\nTest succeeded!\n");
            success++;
    }
    else{
		printf("\n\nTest failed!\n");
        failed++;
    }

    // Test for usual projection

    struct list_node * att4 = (struct list_node *) AK_malloc(sizeof(struct list_node)); 
    AK_Init_L3(&att4);   
    
    printf("\nSelect firstname,lastname from student \n\n");

    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "firstname", sizeof ("firstname"), att4);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "lastname", sizeof ("lastname"), att4);
        
    int test_projection4 = AK_projection("student", "projection_test4", att4, NULL);
    AK_print_table("projection_test4");
    AK_DeleteAll_L3(&att4);

    if (test_projection4 == EXIT_SUCCESS){
            printf("\n\nTest succeeded!\n");
            success++;
    }
    else{
		printf("\n\nTest failed!\n");
        failed++;
    }
  
    AK_EPI;
    return TEST_result(success,failed);
}
