#!/usr/bin/env bash

echo "install gcc 8.3 begin"
yum install vim -y
yum install dos2unix -y
yum install readline-devel ncurses-devel -y
yum install mysql-devel -y
yum install libuuid-devel -y
yum install install autoconf automake libtool -y
yum install centos-release-scl -y
yum install devtoolset-8-toolchain -y
scl enable devtoolset-8 bash
echo "source /opt/rh/devtoolset-8/enable" >> ~/.bash_profile
source ~/.bash_profile

echo "install gcc 8.3 ok"

tar -vxf cmake-3.15.1.tar.gz
cd cmake-3.15.1/
./configure
make && make install
cd ..
rm cmake-3.15.1 -rf
echo "install cmake 3.15 ok"

tar -vxf jemalloc-5.2.0.tar.bz2
cd jemalloc-5.2.0/
./configure
make && make install
cd ..
rm jemalloc-5.2.0 -rf

tar -vxf protobuf-3.10.0.tar.gz
cd protobuf-3.10.0/
./autogen.sh
./configure
make && make install
cd ..
rm protobuf-3.10.0 -rf


echo "add lddconfig"

echo "/usr/lib/" >> /etc/ld.so.conf
echo "/usr/lib64/" >> /etc/ld.so.conf
echo "/usr/local/lib/" >> /etc/ld.so.conf
echo "/usr/local/lib64/" >> /etc/ld.so.conf
ldconfig

echo "ldconfig over"
