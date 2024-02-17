# set root path so that we can activate the script from any directory
$rootPath = (get-item $PSScriptRoot).parent.FullName 

echo "Root path: $rootPath"
del $rootPath\result\dambreak\*.prof
del $rootPath\result\dambreak\*.vtu
.$rootPath\build\mps.exe 2>$rootPath\result\error.log