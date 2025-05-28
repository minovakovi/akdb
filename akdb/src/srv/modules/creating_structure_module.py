import re
from sql_tokenizer import *
import kalashnikovDB as AK47
from modules.get_module import *

# This module contains functions for creating a database structure (sequence, table etc.)


# create sequence
# developd by Danko Sacer
class Create_sequence_command:
    create_seq_regex = r"(?i)^create\s+sequence\s+([a-zA-Z_][a-zA-Z0-9_]*)"
    pattern = None
    matcher = None

    # matches method
    # checks whether given input matches table_exists command syntax
    def matches(self, input):
        print("Trying to match sequance regex")
        self.pattern = re.compile(self.create_seq_regex)
        self.matcher = self.pattern.match(input)
        if self.matcher:
            self.seq_name = self.matcher.group(1)
            return True
        return False


    # executes the create sequence expression
    # neded revision in sequence.c in function AK_sequence_add which receives
    # only int values but posible is also bigint which is default for
    # undefined values
    def execute(self, input):
        print("start parsing..")
        pars = sql_tokenizer()
        tok = pars.AK_parse_create_sequence(input)
        # isinstance needs revision for swig
        '''
            if isinstance(tok, str):
                print "Error: syntax error in expression"
                print string
                print tok
                return False
        '''

        '''
        print("\nSequence name: ", tok.seq_name)
        print("'AS' definition: ", tok.as_value)
        print("'Start with' value: ", tok.start_with)
        print("'Increment by' value: ", tok.increment_by)
        print("'MinValue' value: ", tok.min_value)
        print("'MaxValue' value: ", tok.max_value)
        print("'Cache' value: ", tok.cache)
        print("'Cycle' value: ", tok.cycle)
        '''

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
    create_table_regex = r"(?i)^create\s+table\s+.*"
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
    def execute(self, expression=None):
        if expression:
            self.expr = expression

        parser = sql_tokenizer()
        token = parser.AK_parse_create_table(self.expr)

        if isinstance(token, str):
            print("Error: syntax error in expression")
            print(self.expr)
            print(token)
            return False

        table_name = str(token.tableName)

        if AK47.AK_table_exist(table_name) == 1:
            print("Error: table'" + table_name + "' already exist")
            return "Table'" + table_name + "' already exist"

        sql_to_ak_type = {
            "int": AK47.TYPE_INT,
            "integer": AK47.TYPE_INT,
            "float": AK47.TYPE_FLOAT,
            "varchar": AK47.TYPE_VARCHAR,
            "text": AK47.TYPE_VARCHAR,
        }

        create_table_attributes = []

        for attribute in token.attributes:
            attr_name = str(attribute[0])
            attr_type_raw = attribute[1]

            # Determine the attribute type
            if isinstance(attr_type_raw, str):
                attr_type = attr_type_raw.lower()
            elif hasattr(attr_type_raw, "asList"):
                attr_type = attr_type_raw[0].lower()
            elif isinstance(attr_type_raw, list):
                attr_type = attr_type_raw[0].lower()
            else:
                attr_type = str(attr_type_raw).lower()

            attr_type_base = re.match(r'^\w+', attr_type)
            if attr_type_base:
                ak_type = sql_to_ak_type.get(attr_type_base.group(0))
            else:
                ak_type = None
            if ak_type is None:
                print(f"Unsupported SQL type: {attr_type}")
                return f"Error. Unsupported SQL type: {attr_type}"

            constraints = [str(c).upper() for c in attribute[2:]] if len(attribute) > 2 else []

            #print(f"Attribute: {attr_name}, Type: {ak_type}, Constraints: {constraints}")

            create_table_attributes.append({
                'name': attr_name,
                'type': ak_type,
                'constraints': constraints  # Pass constraints for future use
            })

        '''
        print(f"Received attributes for {table_name}:")
        for attr in create_table_attributes:
            print(f" - {attr['name']} (type {attr['type']})")
        '''
        attribute_count = len(create_table_attributes)

        try:
            AK47.AK_create_table(table_name, create_table_attributes, attribute_count)
            result = "Table created"
        except Exception as e:
            print("Exception while creating table:", e)
            result = "Error. Creating table failed."

        return result


# create index command
# @author Franjo Kovacic


class Create_index_command:
    create_table_regex = r"(?i)^create\s+index\s+\w+\s+on\s+\w+\s*\(\s*[\w\s,]+\)\s+using\s+\w+"
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
    def execute(self, command):
        self.expr = command
        parser = sql_tokenizer()
        token = parser.AK_parse_create_index(command)
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
        args = None
        last = None

        for stupac in token.stupci:
            node = AK47.list_node()
            node.attribute_name = stupac
            node.next = None
            if args is None:
                args = node
            else:
                last.next = node
            last = node

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
    create_trigger_regex = r"(?i)^create\s+trigger\b.*"
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
    def execute(self, expr):
        parser = sql_tokenizer()
        token = parser.AK_parse_trigger(expr)
        # checking syntax
        if isinstance(token, str):
            print("Error: syntax error in expression")
            print(expr)
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
        except Exception as e:
            result = "Error. Creating trigger failed."
        return result