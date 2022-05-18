#ubuntu image
FROM ubuntu

#install required files
RUN apt-get update -y
RUN apt-get install git build-essential swig make doxygen python2.7 dos2unix libffi-dev libssl-dev curl python3 python3-pip -y
RUN curl https://bootstrap.pypa.io/pip/2.7/get-pip.py -o get-pip.py
RUN python2.7 get-pip.py

#installing latex takes some time, comment out the lines 12 and 13 if you do not plan to compile documentation(make doc)
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
RUN apt-get install texlive -y
RUN apt-get install texlive-latex-extra -y
RUN pip3 install virtualenv
RUN pip3 install paramiko
RUN pip3 install pyparsing==2.4.7
RUN pip3 install configparser
RUN pip3 install langdetect

RUN mkdir -p /usr/akdb
VOLUME ["/usr/akdb"]
WORKDIR /usr/akdb
RUN cd /usr/akdb
