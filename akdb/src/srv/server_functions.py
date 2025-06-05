import socket
import paramiko
import threading
import sys
import json
import configparser
import sqlite3
import hashlib

import sqlite3
import hashlib

connection = sqlite3.connect("test.db")
cursor = connection.cursor()

# Brisanje svih zapisa iz tablice example
#cursor.execute("DELETE FROM example")
#cursor.execute("DELETE FROM quiz_questions")

# Kreiranje tablice example s jedinstvenim ključem na stupcu usr
cursor.execute("CREATE TABLE IF NOT EXISTS example (id INTEGER, usr TEXT UNIQUE, pas_hash TEXT)")
#cursor.execute("CREATE TABLE IF NOT EXISTS test (redak varchar(20))")

# Umetanje korisnika u tablicu example
cursor.execute("INSERT OR IGNORE INTO example VALUES (1, 'testingUser', ?)", (hashlib.sha256("testingPass".encode()).hexdigest(),))
cursor.execute("INSERT OR IGNORE INTO example VALUES (2, 'user', ?)", (hashlib.sha256("pass".encode()).hexdigest(),))

# Provjera sadržaja tablice example
#cursor.execute("SELECT * FROM example")
rows = cursor.fetchall()

for row in rows:
    print(row)


cursor.execute("CREATE TABLE IF NOT EXISTS quiz_questions (id INTEGER PRIMARY KEY, question TEXT UNIQUE, answer TEXT)")
questions = [
    ("Kada je započet projekt AKDB?", "2009"),
    ("Koliko je otprilike ljudi dosad radilo na AKDB?", "Najmanje 200"),
    ("Unutar testiranja koliko postoji testova?", "57")
]
cursor.executemany("INSERT OR IGNORE INTO quiz_questions (question, answer) VALUES (?, ?)", questions)

# Prikaz svih pitanja u tablici quiz_questions
cursor.execute("SELECT * FROM quiz_questions")
rows = cursor.fetchall()

for row in rows:
    print(row)

#cursor.execute("PRAGMA table_info(quiz_questions)")
#columns = cursor.fetchall()

# Ispisivanje imena stupaca
#for column in columns:
  #  print(column[1])

# Potvrda promjena i zatvaranje veze s bazom podataka
connection.commit()
connection.close()




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
        connection = sqlite3.connect("test.db")
        cursor = connection.cursor()
        cursor.execute("SELECT usr, pas_hash FROM example WHERE usr = ?", (username,))
        user_data = cursor.fetchone()
        #print("User data from database:", user_data)
        connection.close()  

        if not username or not password:
            print("Korisničko ime ili lozinka nedostaju.")
            return paramiko.AUTH_FAILED
        
        if user_data:
            stored_username, stored_password_hash = user_data
            #print("Received username:", username)
            #print("Received password hash:", hashlib.sha256(password.encode()).hexdigest())
            #print("Stored username:", stored_username)
            #print("Stored password hash:", stored_password_hash)
            if username == stored_username and hashlib.sha256(password.encode()).hexdigest() == stored_password_hash:
                print("Korisnik uspješno autentificiran.")
                return paramiko.AUTH_SUCCESSFUL
        print("Neuspješna autentifikacija.")
        return paramiko.AUTH_FAILED



#Class that handles connection from client to the server
class Connection:
    def __init__(self, conn, addr):
        try:
            self.addr = addr
            self.transport = paramiko.Transport(conn)
            self.transport.add_server_key(paramiko.RSAKey.generate(2048))
            self.transport.start_server(server=ParamikoServer())
            self.channel = self.transport.accept(timeout=1)
        except Exception as e:
            self.addr = False
            print("Error initializing connection:", e)

    def __del__(self):
        if self.channel is not None:
            self.channel.close()
        if self.transport is not None:
            self.transport.close()

    def send_data(self, data):
        try:
            msg = str(data[1]) if not isinstance(data[1], str) else data[1]
            if msg.startswith('Error'):
                self.channel.send(self.pack_output({"success": False, "error_msg": msg}))
            elif msg is False:
                self.channel.send(self.pack_output({"success": False, "error_msg": "There was an error in your command."}))
            elif data[0] == "Select_command":
                self.select_protocol(msg)
            else:
                self.channel.send(self.pack_output({"success": True, "result": msg}))
        except (BrokenPipeError, ConnectionResetError, OSError) as e:
            print("[-] Socket connection lost:", e)
            self.cleanup_channel()  # Optional: shut down the channel cleanly here
        except Exception as e:
            print("[-] Failed to send data:", e)

    def recv_data(self):
        try:
            raw = self.channel.recv(1024)
            if not raw:
                print("[*] Client disconnected (no data)")
                return False
            data = self.unpack_input(raw)
            if isinstance(data, dict):
                return data.get("command") or data.get("continue", False)
            return False
        except (ConnectionResetError, OSError) as e:
            print("[-] recv_data socket error:", e)
            return False
        except Exception as e:
            print("[-] Failed while unpacking data:", e)
            return False

    def pack_output(self, out):
        return json.dumps(out)

    def unpack_input(self, inp):
        return json.loads(inp)

    def select_protocol(self, table):
        if not isinstance(table, str):
            print("Invalid table format")
            return

        l = table.splitlines()
        if not l:
            print("Empty table received")
            return

        if len(l) > n:
            header = [l.pop(0)]
            for i in range(0, len(l), n):
                endrow = min(i + n, len(l))
                data = {
                    "startrow": i,
                    "endrow": endrow,
                    "max": len(l),
                    "end": endrow == len(l),
                    "result": '\n'.join(header + l[i:endrow]),
                    "success": True,
                    "packed_data": True
                }
                try:
                    self.channel.send(self.pack_output(data))
                    print(f"Sent {endrow}/{len(l)} rows to {self.addr[0]}")
                except Exception as e:
                    print("[-] Failed to send data:", e)
                    break

                res = self.recv_data()
                if not res:
                    print("[-] Interrupted by client or socket closed.")
                    break
        else:
            data = {
                "rows": len(l) - 1,
                "result": table,
                "success": True
            }
            try:
                self.channel.send(self.pack_output(data))
            except Exception as e:
                print("[-] Failed to send data:", e)

    def is_alive(self):
        return self.channel is not None and self.transport.is_active()

    def cleanup_channel(self):
        try:
            self.channel.shutdown(socket.SHUT_RDWR)
            self.channel.close()
        except Exception as e:
            print("[-] Error during channel cleanup:", e)
