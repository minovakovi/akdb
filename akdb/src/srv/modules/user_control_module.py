import re
from sql_tokenizer import *
import kalashnikovDB as AK47

# This module contains all functions related to the database users (granting permissions and other ones)


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