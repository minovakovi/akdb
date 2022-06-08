# Kalashnikov DB  {#mainpage}
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/92d1b22c17054c36931b033bac1a7c52)](https://www.codacy.com/app/TheWildHorse/akdb?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mschatten/akdb&amp;utm_campaign=Badge_Grade)

AK (Автомат Калашникова) DB is a simple relational database management system 
developed by students of the Faculty of Organization and Informatics in 
Varaždin, Croatia.

The system is currently in alpha development status and NOT fully functional!
***
## Getting Started
To get you started you can simply clone the repository and install the dependencies:

#### Prerequisites
Ubuntu:
```
sudo apt-get install git build-essential swig make doxygen python-dev texlive-latex-recommended texlive-latex-extra
sudo pip install paramiko
```
Mac:

https://www.python.org/downloads/release/python-272/
```
brew install swig make doxygen mactex-no-gui graphviz
curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py
python2.7 get-pip.py
sudo pip install paramiko
```
#### Clone Kalashnikov DB
Clone the repository using [git][git]:

```
git clone https://github.com/AILab-FOI/akdb.git
cd akdb
```
#### Install Dependencies
```
sudo apt-get install python2.7
```
#### Setting up virtualenv
Install [virtualenv](http://docs.python-guide.org/en/latest/dev/virtualenvs/) via [pip](https://pip.pypa.io/en/stable/installing/):
```
pip install virtualenv
```
You can also use a Python interpreter of your choice.
```
cd my_project_folder
virtualenv -p /usr/bin/python2.7 venv
```
To begin using the virtual environment, it needs to be activated:
```
source venv/bin/activate
```
If you are done working in the virtual environment for the moment, you can deactivate it:
```
deactivate
```
***
### Build the Application
Build (builds bin/akdb, documentation in doc/ and swig interface)
```
cd src
make
make doc
make swig
```
### Run swig tests
Builds swig tests, but fails relAlgebraTests file for unknown reason. You can run tests without error from src/swig folder with instructions from readme in swig folder. 
```
make swig-tests

```
### Run all tests from akdb
if you want to run all tests from doctest and main.c
NOTES:
	sometimes akdb crashes so u have to make clean and build project again
	some test functions are naughty so they access forbidden memory blocks and crash file :(

Go to src/swig and after that, run:
```

python allTests.py

```
Go to src/swig
### Run the Application
```
cd bin/
./akdb help
./akdb <option>
```
### Clean project
Cleaning the project from files generated while compiling use this:
```
make clean
make clean-d
```

### Find Croatian comments
See if there are some leftover Croatian comments in the code:
```
make comments
```

## Directory Layout
## Testing
### Main engine
```
cd bin
./akdb alltest
./akdb test
```
On first use DB file will be initialized. In case it hangs press 1 and Enter
(don't ask why).

#### Server (read README in src/srv)
```
cd src/srv
python server.py
```

#### Client
```
cd src/srv
python akdb_cli.py
```

## Docker

This project can be built and run inside docker container. This is a great alternative to linux VM.
All prerequisites will be downloaded and installed automatically. You can develop in you native environment and test and compile code inside the container.
When you start docker container with akdb, akdb host directory will be automatically mapped in container directory. This means that all the changes you make on your machine are automatically seen inside the container where you can immediately compile and test the code

#### Requirements

For docker to work you have to have docker and make installed on local machine.
* [Docker](https://www.docker.com/) 
* [Make](https://www.gnu.org/software/make/)

**Windows** users also need to turn on drive sharing in docker settings.
To do that: open docker settings -> shared drives -> check the drive on which akdb is located -> apply

#### Commands

Install docker container. This will take some time because all prerequisites above will be downloaded and setup automatically.
For more informations check Dockerfile.
``` 
make docker-install 
```

Run akdb in docker container - Windows OS
``` 
make docker-win 
```

Run akdb in docker container - Mac
``` 
make docker-mac
```

