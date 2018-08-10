#!/bin/bash

#-----------------------------------------------------
#       Script to Run Simulation and Extract Data
#-----------------------------------------------------    
echo "launching mission with both robots attacking"
cd ../simulation_engine
./clean.sh

./launch_reinforce.sh  4 &
last_pid=$!   
sleep 15 

echo "tester deploying robots"
cd shoreside
uPokeDB  targ_shoreside.moos DEPLOY_ALL=true MOOS_MANUAL_OVERRIDE_ALL=false RETURN_ALL=false STATION_KEEP_ALL=false
cd ..
#run simulation for some number of iterations or until tagged or flag captured
count=0
while [[ $count -lt 20 ]] ; do #20
    sleep 1
    count=$((count+1))
done

#kill the simulation
echo "simulator killing application"
kill -INT $last_pid > /dev/null
sleep 3

#kill hte moos application
echo "simulator killing the moos"
ktm
sleep 5

ktm
sleep 5

ktm
sleep 5

cd ../learning_code
args=("$@")
#make test folder and store data
echo "simulator making test result folder"
cd results
mkdir simulation_${args[0]}
cd ..

cd ../simulation_engine
#process log data to extract states, actions, and end states (Felix is the robot exhibiting learning behavior)
echo "simulator grepping log_felix data"
cd m200
cd LOG_FELIX_*
aloggrep L*.alog INP_STAT felix.alog
cd ../../..
mv simulation_engine/m200/LOG_FELIX_*/felix.alog learning_code/results/simulation_${args[0]}/simulation.alog


cd simulation_engine
#clean the test folder to remove old logs
echo "tester cleaning folder"
./clean.sh
sleep 2

cd ../learning_code
