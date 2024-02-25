#!/bin/bash
# Shebang: The first row is for running the script with bash, not other shell like sh

# set variables
projectName="dambreak"
inputDir="input/${projectName}"
settingFile="${inputDir}/settings.yml"
outputDir="result/${projectName}"
errorLogFile="${outputDir}/error.log"
consoleLogFile="${outputDir}/console.log"

# go to root directory so that we can activate the script from any directory
cd $(dirname $0)/..
echo "Current directory: $(pwd)" | tee ${consoleLogFile}

# create and clean output directory
mkdir -p ${outputDir}
# -p, --parent: create parent directories if not exist
rm -rf ${outputDir}*
# -r, --recursive: remove directories and their contents
# -f, --force: ignore nonexistent files and arguments, never prompt
echo "Output directory cleaned: ${outputDir}" | tee --append ${consoleLogFile}

# run simulation
./build/mps --setting ${settingFile} 2> ${errorLogFile} | tee --append ${consoleLogFile}
