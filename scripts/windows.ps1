$rootPath = (get-item $PSScriptRoot).parent.FullName
del $rootPath\result\dambreak\*.prof
del $rootPath\result\dambreak\*.vtu
$rootPath\build\mps.exe 2>$rootPath\result\error.log