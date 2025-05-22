# Use Ubuntu 22.04 LTS (Jammy) as the base image
FROM ubuntu:22.04

# Update APT and install software-properties-common (for add-apt-repository)
# Enable the 'universe' repo (needed for python2.7), then install required packages
RUN apt-get update && apt-get install -y --no-install-recommends software-properties-common \
    && add-apt-repository universe \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
        python2.7 git build-essential swig make doxygen python3-distutils python3-apt python3-dev python3-pip
RUN pip install paramiko
RUN mkdir -p /usr/akdb
VOLUME ["/usr/akdb"]
WORKDIR /usr/akdb
RUN cd /usr/akdb

