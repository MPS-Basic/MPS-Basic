# set variables
$projectName = "dambreak"
$inputDir = "input/" + $projectName
$settingFile = $inputDir + "/settings.yml"
$outputDir = "result/" + $projectName
$errorLogFile = $outputDir + "/error.log"
$consoleLogFile = $outputDir + "/console.log"

# go to root directory so that we can activate the script from any directory
cd (get-item $PSScriptRoot).parent.FullName

# create or clean output directory
New-Item -ItemType Directory -Path $outputDir -Force
# Force: Update the directory if already exists

# run simulation
./build/mps.exe --setting $settingFile --output $outputDir 2> $errorLogFile | Tee-Object -FilePath $consoleLogFile