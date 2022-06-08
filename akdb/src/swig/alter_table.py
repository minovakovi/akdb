import sys
from pyparsing import *

#ALTER TABLE table_name ADD column_name data_type
#ALTER TABLE table_name DROP COLUMN column_name
#ALTER TABLE table_name ALTER COLUMN column_name data_type
#ALTER TABLE table_name MODIFY column_name data_type

LPAR, RPAR, COMMA = list(map(Suppress,"(),"))
alter_stmt = Forward().setName("alter table")

#keywords
(ALTER, TABLE, ADD, DROP, MODIFY, COLUMN ) =  list(map(CaselessKeyword, """ALTER, TABLE, ADD, DROP, MODIFY, COLUMN""".replace(",","").split()))

keyword = MatchFirst((ALTER, TABLE, ADD, DROP, MODIFY, COLUMN )) 

identifier = ~keyword + Word(alphas, alphanums+"_")

table_name = identifier.copy()
column_name = identifier.copy()
data_type = identifier.copy()

alter_core = (ALTER + TABLE + table_name)
adding = (ADD + column_name + data_type) 
dropping = (DROP + COLUMN + column_name)
altering = (ALTER + COLUMN + column_name + data_type)
modifying = (MODIFY + column_name + data_type)

alter_stmt << (alter_core + Optional(adding | dropping | altering | modifying))

tests = """\
    alter table ime1 drop column stupac add stupac tip
    alter table ime2 add stupac tip
    alter table ime3 alter column stupac tip
    alter table ime4 modify stupac tip 
    alter table ime5 drop column stupac alter column stupac tip""".splitlines()

for t in tests:
    print(t)
    try:
        print(alter_stmt.parseString(t).dump())
    except ParseException as pe:
        print(pe.msg)
    print()
    
