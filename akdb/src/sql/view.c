/**
@file view.c Provides functions for views
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
 *
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "view.h"

/**
 * @author Sara Kisic
 * @brief Function that checks if the name of the view already exists in AK_view table
 * @param name Name of the view
 * @return EXIT_ERROR if the name already exists or name
 */
char *AK_check_view_name(char *name)
{
    int i = 0;
    char *result;

    struct list_node *row;
    AK_PRO;

    while ((row = (struct list_node *)AK_get_row(i++, "AK_view")))
    {
        struct list_node *name_elem = AK_GetNth_L2(2, row);
        if (strcmp(name_elem->data, name) == 0)
        {
            result = (char *)(EXIT_ERROR);
            break;
        }
        else
        {
            result = name;
        }
    }
    AK_EPI;
    return result;
}

/**
 * @author Kresimir Ivkovic
 * @brief Function that finds an object's id by its name
 * @param name name of the view
 * @return View's id or EXIT_ERROR
 */
int AK_get_view_object_id(char *name)
{
    int id, i = 0;

    struct list_node *row;
    AK_PRO;
    while ((row = (struct list_node *)AK_get_row(i++, "AK_view")))
    {
        struct list_node *name_elem = AK_GetNth_L2(2, row);
        if (!strcmp(name_elem->data, name))
        {
            memcpy(&id, row->next->data, sizeof(int));
            AK_EPI;
            return id;
        }
    }
    AK_EPI;
    return EXIT_ERROR;
}

/**
 * @author Danko Sačer
 * @brief Function that returns a query by its name
 * @param name name of the view
 * @return query string or EXIT_ERROR
 */
char *AK_get_view_query(char *name)
{
    int i = 0;
    char *query;

    struct list_node *row;
    AK_PRO;

    while ((row = (struct list_node *)AK_get_row(i++, "AK_view")))
    {
        struct list_node *name_elem = AK_GetNth_L2(2, row);
        if (!strcmp(name_elem->data, name))
        {
            struct list_node *query_elem = AK_GetNth_L2(3, row);
            query = query_elem->data;
            AK_EPI;
            return query;
        }
    }
    AK_EPI;
    return (char *)(EXIT_ERROR);
}

/**
 * @author Danko Sačer
 * @brief Function that returns a relation expression by its name
 * param name name of the view
 * @return rel_exp string or EXIT_ERROR
 */
char *AK_get_relation_expression(char *name)
{
    int i = 0;
    char *rel_exp;

    struct list_node *row;
    AK_PRO;

    while ((row = (struct list_node *)AK_get_row(i++, "AK_view")))
    {
        struct list_node *name_elem = AK_GetNth_L2(2, row);
        if (!strcmp(name_elem->data, name))
        {
            struct list_node *rel_exp_elem = AK_GetNth_L2(3, row);
            rel_exp = rel_exp_elem->data;
            AK_EPI;
            return rel_exp;
        }
    }
    AK_EPI;
    return (char *)(EXIT_ERROR);
}

/**
 * @author Kresimir Ivkovic
 * @brief Function that adds a new view to the view table with the corresponding name and value (view query);
 * set_id is optional, if it's not set, the system will determine the new id automatically
 * @param name name og the view
 * @param query query of the view
 * @param rel_exp relation expression of the view
 * @param set_id id of view
 * @return Id of the newly inserted view
 */
int AK_view_add(char *name, char *query, char *rel_exp, int set_id)
{
    char *tblName = "AK_view";
    AK_PRO;

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    int view_id = AK_get_id();
    if (set_id != 0)
        view_id = set_id;
    AK_Insert_New_Element(TYPE_INT, &view_id, tblName, "obj_id", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, name, tblName, "name", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, query, tblName, "query", row_root);
    AK_Insert_New_Element(TYPE_VARCHAR, rel_exp, tblName, "rel_exp", row_root);
    if (AK_check_view_name(name) == EXIT_ERROR)
    {
        AK_EPI;
        return EXIT_ERROR;
    }
    AK_insert_row(row_root);
    AK_EPI;
    return view_id;
}

/**
 * @author Kresimir Ivkovic
 * @brief Function that  removes the view by its object id
 * @param obj_id object id of the view
 * @return Result of AK_delete_row for the view (success or error)
 */
int AK_view_remove_by_object_id(int obj_id)
{
    AK_PRO;

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_INT, &obj_id, "AK_view", "obj_id", row_root);
    int result = AK_delete_row((AK_list_elem)row_root);

    AK_DeleteAll_L3(&row_root);
    AK_EPI;
    return result;
}

/**
 * @author Kresimir Ivkovic
 * @brief Function that renames a view (based on it's name) from "name" to "new_name"
 * @param name name of the view
 * @param new_name new name of the view
 * @return error or success
 */
int AK_view_rename(char *name, char *new_name)
{
    int result = 0, i = 0;
    int view_id;
    char *query;
    char *rel_exp;

    struct list_node *row;
    AK_PRO;

    if (AK_check_view_name(new_name) == EXIT_ERROR)
    {
        AK_EPI;
        return EXIT_ERROR;
    }

    while ((row = (struct list_node *)AK_get_row(i++, "AK_view")))
    {
        struct list_node *name_elem = AK_GetNth_L2(2, row);
        if (!strcmp(name_elem->data, name))
        {
            struct list_node *view_elem = AK_GetNth_L2(1, row);
            struct list_node *query_rel_exp_elem = AK_GetNth_L2(3, row);
            memcpy(&view_id, view_elem->data, sizeof(int));
            query = query_rel_exp_elem->data;
            rel_exp = query_rel_exp_elem->data;
        }
    }

    result = AK_view_remove_by_name(name);
    result = AK_view_add(new_name, query, rel_exp, view_id);
    AK_EPI;
    return result;
}

/**
 * @author Kresimir Ivkovic
 * @brief Function that removes the view by its name by identifying the
 * view's id and passing id to AK_view_remove_by_object_id
 *
 * @param name name of the view
 * @return Result of AK_view_remove_by_object_id
 *  or EXIT_ERROR if no id is found
 */
int AK_view_remove_by_name(char *name)
{
    AK_PRO;

    struct list_node *row_root = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&row_root);

    AK_Update_Existing_Element(TYPE_VARCHAR, name, "AK_view", "name", row_root);
    int result = AK_delete_row(row_root);
    AK_EPI;
    return result;
}

/**
 * @author Kresimir Ivkovic
 * @brief Function that changes the query from a view (determined by it's name) to "query"
 * @param name of the query
 * @param query new query of the view
 * @param rel_exp relation expression of the view
 * @return error or success
 */
int AK_view_change_query(char *name, char *query, char *rel_exp)
{
    AK_PRO;
    int view_id = AK_get_view_object_id(name);

    int result = AK_view_remove_by_name(name);
    result = AK_view_add(name, query, rel_exp, view_id);
    AK_EPI;
    return result;
}

/**
 * @author Darko Hranic
 * @brief Function that shows the data from test view query. Only for test purpose
 * @param rel_exp conditions as string
 */
int AK_test_get_view_data(char *rel_exp)
{
    int primero = 1;
    char *str = strdup(rel_exp);
    char *part;
    struct list_node *expr = (struct list_node *)AK_malloc(sizeof(struct list_node));
    AK_Init_L3(&expr);
    char *srcTable;
    char *destTable = "resultTable";
    strcpy(expr->table, destTable);
    while ((part = strsep(&str, ";")))
    {

        if (primero == 1)
        {
            srcTable = part;
            primero = 0;
        }
        else
        {
            if (!strcmp(part, "<") || !strcmp(part, ">") || !strcmp(part, "=") || !strcmp(part, "OR") || !strcmp(part, "AND"))
            {
                AK_InsertAtEnd_L3(TYPE_OPERATOR, part, sizeof(part), expr);
            }
            else if (!strcmp(part, "firstname"))
            {
                AK_InsertAtEnd_L3(TYPE_ATTRIBS, part, sizeof(part), expr);
            }
            else
            {
                AK_InsertAtEnd_L3(TYPE_VARCHAR, part, sizeof(part), expr);
            }
        }
    }
    int sel1 = AK_selection(srcTable, destTable, expr);
}

/**
 * @author Kresimir Ivkovic, updated by Lidija Lastavec
 * @brief A testing function for view.c functions.
 */
TestResult AK_view_test()
{
    AK_PRO;

    int passed = 0;
    int failed = 0;

    printf("\n*******VIEW TESTS**********\n\n");

    AK_view_add("view1", "SELECT lastname FROM profesor", "profesor;lastname;", 0);
    printf("\n\n");
    AK_view_add("view2", "query2", "firstname;", 0);
    printf("\n\n");

    AK_view_add("view3", "query3", "*;profesor;", 0);
    printf("\n\n");

    AK_view_add("view4", "query4", "2 >;", 0);
    printf("\n\n");

    AK_view_add("view5", "select * from student where firstname=Robert",
                "student;firstname;Robert;=", 0);
    printf("\n\n");

    AK_view_add("view6", "query6", "lastname", 0);
    printf("\n\n");

    AK_view_add("view7", "query7", "student;firstname", 0);
    printf("\n\n");

    AK_print_table("AK_view");

    printf("Obj_id for view1: %d\n", AK_get_view_object_id("view1"));
    printf("Query for view1: %s\n\n", AK_get_view_query("view1"));

    printf("Obj_id for view2: %d\n", AK_get_view_object_id("view2"));
    printf("Query for view2: %s\n\n", AK_get_view_query("view2"));

    printf("Obj_id for view3: %d\n", AK_get_view_object_id("view3"));
    printf("Query for view3: %s\n\n", AK_get_view_query("view3"));

    printf("Obj_id for view4: %d\n", AK_get_view_object_id("view4"));
    printf("Query for view4: %s\n\n", AK_get_view_query("view4"));

    printf("Obj_id for view5: %d\n", AK_get_view_object_id("view5"));
    printf("Query for view5: %s\n\n", AK_get_view_query("view5"));

    printf("\n****TEST 1 - DELETE VIEW BY NAME**** \n");

    int addResult300 = AK_view_add("view300", "query3", "*;profesor;", 0);
    int addResult1 = AK_view_add("view1", "SELECT lastname FROM profesor", "profesor;lastname;", 0);
    int addResult2 = AK_view_add("view2", "query2", "firstname;", 0);
    if (addResult1 == EXIT_ERROR && addResult2 == EXIT_ERROR && addResult300 == EXIT_ERROR)
    {
        printf("\nDeleting views 'view1',  'view2' and 'view300' n");
    }
    else
    {
        printf("\nDeleting views 'view1' and 'view2'  \n");
    }

    int delete1 = AK_view_remove_by_name("view1");
    int delete2 = AK_view_remove_by_name("view2");
    int delete300 = AK_view_remove_by_name("view300");
    if (delete1 != EXIT_ERROR && delete2 != EXIT_ERROR || delete300 != EXIT_ERROR)
    {

        passed++;
        AK_print_table("AK_view");
        printf("\n Test 1 is successful! \n\n");
    }
    else
    {
        failed++;
        printf("\n Test 1 failed!.\n\n");
    }

    printf("\n****TEST 2 - DELETE VIEW BY ID**** \n");
    printf("\nDeleting views 'view2' and 'view6' by Obj_id...\n");
    printf("Obj_id for view2: %d\n", AK_get_view_object_id("view2"));
    printf("Obj_id for view6: %d\n", AK_get_view_object_id("view6"));
    int delete3 = AK_view_remove_by_object_id(AK_get_view_object_id("view2"));
    int delete4 = AK_view_remove_by_object_id(AK_get_view_object_id("view6"));
    if (delete3 != EXIT_ERROR && delete4 != EXIT_ERROR)
    {
        passed++;
        AK_print_table("AK_view");
        printf("\n Test 2 is successful!\n\n");
    }
    else
    {
        failed++;
        printf("\n Test 2 failed!\n\n");
    }

    printf("\n **** TEST 3 - RENAME VIEW ****\n");

    printf("\nRenaming 'view3' to 'view300'...\n");
    int renameView = AK_view_rename("view3", "view300");
    if (renameView != EXIT_ERROR)
    {
        passed++;
        AK_print_table("AK_view");
        printf("\n Test 3 is successful!\n\n");
    }
    else
    {
        failed++;
        printf("\n Test 3 failed!\n\n");
    }

    printf("\n **** TEST 4 - CHANGE QUERY ****\n");

    printf("\nChanging 'view4' query to '44query44'...\n");
    int changeQuery = AK_view_change_query("view4", "44query44", "student;id");
    if (changeQuery != EXIT_ERROR)
    {
        passed++;
        AK_print_table("AK_view");
        printf("\n Test 4 is successful!\n\n");
    }
    else
    {
        failed++;
        printf("\n Test 4 failed!\n\n");
    }

    printf("\n **** TEST 5 - SHOW VIEW DATA ****\n");
    printf("\n Show test view data for view5'...\n");
    int showData = AK_test_get_view_data("student;firstname;Robert;=");
    if (showData != EXIT_ERROR)
    {
        passed++;
        AK_print_table("AK_view");
        printf("\n Test 5 is successful!\n\n");
    }
    else
    {
        failed++;
        printf("\n Test 5 failed!\n\n");
    }

    printf("\n **** TEST 6 - ADD VIEW WITH EXISTING NAME ****\n");
    printf("\n Checking if name 'view4' already exists...\n");
    int addResult = AK_view_add("view4", "test", "lastname;", 0);
    if (addResult == EXIT_ERROR)
    {
        passed++;
        printf("\n Name already exists! View not added.");
        printf("\n Test 6 is successful!\n\n");
    }
    else
    {
        failed++;
        printf("\n Test 6 failed!\n\n");
    }

    printf("\n **** TEST 7 - CHECK VIEW NAME ****\n");
    printf("\n Checks if the name of the view already exist in AK_view table. \n");
    char *checkName = AK_check_view_name("view5");

    if (checkName == EXIT_ERROR)
    {
        passed++;
        printf("\n Name already exists!");
        printf("\n Test 7 is successful!\n\n");
    }
    else
    {
        failed++;
        printf("\n Test 7 failed!\n\n");
    }

    printf("\n *** TEST 8 - GET RELATION EXPRESSION ***\n");
    printf("\n Gets a relation expression by its name. \n ");
    char *getRelationExpression = AK_get_relation_expression("view5");

    if (getRelationExpression != EXIT_ERROR)
    {
        passed++;
        printf("\n View exists! \n");
        printf("\n Test 8 is successful!\n\n");
    }
    else
    {
        failed++;
        printf("\n View doesn't exist! \n");
        printf("\n Test 8 failed!\n\n");
    }

    AK_print_table("AK_view");
    AK_EPI;

    return TEST_result(passed, failed);
}
