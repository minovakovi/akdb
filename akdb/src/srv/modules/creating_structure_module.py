import re
from sql_tokenizer import *
import kalashnikovDB as AK47
from modules.get_module import *

# This module contains functions for creating a database structure (sequence, table etc.)


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