from sql_executor import *
import kalashnikovDB as AK47


# sql_executor tests by Marko Sinko


def is_numeric_test():
    """
	>>> list(map(is_numeric, ["1", "a", "0", "b", "3.3", "4", "c"]))
	[1, None, 0, None, 3.3, 4, None]
	"""


def is_date_test():
    """
	>>> list(map(is_date, ["2011-12-12", "21234", "2013-05-21", "2011.23.45"]))
	[True, False, True, False]
	"""


def is_datetime_test():
    """
	>>> list(map(is_datetime, ["2011-12-12 13:42:30", "2011-12-12", "13:42:30", "13:42:30 2011-12-12", "2011-12-12 33:42:30"]))
	[True, False, False, False, False]
	"""


def is_time_test():
    """
	>>> list(map(is_time, ["12:12:12", "21234", "13:55:66", "2011-23-10", "30:20:10"]))
	[True, False, False, False, False]
	"""


def is_bool_test():
    """
	>>> list(map(is_bool, ["TRUE", "FALSE", "tRue", "True", "False", "faLsE", "istina"]))
	[True, True, True, True, True, True, False]
	"""


def get_attr_type_test():
    """
	>>> list(map(get_type_name, map(get_attr_type, ["1", "1.2", "a", "#", "2021-05-10", "12:12:12", "false", "2011-12-12 13:42:30"])))
	['int', 'float', 'varchar', 'varchar', 'date', 'time', 'boolean', 'datetime']
	"""


# init and load sql_executor function
initialize()
executor = sql_executor()

student_attr_name = ["id_student", "first_name", "last_name", "year_of_birth", "grade_avg"]
student_attr_type = [AK47.TYPE_INT, AK47.TYPE_VARCHAR, AK47.TYPE_VARCHAR, AK47.TYPE_INT, AK47.TYPE_FLOAT]
AK47.create_header_test("student", student_attr_name, student_attr_type)

insert_into1_output = executor.insert("INSERT INTO student VALUES ('1','Marko','Sinko','1990','2.0')")
insert_into2_output = executor.insert(
    "INSERT INTO student(id_student, first_name, last_name, year_of_birth, grade_avg) VALUES ('2','Marinko','Radic','1991','3.3')")
insert_into3_output = executor.insert(
    "INSERT INTO profesor(id_student, first_name, last_name, year_of_birth, grade_avg) VALUES ('2','Marinko','Radic','1991','3.3')")


def insert_into_test():
    """
	>>> insert_into1_output
	True
	>>> insert_into2_output
	True
	>>> insert_into3_output
	False
	"""


# CREATE GROUP TEST
# tokens are not implemented for this test, need to create Create_group_command() token
# for further testing, need implementation, need to check AK_group_add() function
# AK_group_add(tokens.groupname, AK47.NEW_ID) -- weird input params?? groupname-NEW_ID??


# GRANT TEST
# AK47.AK_user_add("100", id(1111), "10")
# grant = Grant_command()
# grant.command = "GRANT INSERT ON student TO Marko"
# grant_output = grant.execute()

# Tokenizer works well, need to check in the Grand_command() class how the grand.execute method
# is executed because it doesn't accept the command like it should. That can be seen from the
# return value when executing the grand.execute() method, need to check the implementation of
# AK_grant_privilege_user(user, table, privilege) function


#  def create_grand_test():
# 	"""
# 	>>> grant_output
#   TODO
# 	"""


# SELECT TEST always returns the whole table, WHERE doesn't work, probably the selection_test() doesn't
# work as well, need to check the implementation in the lower level of the SELECT statement or need to
# test AK_parse_where() tokenizing
# TODO: rewrite select_test() after select.execute() is fixed

# select = Select_command()
# select1_output = select.execute("SELECT * FROM student")
# select2_output = select.execute("SELECT last_name from student")
# select3_output = select.execute("SELECT random_attr from student")
# select4_output = select.execute("SELECT first_name from profesor")
# select5_output = select.execute("SELECT * FROM student WHERE year_of_birth>1990")


# def select_test():
#     """
# 	>>> select1_output
# 	True
# 	>>> select2_output
# 	True
# 	>>> select3_output
# 	False
# 	>>> select4_output
# 	False
# 	>>> select5_output
# 	True
# 	"""


# after the 553 commit it raises a new error: AK_Insert_New_Element_For_Update()
# probably something is wrong with the AK_update_Row(table, column1, column2, key, new_value) function
# also need to check AK_parse_where() tokenizing
# update = Update_command()
# update1_output = update.execute("UPDATE student SET grade_avg = 2.200 WHERE id_student = 1")
# update2_output = update.execute("UPDATE profesor SET grade_avg = 2.200 WHERE id_student = 1")


# def update_test():
#     """
#     >>> update1_output
#     True
#     >>> update2_output
#     False
#     """



sequence = Create_sequence_command()
sequence1_output = sequence.execute("Create sequence numac_1 start with 1 increment by 2 minvalue 0 maxvalue 100 cycle")
# TODO: fix buffer overflow when running this test, check output in sequence_test() below and remove this comment
# sequence2_output = sequence.execute("Create sequence numac_2 start with 1 increment by 2 minvalue 0 maxvalue 100")

def sequence_test():
    """
    >>> sequence1_output
    'Command succesfully executed'
    """


# create table doesn't work
# need to reimplement tokenizing of AK_parse_create_table() because it doesn't have AK_TYPE_INT,AK_TYPE_VARCHAR types that are
# needed for adding attributes to the table
# INT is not used
# need to have this structure: [AK47.TYPE_INT, AK47.TYPE_VARCHAR, AK47.TYPE_VARCHAR, AK47.TYPE_INT, AK47.TYPE_FLOAT] like in the example above
# UPDATE:
# parameter in execute method is not used and it should be removed or the need of using the expr attribute should be changed
# IMPORTANT: execute method returns result that table is created but that table cannot be printed (it says table doesn't exist)
create = Create_table_command()
create.expr = "CREATE TABLE tbl (var1 INT, var2 INT)"
create_table_output = create.execute("CREATE TABLE tbl (var1 INT, var2 INT)")
AK47.AK_print_table("tbl")


def create_table_test():
    """
    >>> create_table_output
    'Table created'
    """


# in the Create_index_command class the execute method doesn't work properly, tokens are tested and they work okay
# need to check AK_create_index function and comments in execute method
index = Create_index_command()
index.expr = "CREATE INDEX Pindex ON student ( stupac1, stupac2 ) USING Btree"
index1_output = index.execute()
index.expr = "create index Pindex on student ( stupac1, stupac2 ) USING Hash"
index2_output = index.execute()


def create_index_text():
    """
    >>> index1_output
    'Index created'
    >>> index2_output
    'Index created'
    """


# in Create_trigger_command class its execute method doesn't work properly. Tokens are tested and they work okay
# need to check AK_trigger_add function - 3rd argument raises error: TypeError: in method 'AK_trigger_add', argument 3 of type 'AK_list *'
trigger = Create_trigger_command()
trigger.expr = "CREATE TRIGGER accept_conn AFTER INSERT ON conn FOR ROW EXECUTE PROCEDURE conn_accepted()"
trigger1_output = trigger.execute()
trigger.expr = "CREATE TRIGGER accept_conn BEFORE DELETE OR INSERT ON conn EXECUTE PROCEDURE conn_accepted()"
trigger2_output = trigger.execute()
trigger.expr = "CREATE TRIGGER accept_conn BEFORE DELETE OR INSERT ON conn FOR EACH STATEMENT EXECUTE PROCEDURE conn_accepted(2,3,'data')"
trigger3_output = trigger.execute()
trigger.expr = "CREATE TRIGGER accept_conn AFTER DELETE OR INSERT OR UPDATE ON conn FOR EACH STATEMENT EXECUTE PROCEDURE conn_accepted(2,10.5,'data')"
trigger4_output = trigger.execute()


def create_trigger_test():
    """
    >>> trigger1_output
    'Trigger created'
    >>> trigger2_output
    'Trigger created'
    >>> trigger3_output
    'Trigger created'
    >>> trigger4_output
    'Trigger created'
    """


# TODO: Test causes SIGSEGV
# drop = Drop_command()
# drop.expr = "DROP TABLE student"


# def drop_test():
#     """
#     >>> drop.execute() != False
#     True
#     """


'''
Need to connect these tokens with sql_executor:
AK_parse_create_user
AK_parse_CreateView -- change name in AK_parse_create_view because it's some kind of standard in sql_executor.py file

AK_alter_table  -- I can't see a valid reason whe this isn't named like AK_parse_alter_table because it can make some problems with the real AK_alter_table function
AK_parse_alter_user
AK_parse_alter_view
AK_parse_alter_index
AK_parse_alter_sequence
'''
