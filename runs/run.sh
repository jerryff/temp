#!/bin/bash
filelist=`ls /home/vagrant/CRC/traces/`
for file in $filelist
do 
filetail=`echo $file  | cut -d . -f4`
if [ "$filetail"x = "trace"x ]
then
	filename=`echo $file  | cut -d . -f2`
	../bin/CMPsim.usetrace.64 -threads 1 -t ../traces/${file} -o ${filename}.stats -cache UL3:1024:64:16 -LLCrepl 0 
fi
done