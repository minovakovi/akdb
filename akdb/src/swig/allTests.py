
import os
import subprocess
from kalashnikovDB import *


def set_catalog_constraints():

	#//Set PRIMARY KEY constraint on all tables of system catalog when it' s implemented

	#/NOT NULL constraints on table AK_constraints_not_null
	retValue = AK_set_constraint_not_null("AK_constraints_not_null", "tableName", "tableNameNotNull")
	retValue = AK_set_constraint_not_null("AK_constraints_not_null", "constraintName", "constraintNameNotNull")
	retValue = AK_set_constraint_not_null("AK_constraints_not_null", "attributeName", "attributeNameNotNull")
	#//NOT NULL constraints on table AK_constraints_unique
	retValue = AK_set_constraint_not_null("AK_constraints_unique", "tableName", "tableName2NotNull")
	retValue = AK_set_constraint_not_null("AK_constraints_unique", "constraintName", "constraintName2NotNull")
	retValue = AK_set_constraint_not_null("AK_constraints_unique", "attributeName", "attributeName2NotNull")
	#//NOT NULL constraints on table AK_sequence
	retValue = AK_set_constraint_not_null("AK_sequence", "name", "nameNotNull")
	retValue = AK_set_constraint_not_null("AK_sequence", "current_value", "current_valueNotNull")
	retValue = AK_set_constraint_not_null("AK_sequence", "increment", "incrementNotNull")
	#//SET NOT NULL CONSTRAINT ON THE REST OF TABLES IN SYSTEM CATALOG!!!

	attributeName="";
	constraintName="";
	#//UNIQUE constraints on table AK_constraints_not_null
	attributeName+="tableName"
	attributeName+= SEPARATOR
	attributeName+=attributeName
	constraintName+= "tableName"
	constraintName+= SEPARATOR
	constraintName+="attributeName"
	constraintName+= "Unique"
	retValue = AK_set_constraint_unique("AK_constraints_not_null", attributeName, constraintName)
	#//UNIQUE constraints on table AK_constraints_unique
	constraintName=""
	constraintName+= "tableName"
	constraintName+= SEPARATOR
	constraintName+="attributeName2"
	constraintName+= "Unique"
	retValue = AK_set_constraint_unique("AK_constraints_unique", attributeName, constraintName)
	#//UNIQUE constraints on table AK_sequence
	retValue = AK_set_constraint_unique("AK_sequence", "name", "nameUnique")
	#//SET UNIQUE CONSTRAINT ON THE REST OF TABLES IN SYSTEM CATALOG!!!








listNumberTest=[]
for x in range (0,2):
	listNumberTest.append(x)

listTests=[
"tablePropertiesTests",
"selectionTests",
"relAlgebraTests",
AK_tarjan_test,
AK_observable_test,
AK_observable_pattern,
AK_allocationbit_test,
AK_allocationtable_test,
AK_thread_safe_block_access_test,
AK_id_test,
#AK_lo_test,
AK_files_test,
AK_fileio_test,
AK_op_rename_test, 
AK_filesort_test,
AK_filesearch_test,
AK_sequence_test,
AK_table_test,
AK_bitmap_test,
AK_btree_test,
AK_hash_test,
AK_memoman_test,
AK_memoman_test2,
AK_rel_eq_assoc_test,
AK_rel_eq_comut_test,
AK_rel_eq_selection_test,
AK_rel_eq_projection_test,
AK_query_optimization_test,
AK_op_union_test,
AK_op_join_test,
AK_op_product_test,
AK_aggregation_test,
AK_op_intersect_test,
AK_op_selection_test,
AK_op_selection_test_pattern,
AK_op_selection_test_redolog,
AK_expression_check_test,
AK_op_difference_test,
AK_op_projection_test,
AK_op_theta_join_test,
AK_test_command,
AK_drop_test,
AK_view_test,
AK_nnull_constraint_test,
#AK_select_test,
AK_trigger_test,
AK_unique_test,
AK_reference_test,
AK_constraint_between_test,
AK_check_constraint_test,
AK_constraint_names_test,
AK_test_Transaction,
AK_recovery_test
]

listTestsStrings=[
["doctest:","tablePropertiesTests"],
["doctest:","selectionTests"],
["doctest:","relAlgebraTests"],
["auxi:","AK_tarjan_test"],
["auxi:","AK_observable_test"],
["auxi:","AK_observable_pattern"],
["dm:","AK_allocationbit_test"],
["dm:","AK_allocationtable_test"],
["dm:","AK_thread_safe_block_access_test"],
["file:","AK_id_test"],
#["file:",AK_lo_test],
["file:","AK_files_test"],
["file:","AK_fileio_test"],
["file:","AK_op_rename_test"], 
["file:","AK_filesort_test"],
["file:","AK_filesearch_test"],
["file:","AK_sequence_test"],
["file:","AK_table_test"],
["idx:","AK_bitmap_test"],
["idx:","AK_btree_test"],
["idx:","AK_hash_test"],
["mm:","AK_memoman_test"],
["mm:","AK_block_test2"],
["opti:","AK_rel_eq_assoc_test"],
["opti:","AK_rel_eq_comut_test"],
["opti:","AK_rel_eq_selection_test"],
["opti:","AK_rel_eq_projection_test"],
["opti:","AK_query_optimization_test"],
["rel:","AK_op_union_test"],
["rel:","AK_op_join_test"],
["rel:","AK_op_product_test"],
["rel:","AK_aggregation_test"],
["rel:","AK_op_intersect_test"],
["rel:","AK_op_selection_test"],
["rel:","AK_op_selection_test_pattern"],
["rel:","AK_op_selection_test_redolog"],
["rel:","AK_expression_check_test"],
["rel:","AK_op_difference_test"],
["rel:","AK_op_projection_test"],
["rel:","AK_op_theta_join_test"],
["sql:","AK_test_command"],
["sql:","AK_drop_test"],
["sql:","AK_view_test"],
["sql:","AK_nnull_constraint_test"],
#["sql:",AK_select_test],
["sql:","AK_trigger_test"],
["sql:","AK_unique_test"],
["sql:","AK_reference_test"],
["sql:","AK_constraint_between_test"],
["sql:","AK_check_constraint_test"],
["sql:","AK_constraint_names_test"],
["trans:","AK_test_Transaction"],
["rec:","AK_recovery_test"]
]

loop=True
AK_inflate_config()
AK_init_disk_manager()
AK_memoman_init()
AK_create_test_tables()
set_catalog_constraints()

while(loop==True):

	print ("==================================TEST=========================================")
	print ("\n \n type in number of test to execute it: \n \n")
	counter=1
	printHelper=0
	pref=""
	for test in listTestsStrings:
		formated=test[0]+" "+test[1]
		if (printHelper<2)and (pref==test[0]):
			print ("{: >10}".format(counter),end="  ")
			print ("{: >40}".format(formated),end="  ")
			printHelper+=1
		elif (pref!=test[0]):
			print ("\n",end="\n")
			print ("{: >10}".format(counter),end="  ")
			print ("{: >40}".format(formated),end="  ")
			printHelper=1
		else:
			print ("{: >10}".format(counter),end="  ")
			print ("{: >40}".format(formated),end="\n")
			printHelper=0
		pref=test[0]
		counter+=1
	

	print ("\n \n \n 0 Exit \n \n")

	numberOfTest=input("Test: ")
	if ((numberOfTest<4)and(numberOfTest>0)):
		if (numberOfTest==1):
			os.system("python -m doctest -v tablePropertiesTests.py")
		elif (numberOfTest==2):
			os.system("python -m doctest -v selectionTests.py")
		elif (numberOfTest==3):
			os.system("python -m doctest -v relAlgebraTests.py")

	elif ((numberOfTest<counter)and(numberOfTest>0)):
		x=numberOfTest-1
		res=listTests[x]()
		TEST_output_results(res)
	elif (numberOfTest==0):
		exit()
	else:
		print ("Nedozvoljeni broj! \n")
	input("Press Enter to continue...")





