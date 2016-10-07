#!/bin/sh

echo "This is an autoscript to run test for the Apriori algorithm coded by Pei Xu."
echo "It will test the following combaination of parameters:\n"
echo "\tMINSUP: {15, 20, 30, 50, 100, 500, 1000}"
echo "\tminconf: {0.8, 0.9, 0.95}"
echo "\thfrange: {5, 10, 20}"
echo "\tmaxleafsize: {10, 50, 100}\n"

#MINSUP[7]=15
#MINSUP[6]=20
MINSUP[5]=30
MINSUP[4]=50
MINSUP[3]=100
MINSUP[2]=500
MINSUP[1]=1000

minconf[0]=0.8
minconf[1]=0.9
minconf[2]=0.95

hfrange[0]=5
hfrange[1]=10
hfrange[2]=20

maxleafsize[0]=10
maxleafsize[1]=50
maxleafsize[2]=100

run_test() {
    while
        echo "\nInput data file name: "
        read data_file
    do
        if [ ! -f "$data_file" ]; then
            echo "No File Exists."
        else
            break
        fi
    done
    for s in "${MINSUP[@]}"
    do
        for c in "${minconf[@]}"
        do
            for h in "${hfrange[@]}"
            do
                for m in "${maxleafsize[@]}"
                do
                    echo "\nBegin testing combination of ${s}-${c}-${h}-${m}"
                    echo "command: ./hcrminer ${s} ${c} ${data_file} ${logfile} ${h} ${m}"
                    logfile="./log/result-${s}-${c}-${h}-${m}-$(date +"%T")"
                    ./hcrminer ${s} ${c} ${data_file} ${logfile} ${h} ${m}
                done
            done
        done
    done
}

while
    echo "\nInput [Y]/[N] to start or exit test: "
    read inp
do
    if [ "$inp" == "Y" ] || [ "$inp" == "y" ]; then
        break
    elif [ "$inp" == "N" ] || [ "$inp" == "n" ]; then
        exit 1
    fi
done

run_test
