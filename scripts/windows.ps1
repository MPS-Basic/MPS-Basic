# go to root directory so that we can activate the script from any directory
cd (get-item $PSScriptRoot).parent.FullName 

Get-ChildItem result/dambreak -Include *.* -Recurse | del # remove all folders/files in result/dambreak
./build/mps.exe input/dambreak/settings.yml 2> result/dambreak/error.log | Tee-Object -FilePath "result/dambreak/console.log" # run simulation