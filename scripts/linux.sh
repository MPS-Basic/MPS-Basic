#!/bin/bash # Shebang: to run the script with bash, not other shell like sh

# go to root directory so that we can activate the script from any directory
cd $(dirname $0)/.. 

mkdir -p result/dambreak/
./build/mps input/dambreak/settings.yml 2>result/dambreak/error.log # run simulation
