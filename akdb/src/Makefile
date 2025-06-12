# KalashnikovDB MakeFile
CC = gcc -pthread -w
DEBUG = -g
CFLAGS = $(DEBUG) -c
CFLAGS += -I./import

LIBS = -lcjson

DISKTARGETS = dm/dbman.o
MEMORYTARGETS = mm/memoman.o
IMPORTTARGETS = import/json_importer.o import/csv_importer.o
FILETARGETS = file/files.o file/fileio.o file/filesearch.o file/filesort.o file/idx/index.o file/idx/btree.o file/idx/hash.o file/idx/bitmap.o file/table.o file/blobs.o
RELOPTARGETS = rel/difference.o rel/intersect.o rel/nat_join.o rel/projection.o rel/selection.o rel/union.o rel/aggregation.o rel/product.o rel/theta_join.o trans/transaction.o
OPTITARGETS = opti/rel_eq_projection.o opti/rel_eq_selection.o opti/rel_eq_assoc.o opti/rel_eq_comut.o opti/query_optimization.o
CONSTRAINTTARGETS = sql/cs/constraint_names.o sql/cs/reference.o sql/cs/between.o sql/cs/nnull.o file/id.o rel/expression_check.o sql/cs/check_constraint.o sql/cs/unique.o
OTHERTARGETS = auxi/test.o auxi/mempro.o sql/trigger.o file/test.o auxi/debug.o rec/archive_log.o sql/command.o auxi/dictionary.o auxi/auxiliary.o auxi/iniparser.o sql/privileges.o sql/function.o file/sequence.o rec/redo_log.o sql/insert.o sql/drop.o sql/view.o auxi/observable.o sql/select.o rec/recovery.o

OBJS = $(OTHERTARGETS) $(CONSTRAINTTARGETS) $(OPTITARGETS) $(RELOPTARGETS) $(DISKTARGETS) $(MEMORYTARGETS) $(FILETARGETS) $(IMPORTTARGETS) tests.o main.o
OUTDIR = ../bin

.PHONY: swig

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

-include $(OBJS:.o=.d)

all: kalashnikov-db

dev: CFLAGS += -Wall
dev: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(OUTDIR)/akdb

kalashnikov-db: CFLAGS += -w
kalashnikov-db: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(OUTDIR)/akdb

asan: CFLAGS += -Wall -fsanitize=address
asan: $(OBJS)
	$(CC) $(OBJS) -fsanitize=address $(LIBS) -o $(OUTDIR)/akdb

clean-d:
	rm -rf *.d auxi/*.d dm/*.d mm/*.d file/*.d trans/*.d file/idx/*.d rec/*.d sql/cs/*.d sql/*.d opti/*.d rel/*.d import/*.d

clean: clean-d
	rm -rf *~ *.o auxi/*.o dm/*.o mm/*.o file/*.o trans/*.o file/idx/*.o rec/*.o sql/cs/*.o sql/*.o opti/*.o rel/*.o ../bin/akdb ../bin/*.log ../doc/* ../bin/kalashnikov.db ../bin/blobs swig/build swig/*.pyc swig/*.so swig/*.log swig/*~ swig/kalashnikovDB_wrap.c swig/kalashnikov.db srv/kalashnikov.db

comments: 
	./tools/getFiles.sh
	python ./tools/comments.py

doc:
	./tools/updateVersion.sh
	doxygen doxygen.config
	cd ../doc/latex && make
	cp ../doc/latex/refman.pdf ../doc/

swig:
	export CFLAGS="-w"; cd swig && swig -python -py3 kalashnikovDB.i && python3 setup.py build_ext --inplace && cd ..

swig-tests:
	export CFLAGS="-Wno-error"; cd swig && python -m doctest -v tablePropertiesTests.py && python -m doctest -v selectionTests.py && python -m doctest -v relAlgebraTests.py && cd ..

docker-install:
	$(MAKE) -C .. docker-install

docker-mac:
	$(MAKE) -C .. docker-mac

docker-win:
	$(MAKE) -C .. docker-win

.DEFAULT_GOAL := all
