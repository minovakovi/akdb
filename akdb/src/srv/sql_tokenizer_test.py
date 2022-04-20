from sql_tokenizer import *


def get_attribute_tokens(tokens, attribute_name):
    return [token.get(attribute_name) for token in tokens]


sql_tokenizer = sql_tokenizer()


grant_commands = ["GRANT SELECT, INSERT, UPDATE, DELETE ON album, song TO Elvis, Jimmy WITH ADMIN OPTION",
                  "grant update on table1, table2 to Ivica, pero22foi1",
                  "Grant insert on drivers to Hamilton, Raikkonen, Alonso"]
grant_tokens = [sql_tokenizer.AK_parse_grant(cmd).asDict() for cmd in grant_commands]


def AK_parse_grant_test():
    """
    @author Boris Kisic
    @brief testing of sql parsing command GRANT
    @return No return value

    >>> get_attribute_tokens(grant_tokens, "privileges")
    [['SELECT', 'INSERT', 'UPDATE', 'DELETE'], ['update'], ['insert']]
    >>> get_attribute_tokens(grant_tokens, "tables")
    [['album', 'song'], ['table1', 'table2'], ['drivers']]
    >>> get_attribute_tokens(grant_tokens, "users")
    [['Elvis', 'Jimmy'], ['Ivica', 'pero22foi1'], ['Hamilton', 'Raikkonen', 'Alonso']]
    >>> get_attribute_tokens(grant_tokens, "grantOption")
    ['ADMIN OPTION', None, None]
    """


drop_commands = ["DROP temporary table if exists tabela1231 cascade",
                 "drop user matija, pero, jura",
                 "Drop column kolona1, kolona_2",
                 "drop index bla on tablica",
                 "drop trigger bla on tablica2",
                 "drop role test",
                 "drop function if exists funcija1",
                 "drop procedure if exists procedurea_df489f"]
drop_tokens = [sql_tokenizer.AK_parse_drop(cmd).asDict() for cmd in drop_commands]


def AK_parse_drop_test():
    """
    >>> get_attribute_tokens(drop_tokens, "opcija1")
    ['temporary', None, None, None, None, None, None, None]
    >>> get_attribute_tokens(drop_tokens, "objekt")
    ['table', 'user', 'column', 'index', 'trigger', 'role', 'function', 'procedure']
    >>> get_attribute_tokens(drop_tokens, "opcija2")
    [['if', 'exists'], None, None, None, None, None, ['if', 'exists'], ['if', 'exists']]
    >>> get_attribute_tokens(drop_tokens, "ime_objekta")
    [['tabela1231'], ['matija', 'pero', 'jura'], ['kolona1', 'kolona_2'], ['bla'], ['bla'], ['test'], ['funcija1'], ['procedurea_df489f']]
    >>> get_attribute_tokens(drop_tokens, "opcija3")
    ['cascade', None, None, None, None, None, None, None]
    >>> get_attribute_tokens(drop_tokens, "onToken")
    [None, None, None, 'on', 'on', None, None, None]
    >>> get_attribute_tokens(drop_tokens, "ime_objekta2")
    [None, None, None, ['tablica'], ['tablica2'], None, None, None]
    """


# IMPORTANT: last 2 commands don't work and because of that if you uncomment them, this test will crash. Better error handling in AK_alter_table() is needed.
alter_table_commands = ["alter table imena drop column srednje_ime",
                        "alter table icecream add flavor varchar(20)"]
                        # "alter table icecream add flavor",
                        # "alter table icecream drop flavor varchar(20)"]
alter_table_tokens = [sql_tokenizer.AK_alter_table(cmd).asDict() for cmd in alter_table_commands]


def AK_alter_table_test():
    """
    @author Krunoslav Bilic
    @brief testing of sql parsing command ALTER TABLE
    @return No return value
    >>> get_attribute_tokens(alter_table_tokens, "statement")
    ['alter', 'alter']

    # ['alter', 'alter', 'alter', 'alter']
    >>> get_attribute_tokens(alter_table_tokens, "operation")
    ['drop', 'add']

    # ['drop', 'add', 'add', 'drop']
    >>> get_attribute_tokens(alter_table_tokens, "table_name")
    ['imena', 'icecream']

    # ['imena', 'icecream', 'icecream', 'icecream']
    >>> get_attribute_tokens(alter_table_tokens, "column_name")
    ['srednje_ime', 'flavor']

    # ['srednje_ime', 'flavor', 'flavor', 'flavor']
    >>> get_attribute_tokens(alter_table_tokens, "data_type")
    [None, ['varchar', '20']]

    # [None, ['varchar', '20'], None, ['varchar', '20']]
    """


trans_commands = ["begin tijelo commit",
                  "begin tijelo rollback",
                  "begin work tijelo commit",
                  "begin transaction isolation level serializable tijelo rollback"]
trans_tokens = [sql_tokenizer.AK_parse_trans(cmd).asDict() for cmd in trans_commands]


def AK_trans_test():
    """
    @author Filip Sostarec
    @brief testing of transaction
    >>> get_attribute_tokens(trans_tokens, "tijelo")
    ['tijelo', 'tijelo', 'tijelo', 'tijelo']
    """


createIndex_commands = ["CREATE INDEX Pindex ON tablica ( stupac1, stupac2 ) USING Btree",
                        "create index Pindex on tablica ( stupac1 ) USING Btree",
                        "create index Pindex on tablica ( stupac1, stupac2 ) USING Hash"]
createIndex_tokens = [sql_tokenizer.AK_parse_createIndex(cmd).asDict() for cmd in createIndex_commands]


def AK_parse_createIndex_test():
    """
    @author Domagoj Tulicic
    @brief testing of sql parsing command CREATE INDEX
    @return No return value
    >>> get_attribute_tokens(createIndex_tokens, "IndexIme")
    ['Pindex', 'Pindex', 'Pindex']
    >>> get_attribute_tokens(createIndex_tokens, "tablica")
    ['tablica', 'tablica', 'tablica']
    >>> get_attribute_tokens(createIndex_tokens, "stupci")
    [['stupac1', 'stupac2'], ['stupac1'], ['stupac1', 'stupac2']]
    >>> get_attribute_tokens(createIndex_tokens, "IndexVrsta")
    ['Btree', 'Btree', 'Hash']
    """


create_sequence_commands = [
    "create sequence sequenca start with 1 increment by 2 minvalue 9 maxvalue 9999999  cache 10 CYCLE",
    "create sequence sequenca start with 1 increment by 2 minvalue 9 maxvalue 9999999  cache 10",
    "create sequence brojac_1 as smallint start with 10 increment by 1 minvalue 5",
    "create sequence brojac_2"]
create_sequence_tokens = [sql_tokenizer.AK_create_sequence(cmd).asDict() for cmd in create_sequence_commands]


def AK_create_sequence_test():
    """
    @author Domagoj Tulicic, Danko Sacer
    @brief testing of sql parsing command CREATE SEQUENCE
    @return No return value
    >>> get_attribute_tokens(create_sequence_tokens, "sekvenca")
    [['sequenca'], ['sequenca'], ['brojac_1'], ['brojac_2']]
    >>> get_attribute_tokens(create_sequence_tokens, "min_value")
    [['9'], ['9'], ['5'], 'no minvalue']
    >>> get_attribute_tokens(create_sequence_tokens, "max_value")
    [['9999999'], ['9999999'], 'no maxvalue', 'no maxvalue']
    >>> get_attribute_tokens(create_sequence_tokens, "increment_by")
    [['2'], ['2'], ['1'], '1']
    >>> get_attribute_tokens(create_sequence_tokens, "cache")
    [['10'], ['10'], '15', '15']
    >>> get_attribute_tokens(create_sequence_tokens, "cycle")
    ['cycle', 'no cycle', 'no cycle', 'no cycle']
    >>> get_attribute_tokens(create_sequence_tokens, "as_value")
    ['bigint', 'bigint', ['smallint'], 'bigint']
    >>> get_attribute_tokens(create_sequence_tokens, "start_with")
    [['1'], ['1'], ['10'], 'no start']
    """


where_commands = [
    "select * from t1 inner join t2 on t1.at1 = t2.at1 left outer join t3 on t2.at2 = t3.at2 where t3.at3 > 0 and t2.at1 in (select count(*) from t3) order by t1.at1 desc limit 100 offset 10",
    "select at1, avg(at2) from t1 cross join t2 on t1.at1 > t2.at1 group by at1 having avg(at2) > 100 order by at1 asc union select count(*) from t3 where t3 in (select distinct on(a) a, b, c from t4)",
    "update t1 set at1 = 1 where at2 = 2 and at3 = 3",
    "update t2 set at1 = 1, at2 = 2, at3 = 3 where at4 = 4",
    "delete from t1 using t2 where t1.at1 = t2.at2",
    "delete from t1 where t1.at1 in (select * from t2 where at2 > 0 and at3 < 0 or at4 = 0) or t1.at2 in (select * from at3)"
]
parsed_where_tokens = list(map(sql_tokenizer.AK_parse_where, where_commands))
where_tokens = [parsed_where_token.condition for parsed_where_token in parsed_where_tokens]


def AK_parse_where_test():
    """
    @author Kresimir Ivkovic
    @brief tests parsing of select and delete statements
    @return No return value

    >>> print where_tokens[0]
    ['WHERE', ['t3', '.', 'at3', '>', '0'], 'AND', ['t2', '.', 'at1', 'IN', ['SELECT', [['COUNT', '*']], 'FROM', 't3']]]
    >>> print where_tokens[1]
    ['WHERE', ['t3', 'IN', ['SELECT', ['DISTINCT ON', ['a']], ['a', 'b', 'c'], 'FROM', 't4']]]
    >>> print where_tokens[2]
    ['WHERE', ['at2', '=', '2'], 'AND', ['at3', '=', '3']]
    >>> print where_tokens[3]
    ['WHERE', ['at4', '=', '4']]
    >>> print where_tokens[4]
    ['WHERE', ['t1', '.', 'at1', '=', 't2', '.', 'at2']]
    >>> print where_tokens[5]
    ['WHERE', ['t1', '.', 'at1', 'IN', ['SELECT', ['*'], 'FROM', 't2', 'WHERE', ['at2', '>', '0'], 'AND', ['at3', '<', '0'], 'OR', ['at4', '=', '0']]], 'OR', ['t1', '.', 'at2', 'IN', ['SELECT', ['*'], 'FROM', 'at3']]]
    """


create_user_commands = ["CREATE USER test1 WITH PASSWORD tt", "CREATE USER user WITH PASSWORD 1pass1",
                        "CREATE USER ub1 IN GROUP grupa",
                        "CREATE USER mmw WITH PASSWORD pas1mac IN GROUP anim CREATEDB VALID UNTIL 2013-22-02"]
create_user_tokens = [sql_tokenizer.AK_parse_create_user(cmd).asDict() for cmd in create_user_commands]


def AK_parse_create_user_test():
    """
    @author Franjo Kovacic
    @brief tests parsing of CREATE USER statement
    @return No return value
    >>> get_attribute_tokens(create_user_tokens, "username")
    ['test1', 'user', 'ub1', 'mmw']
    >>> get_attribute_tokens(create_user_tokens, "password")
    ['tt', '1pass1', None, 'pas1mac']
    >>> get_attribute_tokens(create_user_tokens, "groupName")
    [None, None, 'grupa', 'anim']
    >>> get_attribute_tokens(create_user_tokens, "validUntil")
    [None, None, None, '2013-22-02']
    """


create_table_commands = ["CREATE TABLE tablica (var1 INT NOT NULL, var2 INT PRIMARY KEY)",
                         "CREATE TABLE tabla1 (v2 INT, v4 TEXT, v11 INT AUTO_INCREMENT)"]
create_table_tokens = [sql_tokenizer.AK_parse_create_table(cmd).asDict() for cmd in create_table_commands]


def AK_parse_create_table_test():
    """
    @author Franjo Kovacic
    @brief tests parsing of CREATE TABLE statement
    @return No return value
    >>> get_attribute_tokens(create_table_tokens, "attributes")
    [[['var1', 'int', 'NOT NULL'], ['var2', 'int', 'PRIMARY KEY']], [['v2', 'int'], ['v4', 'text'], ['v11', 'int', 'AUTO_INCREMENT']]]
    >>> get_attribute_tokens(create_table_tokens, "tableName")
    ['tablica', 'tabla1']
    """


insert_into_commands = ["INSERT INTO tablica(vr1, v2, ttt3) VALUES ('a1', 'ss2', 'a2')",
                        "INSERT INTO tablica1 VALUES (11, 'kk2', 'j2')"]
insert_into_tokens = [sql_tokenizer.AK_parse_insert_into(cmd).asDict() for cmd in insert_into_commands]


def AK_parse_insert_into_test():
    """
    @author Franjo Kovacic
    @brief tests parsing of INSERT INTO statement
    @return No return value
    >>> get_attribute_tokens(insert_into_tokens, "tableName")
    ['tablica', 'tablica1']
    >>> get_attribute_tokens(insert_into_tokens, "columns")
    [[['vr1', 'v2', 'ttt3']], None]
    >>> get_attribute_tokens(insert_into_tokens, "columnValues")
    [[["'a1'", "'ss2'", "'a2'"]], [['11', "'kk2'", "'j2'"]]]
    """


create_trigger_commands = [
    "CREATE TRIGGER prihvat_veze AFTER INSERT ON veza FOR ROW EXECUTE PROCEDURE veza_prihvacena()",
    "CREATE TRIGGER prihvat_veze BEFORE  DELETE OR INSERT ON veza EXECUTE PROCEDURE veza_prihvacena()",
    "CREATE TRIGGER prihvat_veze BEFORE DELETE OR INSERT ON veza FOR EACH STATEMENT EXECUTE PROCEDURE veza_prihvacena(2,3,'data')",
    "CREATE TRIGGER prihvat_veze AFTER DELETE OR INSERT OR UPDATE ON veza FOR EACH STATEMENT EXECUTE PROCEDURE veza_prihvacena(2,10.5,'data')"]
create_trigger_tokens = [sql_tokenizer.AK_parse_trigger(cmd).asDict() for cmd in create_trigger_commands]


def AK_create_trigger_test():
    """
    @author Davorin Vukelic
    @brief testing of sql parsing command AK_create_trigger_test
    @return No return value
    >>> get_attribute_tokens(create_trigger_tokens, "name")
    ['prihvat_veze', 'prihvat_veze', 'prihvat_veze', 'prihvat_veze']
    >>> get_attribute_tokens(create_trigger_tokens, "whenOption")
    ['after', 'before', 'before', 'after']
    >>> get_attribute_tokens(create_trigger_tokens, "EventOption1")
    ['insert', 'delete', 'delete', 'delete']
    >>> get_attribute_tokens(create_trigger_tokens, "EventOption2")
    [None, 'insert', 'insert', 'insert']
    >>> get_attribute_tokens(create_trigger_tokens, "EventOption3")
    [None, None, None, 'update']
    >>> get_attribute_tokens(create_trigger_tokens, "tableName")
    ['veza', 'veza', 'veza', 'veza']
    >>> get_attribute_tokens(create_trigger_tokens, "whatOption")
    ['row', None, 'statement', 'statement']
    >>> get_attribute_tokens(create_trigger_tokens, "functionName")
    ['veza_prihvacena', 'veza_prihvacena', 'veza_prihvacena', 'veza_prihvacena']
    >>> get_attribute_tokens(create_trigger_tokens, "params")
    [None, None, ['2', '3', "'data'"], ['2', '10.5', "'data'"]]
    """


# function AKTokenizeCreateFunction() (sql_tokenizer.py) currently doesn't work, throws TypeError: 'And' object is not iterable
# TODO: fix this error and implement doctest for this function

# old test function:
# def AKTokenizeCreateFunctionTest():
#     string1 = "CREATE FUNCTION myFunction(IN number INT DEFAULT 12) RETURNS TABLE(varijabla1 INT,  varijabla2 FLOAT) AS SELECT * FROM tablica"
#     print sql_tokenizer.AKTokenizeCreateFunction(string1, "INT|FLOAT|DECIMAL")


create_view_commands = ["CREATE VIEW TestView AS SELECT * FROM someTable",
                        "CREATE TEMP VIEW TestView AS SELECT * FROM someTable"]
create_view_tokens = [sql_tokenizer.AK_parse_CreateView(cmd).asDict() for cmd in create_view_commands]


def AK_parse_CreateView_test():
    """
    >>> get_attribute_tokens(create_view_tokens, "name")
    ['TestView', 'TestView']
    >>> get_attribute_tokens(create_view_tokens, "mode")
    [None, 'TEMP']
    >>> get_attribute_tokens(create_view_tokens, "query")
    ['SELECT * FROM someTable', 'SELECT * FROM someTable']
    """


alter_sequence_commands = ["ALTER SEQUENCE sequenca INCREMENT BY 2 MINVALUE 9 MAXVALUE 9999999  cache 10 CYCLE",
                           "ALTER sequence sequenca increment by 2 minvalue 9 maxvalue 9999999  cache 10",
                           "ALTER SEQUENCE serial RESTART WITH 105"]
alter_sequence_tokens = [sql_tokenizer.AK_parse_alter_sequence(cmd).asDict() for cmd in alter_sequence_commands]


def AK_alter_sequence_test():
    """
    @author Marinko Radic
    @brief testing of sql parsing command ALTER SEQUENCE
    @no return value
    >>> get_attribute_tokens(alter_sequence_tokens, "cache")
    ['10', '10', None]
    >>> get_attribute_tokens(alter_sequence_tokens, "increment")
    ['2', '2', None]
    >>> get_attribute_tokens(alter_sequence_tokens, "maxvalue")
    ['9999999', '9999999', None]
    >>> get_attribute_tokens(alter_sequence_tokens, "minvalue")
    ['9', '9', None]
    >>> get_attribute_tokens(alter_sequence_tokens, "sequenceName")
    ['sequenca', 'sequenca', 'serial']
    >>> get_attribute_tokens(alter_sequence_tokens, "restartValue")
    [None, None, '105']
    """


alter_view_commands = ["ALTER VIEW foo RENAME TO bar", "ALTER VIEW nek ALTER COLUMN mag SET DEFAULT neke"]
alter_view_tokens = [sql_tokenizer.AK_parse_alter_view(cmd).asDict() for cmd in alter_view_commands]


def AK_alter_view_test():
    """
    @author Marinko Radic
    @brief testing of sql parsing command ALTER VIEW
    @no return value
    >>> get_attribute_tokens(alter_view_tokens, "name")
    ['foo', 'nek']
    >>> get_attribute_tokens(alter_view_tokens, "newName")
    ['bar', None]
    >>> get_attribute_tokens(alter_view_tokens, "columnName")
    [None, 'mag']
    >>> get_attribute_tokens(alter_view_tokens, "expression")
    [None, 'neke']
    """


alter_index_commands = ["ALTER INDEX distributors RENAME TO suppliers"]
alter_index_tokens = [sql_tokenizer.AK_parse_alter_index(cmd).asDict() for cmd in alter_index_commands]


def AK_alter_index_test():
    """
     @author Marinko Radic
     @brief testing of sql parsing command ALTER INDEX
     @no return value
    >>> get_attribute_tokens(alter_index_tokens, "name")
    ['distributors']
    >>> get_attribute_tokens(alter_index_tokens, "newName")
    ['suppliers']
    """


alter_user_commands = ["ALTer USEr davide WITH PASSWORD hu8jmn3", "ALTER USER manuel VALID UNTIL 2013-22-02",
                       "ALTER USER miriam CREATEUSER CREATEDB", "ALTER USER marinac RENAME TO marinac666",
                       "ALTER USER dd VALID UNTIL 1", "ALTER USER marinac RENAME TO marinac666 "]
alter_user_tokens = [sql_tokenizer.AK_parse_alter_user(cmd).asDict() for cmd in alter_user_commands]


def AK_alter_user_test():
    """
    @author Marinko Radic
    @brief testing of sql parsing command ALTER USER
    @no return value
    >>> get_attribute_tokens(alter_user_tokens, "username")
    ['davide', 'manuel', 'miriam', 'marinac', 'dd', 'marinac']
    >>> get_attribute_tokens(alter_user_tokens, "password")
    ['hu8jmn3', None, None, None, None, None]
    >>> get_attribute_tokens(alter_user_tokens, "validUntil")
    [None, '2013-22-02', None, None, '1', None]
    >>> get_attribute_tokens(alter_user_tokens, "newname")
    [None, None, None, 'marinac666', None, 'marinac666']
    """

