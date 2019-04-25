#!/usr/bin/env python2

import socket
import paramiko
import threading
import sys

sys.path.append("../swig/")
import kalashnikovDB as ak47
import sql_executor as sqle

class ParamikoServer(paramiko.ServerInterface):
    def __init__(self):
        self.event = threading.Event()
    def check_channel_request(self, kind, chanid):
        if kind == 'session':
            return paramiko.OPEN_SUCCEEDED
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED
    def check_auth_password(self, username, password):
        usr = "testingUser" #TODO change to real user
        pas = "testingPass" #TODO change to real pass
        if (username == usr) and (password == pas):
            return paramiko.AUTH_SUCCESSFUL
        return paramiko.AUTH_FAILED

def process_command(cmd):
    executor = sqle.sql_executor()
    return executor.execute(cmd)

sqle.initialize()


HOST = "localhost"
PORT = 1998

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

sock.bind((HOST,PORT))
sock.listen(1)

conn, addr = sock.accept()
rsa_key = paramiko.RSAKey.generate(2048)
transport = paramiko.Transport(conn)
transport.add_server_key(rsa_key)
transport.start_server(server=ParamikoServer())
channel = transport.accept()

channel.send("Hello")
cmd = channel.recv(1024).strip()
print cmd
out = process_command(cmd)
print out
channel.send(out)

channel.close()
transport.close()
conn.close()
sock.close()

