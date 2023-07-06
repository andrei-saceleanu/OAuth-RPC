#!/bin/bash


for file in $(ls *.c)
do
	mv ${file} ${file%.c}.cpp
done
