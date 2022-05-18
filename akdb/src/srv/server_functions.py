import socket
import paramiko
import threading
import sys
import json
import configparser

sys.path.append("../swig/")
import kalashnikovDB as AK47
import sql_executor as sqle

config = configparser.ConfigParser()
config.read('config.ini')
n = int(config["select_options"]["number_of_rows_in_packet"])

#Interface to override classic python server support
class ParamikoServer(paramiko.ServerInterface):
    def __init__(self):
        self.event = threading.Event()
    #Function that checks if the channel can be opened for a requesting client
    def check_channel_request(self, kind, chanid):
        if kind == 'session':
            return paramiko.OPEN_SUCCEEDED
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED
    #Function that checks if the clients username and password match
    def check_auth_password(self, username, password):
        usr = "testingUser" #TODO get users and passwords from database
        #Testing the data format of AK_user_get_id(usr)
        AK47.AK_user_get_id(usr)
        #SIGSEGV ISSUE again
        #return AK47.AK_user_get_id(usr)
        pas = "testingPass"
        #local login using swig

    #Fran fix:
    #if(AK47.AK_user_check_pass(username,password)==1)
    #    return paramiko.AUTH_SUCCESSFUL
    #return paramiko.AUTH_FAILED




    #    if(AK47.AK_user_get_id(usr)!=null):
    #       return paramiko.AUTH_SUCCESSFUL
    #    elif(username == usr) and (password == pas):
    #        return paramiko.AUTH_SUCCESSFUL
    #         return paramiko.AUTH_FAILED
    #    pas = "testingPass"
        if (username == usr) and (password == pas):
            return paramiko.AUTH_SUCCESSFUL
        return paramiko.AUTH_FAILED
#Class that handles connection from client to the server
class Connection:
    #Constructor of the class 
    def __init__(self, conn, addr):
        try:
            self.addr = addr
            self.transport = paramiko.Transport(conn)
            self.transport.add_server_key(paramiko.RSAKey.generate(2048))
            self.transport.start_server(server=ParamikoServer())
            self.channel = self.transport.accept(timeout=1)
        except Exception as e:
            self.addr = False
    #Destructor of the class
    def __del__(self):
        if self.channel is not None:
            self.channel.close()
        if self.transport is not None:
            self.transport.close()
    #Function that handles sending data to the client
    def send_data(self, data):
        try:
            if data[1].startswith('Error'):
                self.channel.send(self.pack_output({"success": False, "error_msg": data[1]}))
            elif data[1] == False:                
                self.channel.send(self.pack_output({"success": False, "error_msg": "There was an error in your command."}))
            elif data[0] == "Select_command":
                self.select_protocol(data[1])
            # Perhaps handle other cases differently
            else:
                self.channel.send(self.pack_output({"success": True, "result": data[1]}))
        except Exception as e:
            self.channel.send(self.pack_output({"success": False, "error_msg": "[-] Internal server error: %s" %e}))
    #Function that handles recieving data from the client
    def recv_data(self):
        try:
            data = self.unpack_input(self.channel.recv(1024))
            if type(data) is dict:
                if "command" in data:
                    return data["command"]
                elif "continue" in data:
                    return data["continue"]
            return False
        except Exception as e:
            print("[-] Failed while unpacking data: %s" %e)
            return False

    #Function that formats data into a JSON format
    def pack_output(self, out):
        return json.dumps(out)
    #Fucntion that reads JSON format and converts it to XY
    def unpack_input(self, inp):
        return json.loads(inp)
    # Function that splits table by newline and sends 1000 by 1000 rows
    def select_protocol(self, table):
        l = table.splitlines()
        print(n)
        print(len(l))
        if (len(l) > n):
            header = [l.pop(0)]
            for i in range(0, len(l), n):
                if i+n >= len(l):
                    end = True
                    endrow = len(l)
                else:
                    end = False
                    endrow = i+n
                if i == 0:
                    data = self.pack_output({"startrow": i, "endrow": endrow, "max": len(l),"end": end, "result": '\n'.join(header + l[i:i + n]), "success": True, "packed_data": True})
                else:
                    data = self.pack_output({"startrow": i, "endrow": endrow, "max": len(l),"end": end, "result": '\n'.join(l[i:i + n]), "success": True, "packed_data": True})
                self.channel.send(data)
                if end == False:
                    print("Sent " + str(i+n) + "/" + str(len(l)) + " rows to " + self.addr[0])
                    res = self.recv_data()
                    if res:
                        continue
                    else:
                        print("Interrupted by client.")
                        break
                else:
                    print("Sent all " + str(len(l)) + " rows to " + self.addr[0])
                    break
        else:
            data = self.pack_output({"rows": len(l)-1, "result": table, "success": True})
            self.channel.send(data)


    #Function that checks if the server is still running
    def is_alive(self):
        if self.channel is not None and self.transport.is_active():
            return True
        return False
