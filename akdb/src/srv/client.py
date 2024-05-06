import paramiko
import tests
from colors import bcolors
import json
import getpass
import readline
import threading
import datetime
from getpass import getpass as password_input
from paramiko.ssh_exception import BadHostKeyException 
import time  # Dodano

class Client:
    def __init__(self, username, password, host="127.0.0.1", port=1998):
        self.sock = paramiko.SSHClient()
        self.sock.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.session = None
        self.host = host
        self.port = port
        self.working = True
        self.username = username
        self.password = password
        self.command_history = [] 
        self.history_index = 0 

    def __del__(self):
        self.working = False
        if self.session is not None:
            self.session.close()
            print("[+] Session closed succesfully")
        self.sock.close()
        print("[+] Socket closed successfully")

    def start(self):
        connected = False
        while not connected:
            print("[*] Attempting to connect...")
            try:
                self.sock.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                self.sock.connect(
                    hostname=self.host,
                    port=self.port,
                    username=self.username,
                    password=self.password,
                    timeout=600
                )
                connected = True
                print("[*] Connection established.")
            except paramiko.ssh_exception.AuthenticationException:
                print(bcolors.YELLOW + "Invalid credentials. Please try again." + bcolors.ENDC)
                self.username = input(bcolors.YELLOW + "Username: " + bcolors.ENDC)
                self.password = password_input(prompt=bcolors.YELLOW + "Password: " + bcolors.ENDC)
                if self.username and self.password:
                    self.username = ""
                    self.password = ""
            except BadHostKeyException as e:  # Change here
                print(bcolors.YELLOW + "Host key verification failed." + bcolors.ENDC)
                accept_key = input(bcolors.YELLOW + "Do you want to accept the new key? (yes/no): " + bcolors.ENDC).lower()
                if accept_key.lower() == "yes":
                    self.sock.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                    try:
                        self.sock.connect(
                            hostname=self.host,
                            port=self.port,
                            username=self.username,
                            password=self.password,
                            timeout=600
                        )
                        connected = True
                        print("[*] Connection established.")
                    except paramiko.ssh_exception.AuthenticationException:
                        print(bcolors.YELLOW + "Invalid credentials. Please try again." + bcolors.ENDC)
                        self.username = input(bcolors.YELLOW + "Username: " + bcolors.ENDC)
                        self.password = password_input(prompt=bcolors.YELLOW + "Password: " + bcolors.ENDC)
                    except Exception as e:
                        print(bcolors.FAIL + "[ERROR] Connection failed: " + str(e) + bcolors.ENDC)
                        time.sleep(2)
                else:
                    break

        self.session = self.sock.get_transport().open_session()
        print("[+] Successfully connected to server")

        while self.working:
            try:
                cmd = input("akdb> ")
                if cmd.lower() == "clearhistory":
                    self.command_history = []
                    print("History cleared.")
                    continue  # Preskoči daljnju obradu i nastavi petlju
                self.command_history.append(cmd)
                if cmd =="time":
                    self.show_current_time()
                    month = datetime.datetime.now().month
                    year = datetime.datetime.now().year
                    calendar_output = tests.generate_calendar(month, year)
                    print(calendar_output)
                if cmd.strip() == "":
                    continue
                if cmd == "\033[A":  # A ovdje je escape karakter koji gornja strelica šalje
                    if history_index > -len(self.command_history):
                        history_index -= 1
                    cmd = self.command_history[history_index]
                    print(">>", cmd)  # Ispis zadnje unesene naredbe
                elif cmd == "\033[B":
                    if history_index < -1:
                        history_index += 1
                    if history_index == -1:
                        cmd = ""
                    else:
                        cmd = self.command_history[history_index]
                    print(">>", cmd)  # Ispis 
                #From this point forward until self.send_command(cmd), all we do is test the supported functions within client/server
                if(cmd=="testme"):
                    failcounter=0
                    passcounter=0
                    #Test for create table
                    try:
                        print((bcolors.HEADER+"Testing CREATE TABLE"+bcolors.ENDC))
                        cmd=tests.CreateTableTest()
                    except Exception as e:
                        print((bcolors.FAIL + "ERROR occurred while executing the CREATE TABLE test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                     #Test for listing out all commands
                    try:
                        print((bcolors.HEADER+"Testing printing out all commands"+bcolors.ENDC))
                        cmd=tests.AllCommandsTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the PRINT OUT ALL COMMANDS test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                     #Test for checking if the table exists
                    try:
                        print((bcolors.HEADER+"Testing if the table exists"+bcolors.ENDC))
                        cmd=tests.TableExistsTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the TABLE EXISTS test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for for index creation
                    try:
                        print((bcolors.HEADER+"Testing creating an index"+bcolors.ENDC))
                        cmd=tests.CreateIndexTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing CREATING AN INDEX test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for user creating
                    try:
                        print((bcolors.HEADER+"Testing user creation"+bcolors.ENDC))
                        cmd=tests.CreateUserTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing USER CREATION test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for trigger creation
                    try:
                        print((bcolors.HEADER+"Testing trigger creation"+bcolors.ENDC))
                        cmd=tests.CreateTriggerTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the TRIGGER CREATION test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for sequence creation
                    try:
                        print((bcolors.HEADER+"Testing sequence creation"+bcolors.ENDC))
                        cmd=tests.CreateSequenceTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the SEQUENCE CREATION test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for view creation
                    try:
                        print((bcolors.HEADER+"Testing view creation"+bcolors.ENDC))
                        cmd=tests.CreateViewTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the VIEW CREATION test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for function creation
                    try:
                        print((bcolors.HEADER+"Testing function creation"+bcolors.ENDC))
                        cmd=tests.CreateFunctionTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the FUNCTION CREATION test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing index
                    try:
                        print((bcolors.HEADER+"Testing for altering an existing index"+bcolors.ENDC))
                        cmd=tests.AlterIndexTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing test for ALTERING AN EXISTING INDEX: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing table
                    try:
                        print((bcolors.HEADER+"Testing for altering an existing table"+bcolors.ENDC))
                        cmd=tests.AlterTableTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the ALTERING AN EXISTING TABLE test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing sequence
                    try:
                        print((bcolors.HEADER+"Testing for altering an existing sequence"+bcolors.ENDC))
                        cmd=tests.AlterSequenceTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the ALTERING AN EXISTING SEQUENCE test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing view
                    try:
                        print((bcolors.HEADER+"Testing for altering an existing view"+bcolors.ENDC))
                        cmd=tests.AlterViewTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the ALTERING AN EXISTING VIEW test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for 'select' operator
                    try:
                        print((bcolors.HEADER+"Testing 'select' operator"+bcolors.ENDC))
                        cmd=tests.SelectTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the SELECT OPERATOR test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for 'update' operator
                    try:
                        print((bcolors.HEADER+"Testing 'update' operator"+bcolors.ENDC))
                        cmd=tests.UpdateTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the UPDATE OPERATOR test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for 'insert' operator
                    try:
                        print((bcolors.HEADER+"Testing 'insert' operator"+bcolors.ENDC))
                        cmd=tests.InsertTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the INSERT OPERATOR test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for dropping a table
                    try:
                        print((bcolors.HEADER+"Testing dropping a table"+bcolors.ENDC))
                        cmd=tests.DropTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the DROPPING A TABLE test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for starting a begin-end block
                    try:
                        print((bcolors.HEADER+"Testing begin-end block"+bcolors.ENDC))
                        cmd=tests.BeginTest()
                    except Exception as e:
                        print((bcolors.FAIL+"ERROR occurred while executing the BEGIN-END block test: {0}"+bcolors.ENDC.format(e)))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for writing out history of actions
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((bcolors.FAIL+"Test failed"+bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((bcolors.OKGREEN+"Test passed"+bcolors.ENDC))
                        passcounter=passcounter+1
                    print((bcolors.OKGREEN+"Amount of passed tests: "+str(passcounter)+bcolors.ENDC))
                    print((bcolors.FAIL+"Amount of failed tests: "+str(failcounter)+bcolors.ENDC))
                #Testing block ends here. For the sole purposes of not messing with anybody else working here, tests will stay in this file until all the work on
                #python (client-server) components is done
                else:
                    self.send_command(cmd)
                    out = self.recv_data()
                if cmd == "history":
                    print(bcolors.HEADER + "History of commands:" + bcolors.ENDC)
                    for command in self.command_history:
                        print(command)
                    #tests.HistoryTest()
                if cmd == "help":
                    tests.Help()
                if cmd == "quiz":
                    tests.start_quiz()

               # if cmd == "time":
                #    print(tests.TimeTest())


                

            except KeyboardInterrupt:
                self.working = False


    def send_command(self, cmd):
        #TODO implement protocol
        if len(cmd) > 0:
            try:
                self.session.send(self.pack_data({"command": cmd}))
            except Exception as e:
                print(("[-] Sending failed: %s" %e))

    def recv_data(self):
        #TODO implement protocol
        out = self.session.recv(1024)
        rec = self.unpack_data(out)
        if type(rec) is dict:
            if "success" in rec and rec["success"]:
                if "packed_data" in rec:
                    self.select_protocol(rec)
                else:
                    print((rec["result"]))
            else:
                print(("There was a problem. Error message: " + rec["error_msg"]))
        return rec

    def select_protocol(self, res):
        rec = res
        while "end" in rec and rec["end"] == False:
            print((rec["result"]))
            print((str(rec["startrow"]) + "-" + str(rec["endrow"]) + "/" + str(rec["max"])))
            cont = input("Continue (yes/no)? ")
            if cont == "yes":
                self.session.send(self.pack_data({"continue": True}))
                out = self.session.recv(1024)
                rec = self.unpack_data(out)
            else:
                self.session.send(self.pack_data({"continue": False}))
                return False
        if rec["success"] == False:
            print(("Interrupted by server: " + rec["error_msg"]))
            return False
        print((rec["result"]))
        print((str(rec["startrow"]) + "-" + str(rec["endrow"]) + "/" + str(rec["max"])))

    # packs data into json
    def pack_data(self, data):
        return json.dumps(data)

    # unpacks json data
    def unpack_data(self, data):
        return json.loads(data)
    
    def show_current_time(self):
            print("Press CTRL+C to exit time display.")
            try:
                while True:
                    now = datetime.datetime.now()
                    current_time = now.strftime("%Y-%m-%d %H:%M:%S")  
                    print("\rCurrent date and time:", current_time, end="", flush=True)  
                    time.sleep(1) 
            except KeyboardInterrupt:
                print("\nExiting time display.")  