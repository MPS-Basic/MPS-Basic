#!/bin/bash 
# Shebang: The first row is for running the script with bash, not other shell like sh

# go to root directory so that we can activate the script from any directory
cd $(dirname $0)/.. 

mkdir -p result/dambreak/ # remove all folders/files in result/dambreak
./build/mps input/dambreak/settings.yml 2> result/dambreak/error.log | tee result/dambreak/console.log # run simulation
