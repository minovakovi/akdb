import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../swig')))

import re
from sql_tokenizer import *
import kalashnikovDB as AK47
from modules.get_module import *

# This module contains functions for creating a database structure (sequence, table etc.)


# create sequence
# developd by Danko Sacer
class Create_sequence_command:
    """
    Handles 'create sequence' SQL commands.

    >>> cmd = Create_sequence_command()
    >>> cmd.matches('create sequence myseq') is not None
    True
    >>> cmd.execute('create sequence myseq') # doctest: +SKIP
    # Output and return value depend on C extension and parser
    """
    create_seq_regex = r"(?i)^create sequence(\s([a-zA-Z0-9_]+))+?$"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        # print("Trying to match sequance regex")
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
        # Post-process raw values from tokenizer for executor logic
        def to_int(val, default):
            try:
                return int(val)
            except (ValueError, TypeError):
                return default

        # Set defaults based on as_value
        as_value = tok.as_value
        min_default = max_default = start_default = None
        if as_value == 'smallint':
            min_default = -32768
            max_default = 32767
        elif as_value == 'int':
            min_default = -2147483648
            max_default = 2147483647
        elif as_value == 'bigint':
            min_default = -9223372036854775808
            max_default = 9223372036854775807
        else:
            min_default = 0
            max_default = 255

        min_value = to_int(tok.min_value, min_default) if tok.min_value != 'no minvalue' else min_default
        max_value = to_int(tok.max_value, max_default) if tok.max_value != 'no maxvalue' else max_default
        start_with = to_int(tok.start_with, min_value) if tok.start_with != 'no start' else min_value
        increment_by = to_int(tok.increment_by, 1)
        cache = to_int(tok.cache, 15)
        cycle = 1 if tok.cycle == 'cycle' else 0

        # Check if sequence already exists before creating
        try:
            seq_id = AK47.AK_sequence_get_id(str(tok.seq_name))
            if seq_id != -1:
                print(f"Sequence '{tok.seq_name}' already exists with id {seq_id}. Skipping creation.")
                return "Sequence already exists"
        except Exception as e:
            print(f"Error checking sequence existence: {e}")

        # Print parameters for debugging
        print(f"Calling AK_sequence_add with: name={tok.seq_name}, start_with={start_with}, increment_by={increment_by}, max_value={max_value}, min_value={min_value}, cycle={cycle}")

        try:
            AK47.AK_sequence_add(str(tok.seq_name), start_with, increment_by, max_value, min_value, cycle)
            result = "Command succesfully executed"
        except Exception as e:
            print(f"Error in AK_sequence_add: {e}")
            result = "ERROR creating sequence didn't pass"

        AK47.AK_print_table("AK_sequence")
        return result

# create table command
# @author Franjo Kovacic
class Create_table_command:
    """
    Handles 'create table' SQL commands.

    >>> cmd = Create_table_command()
    >>> cmd.matches('create table students (id int, name varchar)') is not None
    True
    >>> cmd.matches('CREATE TABLE students (id int, name varchar)') is not None
    True
    >>> cmd.matches('create table') is not None
    False
    >>> cmd.execute('create table students (id int, name varchar)') # doctest: +SKIP
    # Output and return value depend on C extension and parser
    """
    create_table_regex = r"^create table(\s([a-zA-Z0-9_\(\),'.]+))+?"
    pattern = None
    matcher = None
    expr = None
    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        # print("Trying to match create table regex")
        self.pattern = re.compile(self.create_table_regex, re.IGNORECASE)
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
        if (AK47.AK_table_exist(table_name) == 1):
                print("Error: table'" + table_name + "' already exist")
                return "Table'" + table_name + "' already exist"
           
        '''
            For some reason, AK_table_exist won't work, it always just exits here, so it's commented out
            
        '''
        # get attributes
        '''
            Create table in table.c currently takes only name and type of attributes.
            Parsing works for other attribute properties as well, so it should be added here when possible.
        '''
        
        create_table_attributes = []
        # print(len(token.attributes))
        for attribute in token.attributes:
            create_table_attributes.append(
                [{'name': str(attribute[0])}, {'type': str(attribute[1])}])
            # print(create_table_attributes)
        # print(create_table_attributes)
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
    """
    Handles 'create index' SQL commands.

    >>> cmd = Create_index_command()
    >>> cmd.matches('create index idx_students on students (id) using btree') is not None
    True
    >>> cmd.matches('CREATE INDEX idx_students ON students (id) USING btree') is not None
    True
    >>> cmd.matches('create index') is not None
    False
    >>> cmd.execute() # doctest: +SKIP
    # Output and return value depend on C extension and parser
    """
    create_table_regex = r"^create index(\s([a-zA-Z0-9_]+))+?"
    pattern = None
    matcher = None
    expr = None
    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        self.pattern = re.compile(self.create_table_regex, re.IGNORECASE)
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
    """
    Handles 'create trigger' SQL commands.

    >>> cmd = Create_trigger_command()
    >>> cmd.matches('create trigger trg_students') is not None
    True
    >>> cmd.matches('CREATE TRIGGER trg_students') is not None
    True
    >>> cmd.matches('create trigger') is not None
    False
    >>> cmd.execute() # doctest: +SKIP
    # Output and return value depend on C extension and parser
    """
    create_trigger_regex = r"^create trigger(\s([a-zA-Z0-9_]+))+?"
    pattern = None
    matcher = None
    expr = None
    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        # print("Trying to match trigger regex")
        self.pattern = re.compile(self.create_trigger_regex, re.IGNORECASE)
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

if __name__ == "__main__":
    import doctest
    doctest.testmod()