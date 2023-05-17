import re
import kalashnikovDB as AK47

# This module is used to print the content on the screen


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
