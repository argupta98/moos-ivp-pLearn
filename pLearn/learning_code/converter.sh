#!/bin/bash

COUNT=0
fname="results_2"
out_name="test.csv"
#-----------------------------------------------------
#        Evan and Felix Attacking
#-----------------------------------------------------    
echo "converter making test data folder"
mkdir results_dataset
alright=1
count=0
while [[ $alright == 1 ]] ; do
    if [[ -d $fname/results_EA_FA/test_$count/ ]]
    then
        python log_converter.py $fname/results_EA_FA/test_$count/felix.alog results_dataset/data_$COUNT.csv attack
	COUNT=$((COUNT+1))
	python log_converter.py $fname/results_EA_FA/test_$count/evan.alog results_dataset/data_$COUNT.csv attack
        count=$((count+1))
	COUNT=$((COUNT+1))
    else
        alright=0
    fi
done

alright=1
count=0
while [[ $alright == 1 ]] ; do
    if [[ -d $fname/results_ED_FD/test_$count/ ]]
    then
        python log_converter.py $fname/results_ED_FD/test_$count/felix.alog results_dataset/data_$COUNT.csv defend
	COUNT=$((COUNT+1))
	python log_converter.py $fname/results_ED_FD/test_$count/evan.alog results_dataset/data_$COUNT.csv defend
        count=$((count+1))
	COUNT=$((COUNT+1))
    else
        alright=0
    fi
done

alright=1
count=0
while [[ $alright == 1 ]] ; do
    if [[ -d $fname/results_EA_FD/test_$count/ ]]
    then
        python log_converter.py $fname/results_EA_FD/test_$count/felix.alog results_dataset/data_$COUNT.csv defend
	COUNT=$((COUNT+1))
	python log_converter.py $fname/results_EA_FD/test_$count/evan.alog results_dataset/data_$COUNT.csv attack
        count=$((count+1))
	COUNT=$((COUNT+1))
    else
        alright=0
    fi
done

alright=1
count=0
while [[ $alright == 1 ]] ; do
    if [[ -d $fname/results_ED_FA/test_$count/ ]]
    then
	python log_converter.py $fname/results_ED_FA/test_$count/felix.alog results_dataset/data_$COUNT.csv attack
	COUNT=$((COUNT+1))
	python log_converter.py $fname/results_ED_FA/test_$count/evan.alog results_dataset/data_$COUNT.csv defend
        count=$((count+1))
	COUNT=$((COUNT+1))
    else
        alright=0
    fi
done

#merge all the generated files into 1 .csv

alright=1
count=0
while [[ $alright == 1 ]] ; do
    if [[ -f results_dataset/data_$count.csv ]]
    then
	cat results_dataset/data_$count.csv >> results_dataset/$out_name
	rm results_dataset/data_$count.csv
        count=$((count+1))
    else
        alright=0
    fi
done 
