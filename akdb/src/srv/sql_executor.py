from sql_tokenizer import *

import sys
sys.path.append('../swig/')
import kalashnikovDB as AK47

from modules.conditional_module import *
from modules.get_module import *
from modules.print_module import *
from modules.creating_structure_module import *
from modules.table_module import *
from modules.data_manipulation_module import *
from modules.user_control_module import *

def initialize():
    AK47.AK_inflate_config()
    AK47.AK_init_disk_manager()
    AK47.AK_memoman_init()


# sql_executor
# contaions methods for sql operations
class Sql_executor:

    # initialize classes for the available commands
    print_system_table_command = Print_system_table_command()
    print_command = Print_table_command()
    table_details_command = Table_details_command()
    table_exists_command = Table_exists_command()
    create_sequence_command = Create_sequence_command()
    create_table_command = Create_table_command()
    create_index_command = Create_index_command()
    create_trigger_command = Create_trigger_command()
    insert_into_command = Insert_into_command()
    #create_group_command = Create_group_command()
    grant_command = Grant_command()
    select_command = Select_command()
    update_command = Update_command()
    drop_command = Drop_command()

    # Missing delete from

    # add command instances to the commands array
    commands = [print_command, table_details_command, table_exists_command, create_sequence_command, create_table_command,
                create_index_command, create_trigger_command, insert_into_command, grant_command, select_command, update_command, drop_command, print_system_table_command]

    # commands for input
    # checks whether received command matches any of the defined commands for kalashnikovdb,
    # and call its execution if it matches
    def commands_for_input(self, command):
        if isinstance(command, str) and len(command) > 0:
            drop_cmd = Drop_command()
            if drop_cmd.matches(command):
                return ("DropCommand", drop_cmd.execute(command))

            for elem in self.commands:
                if elem.matches(command) is not None:
                    print(elem)
                    return (elem.__class__.__name__, elem.execute(command))
        return ("",  "Error. Wrong command: " + command)


    # execute method
    # called when a new command is received (from client)
    def execute(self, command):
    # execute "\t <ime_tabele>?" command
        if isinstance(command, str) and command.startswith("\\t ") and command.endswith("?"):
            tablename = command[3:-1].strip()
            exists = AK47.AK_table_exist(tablename)
            if exists:
                return ("TableExistsCommand", f"✔ Table '{tablename}' exists.")
            else:
                return ("TableExistsCommand", f"✖ Table '{tablename}' does NOT exist.")
    
        return self.commands_for_input(command)


    # insert
    # executes the insert expression
    # @param self object pointer
    # @parama expr the expression to be executed
    def insert(self, expr):
        parser = sql_tokenizer()
        token = parser.AK_parse_insert_into(expr)
        
        if isinstance(token, str):
            print("Error: syntax error in expression")
            #print(expr)
            #print(token)
            return False
        
        table_name = str(token.tableName)
        
        # is there a table
        if (AK47.AK_table_exist(table_name) == 0):
            print("Error: table '" + table_name + "' does not exist")
            return False
        # data values for insert
        insert_attr_values = [x.replace(
            "'", "") for x in list(token.columnValues[0])]
        # data types for insert
        insert_attr_types = [get_attr_type(
            x.replace("'", "")) for x in list(token.columnValues[0])]
        # get array of attributes for table
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
        # values for insert
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

    def drop(self, expr):
        parser = sql_tokenizer()
        token = parser.AK_parse_drop(expr)

        if isinstance(token, str):
            print("Error: syntax error in expression")
            return False

        objekt = token.objekt.lower()
        ime_objekta = token.ime_objekta.replace("'", "").replace("`", "").strip()

        _dropType = {
            "table": AK47.OBJECT_TYPE_TABLE,
            "index": AK47.OBJECT_TYPE_INDEX,
            "sequence": AK47.OBJECT_TYPE_SEQUENCE,
            "trigger": AK47.OBJECT_TYPE_TRIGGER,
            "view": AK47.OBJECT_TYPE_VIEW
        }

        if objekt not in _dropType:
            print(f"Error: unknown object type '{objekt}'")
            return False

        if AK47.AK_drop(_dropType[objekt], ime_objekta) == AK47.EXIT_SUCCESS:
            return True
        else:
            print(f"Error: failed to drop {objekt} '{ime_objekta}'")
            return False

