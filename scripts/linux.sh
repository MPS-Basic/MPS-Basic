#!/bin/bash
cd $(dirname $0)/.. # go to root directory
mkdir -p result/dambreak/ # remove all folders/files in result/dambreak
./build/mps input/dambreak/settings.yml 2> result/dambreak/error.log | tee result/dambreak/console.log # run simulation
