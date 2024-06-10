import textwrap
import getpass
from client import Client

def print_box(text):
    lines = textwrap.wrap(text, width=50)
    border = '+' + '-' * 52 + '+'
    print(border)
    for line in lines:
        print('| {:^50} |'.format(line))
    print(border)

def main():
    print_box("Testing username: testingUser")
    username = input("Please enter your username: ")

    print_box("Testing password: testingPass")
    password = getpass.getpass(prompt="Please enter your password: ")

    c = Client(username=username, password=password)
    c.start()

if __name__ == "__main__":
    main()
