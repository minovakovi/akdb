//THIS FILE IS PART 2 OF REL_EQ_ASSOC.C   ORIGINALLY IT WAS PART OF IT, BUT IN ORDER TO SPLIT THE CODE AND MAKE IT MORE READABLE IT MADE INTO 2 PARTS

#include "rel_eq_assoc.h"
#include "rel_eq_projection.h"
/**
 * @author Dino Laktašić.
 * @brief Function for printing RA expresion struct list_node
 * @param *list_rel_eq RA expresion as the struct list_node
 * @return optimised RA expresion as the struct list_node
 */
void AK_print_rel_eq_assoc(struct list_node *list_rel_eq) {
    AK_PRO;
    struct list_node *list_elem = (struct list_node *) AK_First_L2(list_rel_eq);

    printf("\n");
    while (list_elem != NULL) {
        printf("Type: %i, size: %i, data: %s\n", list_elem->type, list_elem->size, list_elem->data);
        list_elem = list_elem->next;
    }
    AK_EPI;
}
/**
 * @author Dino Laktašić.
 * @brief Function for testing relational equivalences regarding associativity
 * @return No return value
 */
TestResult AK_rel_eq_assoc_test() {
    AK_PRO;
    printf("rel_eq_assoc.c: Present!\n");
    printf("\n********** REL_EQ_ASSOCIATIVITY TEST by Dino Laktašić **********\n");

    int success=0;
    int failed=0;
    int result;

    //create header
    AK_header t_header[MAX_ATTRIBUTES];
    AK_header *temp;

    temp = (AK_header*) AK_create_header("id", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("firstname", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 1, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("job", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 2, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("year", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 3, temp, sizeof (AK_header));
    temp = (AK_header*) AK_create_header("weight", TYPE_FLOAT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header + 4, temp, sizeof (AK_header));
    memset(t_header + 5, '\0', MAX_ATTRIBUTES - 5);

    //TEST 1
    //create table
    char *tblName = "profesor";

    int startAddress = AK_initialize_new_segment(tblName, SEGMENT_TYPE_TABLE, t_header);

    if (startAddress != EXIT_ERROR){
	    success++;
        printf("\nTABLE %s CREATED!\n", tblName);
    }else{
        failed++;
        printf("\nTest failed!\n");
    }

    printf("rel_eq_associativity_test: After segment initialization: %d\n", AK_num_attr(tblName));

    struct list_node *row_root = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root);

    int id = 35890, year = 1999;
    float weight = 80.00;
    
    //TEST 2
    //insert rows in table student
    id++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &id, tblName, "id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Mirko", tblName, "firstname", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Sestak", tblName, "job", row_root);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root);
    AK_insert_row(row_root);
    int insertedRow = AK_reference_check_entry(row_root);
    

    id++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &id, tblName, "id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Igor", tblName, "firstname", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Mesaric", tblName, "job", row_root);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root);
    AK_insert_row(row_root);
    int insertedRow2 = AK_reference_check_entry(row_root);

    id++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &id, tblName, "id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dalibor", tblName, "firstname", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Slunjski", tblName, "job", row_root);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root);
    AK_insert_row(row_root);
    int insertedRow3 = AK_reference_check_entry(row_root);

    id++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &id, tblName, "id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dino", tblName, "firstname", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Alagic", tblName, "job", row_root);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root);
    AK_insert_row(row_root);
    int insertedRow4 = AK_reference_check_entry(row_root);

    id++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &id, tblName, "id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Miroslav", tblName, "firstname", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Zver", tblName, "job", row_root);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root);
    AK_insert_row(row_root);
    int insertedRow5 = AK_reference_check_entry(row_root);

    id++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root);
    AK_Insert_New_Element(TYPE_INT, &id, tblName, "id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Josip", tblName, "firstname", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, "Vincek", tblName, "job", row_root);
    AK_Insert_New_Element(TYPE_INT, &year, tblName, "year", row_root);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName, "weight", row_root);
    AK_insert_row(row_root);
    int insertedRow6 = AK_reference_check_entry(row_root);

    if ((insertedRow && insertedRow2 && insertedRow3 && insertedRow4 && insertedRow5 && insertedRow6) != EXIT_ERROR){
	    success++;  
    }else{
        failed++;
    }

    printf("rel_eq_associativity_test: After data insertion: %d\n", AK_num_attr(tblName));

    //create header
    AK_header t_header2[MAX_ATTRIBUTES];
    AK_header *temp2;

    temp2 = (AK_header*) AK_create_header("mbr", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("firstname", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 1, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("lastname", TYPE_VARCHAR, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 2, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("year", TYPE_INT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 3, temp2, sizeof (AK_header));
    temp2 = (AK_header*) AK_create_header("weight", TYPE_FLOAT, FREE_INT, FREE_CHAR, FREE_CHAR);
    memcpy(t_header2 + 4, temp2, sizeof (AK_header));
    memset(t_header2 + 5, '\0', MAX_ATTRIBUTES - 5);

    //TEST 3
    //create table
    char *tblName2 = "student";

    int startAddress2 = AK_initialize_new_segment(tblName2, SEGMENT_TYPE_TABLE, t_header2);

    if (startAddress2 != EXIT_ERROR){
        success++;
        printf("\nTABLE %s CREATED!\n", tblName2);
    }else{
        failed++;
        printf("\nTest failed!\n");
    }


    printf("rel_eq_associativity_test: After segment initialization: %d\n", AK_num_attr(tblName2));

    struct list_node *row_root2 = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&row_root2);

    int mbr = 35890;
    year = 1999;
    weight = 80.00;

    //TEST 4
    //insert rows in table student
    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Matija", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Sestak", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow7 = AK_reference_check_entry(row_root2);

    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Igor", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Mesaric", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow8 = AK_reference_check_entry(row_root2);

    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dalibor", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Slunjski", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow9 = AK_reference_check_entry(row_root2);

    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dino", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Alagic", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow10 = AK_reference_check_entry(row_root2);

    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Miroslav", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Zver", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow11 = AK_reference_check_entry(row_root2);

    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Josip", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Vincek", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow12 = AK_reference_check_entry(row_root2);

    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Netko", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Netkic", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow13 = AK_reference_check_entry(row_root2);

    mbr++;
    year++;
    weight += 0.75;
    AK_DeleteAll_L3(&row_root2);
    AK_Insert_New_Element(TYPE_INT, &mbr, tblName2, "mbr", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Dino", tblName2, "firstname", row_root2);
    AK_Insert_New_Element(TYPE_VARCHAR, "Laktašić", tblName2, "lastname", row_root2);
    AK_Insert_New_Element(TYPE_INT, &year, tblName2, "year", row_root2);
    AK_Insert_New_Element(TYPE_FLOAT, &weight, tblName2, "weight", row_root2);
    AK_insert_row(row_root2);
    int insertedRow14 = AK_reference_check_entry(row_root2);
    AK_DeleteAll_L3(&row_root2);
    

    if ((insertedRow7 && insertedRow8 && insertedRow9 && insertedRow10 && insertedRow11 && insertedRow12 && insertedRow13 && insertedRow14) != EXIT_ERROR){
	    success++;  
    }else{
        failed++;
    }

    printf("rel_eq_associativity_test: After data insertion: %d\n", AK_num_attr(tblName2));
    //-----------------------------------------------------------------------------------------

    struct list_node *expr = (struct list_node *) AK_malloc(sizeof (struct list_node));
    AK_Init_L3(&expr);

    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L2;L3;L4", sizeof ("L1;L2;L3;L4"), expr); //projection attribute
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "L1;L4;L3;L2;L5", sizeof ("L1;L4;L3;L2;L5"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "s", sizeof ("s"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`L1` > 100 OR `L2` < 50", sizeof ("`L1` > 100 OR `L2` < 50"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "profesor", sizeof ("profesor"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "u", sizeof ("u"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "p", sizeof ("p"), expr);
    AK_InsertAtEnd_L3(TYPE_ATTRIBS, "mbr;firstname;job", sizeof ("mbr;firstname;job"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "profesor", sizeof ("profesor"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "t", sizeof ("t"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "`mbr` = `id`", sizeof ("`mbr` = `id`"), expr); //theta join attribute

    AK_InsertAtEnd_L3(TYPE_OPERAND, "student", sizeof ("student"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "profesor", sizeof ("profesor"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "n", sizeof ("n"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "mbr;job", sizeof ("mbr;job"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERAND, "profesor", sizeof ("profesor"), expr);
    AK_InsertAtEnd_L3(TYPE_OPERATOR, "n", sizeof ("n"), expr);
    AK_InsertAtEnd_L3(TYPE_CONDITION, "mbr;job", sizeof ("mbr;job"), expr);

    AK_print_rel_eq_assoc(AK_rel_eq_assoc(expr));

    printf("\n------------------> TEST_ASSOCIATIVITY_FUNCTIONS <------------------\n\n");
    printf("...");

    AK_DeleteAll_L3(&expr);

    AK_EPI;
    
    return TEST_result(success,failed);
}