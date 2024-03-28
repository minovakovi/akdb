![AKDB](https://i.imgur.com/svgx1FX.png)

# AKDB

[https://minovakovi.github.io/akdb/](https://minovakovi.github.io/akdb/)

An experimental relational DBMS developed by students @ Faculty of Organization and Informatics, University of Zagreb

## What is AKDB?

AK (Russian: Автомат Калашникова, English: Avtomat Kalasnikova) DB is a simple relational database management system developed by professor Markus Schatten of Faculty of Organization and Informatics in Varaždin, Croatia. It is also regularly developed, maintained and fixed by graduate students attending 'Physical Database Modeling' course every spring.

The development started in 2009 and is still ongoing, as building a custom DBMS is not an easy task. Most of it is written in C, with some Python for Server stuff and documentation which is generated automatically and can be viewed in HTML or LaTeX form. More on that in Installation and Build pages. 
## TESTING
Since the project is still in development, tests are required to ensure its stability and functionality. There are couple of dozen tests that check whether certain parts of the application work correctly.

This will write out the list of current tests that can be performed. As of 1st of June 2022 they are numbered and divided into few categories:

Auxiliary:

  1. TRAJAN ( AK_tarjan )
  2. OBSERVABLE ( AK_observable )
  3. OBSERVABLE PATTERN ( AK_observable_patte )
  4. MEMPRO ( auxi: AK_mempro )
  5. DICTIONARY ( AK_dictionary )
  6. PARSER ( AK_iniparser )

Disk manager:
  
  7. ALLOCATION BIT ( AK_allocationbit )
  8. ALLOCATION TABLE ( AK_allocationtable )
  9. THREAD SAFE BLOCK ( AK_thread_safe_block_ )
   
File system:

  10. ID ( AK_id )
  11. LO ( AK_lo )
  12. FILES ( AK_files_test )
  13. FILE IO ( AK_fileio_test )
  14. OP RENAME ( AK_op_rename )
  15. FILE SORTING ( AK_filesort )
  16. FILE SEARCH ( AK_filesearch )
  17. SEQUENCE ( AK_sequence )
  18. TABLE TEST ( AK_table_test )

Index:

  19. BITMAP ( AK_bitmap )
  20. BINARY TREE ( AK_btree )
  21. HASH ( AK_hash )
  
Memory manager: 

  22. MEMORY MANAGER ( AK_memoman )
  23. BLOCK MEMORY ( AK_block )
  
Optimization:

  24. RE ASSOCIATION ( AK_rel_eq_assoc )
  25. RE COMMUTATION ( AK_rel_eq_comut )
  26. RE SELECTION ( AK_rel_eq_selection )
  27. RE PROJECTION ( AK_rel_eq_projectio )
  28. QUERY OPTIMIZATION ( AK_query_optimizati )
  
Relational operators:

  29. UNION ( AK_op_union )
  30. JOIN  ( AK_op_join )
  31. PRODUCT ( AK_op_product )
  32. AGGREGATION ( AK_aggregation )
  33. GROUP BY (AK_group_by)
  34. HAVING (AK_selection_having)
  35. INTERSECTION ( AK_op_intersect )
  36. SELECTION ( AK_op_selection )
  37. SELECTION PATTERN ( AK_op_selection_patt  )
  38. CHECK EXPRESSION ( AK_expression_check_  )
  39. DIFFERENCE ( AK_op_difference )
  40. PROJECTION ( AK_op_projection )
  41. THETA JOIN ( AK_op_theta_join )

SQL tests:

  42. COMMAND ( AK_command )
  43. DROP  ( AK_drop )
  44. VIEW ( AK_view )
  45. NNULL ( AK_nnull )
  46. SELECT ( AK_select )
  47. TRIGGER ( AK_trigger )
  48. UNIQUE ( AK_unique )
  49. FUNCTION ( AK_function  )
  50. PRIVILEGES ( AK_privileges )
  51. REFERENCE ( AK_reference )
  52. CONSTRAINT BETWEEN ( AK_constraint_betwee )
  53. CHECK ( AK_check_constraint )
  54. CONSTRAINT NAMES ( AK_constraint_names )
  55. INSERT ( AK_insert )
  
Transaction:

  56. TRANSACTION ( AK_transaction )
  
Recovery:

  57. RECOVERY ( AK_recovery )
