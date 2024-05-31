import kalashnikovDB as AK47
import re

# All functions related to tables are contained here except for the drop command which
# belongs to the data manipulation module since we can also drop the view, index, trigger etc.


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