#!/bin/bash
# My first script

for (( i=1; i <= 51; i++ ))    
do
	
	if (($i%7==0 )); 
		then
		echo " usao " $i
		cd ..
		cd src
		make clean
		make
		cd ..
		cd bin
	fi
	echo "test broj :" $i
	./akdb test $i

done