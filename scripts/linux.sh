#!/bin/bash
cd $(dirname $0)/.. # go to root directory
mkdir -p result/dambreak/
./build/mps input/dambreak/settings.yml 2>result/dambreak/error.log # run simulation
