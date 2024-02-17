# set root path so that we can activate the script from any directory
$rootPath = (get-item $PSScriptRoot).parent.FullName 

Get-ChildItem $rootPath/result/dambreak -Include *.* -Recurse | del # remove all folders/files in result/dambreak
.$rootPath/build/mps.exe $rootPathinput/dambreak/settings.yml 2> $rootPath/result/dambreak/error.log | Tee-Object -FilePath "result/dambreak/console.log" # run simulation