# set variables
$projectName = "dambreak"
$inputDir = "input/" + $projectName
$settingFile = $inputDir + "/settings.yml"
$outputDir = "result/" + $projectName
$errorLogFile = $outputDir + "/error.log"
$consoleLogFile = $outputDir + "/console.log"

# go to root directory so that we can activate the script from any directory
cd (get-item $PSScriptRoot).parent.FullName
Write-Output "Current directory: $PWD" | Tee-Object -FilePath $consoleLogFile

if (!(Test-Path -Path $outputDir)) {
  # create output directory if not exist
  New-Item -ItemType Directory -Path $outputDir
  Write-Output "Output directory created: $outputDir" | Tee-Object -FilePath $consoleLogFile -Append
}
else {
  # clean output directory if exist
  Get-ChildItem $outputDir | Remove-Item -Recurse
  Write-Output "Output directory cleaned: $outputDir" | Tee-Object -FilePath $consoleLogFile -Append
}

# run simulation
./build/mps.exe --setting $settingFile --output $outputDir 2> $errorLogFile | Tee-Object -FilePath $consoleLogFile -Append