FROM gnuhub/base-docker
MAINTAINER Stallman Wang "gnuhub@gmail.com"

ADD /docker/etc/apt/sources.list /etc/apt/sources.list
ADD /docker/etc/apt/trusted.gpg /etc/apt/trusted.gpg
RUN sudo apt-get update
RUN apt-get install -y --force-yes build-essential
RUN apt-get install -y m4
RUN apt-get install -y flex
RUN apt-get install -y bison
RUN apt-get install -y gawk
RUN apt-get install -y texinfo
RUN apt-get install -y autoconf
RUN apt-get install -y libtool
RUN apt-get install -y pkg-config

RUN apt-get install -y openssl 
RUN apt-get install -y curl 
RUN apt-get install -y libreadline6 
RUN apt-get install -y git 
RUN apt-get install -y zlib1g 
RUN apt-get install -y autoconf 
RUN apt-get install -y automake 
RUN apt-get install -y libtool 
RUN apt-get install -y imagemagick 
RUN apt-get install -y make
RUN apt-get install -y tree
RUN apt-get install -y gdb
RUN apt-get install -y libopencv-dev libtesseract-dev git cmake build-essential libleptonica-dev
RUN apt-get install -y liblog4cplus-dev libcurl3-dev
RUN apt-get install -y beanstalkd
RUN apt-get install -y wget
