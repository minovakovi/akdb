# KalashnikovDB Docker MakeFile

CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

all: kalashnikov-db

kalashnikov-db: sql/function.o sql/api_functions.o
    $(CC) $(CFLAGS) -o ../bin/akdb sql/function.o sql/api_functions.o $(LDFLAGS)

sql/api_functions.o: sql/api_functions.c
	$(CC) $(CFLAGS) -c -o sql/api_functions.o sql/api_functions.c

sql/function.o: sql/function.c
	$(CC) $(CFLAGS) -c -o sql/function.o sql/function.c

clean:
	rm -f ../bin/akdb sql/api_functions.o sql/function.o