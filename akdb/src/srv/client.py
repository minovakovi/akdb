import time
import paramiko
import tests
import colors
import json

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
            print((self.username, self.password))
            try:
                self.sock.connect(
                    hostname=self.host,
                    port=self.port,
                    username=self.username,
                    password=self.password,
                    #Added timeout, previously multiple instances would clog the entire system
                    timeout=600
                )
                connected = True
                print("[*] Connection established.")
            except paramiko.ssh_exception.AuthenticationException:
                self.username = input("Username: ")
                self.password = input("Password: ")
            except Exception as e:
                # wait 2 seconds before attempting to connect again
                print(e)
                time.sleep(2)

        self.session = self.sock.get_transport().open_session()
        print("[+] Successfully conected to server")

        while self.working:
            try:
                cmd = input("akdb> ")
                #From this point forward until self.send_command(cmd), all we do is test the supported functions within client/server
                if(cmd=="testme"):
                    failcounter=0
                    passcounter=0
                    #Test for create table
                    try:
                        print((colors.bcolors.HEADER+"Testing CREATE TABLE"+colors.bcolors.ENDC))
                        cmd=tests.CreateTableTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                     #Test for listing out all commands
                    try:
                        print((colors.bcolors.HEADER+"Testing printing out all commands"+colors.bcolors.ENDC))
                        cmd=tests.AllCommandsTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                     #Test for checking if the table exists
                    try:
                        print((colors.bcolors.HEADER+"Testing if the table exists"+colors.bcolors.ENDC))
                        cmd=tests.TableExistsTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for for index creation
                    try:
                        print((colors.bcolors.HEADER+"Testing creating an index"+colors.bcolors.ENDC))
                        cmd=tests.CreateIndexTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for user creating
                    try:
                        print((colors.bcolors.HEADER+"Testing user creation"+colors.bcolors.ENDC))
                        cmd=tests.CreateUserTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for trigger creation
                    try:
                        print((colors.bcolors.HEADER+"Testing trigger creation"+colors.bcolors.ENDC))
                        cmd=tests.CreateTriggerTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for sequence creation
                    try:
                        print((colors.bcolors.HEADER+"Testing sequence creation"+colors.bcolors.ENDC))
                        cmd=tests.CreateSequenceTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for view creation
                    try:
                        print((colors.bcolors.HEADER+"Testing view creation"+colors.bcolors.ENDC))
                        cmd=tests.CreateViewTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for function creation
                    try:
                        print((colors.bcolors.HEADER+"Testing function creation"+colors.bcolors.ENDC))
                        cmd=tests.CreateFunctionTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing index
                    try:
                        print((colors.bcolors.HEADER+"Testing for altering an existing index"+colors.bcolors.ENDC))
                        cmd=tests.AlterIndexTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing table
                    try:
                        print((colors.bcolors.HEADER+"Testing for altering an existing table"+colors.bcolors.ENDC))
                        cmd=tests.AlterTableTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing sequence
                    try:
                        print((colors.bcolors.HEADER+"Testing for altering an existing sequence"+colors.bcolors.ENDC))
                        cmd=tests.AlterSequenceTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for altering an existing view
                    try:
                        print((colors.bcolors.HEADER+"Testing for altering an existing view"+colors.bcolors.ENDC))
                        cmd=tests.AlterViewTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for 'select' operator
                    try:
                        print((colors.bcolors.HEADER+"Testing 'select' operator"+colors.bcolors.ENDC))
                        cmd=tests.SelectTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for 'update' operator
                    try:
                        print((colors.bcolors.HEADER+"Testing 'update' operator"+colors.bcolors.ENDC))
                        cmd=tests.UpdateTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for 'insert' operator
                    try:
                        print((colors.bcolors.HEADER+"Testing 'insert' operator"+colors.bcolors.ENDC))
                        cmd=tests.InsertTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for dropping a table
                    try:
                        print((colors.bcolors.HEADER+"Testing dropping a table"+colors.bcolors.ENDC))
                        cmd=tests.DropTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for starting a begin-end block
                    try:
                        print((colors.bcolors.HEADER+"Testing begin-end block"+colors.bcolors.ENDC))
                        cmd=tests.BeginTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1

                    #Test for writing out history of actions
                    try:
                        print((colors.bcolors.HEADER+"Testing writing out history of actions"+colors.bcolors.ENDC))
                        cmd=tests.HistoryTest()
                    except Exception as e:
                        print(("-No tests are currently implemented/or not working" %e))
                    self.send_command(cmd)
                    out = self.recv_data()
                    if "Wrong" in out:
                        print((colors.bcolors.FAIL+"Test failed"+colors.bcolors.ENDC))
                        failcounter=failcounter+1
                    else:
                        print((colors.bcolors.OKGREEN+"Test passed"+colors.bcolors.ENDC))
                        passcounter=passcounter+1
                    print((colors.bcolors.OKGREEN+"Amount of passed tests: "+str(passcounter)+colors.bcolors.ENDC))
                    print((colors.bcolors.FAIL+"Amount of failed tests: "+str(failcounter)+colors.bcolors.ENDC))
                #Testing block ends here. For the sole purposes of not messing with anybody else working here, tests will stay in this file until all the work on
                #python (client-server) components is done
                else:
                    self.send_command(cmd)
                    out = self.recv_data()
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
