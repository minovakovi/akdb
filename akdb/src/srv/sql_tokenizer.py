# @file sql_tokenizer.py - Parsing commands

from pyparsing import *
import re


class sql_tokenizer:

    def AK_parse_grant(self, string):
        '''
        @author Boris Kisic, edited by Zvonimir Kapes
        @brief sql parsing of GRANT command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''
        ident = Word(alphas, alphanums + "_$").setName("identifier")
        grantToken = Keyword("grant", caseless=True)
        onToken = Keyword("on", caseless=True)
        toToken = Keyword("to", caseless=True)
        withToken = Keyword("with", caseless=True)
        privilege = delimitedList(ident, ",", combine=True)
        privilegeList = Group(delimitedList(privilege))
        table = delimitedList(ident, ",", combine=True)
        tableList = Group(delimitedList(table))
        userName = delimitedList(ident, ",", combine=True)
        userNameList = Group(delimitedList(userName))
        groupToken = Keyword("group", caseless=True).setResultsName("group")

        grantStmt = Forward()
        grantStmt << (grantToken + privilegeList.setResultsName("privileges") +
                      onToken + tableList.setResultsName("tables") +
                      toToken + Optional(groupToken) + userNameList.setResultsName("users") +
                      Optional(withToken + restOfLine.setResultsName("grantOption")))

        try:
            tokens = grantStmt.parseString(string)

        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        return tokens

    def AK_parse_drop(self, string):
        '''
        @autor mparavac
        @brief sql parsing of DROP command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''
        ident = Word(alphas, alphanums + "_$").setName("identifier")
        dropToken = Keyword("drop", caseless=True)
        objectToken = (Keyword("table", caseless=True) | Keyword("user", caseless=True) | Keyword("column", caseless=True) |
                       Keyword("index", caseless=True) | Keyword("sequence", caseless=True) |
                       Keyword("function", caseless=True) | Keyword("procedure", caseless=True) |
                       Keyword("schema", caseless=True) | Keyword(
            "trigger", caseless=True) | Keyword("role", caseless=True)
        )
        onToken = Keyword("on", caseless=True)
        optionalToken1 = Keyword("temporary", caseless=True)
        optionalToken2 = Keyword("if", caseless=True) + \
            Keyword("exists", caseless=True)
        optionalToken3 = Keyword("cascade", caseless=True) | Keyword(
            "restrict", caseless=True)
        objectName = delimitedList(ident, ",", combine=True)
        objectNameList = Group(delimitedList(objectName))
        objectName2 = delimitedList(ident, ",", combine=True).setParseAction(upcaseTokens)
        objectNameList2 = Group(delimitedList(objectName))
        optionalName = delimitedList(ident, ",", combine=True).setParseAction(upcaseTokens)
        optionalNameList = Group(delimitedList(objectName))
        sequenceName = delimitedList(ident, ",", combine=True).setParseAction(upcaseTokens)
        sequenceNameList = Group(delimitedList(sequenceName))

        dropStmt = Forward()
        dropStmt << (dropToken + Optional(optionalToken1.setResultsName("opcija1")) + objectToken.setResultsName("objekt") +
                     Optional(optionalToken2.setResultsName("opcija2")) + objectNameList.setResultsName("ime_objekta") +
                     Optional(optionalToken3.setResultsName("opcija3")) + Optional(onToken.setResultsName("onToken") +
                                                                                   Optional(objectNameList2.setResultsName("ime_objekta2")))

                     )
        try:
            tokens = dropStmt.parseString(string)

        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_alter_table(self, string):
        '''
        @autor Krunoslav Bilic
        @brief sql parsing of ALTER TABLE command (ref. http://www.w3schools.com/sql/sql_alter.asp )
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''
        LPAR, RPAR, COMMA = list(map(Suppress, "(),"))

        # keywords
        alterToken = Keyword(
            "alter", caseless=True).setResultsName("statement")
        addToken = Keyword("add", caseless=True).setResultsName("operation")
        dropToken = Keyword("drop", caseless=True).setResultsName("operation")
        tableToken = Keyword("table", caseless=True)
        columnToken = Keyword("column", caseless=True)

        # definiranje gramatike
        slozeni = LPAR + ZeroOrMore(CharsNotIn(")")) + RPAR
        jednostavni = Word(alphas, alphanums+"_\"':-")

        # dohvacanje naziva tablice i stupca
        table_name = jednostavni.copy().setResultsName("table_name")
        column_name = jednostavni.copy().setResultsName("column_name")
        # definiranje tipa podataka iz add naredbe
        field_def = OneOrMore(jednostavni | slozeni)
        field_list_def = delimitedList(field_def).setResultsName("data_type")

        # definiranje osnove upita te operacija dodavanja i uklanjanja stupca tablice
        alter_core = (alterToken + tableToken + table_name)
        adding = (addToken + column_name + field_list_def)
        dropping = (dropToken + columnToken + column_name)

        # definiranje same naredbe
        alter_stmt = Forward()
        alter_def = (dropping) | (adding)
        alter_stmt << (alter_core + alter_def)

        try:
            tokens = alter_stmt.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_parse_createIndex(self, string):
        '''
        @autor Domagoj Tulicic 
        @brief sql parsing of CREATE INDEX command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string
        '''

        ident = Word(alphas, alphanums + "_$").setName("identifier")
        nazivIndexa = Word(alphas, alphanums + "_$").setName("identifier")
        nazivTablice = Word(alphas, alphanums + "_$").setName("identifier")
        createToken = Keyword("create", caseless=True)
        indexToken = Keyword("index", caseless=True)
        onToken = Keyword("on", caseless=True)
        usingToken = Keyword("using", caseless=True)
        column = delimitedList(ident, ",", combine=True)
        columnList = Group(delimitedList(column))
        lzagrada = Suppress("(")
        dzagrada = Suppress(")")

        createIndexStmt = Forward()
        createIndexStmt << (createToken + indexToken + nazivIndexa.setResultsName("IndexIme") + onToken +
                            nazivTablice.setResultsName("tablica") + lzagrada + columnList.setResultsName("stupci") + dzagrada +
                            usingToken + restOfLine.setResultsName("IndexVrsta"))
        try:
            tokens = createIndexStmt.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_create_sequence(self, string):
        '''
        @autor Domagoj Tulicic modified by Danko Sacer
        @brief sql parsing of CREATE SEQUENCE command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string
        '''

        LPAR, RPAR, COMMA = list(map(Suppress, "(),"))
        (CREATE, SEQUENCE, AS, START, WITH, INCREMENT, BY, MINVALUE, MAXVALUE,  CACHE) = list(map(CaselessKeyword,
                                                                                                  """CREATE, SEQUENCE, AS, START, WITH, INCREMENT, BY, MINVALUE, MAXVALUE, CACHE""".replace(",", "").split()))

        keyword = MatchFirst((CREATE, SEQUENCE, AS, START,
                             WITH, INCREMENT, BY, MINVALUE, MAXVALUE, CACHE))
        cycleToken = Keyword("cycle", caseless=True).setResultsName("cycle")

        identifier = ~keyword + Word(alphas, alphanums+"_")
        identifier2 = ~keyword + Word(nums)

        sequence_name = identifier.copy().setResultsName("sekvenca")
        as_value = identifier.copy().setResultsName("as_value")
        min_value = identifier2.copy().setResultsName("min_value")
        max_value = identifier2.copy().setResultsName("max_value")
        start_with = identifier2.copy().setResultsName("start_with")
        increment_by = identifier2.copy().setResultsName("increment_by")
        cache_value = identifier2.copy().setResultsName("cache")

        sequence_stmt = Forward()
        sequence_stmt << (CREATE + SEQUENCE + sequence_name +
                          (Optional((AS), default=AS) + Optional((as_value), default="bigint")) +
                          (Optional((START), default=START) + Optional((WITH), default=WITH) +
                              Optional((start_with), default="no start")) +
                          (Optional((INCREMENT), default=INCREMENT) + Optional((BY), default=BY) +
                              Optional((increment_by), default="1")) +
                          (Optional((MINVALUE), default=MINVALUE) +
                              Optional((min_value), default="no minvalue")) +
                          (Optional((MAXVALUE), default=MAXVALUE) +
                              Optional((max_value), default="no maxvalue")) +
                          (Optional((CACHE), default=CACHE) +
                              Optional((cache_value), default="15")) +
                          Optional((cycleToken), default="no cycle"))

        try:
            tokens = sequence_stmt.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()

        if(tokens.cycle == "cycle"):
            tokens.cycle = 1
        else:
            tokens.cycle = 0
        # definiranje min, max i start default vrijednosti na temelju tipa sequence
        if(tokens.as_value[0] == "smallint"):
            if(tokens.min_value == "no minvalue"):
                tokens.min_value = "-32768"
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value
                else:
                    tokens.start_with = tokens.start_with[0]
            else:
                tokens.min_value = tokens.min_value[0]
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value[0]
                else:
                    tokens.start_with = tokens.start_with[0]
            if(tokens.max_value == "no maxvalue"):
                tokens.max_value = "32767"
            else:
                tokens.max_value = tokens.max_value[0]

        elif(tokens.as_value[0] == "int"):
            if(tokens.min_value == "no minvalue"):
                tokens.min_value = "-2147483648"
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value
                else:
                    tokens.start_with = tokens.start_with[0]
            else:
                tokens.min_value = tokens.min_value[0]
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value[0]
                else:
                    tokens.start_with = tokens.start_with[0]
            if(tokens.max_value == "no maxvalue"):
                tokens.max_value = "2147483647"
            else:
                tokens.max_value = tokens.max_value[0]

        elif(tokens.as_value[0] == "bigint" or tokens.as_value == "bigint"):
            if(tokens.min_value == "no minvalue"):
                tokens.min_value = "-9223372036854775808"
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value
                else:
                    tokens.start_with = tokens.start_with[0]
            else:
                tokens.min_value = tokens.min_value[0]
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value[0]
                else:
                    tokens.start_with = tokens.start_with[0]
            if(tokens.max_value == "no maxvalue"):
                tokens.max_value = "9223372036854775807"
            else:
                tokens.max_value = tokens.max_value[0]

        elif(tokens.as_value[0] == "tinyint" or tokens.as_value[0] == "numeric" or tokens.as_value[0] == "decimal"):
            if(tokens.min_value == "no minvalue"):
                tokens.min_value = "0"
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value
                else:
                    tokens.start_with = tokens.start_with[0]
            else:
                tokens.min_value = tokens.min_value[0]
                if(tokens.start_with == "no start"):
                    tokens.start_with = tokens.min_value[0]
                else:
                    tokens.start_with = tokens.start_with[0]
            if(tokens.max_value == "no maxvalue"):
                tokens.max_value = "255"
            else:
                tokens.max_value = tokens.max_value[0]

        if(tokens.cache != "15"):
            tokens.cache = tokens.cache[0]
        if(tokens.increment_by != "1"):
            tokens.increment_by = tokens.increment_by[0]
        if(tokens.as_value != "bigint"):
            tokens.as_value = tokens.as_value[0]

        return tokens

    def AK_parse_where(self, string):
        '''
        @author Kresimir Ivkovic, updated by Davor Tomala
        @brief parser for select, update, delete and any function containing the "WHERE" clause (automatically detects statement type)
        @param string: sql query as string
        @return tokenized query
        '''
        tokens = None

        # literals
        deleteLit = CaselessKeyword("DELETE")
        selectLit = CaselessKeyword("SELECT")
        updateLit = CaselessKeyword("UPDATE")
        fromLit = CaselessKeyword("FROM")
        usingLit = CaselessKeyword("USING")
        whereLit = CaselessKeyword("WHERE")
        setLit = CaselessKeyword("SET")
        lBracket = Suppress(Literal("("))
        rBracket = Suppress(Literal(")"))
        valuesLiteral = Suppress(CaselessLiteral("VALUES"))
        comma = Suppress(Literal(","))

        # identifier
        identifier = Word(alphas, alphanums + "_$")
        tableName = identifier
        userName = Word(alphas, alphanums)

        # values
        E = CaselessLiteral("e")
        arithSign = Word("+-", exact=1)
        realNum = Combine(Optional(arithSign)+(Word(nums)+"."+Optional(Word(nums))
                          | ("."+Word(nums)))+Optional(E+Optional(arithSign)+Word(nums)))
        intNum = Combine(Optional(arithSign)+Word(nums) +
                         Optional(E+Optional("+")+Word(nums)))
        value = quotedString | realNum | intNum
        valuesList = Group(delimitedList(value))
        values = lBracket+valuesList+rBracket

        # columns
        columnName = tableName+'.'+identifier | identifier
        colAs = columnName + CaselessKeyword("AS") + identifier
        aggrfn = Group((CaselessKeyword("AVG") | CaselessKeyword("SUM") | CaselessKeyword(
            "COUNT") | CaselessKeyword("MIN") | CaselessKeyword("MAX")) + lBracket + ('*' | columnName) + rBracket)
        # select stmt can contain aggr. functions in their att list, so a special columnName is needed for select
        columnNameSelect = Group(colAs) | aggrfn | (
            tableName+'.'+identifier) | identifier
        columnNameList = Group(delimitedList(columnName))
        # select can contain an asterisk for "all attributes" instead of att names
        columnNameListSelect = Group(delimitedList(columnNameSelect) | '*')
        columns = lBracket+columnNameList+rBracket

        # data types
        dataSize = lBracket+intNum+rBracket
        floatType = CaselessKeyword("float")
        integerType = CaselessKeyword("integer")
        varcharType = CaselessKeyword("varchar")+dataSize

        # predicate(limited)
        binrelop = oneOf(["<", "<=", ">=", ">", "=", "!="])
        sqlIN = CaselessKeyword("IN")
        sqlBetween = CaselessKeyword("BETWEEN")
        sqlLike = CaselessKeyword("LIKE")
        sqlOR = CaselessKeyword("OR")
        sqlAND = CaselessKeyword("AND")
        sqlUnion = CaselessKeyword("UNION")+Optional(CaselessKeyword("ALL"))
        sqlIntersect = CaselessKeyword(
            "INTERSECT")+Optional(CaselessKeyword("ALL"))
        sqlExcept = CaselessKeyword("EXCEPT")+Optional(CaselessKeyword("ALL"))
        predicate = Forward()
        predicate << columnName+binrelop+value + \
            ZeroOrMore((sqlOR | sqlAND | sqlIN |
                       sqlBetween | sqlLike)+predicate)

        selectInner = Forward()
        expression = Forward()
        where = Forward()

        expression << Group(
            (columnName.setResultsName("expLval") + binrelop.setResultsName("whereOp") + columnName.setResultsName("expRval")) |
            (aggrfn.setResultsName("expLval") + binrelop.setResultsName("whereOp") + columnName.setResultsName("expRval")) |
            (columnName.setResultsName("expLval") + binrelop.setResultsName("whereOp") + value.setResultsName("expRval")) |
            (aggrfn.setResultsName("expLval") + binrelop.setResultsName("whereOp") + value.setResultsName("expRval")) |
            (columnName.setResultsName("expLval") + sqlIN.setResultsName("whereOp") + values.setResultsName("expRval")) |
            (columnName.setResultsName("expLval") + sqlIN.setResultsName("whereOp") +
             Group(selectInner.setResultsName("expRval")))
        ) + ZeroOrMore((sqlOR | sqlAND) + expression)

        where << whereLit.setResultsName(
            "where")+expression.setResultsName("expression")

        select = Forward()
        join = Forward()
        distinct = CaselessKeyword("DISTINCT ON") + \
            lBracket+columnNameList+rBracket
        limit = CaselessKeyword("LIMIT")+intNum.setResultsName("limitVal")
        offset = CaselessKeyword("OFFSET")+intNum.setResultsName("offsetVal")
        groupBy = CaselessKeyword("GROUP BY")+columnNameList.setResultsName(
            "groupByCol")+Optional(CaselessKeyword("HAVING")+expression)
        orderBy = CaselessKeyword("ORDER BY")+columnNameList + \
            Optional(CaselessKeyword("ASC") | CaselessKeyword("DESC"))

        joinStatement = Optional(
            CaselessKeyword("INNER") |
            CaselessKeyword("CROSS") |
            (
                CaselessKeyword("LEFT") |
                CaselessKeyword("RIGHT") |
                CaselessKeyword("FULL")
            )+Optional(CaselessKeyword("OUTER"))
        )+CaselessKeyword("JOIN")+columnName.setResultsName("joinCol")+CaselessKeyword("ON")+expression

        join << joinStatement + ZeroOrMore(joinStatement)

        selectStatement = selectLit.setResultsName("commandName") +\
            Optional(Group(distinct.setResultsName("distinct"))) +\
            columnNameListSelect.setResultsName("attributes") +\
            fromLit.setResultsName("from") +\
            tableName.setResultsName("tableName") +\
            Optional(Group(join.setResultsName("join"))) +\
            Optional(where.setResultsName("condition")) +\
            Optional(Group(groupBy.setResultsName("group"))) +\
            Optional(Group(orderBy.setResultsName("order"))) +\
            Optional(Group(limit.setResultsName("limit"))) +\
            Optional(Group(offset.setResultsName("offset")))

        select << selectStatement + \
            ZeroOrMore((sqlUnion | sqlIntersect | sqlExcept) +
                       select) + stringEnd
        selectInner << lBracket+selectStatement+rBracket + \
            ZeroOrMore((sqlUnion | sqlIntersect | sqlExcept) + selectInner)

        deleteFrom = Forward()
        deleteFrom << deleteLit.setResultsName("commandName") +\
            fromLit.setResultsName("from") +\
            tableName.setResultsName("tableName") +\
            Optional(usingLit.setResultsName("using")+tableName.setResultsName("usingTable")) +\
            Optional(where.setResultsName("condition"))+stringEnd

        update = Forward()
        updateStatement = updateLit.setResultsName("commandName") +\
            tableName.setResultsName("tableName") +\
            setLit.setResultsName("set") +\
            columnNameList.setResultsName("columnNames") +\
            binrelop.setResultsName("operator") +\
            valuesList.setResultsName("columnValues")
        updatePair = Literal(",") + columnNameList.setResultsName("columnNames") + \
            binrelop.setResultsName("operator") + \
            valuesList.setResultsName("columnValues")

        update << updateStatement + \
            ZeroOrMore(updatePair) + \
            Optional(where.setResultsName("condition")) + stringEnd

        sqlGrammar = select | deleteFrom | update

        try:
            tokens = sqlGrammar.parseString(string)
        except ParseException as err:
            return "\n\t"+"Syntax error at char "+str(err.loc)+" "+err.msg+":\n" +\
                '\t"'+string+'"' +\
                '\n\t-'+''.join(['-' for i in range(0, err.loc)])+'^'+'\n'
        return tokens

    def AK_parse_create_user(self, string):
        '''
        @author Franjo Kovacic
        @brief sql parsing of CREATE USER command
        @param string sql command as string
        @return if command is successfully parsed returns list od tokens, else returns error message as string
        '''

        createUserToken = Keyword("CREATE USER", caseless=True)
        setPasswordToken = Keyword("WITH PASSWORD", caseless=True)
        createDBToken = Keyword("CREATEDB", caseless=True)
        createUsToken = Keyword("CREATEUSER", caseless=True)
        inGroupToken = Keyword("IN GROUP", caseless=True)
        validUntilToken = Keyword("VALID UNTIL", caseless=True)

        tokens = Word(alphanums+"_:.-")
        userName = tokens.copy().setResultsName("username")
        password = tokens.copy().setResultsName("password")
        groupName = tokens.setResultsName("groupName")
        validUntil = tokens.setResultsName("validUntil")

        constraints = ZeroOrMore(setPasswordToken + password.setResultsName("password") | createDBToken | createUsToken.setResultsName(
            "createUser") | inGroupToken + groupName.setResultsName("groupName") | validUntilToken + validUntil.setResultsName("validUntil"))

        createUserStmt = createUserToken.setResultsName("commandName") +\
            userName.setResultsName("username") +\
            Optional(constraints) +\
            stringEnd

        try:
            tokens = createUserStmt.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_parse_create_table(self, string):
        '''
        @author Franjo Kovacic
        @brief sql parsing of CREATE TABLE command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string
        '''

        createTableToken = Keyword("CREATE TABLE", caseless=True)
        notNullToken = Keyword("NOT NULL", caseless=True)
        uniqueToken = Keyword("UNIQUE", caseless=True)
        primaryKeyToken = Keyword("PRIMARY KEY", caseless=True)
        defaultToken = Keyword("DEFAULT", caseless=True)
        foreignKeyToken = Keyword("FOREIGN KEY", caseless=True)
        referencesToken = Keyword("REFERENCES", caseless=True)
        autoIncrementToken = Keyword("AUTO_INCREMENT", caseless=True)
        checkToken = Keyword("CHECK", caseless=True)
        lBracket = Suppress(Literal("("))
        rBracket = Suppress(Literal(")"))
        comma = Suppress(Literal(","))

        identifier = Word(alphas, alphanums + "_$")
        tableName = identifier
        columnName = identifier

        columnNameList = Group(delimitedList(columnName))
        columns = lBracket+columnNameList+rBracket

        # values
        E = CaselessLiteral("e")
        arithSign = Word("+-", exact=1)
        realNum = Combine(Optional(arithSign)+(Word(nums)+"."+Optional(Word(nums))
                          | ("."+Word(nums)))+Optional(E+Optional(arithSign)+Word(nums)))
        intNum = Combine(Optional(arithSign)+Word(nums) +
                         Optional(E+Optional("+")+Word(nums)))
        value = quotedString | realNum | intNum
        valuesList = Group(delimitedList(value))
        values = lBracket+valuesList+rBracket

        # types
        dataSize = lBracket+intNum+rBracket
        floatType = CaselessKeyword("float")
        integerType = CaselessKeyword("int")
        varcharType = CaselessKeyword("varchar")+dataSize
        textType = CaselessKeyword("text")

        # predicate(limited)
        binrelop = oneOf(["<", "<=", ">=", ">", "=", "!="])
        sqlOR = CaselessKeyword("OR")
        sqlAND = CaselessKeyword("AND")
        predicate = Forward()
        predicate << columnName+binrelop+value + \
            ZeroOrMore((sqlOR | sqlAND)+predicate)

        # attribute constraint
        defaultConstraint = defaultToken + value
        foreignKeyConstraint = foreignKeyToken + referencesToken + tableName + columns
        checkConstraint = Group(CaselessKeyword("CHECK") + predicate)
        constraint = notNullToken | uniqueToken | primaryKeyToken | checkConstraint | autoIncrementToken
        constraints = ZeroOrMore(constraint)

        # constraint at the end
        pkUnique = uniqueToken | primaryKeyToken
        pkUniqueEConstraint = pkUnique + columns
        foreignKeyEConstraint = foreignKeyToken+columnName + \
            referencesToken+tableName+lBracket+columnName+rBracket
        checkEConstraint = "ERROR"
        endConstraint = Group(CaselessKeyword("CONSTRAINT")+identifier +
                              (pkUniqueEConstraint | checkEConstraint | foreignKeyEConstraint))

        endConstraints = Group(delimitedList(endConstraint))

        # attributes (create table)
        attributeType = floatType | integerType | varcharType | textType
        attributeDefinition = Group(identifier+attributeType+constraints)
        attributeDefinitionList = Group(delimitedList(attributeDefinition))
        attributes = attributeDefinitionList

        createTableStmt = createTableToken.setResultsName("commandName") +\
            tableName.setResultsName("tableName") +\
            lBracket +\
            attributes.setResultsName("attributes") +\
            Optional(comma+endConstraints).setResultsName("endConstraints") +\
            rBracket+stringEnd

        try:
            tokens = createTableStmt.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_parse_insert_into(self, string):
        '''
        @author Franjo Kovacic
        @brief sql parsing of INSERT INTO command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string
        '''

        insertIntoToken = Keyword("INSERT INTO", caseless=True)
        valuesToken = Keyword("VALUES", caseless=True)
        lBracket = Suppress(Literal("("))
        rBracket = Suppress(Literal(")"))
        comma = Suppress(Literal(","))

        identifier = Word(alphas, alphanums + "_$")
        tableName = identifier

        columnName = identifier
        columnNameList = Group(delimitedList(columnName))
        columns = lBracket+columnNameList+rBracket

        # values
        E = CaselessLiteral("e")
        arithSign = Word("+-", exact=1)
        realNum = Combine(Optional(arithSign)+(Word(nums)+"."+Optional(Word(nums))
                          | ("."+Word(nums)))+Optional(E+Optional(arithSign)+Word(nums)))
        intNum = Combine(Optional(arithSign)+Word(nums) +
                         Optional(E+Optional("+")+Word(nums)))
        value = quotedString | realNum | intNum
        valuesList = Group(delimitedList(value))
        values = lBracket+valuesList+rBracket

        insertStmt = insertIntoToken.setResultsName("commandName") +\
            tableName.setResultsName("tableName") +\
            Optional(columns).setResultsName("columns") +\
            valuesToken +\
            values.setResultsName("columnValues")+stringEnd

        try:
            tokens = insertStmt.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        return tokens

    def AK_parse_trigger(self, string):
        '''
        @author Davorin Vukelic
        @brief sql parsing of TRIGGER command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''

        createToken = Keyword("create trigger", caseless=True)
        whenToken = (Keyword("after", caseless=True) |
                     Keyword("before", caseless=True))
        actionToken = (Keyword("delete", caseless=True) | Keyword(
            "insert", caseless=True) | Keyword("update", caseless=True))
        onToken = Keyword("on", caseless=True)
        forToken = Keyword("for", caseless=True)
        eachToken = Keyword("each", caseless=True)
        whatToken = (Keyword("row", caseless=True) |
                     Keyword("statement", caseless=True))
        executeToken = Keyword("each", caseless=True)
        procedureToken = Keyword("execute procedure", caseless=True)
        orToken = Keyword("or", caseless=True)
        eventToken = (Keyword("insert", caseless=True) | Keyword(
            "update", caseless=True) | Keyword("delete", caseless=True))

        tokens = Word(alphas, alphanums + "_$")

        trigName = tokens.copy().setResultsName("name")
        event = tokens.copy().setResultsName("event")
        table = tokens.copy().setResultsName("tableName")
        function = tokens.copy().setResultsName("functionName")
        ident = Word(nums, alphas)
        param = delimitedList(ident, ",", combine=True)
        paramList = Group(delimitedList(param))
        lBracket = Suppress("(")
        rBracket = Suppress(")")

        # params
        E = CaselessLiteral("e")
        arithSign = Word("+-", exact=1)
        realNum = Combine(Optional(arithSign)+(Word(nums)+"."+Optional(Word(nums))
                          | ("."+Word(nums)))+Optional(E+Optional(arithSign)+Word(nums)))
        intNum = Combine(Optional(arithSign)+Word(nums) +
                         Optional(E+Optional("+")+Word(nums)))
        value = quotedString | realNum | intNum
        valuesList = Group(delimitedList(value))

        triggerStmt = Forward()
        triggerStmt << (createToken+trigName+whenToken.setResultsName("whenOption") +
                        eventToken.setResultsName("EventOption1")+Optional(orToken +
                                                                           eventToken.setResultsName("EventOption2")+Optional(orToken +
                                                                                                                              eventToken.setResultsName("EventOption3")))+onToken+table +
                        Optional(forToken+Optional(eachToken)+whatToken.setResultsName("whatOption")) +
                        procedureToken+function+lBracket+Optional(valuesList.setResultsName("params"))+rBracket)

        try:
            tokens = triggerStmt.parseString(string)

        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_parse_trans(self, string):
        '''
        @author Filip Sostarec
        @brief sql parsing of BEGIN TRANS command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''

        beginToken = Keyword("begin", caseless=True)
        transToken = (Keyword("work", caseless=True) |
                      Keyword("transaction", caseless=True))
        isolationToken = Keyword("isolation level", caseless=True)
        transmodeToken = (Keyword("serializable", caseless=True) | Keyword("repeatable read", caseless=True) |
                          Keyword("read commited", caseless=True) | Keyword("read uncommited", caseless=True))
        commitToken = (Keyword("commit", caseless=True) |
                       Keyword("rollback", caseless=True))

        tokens = Word(alphas, alphanums + "_$")

        tijelo = tokens.copy().setResultsName("tijelo")

        transStmt = Forward()
        transStmt << (beginToken+Optional(transToken) +
                      Optional(isolationToken+transmodeToken)+tijelo+commitToken)

        try:
            tokens = transStmt.parseString(string)

        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AKTokenizeCreateFunction(self, sql, types):

        typesPattern = Regex(types, re.IGNORECASE)
        createFunction = CaselessKeyword(
            "CREATE") + CaselessKeyword("FUNCTION")
        functionName = Word(alphanums + "_") + Literal("(")
        argumentMode = Optional(Regex("IN|OUT", re.IGNORECASE))
        argumentNameAndOrType = Optional(Regex("[A-Za-z0-9]+")) + typesPattern
        defaultArgument = Optional(
            And(Regex("DEFAULT|=", re.IGNORECASE) + Word(alphanums)))
        argument = argumentMode+argumentNameAndOrType+defaultArgument
        functionArguments = Optional(
            delimitedList(argument, ",")) + Literal(")")
        returnTableAttributes = Literal(
            "(") + delimitedList(Word(alphanums) + typesPattern, ",") + Literal(")")
        returnVariant1 = And(CaselessKeyword("RETURNS") + typesPattern)
        returnVariant2 = And(CaselessKeyword("RETURNS") +
                             CaselessKeyword("TABLE") + returnTableAttributes)
        returnType = Or(returnVariant1 + returnVariant2)
        language = Optional(CaselessKeyword("LANGUAGE") +
                            Regex("SQL|PLSQL|INTERNAL|C", re.IGNORECASE))
        functionCode = CaselessKeyword("AS") + Regex(".*")

        wholeSyntax = createFunction + functionName + argumentMode + \
            functionArguments + returnType + functionCode
        parsedCommand = wholeSyntax.parseString(sql)

        return parsedCommand

    # Author: Tomislav Dukaric
      # Parameter "sql" is the definition query to create a function
      # to create a view object and store it into the database
      # The method returns a list of keywords used inside the query

    def AK_parse_CreateView(self, sql):

        create = CaselessKeyword("CREATE")
        mode = Optional(Regex("TEMP|TEMPORARY", re.IGNORECASE))
        view = CaselessKeyword("VIEW")
        viewName = Word(alphanums + "_")
        as_key = CaselessKeyword("AS")
        query = Regex(".*")

        wholeSyntax = create + mode.setResultsName("mode") + view + viewName.setResultsName(
            "name") + as_key + query.setResultsName("query")

        try:
            parsedCommand = wholeSyntax.parseString(sql)

        except ParseException as err:
            return ""*err.loc + "^\n" + err.msg
        print()
        return parsedCommand

    def AK_parse_alter_user(self, string):
        '''
        @author Marinko Radic
        @brief sql parsing of ALTER USER command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
          '''
        alterUserToken = Keyword("ALTER USER", caseless=True)
        passwordToken = Keyword("WITH PASSWORD", caseless=True)
        createDB = Keyword("CREATEDB", caseless=True)
        createUshortToken = Keyword("CREATEUSER", caseless=True)
        validToken = Keyword("VALID UNTIL", caseless=True)
        renameToToken = Keyword("RENAME TO", caseless=True)

        tokens = Word(alphanums+"_:.-")

        user = tokens.copy().setResultsName("username")
        password = tokens.copy().setResultsName("password")
        valid = tokens.copy().setResultsName("validUntil")
        newName = tokens.copy().setResultsName("newname")

        constraints = passwordToken + password.setResultsName("password") | createUshortToken + createDB | validToken + valid.setResultsName(
            "validUntil") | renameToToken + newName.setResultsName("newname")

        alterUserCmd = alterUserToken.setResultsName(
            "commandName") + user.setResultsName("username") + Optional(constraints)

        try:
            tokens = alterUserCmd.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_parse_alter_view(self, string):
        '''
        @author Marinko Radic
        @brief sql parsing of ALTER VIEW command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''
        alterViewToken = Keyword("ALTER VIEW", caseless=True)
        setShemaToken = Keyword("SET SHEMA", caseless=True)
        renameToToken = Keyword("RENAME TO", caseless=True)
        ownerToToken = Keyword("OWNER TO", caseless=True)
        setDefaultToken = Keyword("SET DEFAULT", caseless=True)
        dropDefaultToken = Keyword("DROP DEFAULT", caseless=True)
        alterColumnToken = Keyword("ALTER COLUMN", caseless=True)
        tokens = Word(alphanums+"_:.-")

        name = tokens.copy().setResultsName("name")
        columnName = tokens.copy().setResultsName("columnName")
        expression = tokens.copy().setResultsName("expression")
        newName = tokens.copy().setResultsName("newName")
        alterViewCmd = alterViewToken + name.setResultsName("name") + alterColumnToken + columnName.setResultsName("columnName") + setDefaultToken + expression.setResultsName("expression") | alterViewToken + name.setResultsName("name") + alterColumnToken + columnName.setResultsName(
            "columnName") + dropDefaultToken | alterViewToken + name.setResultsName("name") + ownerToToken + newName.setResultsName("newName") | alterViewToken + name.setResultsName("name") + setShemaToken + newName.setResultsName("newName") | alterViewToken + name.setResultsName("name") + renameToToken + newName.setResultsName("newName")

        try:
            tokens = alterViewCmd.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_parse_alter_index(self, string):
        '''
        @author Marinko Radic
        @brief sql parsing of ALTER INDEX command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''
        alterIndexToken = Keyword("ALTER INDEX", caseless=True)
        renameToToken = Keyword("RENAME TO", caseless=True)

        tokens = Word(alphanums+"_:.-")
        name = tokens.copy().setResultsName("name")
        newName = tokens.copy().setResultsName("newName")

        alterIndexCmd = alterIndexToken + \
            name.setResultsName("name") + renameToToken + \
            newName.setResultsName("newName")

        try:
            tokens = alterIndexCmd.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens

    def AK_parse_alter_sequence(self, string):
        '''
        @author Marinko Radic
        @brief sql parsing of ALTER SEQUNCE command
        @param string sql command as string
        @return if command is successfully parsed returns list of tokens, else returns error message as string 
        '''
        alterSequenceToken = Keyword("ALTER SEQUENCE", caseless=True)
        incrementByToken = Keyword("INCREMENT BY", caseless=True)
        minValueToken = Keyword("MINVALUE", caseless=True)
        NoMinValueToken = Keyword("NO MINVALUE", caseless=True)
        maxValueToken = Keyword("MAXVALUE", caseless=True)
        noMaxValueToken = Keyword("NO MAXVALUE", caseless=True)
        restartWithToken = Keyword("RESTART WITH", caseless=True)
        cacheToken = Keyword("CACHE", caseless=True)
        noCacheToken = Keyword("NO CACHE", caseless=True)
        cycleToken = Keyword("CYCLE", caseless=True)
        noCycleToken = Keyword("NO CYCLE", caseless=True)
        setShemaToken = Keyword("SET SHEMA", caseless=True)

        tokens = Word(alphanums+"_")
        tokens2 = Word(nums)
        sequenceName = tokens.copy().setResultsName("sequenceName")
        increment = tokens2.copy().setResultsName("increment")
        minValue = tokens2.copy().setResultsName("minvalue")
        maxValue = tokens2.copy().setResultsName("maxvalue")
        cache = tokens.copy().setResultsName("cache")
        newName = tokens.copy().setResultsName("newName")
        restartValue = tokens2.copy().setResultsName("restartValue")
        constraints = ZeroOrMore(restartWithToken + restartValue.setResultsName("restartValue") |
                                 incrementByToken + increment.setResultsName("increment") |
                                 minValueToken + minValue.setResultsName("minvalue") | NoMinValueToken | maxValueToken + maxValue.setResultsName("maxvalue") | noMaxValueToken |
                                 cacheToken + cache.setResultsName("cache") | cycleToken | noCycleToken)

        alterSequenceCmd = alterSequenceToken.setResultsName(
            "commandName") + sequenceName.setResultsName("sequenceName") + Optional(constraints)
        try:
            tokens = alterSequenceCmd.parseString(string)
        except ParseException as err:
            return " "*err.loc + "^\n" + err.msg
        print()
        return tokens
