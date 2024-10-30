#!/bin/bash
# Runner that seplates the result into a new folder with a timestamp

# set variables
projectName="dambreak"
inputDir="input/${projectName}"
settingFile="${inputDir}/settings.yml"

timestamp=$(date +%Y%m%d%H%M%S)
outputDir="result/${timestamp}-${projectName}"
errorLogFile="${outputDir}/error.log"
consoleLogFile="${outputDir}/console.log"

# go to root directory so that we can activate the script from any directory
cd $(dirname $0)/..

# create output directory if not exist
mkdir -p ${outputDir}

# run simulation
./build/mps --setting ${settingFile} --output ${outputDir} 2> ${errorLogFile} | tee ${consoleLogFile}
