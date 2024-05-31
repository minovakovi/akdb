import socket
import paramiko
import threading
import signal
import sys, os
import time

sys.path.append("../swig/")
import kalashnikovDB as AK47
import sql_executor as sqle
import server_functions as srv
#Refactored and documented by Marko Vertus
#Creates a threaded wrapper
def Threaded(fn):
    def wrapper(*args, **kwargs):
        t = threading.Thread(target=fn, args=args, kwargs=kwargs)
        t.start()
    return wrapper
#Main class of the server.py
class Server:
    #Executor for executing sql commands
    executor = sqle.Sql_executor()
           
    #Constructor of Server class
    def __init__(self, host="localhost", port=1998):
        self.host = host;
        self.port = port;
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.transport = None
        self.channel = None
        self.working = True
        self.accepting = True
        signal.signal(signal.SIGINT, self.signal_handler)
        print(self.host)
        print(self.port)
        sqle.initialize()
    #Destructor of Server class
    def __del__(self):
        self.working = False
        self.sock.close()
    # Interrupt handler
    def signal_handler(self, signal, frame):
        self.working = False
        self.sock.close()
        print('Interrupted. Server closing.')
        os._exit(1)
    #Functions that starts the server by binding the socket to the IP and port
    def start(self):
        try:
            self.sock.bind((self.host, self.port))
        except Exception as e:
            print("[-] Failed to start server: %s" %e)
            self.__del__()
        print("[-] Server started")

        self.sock.listen(1)
        self.input_listen()
        
        while self.working:
            if self.accepting:
                print("Awaiting connection")
                conn, addr = self.sock.accept()
                if self.accepting:
                    ssh_conn = srv.Connection(conn, addr)
                    if ssh_conn.addr != False:
                        print("[*] Incoming connection from %s" %addr[0])
                        self.handle_connection(ssh_conn)
    #Function for handling connections and also disconnects from the server
    @Threaded
    def handle_connection(self, conn):
        while conn.is_alive():
            data = conn.recv_data()
            out = Server.process_command(data)
            conn.send_data(out)   
        print("[*] Client from %s disconnected from server" % conn.addr[0])
        del conn
    # Command listener
    @Threaded
    def input_listen(self):
        while self.working:
            s = input()
            if self.accepting == True and s == "stop":
                self.accepting = False
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.connect(("localhost", 1998))
                print("[-] Server closed") 
            if self.accepting == False and s == "start":
                self.accepting = True
                print("[-] Server resumed") 
    #Function for executing SQL commands via sql.executor()
    @staticmethod
    def process_command(cmd):
        try:
            out = Server.executor.execute(cmd)
        except Exception as e:
            err = "[-] Failed command execution: %s" %e
            out = err
            print(err)
        return out
# Starts up the server
s = Server()
s.start()
