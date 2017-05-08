#!/bin/bash
filelist=`ls /home/vagrant/CRC/traces/`
n=0
for file in $filelist
do 
filetail=`echo $file  | cut -d . -f4`
if [ "$filetail"x = "trace"x ]
then
	filename=`echo $file  | cut -d . -f2`
	echo $n
	echo ${filename}
	time ../bin/CMPsim.usetrace.64 -threads 1 -t ../traces/${file} -o ../my2/${filename}.stats -cache UL3:1024:64:16 -LLCrepl 2 > ../output.txt
	n=`expr $n + 1`
fi
done