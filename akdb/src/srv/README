-------------------------------
Using Server and client in AKDB
-------------------------------

DB USER
-----------------
1. Starting Server
2. Starting Client
3. Entering commands

1. Starting Server
	$ python server.py

2. Starting Client
	$ python akdb_cli.py

3. Entering commands:
	When in client enter:
		akdb>\?
	Help will appear, with instructions.


DB DEV
-------------------
1. Server
2. Client
3. Adding new commands

1. Server
	- You can change port number for the Server by changing the "port" variable
	- Server is currently calling sql_executor#execute from sql_executor.py

2. Client
	- is built with pythons 'cmd' library (check online for further references about methods)
	- if server port is changed, clients port has to be changed also
	- Client currently supports only few commands, but that can be easily expanded in future

3. Adding new commands
	- Every new command needs to have a class in sql_executor.py with two methods: 
		- 'matches' - checks if entered command matches the syntax of the new command (via python regexes)
		- 'execute' - does what the new command is supposed to do

	- After class definition, it's instance is to be created in sql_executor class (sql_executor.py), and added to 'commands' array in the same class.

	- That is all. When new command is invoked, the server will call 'execute', and go through all the commands. If one of them 'matches' its 'execute' will be started and returned to client.