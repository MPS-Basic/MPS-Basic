#!/bin/bash
cd $(dirname $0)/.. # go to root directory
mkdir -p result/dambreak/prof # delete prof files
mkdir -p result/dambreak/vtu # delete vtu files
./build/mps result/dambreak/settings.yml 2>result/dambreak/error.log # run simulation
