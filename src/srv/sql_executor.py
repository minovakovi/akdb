#!/usr/bin/env python

from sql_tokenizer import *
import time

import sys
sys.path.append('../swig/')
import kalashnikovDB as ak47
import test

# Helper function to determine number type

def is_numeric(lit):
    # Handle '0'
    if lit == '0': return 0
    # Hex/Binary
    litneg = lit[1:] if lit[0] == '-' else lit
    if litneg[0] == '0':
        if litneg[1] in 'xX':
            return int(lit,16)
        elif litneg[1] in 'bB':
            return int(lit,2)
        else:
            try:
                return int(lit,8)
            except ValueError:
                pass
    # Int/Float
    try:
        return int(lit)
    except ValueError:
        pass
    try:
        return float(lit)
    except ValueError:
        pass

# Helper function to determine if the string of DATE type

def is_date(lit):
	try:
	  time.strptime(lit, '%Y-%m-%d')
	  return True
	except ValueError:
	  return False

# Helper function to determine if the string of DATETIME type

def is_datetime(lit):
	returnVal = 2
	try:
		time.strptime(lit, '%Y-%m-%d %H:%M:%S')
		returnVal += 1
	except ValueError:
		returnVal -= 1
	try:
		time.strptime(lit, '%Y-%m-%d %H:%M:%S.%f')
		returnVal += 1
	except ValueError:
		returnVal -= 1
	if returnVal > 0:
		return True
	else:
		return False

# Helper function to determine if the string of TIME type

def is_time(lit):
	try:
		time.strptime(lit, '%H:%M:%S')
		return True
	except ValueError:
		return False

# Helper function to determine if the string of BOOL type

def is_bool(lit):
	return lit.lower() in ("true","false")

# Return attribute type

def get_attr_type(value):

	integer = type(is_numeric(value)) == int
	decimal = type(is_numeric(value)) == float
	varchar = isinstance(value,str)
	date = is_date(value.replace("'",""))
	datetime = is_datetime(value.replace("'",""))
	time = is_time(value.replace("'",""))
	boolean = is_bool(value.replace("'",""))

	if integer:
		return ak47.TYPE_INT
	elif decimal:
		return ak47.TYPE_FLOAT
	elif date:
		return ak47.TYPE_DATE
	elif datetime:
		return ak47.TYPE_DATETIME
	elif time:
		return ak47.TYPE_TIME
	elif boolean:
		return ak47.TYPE_BOOL
	elif varchar:
		return ak47.TYPE_VARCHAR
	else:
		print "UNDEFINED"

class sql_executor:

	# Insert sa testovima bez unosa
	def insert(self,expr):
		parser = sql_tokenizer()
		token = parser.AK_parse_insert_into(expr)
		# vrijednosti podataka za unos
		insert_attr_values = map(lambda x: x.replace("'",""),list(token.columnValues[0]))
		# tipovi podataka za unos
		insert_attr_types = map(lambda x: get_attr_type(x.replace("'","")),list(token.columnValues[0]))
		#Dohvatiti listu atributa tablice
		table_attr_names = str(ak47.AK_rel_eq_get_atrributes_char(str(token.tableName))).split(";")
		#Dohvatiti tipove atributa tablice
		table_attr_types = str(ak47.AK_get_table_atribute_types(str(token.tableName))).split(";")
		# imena atributa za unos
		insert_attr_names = table_attr_names
		# navedeni su atributi za unos
		if(token.columns):
			insert_attr_names = []
			table_types_temp = table_attr_types
			table_attr_types = []
			insert_columns = list(token.columns[0])
			for index,col in enumerate(insert_columns):
				if col not in table_attr_names:
					print "Error: table has no attribute " + str(col)
					return False
			#provjera atributa za unos
			for ic,col in enumerate(insert_columns):
				for ia,tab in enumerate(table_attr_names):
					if col == tab:
						insert_attr_names.append(tab)
						table_attr_types.append(int(table_types_temp[ia]))
			
			if (len(insert_columns) == len(insert_attr_values)):
				for index,tip in enumerate(insert_attr_types):
					if int(insert_attr_types[index]) != int(table_attr_types[index]):
						print "Error: type error at index " + str(index)
						return False
			else:
				print "Error: attribute names number not matching attribute values number"
				return False
		# navedene su samo vrijednosti za unsot
		elif (len(table_attr_names) < len(insert_attr_values)):
			print "Error: too many attibutes, table has " + str(len(table_attr_names)) 
			return False
		elif (len(table_attr_names) > len(insert_attr_values)):
			print "Error: too few attibutes, table has " + str(len(table_attr_names))
			return False
		else:
			for index,tip in enumerate(insert_attr_types):
				if insert_attr_types[index] != int(table_attr_types[index]):
					print "Error: type error at index " + str(index)
					return False
		if(ak47.insert_data_test(str(token.tableName),insert_attr_names,insert_attr_values,insert_attr_types) == ak47.EXIT_SUCCESS):
			return True
		else:
			return False
		return False

# Testiranje insertcd 

bulbasaur = sql_executor()

ak47.AK_inflate_config()
ak47.AK_init_disk_manager()
ak47.AK_memoman_init()

student_attr_name = ["id_student", "firstname", "lastname", "year", "weight"]
student_attr_type = [ak47.TYPE_INT, ak47.TYPE_VARCHAR, ak47.TYPE_VARCHAR, ak47.TYPE_INT, ak47.TYPE_FLOAT]

ak47.create_header_test("student", student_attr_name, student_attr_type)

print "pero: " + str(bulbasaur.insert("INSERT INTO student VALUES ('2','Pero','Peric','1991','100.2')"))
print "ivo: " + str(bulbasaur.insert("INSERT INTO student(id_student,firstname,lastname, year, weight) VALUES ( '3','Ivo','Ivic','2002','90.2')"))
print "marko: " + str(bulbasaur.insert("INSERT INTO student VALUES ('4','Marko','2','1998','80.3')"))
print "luka: " + str(bulbasaur.insert("INSERT INTO student(id_student,firstname,lastname, yearz, weight) VALUES ( '5','Luka','Lukic','1989','80.2')"))
ak47.AK_print_table("student")

