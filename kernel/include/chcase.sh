#!/bin/bash

for file in `ls | grep '[A-Z]'` 
do
str=`echo $file|tr 'A-Z' 'a-z'`
mv $file $str
done 
