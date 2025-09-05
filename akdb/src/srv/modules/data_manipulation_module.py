import re
from sql_tokenizer import *
import kalashnikovDB as AK47
from modules.conditional_module import *
from modules.get_module import *
from modules.print_module import *
from modules.creating_structure_module import *
from modules.table_module import *
from modules.sql_error_module import *

# This module contains functions for manipulation with 
# data in the database (insert, update, select etc.)


# Insert into
class Insert_into_command:

    insert_into_regex = r"^(?i)insert into(\s([a-zA-Z0-9_]+))+?$"
    #insert_into_regex = r"insert\s+into\s+([a-zA-Z_][a-zA-Z0-9_]*)*\s*\(.*\)\s*values\s*\((.*)\)"
    pattern = None
    matcher = None

    def matches(self, inp):
        self.pattern = re.compile(self.insert_into_regex)
        self.matcher = self.pattern.match(inp)
        print(self.matcher)
        return self.matcher if self.matcher is not None else None

    def execute(self):
        expr = self.matcher.group(0)
        parser = sql_tokenizer()
        token = parser.AK_parse_insert_into(expr)
        if isinstance(token, str):
            print("Error: syntax error in expression")
            print(expr)
            print(token)
            return False
        table_name = str(token.tableName)
        # does the table exist
        if (AK47.AK_table_exist(table_name) == 0):
            print("Error: table '" + table_name + "' does not exist")
            return False
        # data values for insert
        insert_attr_values = [x.replace(
            "'", "") for x in list(token.columnValues[0])]
        # data types for insert
        insert_attr_types = [get_attr_type(
            x.replace("'", "")) for x in list(token.columnValues[0])]
        # get attributes array for table
        table_attr_names = str(
            AK47.AK_rel_eq_get_atrributes_char(table_name)).split(";")
        # get attribute types for table
        table_attr_types = str(
            AK47.AK_get_table_atribute_types(table_name)).split(";")
        # attribute names for insert
        insert_attr_names = table_attr_names
        # attributes for insert
        if(token.columns):
            insert_attr_names = []
            table_types_temp = table_attr_types
            table_attr_types = []
            insert_columns = list(token.columns[0])
            for index, col in enumerate(insert_columns):
                if col not in table_attr_names:
                    print("\nError: table has no attribute '" + str(col) + "':")
                    akdbError(expr, col)
                    return False
            # check attributes for insert
            for ic, col in enumerate(insert_columns):
                for ia, tab in enumerate(table_attr_names):
                    if col == tab:
                        if tab not in insert_attr_names:
                            insert_attr_names.append(tab)
                            table_attr_types.append(int(table_types_temp[ia]))
                        else:
                            print("\nError: duplicate attribute " + tab + ":")
                            akdbError(expr, tab)
                            return False

            if (len(insert_columns) == len(insert_attr_values)):
                for index, tip in enumerate(insert_attr_types):
                    if int(insert_attr_types[index]) != int(table_attr_types[index]):
                        type_name = get_type_name(int(table_attr_types[index]))
                        print("\nError: type error for attribute '" + insert_attr_names[index] + "':")
                        akdbError(expr, insert_attr_values[index])
                        print("Expected: " + type_name)
                        return False
            else:
                print("\nError: attribute names number not matching attribute values number supplied for table '" + table_name + "':")
                akdbError(expr, insert_columns[0])
                return False
        # only values for insert
        elif (len(table_attr_names) < len(insert_attr_values)):
            print("\nError: too many attibutes, table " + str(token.tableName) + " has " + str(len(table_attr_names)))
            return False
        elif (len(table_attr_names) > len(insert_attr_values)):
            print("\nError: too few attibutes, table " + str(token.tableName) + " has " + str(len(table_attr_names)))
            return False
        else:
            for index, tip in enumerate(insert_attr_types):
                if insert_attr_types[index] != int(table_attr_types[index]):
                    type_name = get_type_name(int(table_attr_types[index]))
                    print("\nError: type error for attribute '" + insert_attr_names[index] + "':")
                    akdbError(expr, insert_attr_values[index])
                    print("Expected: " + type_name)
                    return False
        if(AK47.insert_data_test(table_name, insert_attr_names, insert_attr_values, insert_attr_types) == AK47.EXIT_SUCCESS):
            return True
        else:
            return False


# select
# ALERT ******************************************************************
# ----- translate expression from infix to postfix                                                  *
# ----- add rest of the AKdb types to 'get_attr_type()' ->------------------|                       *
# -----  |-->  such as TYPE_ATTRIBS, TYPE_OPERAND, TYPE_OPERATOR, etc.      |                       *
# ----- fix 'selection_test()' -> 'AK_selection':  ->-------------------------------------------|   *
# -----  |-->  it selects all (*) data instead of taking 'selected attributes' in account  ->---|   *
# -----  |-->  similar problems also found with WHERE clause    ->------------------------------|   *
# -----  |-->  Justin Case check select tokenizer/executor logic                                |   *
# ----- Warning: contains HC-ed data for testing purpose                                            *
# ALERT ******************************************************************
# @author Davor Tomala
# executes the select expression
# @param self object pointer
# @parama expr the expression to be executed
class Select_command:

    #Commented regex is not valid, rewrite it.
    #select_command_regex = r".*" #r"^(?i)select(\s([a-zA-Z0-9_]+))+?$"
    
    select_command_regex = r"select([^*](?!from))*\*?([^*](?!from))* from"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches select command syntax
    def matches(self, input):
        self.pattern = re.compile(self.select_command_regex)
        self.matcher = self.pattern.match(input)
        #print(self.matcher)
        return self.matcher if self.matcher is not None else None

    # execute method
    # defines what is called when select command is invoked
    def execute(self, expr):
        token = sql_tokenizer().AK_parse_where(expr)
        
        # Syntax error? Why? Why is there a select when I'm trying to insert the data?
        # Selection table name
        #try:
        table_name = token.tableName
        #except AttributeError:
        #    #print("Error")
        #    return "Not a valid table name provided."
        
        #return "a\n1\n2\n3\n4\n5"
        if (AK47.AK_table_exist(table_name) == 0):
            print(f"Error: Failed because the table {table_name} does not exist.")
            return("Table '" + table_name + "' does not exist.")


        # Get table attribute list
        table_attr_names = str(
            AK47.AK_rel_eq_get_atrributes_char(table_name)).split(";")
        # Get table attribute type
        table_attr_types = str(
            AK47.AK_get_table_atribute_types(table_name)).split(";")
        # Attribute names for selection (*)
        select_attr_names = table_attr_names
        # WHERE condition
        condition = token.condition if (
            token.condition is not None) else ''  # keep an eye on this test
        # Expression
        expression = []
        # Expression types
        expr_types = []
        # Result table name (randomized)
        resultTable = "student"
        # Specific attributes for selection
        if(token.attributes):
            if(token.attributes[0] == '*'):
                
                table_types_temp = table_attr_types
                table_attr_types = []
                print(table_types_temp)
                for index, name in enumerate(select_attr_names):
                    table_attr_types.append(int(table_types_temp[index]))
                    expr_types.append(get_attr_type(table_types_temp[index]))
                
            else:
                select_attr_names = []
                table_types_temp = table_attr_types
                table_attr_types = []
                select_columns = list(token.attributes)
                for index, col in enumerate(select_columns):
                    if col not in table_attr_names:
                        print("\nError: table has no attribute " + str(col) + ":")
                        akdbError(expr, col)
                        return False
                # Check attributes for selection
                for ic, col in enumerate(select_columns):
                    for ia, tab in enumerate(table_attr_names):
                        if col == tab:
                            if tab not in select_attr_names:
                                select_attr_names.append(tab)
                                table_attr_types.append(
                                    int(table_types_temp[ia]))
                                expr_types.append(get_attr_type(
                                    table_types_temp[index]))
                            else:
                                print("\nError: duplicate attribute " + tab + ":")
                                akdbError(expr, tab)
                                return False

                # SELECT too many attributes
                if (len(select_attr_names) > len(table_attr_names)):
                    print("\nError: too many attibutes, table " + str(token.tableName) + " has " + str(len(table_attr_names)))
                    return False

        # SELECT * ...
        else:
            table_types_temp = table_attr_types
            table_attr_types = []

            for index, name in enumerate(select_attr_names):
                table_attr_types.append(int(table_types_temp[index]))
                expr_types.append(get_attr_type(table_types_temp[index]))

        # WHERE ...
        if (condition != ''):
            # condition attribute types
            condition_attr_types = [get_attr_type(
                x.replace("'", "")) for x in list(token.condition.expression[0])]
            for cond in condition_attr_types:
                expr_types.append(cond)

        expression.append(expr)

        #Make a TypeMap for mapping from a PyList (Python) to a struct list_node (C) in SWIG interface
        #Call next function: int AK_select(char *srcTable, char *destTable, struct list_node *attributes, struct list_node *condition, struct list_node *ordering)
        
        # TEST DATA!
        print(expression)
        expression = ["year", "1990", ">"]
        expr_types = [AK47.TYPE_ATTRIBS, AK47.TYPE_INT, AK47.TYPE_OPERATOR]

        # if(AK47.AK_selection(table_name, resultTable, expression) ==
        # EXIT_SUCCESS):
        if(AK47.selection_test(table_name, resultTable, expression, expr_types) == 1):
            return True
        else:
            return False
        return False


# update
# ALERT *****************************************************************************************************                                                              *
# ----- reimplementation of 'AK_update[_row]' for simplicity !?                                             *
# ----- 'AK_update_row' returns valid answer (True/EXIT_SUCCESS) although change isin't visible  ->-----|   *
# ----- |-->  possible problems with update tokenizer/executor logic   ->-------------------------------|   *
# ----- |-->  or AK_update row implementation                                                           |   *
# ALERT ******************************************************************
# @author Davor Tomala
# executes the update expression
# @param self object pointer
# @parama expr the expression to be executed
class Update_command:

    update_command_regex = r"(?i)^update\s+\w+\s+set\s+.*"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches update command syntax
    def matches(self, input):
        self.pattern = re.compile(self.update_command_regex)
        self.matcher = self.pattern.match(input)
        return self.matcher if self.matcher is not None else None

    # execute method
    # defines what is called when update command is invoked
    def execute(self, expr):
        token = sql_tokenizer().AK_parse_where(expr)
        # Update table name
        table_name = str(token.tableName)

        if (AK47.AK_table_exist(table_name) == 0):
            print("Error: table '" + table_name + "' does not exist")
            return False

        # Update values
        # update_attr_values = map(lambda x: x.replace("'",""),list(token.columnValues[0]))
        # Get table attribute list
        table_attr_names = str(
            AK47.AK_rel_eq_get_atrributes_char(table_name)).split(";")
        # Get table attribute type
        table_attr_types = str(
            AK47.AK_get_table_atribute_types(table_name)).split(";")
        # Attribute names for update
        update_attr_names = table_attr_names
        # WHERE condition
        condition = token.condition if (
            token.condition is not None) else ''  # keep an eye on this test

        # Attributes for update
        if(token.columnNames):
            update_attr_names = []
            table_types_temp = table_attr_types
            table_attr_types = []
            update_columns = list(token.columnNames)
            for index, col in enumerate(update_columns):
                if col not in table_attr_names:
                    print("\nError: table has no attribute " + str(col) + ":")
                    akdbError(expr, col)
                    return False
            # Check attributes for update
            for ic, col in enumerate(update_columns):
                for ia, tab in enumerate(table_attr_names):
                    if col == tab:
                        if tab not in update_attr_names:
                            update_attr_names.append(tab)
                            table_attr_types.append(int(table_types_temp[ia]))
                        else:
                            print("\nError: duplicate attribute " + tab + ":")
                            akdbError(expr, tab)
                            return False

            # UPDATE too many attributes
            if (len(update_attr_names) > len(table_attr_names)):
                print("\nError: too many attibutes, table " + str(token.tableName) + " has " + str(len(table_attr_names)))
                return False

        else:
            print("\nError: No attributes for for update!")
            return False

        # WHERE ...
        if (condition != ''):
            # condition attribute types
            condition_attr_types = [get_attr_type(
                x.replace("'", "")) for x in list(token.condition[1])]

        # Prepare update data element
        # This is Test Data!
        # Iteration required for more than one attribute!
        element = AK47.list_node()
        AK47.AK_Init_L3(id(element))
        AK47.AK_DeleteAll_L3(id(element))

        updateColumn = token.columnNames[0]
        whereColumn = token.condition[1][0]
        whereValue = token.condition[1][2]
        newValue = token.columnValues[0]

        if type(whereValue) == int:
            AK47.AK_Update_Existing_Element(
                AK47.TYPE_INT, whereValue, table_name, updateColumn, element)
        elif type(whereValue) == float:
            AK47.AK_Update_Existing_Element(
                AK47.TYPE_FLOAT, whereValue, table_name, updateColumn, element)
        # elif type(whereValue) == str:
           # AK47.AK_Insert_New_Element_For_Update(AK47.TYPE_VARCHAR, whereValue, table_name, updateColumn, element, 1)

        if type(newValue) == int:
            AK47.AK_Insert_New_Element(
                AK47.TYPE_INT, newValue, table_name, whereColumn, element)
        elif type(newValue) == float:
            AK47.AK_Insert_New_Element(
                AK47.TYPE_FLOAT, newValue, table_name, whereColumn, element)
        # elif type(newValue) == str:
            #AK47.AK_Insert_New_Element_For_Update(AK47.TYPE_VARCHAR, newValue, table_name, whereColumn, element, 0)

            #update_Row(table, column1, column2, key, new_value)
        # if(AK47.update_Row(table_name, 'weight', 'id_student', 1, 80) == EXIT_SUCCESS):
        # if(AK47.AK_update_row(element) == AK47.EXIT_SUCCESS):
            # return True
        else:
            return False
        return False


# Drop
#@author Filip Sostarec
class Drop_command:

    drop_regex = r"^(?i)drop(\s([a-zA-Z0-9_\(\),'\.]+))+?$"
    pattern = None
    matcher = None
    expr = None
    
    _dropType = {
        "table": 0,
        "index": 1,
        "view": 2,
        "sequence": 3,
        "trigger": 4,
        "function": 5,
        "user": 6,
        "group": 7,
        "constraint": 8
    }

    def matches(self, inp):
        self.pattern = re.compile(self.drop_regex)
        self.matcher = self.pattern.match(inp)
        self.expr = inp
        if (self.matcher is not None):
            return self.matcher
        else:
            return None

    def execute(self):
        parser = sql_tokenizer()
        token = parser.AK_parse_drop(self.expr)
        print(f"{token=}")
        if isinstance(token, str):
            print("Error: syntax error in expression")
            print(token)
            return False
        objekt = str(token.objekt)
        # Removes all []' from string
        translation_table = str.maketrans("", "", "[]'")
        table_name = str(token.ime_objekta)
        table_name = table_name.translate(translation_table)

        drop_args = AK47.drop_arguments()
        drop_args.value = table_name

        if (AK47.AK_table_exist(table_name) == 0):
            print("Error: table '" + table_name + "' does not exist")
            return False
        
        AK47.AK_drop(self._dropType[objekt], drop_args)
