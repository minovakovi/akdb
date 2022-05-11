from sql_tokenizer import *
import time

import sys
sys.path.append('../swig/')
import kalashnikovDB as AK47
import test
import re
import ctypes


def initialize():
    AK47.AK_inflate_config()
    AK47.AK_init_disk_manager()
    AK47.AK_memoman_init()

# is_numeric
# returns int or float value based on input type
# @param lit the value to be checked


def is_numeric(lit):
    # Handle '0'
    if lit == '0':
        return 0
    # Hex/Binary
    litneg = lit[1:] if lit[0] == '-' else lit
    if litneg[0] == '0':
        if litneg[1] in 'xX':
            return int(lit, 16)
        elif litneg[1] in 'bB':
            return int(lit, 2)
        else:
            try:
                return int(lit, 8)
            except ValueError:
                pass
    # Int/Float
    try:
        return int(lit)
    except ValueError:
        pass
    try:
        return float(lit)
    except ValueError:
        pass

# is_date
# returns true if the input value is in date format
# @param lit the value to be checked


def is_date(lit):
    try:
        time.strptime(lit, '%Y-%m-%d')
        return True
    except ValueError:
        return False

# is_datetime
# returns true if the input value is in datetime format
# @param lit the value to be checked


def is_datetime(lit):
    try:
        time.strptime(lit, '%Y-%m-%d %H:%M:%S')
        return True
    except ValueError:
        return False


# is_time
# returns true if the input value is in time format
# @param lit the value to be checked
def is_time(lit):
    try:
        time.strptime(lit, '%H:%M:%S')
        return True
    except ValueError:
        return False

# is_bool
# returns true if the input value is boolean
# @param lit the value to be checked


def is_bool(lit):
    return lit.lower() in ("true", "false")

# get_attr_type
# returns value type code in AK47 notation defined in constants.c
# @param value the value to be checked


def get_attr_type(value):

    integer = type(is_numeric(value)) == int
    decimal = type(is_numeric(value)) == float
    varchar = isinstance(value, str)
    date = is_date(value.replace("'", ""))
    datetime = is_datetime(value.replace("'", ""))
    time = is_time(value.replace("'", ""))
    boolean = is_bool(value.replace("'", ""))

    if integer:
        return AK47.TYPE_INT
    elif decimal:
        return AK47.TYPE_FLOAT
    elif date:
        return AK47.TYPE_DATE
    elif datetime:
        return AK47.TYPE_DATETIME
    elif time:
        return AK47.TYPE_TIME
    elif boolean:
        return AK47.TYPE_BOOL
    elif varchar:
        return AK47.TYPE_VARCHAR
    else:
        print("UNDEFINED")

# get_type_name
# returns type name for supplied type code in AK47 notation defined in constants.c
# @param code the type code to be checked


def get_type_name(code):

    if code == AK47.TYPE_INT:
        return "int"
    elif code == AK47.TYPE_FLOAT:
        return "float"
    elif code == AK47.TYPE_DATE:
        return "date"
    elif code == AK47.TYPE_DATETIME:
        return "datetime"
    elif code == AK47.TYPE_TIME:
        return "time"
    elif code == AK47.TYPE_BOOL:
        return "boolean"
    elif code == AK47.TYPE_VARCHAR:
        return "varchar"
    else:
        return "UNDEFINED"

# akdbError
# prints expression expr and a pointer at the position of error item
# @param expr the expression to be printed
# @param item item where the error occured


def akdbError(expr, item):
    print(expr)
    print(" " * expr.index(item) + "^")

# print_table_command
# defines the structure of print command and its execution


class Print_table_command:
    print_regex = r"^\\p\s+([a-zA-Z0-9_]+)\s*$"
    pattern = None
    matcher = None
    # matches method
    # checks whether given input matches print command syntax

    def matches(self, input):
        self.pattern = re.compile(self.print_regex)
        self.matcher = self.pattern.match(input)
        return self.matcher if self.matcher is not None else None

    # execute method
    # defines what is called when print command is invoked
    def execute(self):
        AK47.AK_print_table(self.matcher.group(1))
        return "OK"

# print_system_table_command
# defines the structure of print system table command and its execution

class Print_system_table_command:
    print_system_table_regex = r"^\\ps\s+([a-zA-Z0-9_]+)\s*$"
    pattern = None
    matcher = None
    # matches method
    # checks whether given input matches print command syntax

    def matches(self, input):
        self.pattern = re.compile(self.print_system_table_regex)
        self.matcher = self.pattern.match(input)
        return self.matcher if self.matcher is not None else None

    # execute method
    # defines what is called when table_exists command is invoked
    def execute(self, cmd):
        string = self.matcher.group(1)
        string = string.encode('utf-8')
        AK47.AK_print_table(string)
        return "OK"

# table_details_command
# defines the structure of table details command and its execution
class Table_details_command:

    table_details_regex = r"^\\d\s+([a-zA-Z0-9_]+)\s*$"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches table_details command syntax
    def matches(self, input):
        self.pattern = re.compile(self.table_details_regex)
        self.matcher = self.pattern.match(input)
        return self.matcher if self.matcher is not None else None

    # execute method
    # defines what is called when table_details command is invoked
    def execute(self, cmd):
        print("Printing out: ")
        result = "Number of attributes: " + \
            str(AK47.AK_num_attr(self.matcher.group(1)))
        result += "\nNumber od records: " + \
            str(AK47.AK_get_num_records(self.matcher.group(1)))
        return result


# table_exists_command
# defines the structure of table exists command and its execution
class Table_exists_command:

    table_details_regex = r"^\\t\s+([a-zA-Z0-9_]+)\?\s*$"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        self.pattern = re.compile(self.table_details_regex)
        self.matcher = self.pattern.match(input)
        return self.matcher if self.matcher is not None else None

    # execute method
    # defines what is called when table_exists command is invoked
    def execute(self):
        if AK47.AK_table_exist(self.matcher.group(1)) == 0:
            result = "Table does not exist."
        else:
            result = "Table exists. You can see it by typing \p <table_name>."
        return result

# create sequence
# developd by Danko Sacer
class Create_sequence_command:
    create_seq_regex = r"^(?i)create sequence(\s([a-zA-Z0-9_]+))+?$"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        print("Trying to match sequance regex")
        self.pattern = re.compile(self.create_seq_regex)
        self.matcher = self.pattern.match(input)
        if (self.matcher is not None):
            message = self.matcher
        else:
            message = None

        return message

    # executes the create sequence expression
    # neded revision in sequence.c in function AK_sequence_add which receives
    # only int values but posible is also bigint which is default for
    # undefined values
    def execute(self, input):
        print("start parsing..")
        pars = sql_tokenizer()
        tok = pars.AK_create_sequence(input)
        # isinstance needs revision for swig
        '''
            if isinstance(tok, str):
                print "Error: syntax error in expression"
                print string
                print tok
                return False
        '''
        print("\nSequence name: ", tok.seq_name)
        print("'AS' definition: ", tok.as_value)
        print("'Start with' value: ", tok.start_with)
        print("'Increment by' value: ", tok.increment_by)
        print("'MinValue' value: ", tok.min_value)
        print("'MaxValue' value: ", tok.max_value)
        print("'Cache' value: ", tok.cache)
        print("'Cycle' value: ", tok.cycle)

        # Check for sequence name, if already exists in database return false
        # Needs more revision for swig after buffer overflow is handled
        '''
            names = AK47.AK_get_column(1, "AK_sequence")
            for name in set(names):
                if(name==tok.seq_name):
                    error = "ERROR the name is already used"
                    return error
        '''
        # executing create statement
        try:
            AK47.AK_sequence_add(str(tok.seq_name), int(tok.start_with), int(
                tok.increment_by), int(tok.max_value), int(tok.min_value), int(tok.cycle))
            result = "Command succesfully executed"
        except:
            result = "ERROR creating sequence didn't pass"

        AK47.AK_print_table("AK_sequence")
        return result

# create table command
# @author Franjo Kovacic


class Create_table_command:
    create_table_regex = r"^(?i)create table(\s([a-zA-Z0-9_\(\),'\.]+))+?"
    pattern = None
    matcher = None
    expr = None

    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        print("Trying to match create table regex")
        self.pattern = re.compile(self.create_table_regex)
        self.matcher = self.pattern.match(input)
        self.expr = input
        if (self.matcher is not None):
            return self.matcher
        else:
            return None

    # executes the create table expression
    def execute(self, expression):
        parser = sql_tokenizer()
        token = parser.AK_parse_create_table(self.expr)
        # checking syntax
        if isinstance(token, str):
            print("Error: syntax error in expression")
            print(self.expr)
            print(token)
            return False
        # get table name
        table_name = str(token.tableName)
        # table should not exist yet
        '''
            For some reason, AK_table_exist won't work, it always just exits here, so it's commented out
            if (AK47.AK_table_exist(table_name) == 1):
                print "Error: table'" + table_name + "' already exist"
                return False
        '''
        # get attributes
        '''
            Create table in table.c currently takes only name and type of attributes.
            Parsing works for other attribute properties as well, so it should be added here when possible.
        '''
        create_table_attributes = []
        for attribute in token.attributes:
            create_table_attributes.append(
                [{'name': str(attribute[0])}, {'type': get_attr_type(str(attribute[1]))}])
        attribute_count = len(create_table_attributes)
        # executing
        try:
            AK47.AK_create_table(
                table_name, create_table_attributes, attribute_count)
            result = "Table created"
        except:
            result = "Error. Creating table failed."
        return result

# create index command
# @author Franjo Kovacic


class Create_index_command:
    create_table_regex = r"^(?i)create index(\s([a-zA-Z0-9_]+))+?$"
    pattern = None
    matcher = None
    expr = None

    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        self.pattern = re.compile(self.create_table_regex)
        self.matcher = self.pattern.match(input)
        self.expr = input
        if (self.matcher is not None):
            return self.matcher
        else:
            return None

    # executes the create index expression
    def execute(self):
        parser = sql_tokenizer()
        token = parser.AK_parse_createIndex(self.expr)
        # checking syntax
        if isinstance(token, str):
            print("Error: syntax error in expression")
            print(self.expr)
            print(token)
            return False
        # get table name
        table_name = str(token.tablica)
        # check if table exist
        '''
            For some reason, AK_table_exist won't work, it always just exits here, so it's commented out
            if (AK47.AK_table_exist(table_name) == 0):
            print "Error: table '"+ table_name +"' does not exist"
            return False
        '''
        # get index name
        index = str(token.IndexIme)
        # get other expression tokens
        args = AK47.list_node()
        args.attribute_name = token.stupci[0]
        for stupac in token.stupci[1:]:
            next = AK47.list_node()
            next.attribute_name = stupac
            args.next = next

        try:
            AK47.AK_create_Index_Table(table_name, args)
            result = "Index created"
        except Exception as e:
            print(e)
            result = "Error. Creating index failed."
        return result

# create trigger command
# @author Franjo Kovacic


class Create_trigger_command:
    create_trigger_regex = r"^(?i)create trigger(\s([a-zA-Z0-9_]+))+?$"
    pattern = None
    matcher = None
    expr = None

    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        print("Trying to match trigger regex")
        self.pattern = re.compile(self.create_trigger_regex)
        self.matcher = self.pattern.match(input)
        self.expr = input
        if (self.matcher is not None):
            return self.matcher
        else:
            return None

    # executes the create trigger expression
    def execute(self):
        parser = sql_tokenizer()
        token = parser.AK_parse_trigger(self.expr)
        # checking syntax
        if isinstance(token, str):
            print("Error: syntax error in expression")
            print(self.expr)
            print(token)
            return False
        # get table name
        table_name = str(token.tableName)
        # check if table exist
        '''
            For some reason, AK_table_exist won't work, it always just exits here, so it's commented out
            if (AK47.AK_table_exist(table_name) == 0):
                print "Error: table '"+ table_name +"' does not exist"
                return False
        '''
        # get trigger name
        trigger = str(token.name)
        p = list()
        p.append(token.EventOption1)
        p.append(token.EventOption2)
        p.append(token.EventOption3)
        # executing
        '''
            Not working
            TypeError: in method 'AK_trigger_add', argument 3 of type 'AK_list *'
        '''
        try:
            AK47.AK_trigger_add(trigger, token.whatOption,
                                p, table_name, token.functionName)
            result = "Trigger created"
        except:
            result = "Error. Creating trigger failed."
        return result

# Insert into


class Insert_into_command:

    insert_into_regex = r"^(?i)insert into(\s([a-zA-Z0-9_]+))+?$"
    pattern = None
    matcher = None

    def matches(self, inp):
        self.pattern = re.compile(self.insert_into_regex)
        self.matcher = self.pattern.match(inp)
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
        return False

#
'''
## create group command
# @author Zvonimir Kapes
class Create_group_command:
	table_details_regex = r"^create group\s+"
	pattern = None
	matcher = None
	command = None

	def matches(self, input):
		self.pattern = re.compile(self.table_details_regex, re.IGNORECASE)
		self.matcher = self.pattern.match(input)
		self.command = input
		return self.matcher != None

	def execute(self):
		print "Printing out: "
		result = "CREATE GROUP "

		pars = sql_tokenizer()
		tokens = pars.AK_create_group(self.command)

		if isinstance(tokens, basestring):
			result = "Wrong command!"
		else:
			print "group id: ", AK47.AK_group_get_id(tokens.groupname)

		if AK47.AK_group_get_id(tokens.groupname) != -1:
			result += "ERROR: Group " + str(tokens.groupname) +" already exists"
			return result

		result += str(AK47.AK_group_add(tokens.groupname, AK47.NEW_ID))
		if tokens.users:
			for user in tokens.users:
				print "...added user: ", user
				# User insert does not work. Password needs to be inserted in hash format.
				#result += str(AK47.AK_user_add(user, password, AK47.NEW_ID))
				#result += str(AK47.AK_add_user_to_group(user, tokens.groupname))
		return result
'''

# grant command
# @author Zvonimir Kapes


class Grant_command:
    table_details_regex = r"^grant\s+"
    pattern = None
    matcher = None
    command = None

    def matches(self, input):
        self.pattern = re.compile(self.table_details_regex, re.IGNORECASE)
        self.matcher = self.pattern.match(input)
        self.command = input
        return self.matcher if self.matcher is not None else None

    def execute(self):
        print("Printing out: ")
        result = "GRANT "

        pars = sql_tokenizer()
        tokens = pars.AK_parse_grant(self.command)
        if isinstance(tokens, str):
            result = "Wrong command!"
        else:
            for user in tokens.users:
                msg = "...grant to: " + user + "\n"
                for privilege in tokens.privileges:
                    msg += "...granted privilege: " + privilege + "\n"
                    for table in tokens.tables:
                        msg += "...on table: " + table + "\n"
                        if tokens.group:
                            res = AK47.AK_grant_privilege_group(
                                user, table, privilege)
                            result += str(res)

                            if res != -1:
                                print(msg)
                                msg = ""
                            else:
                                print("ERROR: Group or table does not exsist: " + str(user) + str(table))
                        else:
                            res = AK47.AK_grant_privilege_user(
                                user, table, privilege)
                            result += str(res)

                            if res != -1:
                                print(msg)
                                msg = ""
                            else:
                                print("ERROR: User or table does not exsist: " + str(user) + str(table))
        return result


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
    select_command_regex = r".*" #r"^(?i)select(\s([a-zA-Z0-9_]+))+?$"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches select command syntax
    def matches(self, input):
        self.pattern = re.compile(self.select_command_regex)
        self.matcher = self.pattern.match(input)
        return self.matcher if self.matcher is not None else None

    # execute method
    # defines what is called when select command is invoked
    def execute(self, expr):
        token = sql_tokenizer().AK_parse_where(expr)
        # Selection table name
        table_name = str(token.tableName)
        return "a\n1\n2\n3\n4\n5"
        if (AK47.AK_table_exist(table_name) == 0):
            print("Error: table '" + table_name + "' does not exist")
            return False

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

    update_command_regex = r"^(?i)update(\s([a-zA-Z0-9_]+))+?$"
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

# sql_executor
# contaions methods for sql operations
class sql_executor:

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

    # add command instances to the commands array
    commands = [print_command, table_details_command, table_exists_command, create_sequence_command, create_table_command,
                create_index_command, create_trigger_command, insert_into_command, grant_command, select_command, update_command, drop_command, print_system_table_command]

    # commands for input
    # checks whether received command matches any of the defined commands for kalashnikovdb,
    # and call its execution if it matches
    def commands_for_input(self, command):
        if isinstance(command, str) and len(command) > 0:
            for elem in self.commands:
                if elem.matches(command) is not None:
                    return (elem.__class__.__name__, elem.execute(command))
        return ("",  "Error. Wrong command: " + command)

    # execute method
    # called when a new command is received (from client)
    def execute(self, command):
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
            print(expr)
            print(token)
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
        return False
