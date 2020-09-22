#!/bin/bash


while getopts rR opt
do
	case $opt in
		r)
            rm CMakeCache.txt -f
            rm CMakeFiles -rf
            cmake ./
			make clean
			;;
		R)
            rm CMakeCache.txt -f
            rm CMakeFiles -rf
            cmake ./
			make clean
			;;
		*)
			;;
	esac
done

cpu=`cat /proc/cpuinfo| grep "processor"| wc -l`
make -j${cpu}

cd $root_dir
