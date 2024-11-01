#!/bin/bash
# Runner that separates the result into a new folder with a timestamp

if [ -z "$1" ]; then
    echo "Usage: $0 <projectName> [--note <note>]"
    echo "Following directory will be created: result/<timestamp>-<projectName>-<note>"
    exit 1
fi

projectName="$1"
inputDir="input/${projectName}"
settingFile="${inputDir}/settings.yml"

note=""
if [ "$2" == "--note" ] && [ -n "$3" ]; then
    note="$3"
fi

timestamp=$(date +%Y%m%d%H%M%S)
outputDir="result/${timestamp}-${projectName}-${note}"
errorLogFile="${outputDir}/error.log"
consoleLogFile="${outputDir}/console.log"

# go to root directory so that we can activate the script from any directory
cd $(dirname $0)/..

# create output directory if not exist
mkdir -p ${outputDir}

# run simulation
./build/mps --setting ${settingFile} --output ${outputDir} 2> ${errorLogFile} | tee ${consoleLogFile}
