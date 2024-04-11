#Class that contains functions for automatic client-server communication testing 
#Written by : Marko Vertus
import sys
import time
from colors import bcolors


#Test functions

#Test for creating a table
def CreateTableTest():
    return "create table test(redak varchar(20))"

#Test for listing out all the commands currently supported
def AllCommandsTest():
    return "\?"

#Test for printing out details on a given table
def TableDetailsTest():
    return "\d test"

#Test for printing out the table
def PrintTableTest():
    return "\p test"

#Test for checking if the table exists or not
def TableExistsTest():
    return "\t test?"

#Test for creating a new index on a table
#Currently not really supported/implemented
def CreateIndexTest():
    return "create index testniIndex on table test(redak)"

#Test for creating a new user with a new password
def CreateUserTest():
    return "create user testniKorisnik with password testnaLozinka"

#Test for creating a new trigger on a table using some function (currently written as xy)
#Currently not really supported/implemented
#See issue 176 for more detail
def CreateTriggerTest():
    return "create trigger testniTrigger on test for each row execute xy;"

#Test for creating a new sequence 
def CreateSequenceTest():
    return "create sequence testniSequence start with 1 increment by 1 maxvalue 999 cycle;"

#Test for creating a new view for a given table
#Currently not really supported/implemented
def CreateViewTest():
    return "create view comedies as select * from films where kind='comedy'"

#Test for creating a new function
#Currently not really supported/implemented
def CreateFunctionTest():
    return "create function testnaFunkcija(integer,integer) returns integer as 'select $1+$2' language sql "

#Test for altering an existing index
#Currently not really supported/implemented
def AlterIndexTest():
    return "alter index testniIndex rename to testniIndex2"

#Test for altering an existing table
#Currently not really supported/implemented
def AlterTableTest():
    return "alter table test rename to test2"

#Test for altering an existing sequence
#Currently not really supported/implemented
def AlterSequenceTest():
    return "alter sequence testniSequence rename to testniSequence2"

#Test for altering an existing view
#Currently not really supported/implemented
def AlterViewTest():
    return "alter view comedies rename to horror"

#Test for 'select' operator
def SelectTest():
    return "select * from test"

#Test for 'update' operator
def UpdateTest():
    return "update test set redak=1 where redak=2"

#Test for 'insert' operator
def InsertTest():
    return "insert into test(redak) values('test')"

#Testing for granting permissions on tables
def GrantTest():
    return "grant insert on test to public"

#Test for dropping a table
def DropTest():
    return "drop table test"

#Test for starting a begin-end block
#Currently not really supported/implemented
def BeginTest():
    return "begin"

#Test for writing out history of actions
def HistoryTest():
    return "history"

from colors import bcolors

def Help():
    print(f"{bcolors.RED}Welcome to the AkDB Client.{bcolors.ENDC}")
    print("--------------------------------------------------------------|")
    print(f"{bcolors.RED}Available commands:{bcolors.ENDC}                                           |")
    print(f"    {bcolors.RED}help{bcolors.ENDC}   - Show this help message - {bcolors.OKGREEN}Radi{bcolors.ENDC}                    |")
    print(f"    {bcolors.RED}history{bcolors.ENDC} - List all previously typed commands - {bcolors.OKGREEN}Radi{bcolors.ENDC}       |")
    print(f"    {bcolors.RED}testme{bcolors.ENDC}  - Executes all tests - {bcolors.OKGREEN}Radi{bcolors.ENDC}                       |")
    print(f"    {bcolors.RED}\p{bcolors.ENDC}      - print table command                             |")
    print(f"    {bcolors.RED}\ps{bcolors.ENDC}     - print system table command                      |")
    print(f"    {bcolors.RED}\d{bcolors.ENDC}      - table details command                           |")
    print(f"    {bcolors.RED}\_t{bcolors.ENDC}     - table details command                           |")
    print(f"    {bcolors.RED}?{bcolors.ENDC}       - Help will appear, with instructions             |")
    print(f"    {bcolors.RED}\q{bcolors.ENDC}      - Disconnect from the server and quit akdb client |")
    print(f"    {bcolors.RED}select{bcolors.ENDC}  - select * from test                              |")
    print(f"    {bcolors.RED}grant{bcolors.ENDC}   - grant update on test to public                  |")
    print(f"    {bcolors.RED}create_table{bcolors.ENDC} - create table test(name varchar(50))        |")
    print(f"    {bcolors.RED}create_index{bcolors.ENDC} - create index indeks on test(name)          |")
    print("--------------------------------------------------------------|")
