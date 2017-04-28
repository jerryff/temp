#!/bin/bash
filelist=`ls /home/vagrant/CRC/traces/`
for file in $filelist
do 
filetail=`echo $file  | cut -d . -f4`
if ["$filetail"x="trace"x]
then
	echo $file
fi
done