# Execution

[TOC]

## Requirements
### Execution
- Git
- cmake (newer than 3.9)
- C++ 17 compiler
- OpenMP 5.0 and above (optional)

### Development
- Doxygen and Graphviz (optional, for building documents)

### Dependencies
- Install dependencies as follows before the first build.
	```bash
	git submodule update --init eigen
	```

## Command Line Execution

@note It's easier to do it in the VS code as shown later,
but it's better to do it in command line for your understanding.

### Build
```bash
cmake -S . -B build # Generate build system
# -S: Source tree. The location of CMakeLists.txt
# -B: Build tree. Directory for storing products for builds
cmake --build build # Execute build
```

- You don't have to make `build` directory (`mkdir build`) 
  and move to the directory (`cd build`), thanks to the `-B build` option. 
  The `build` directory will be automatically generated if you don't have one.

### Execution
#### Windows
```powershell
Get-ChildItem result/dambreak -Include *.* -Recurse | del # remove all folders/files in result/dambreak
./build/mps.exe input/dambreak/settings.yml 2> result/dambreak/error.log | Tee-Object -FilePath "result/dambreak/console.log" # run simulation
```
The execution command consists of three parts.
1. Pass the location of the input file and execute.
	```bash
	./build/mps.exe input/dambreak/settings.yml
	```
2. Change standard error output to the specified file.
	```bash
	2> result/dambreak/error.log
	```
3. Save the console output to the specified file.
	```bash
	Tee-Object -Filepath "result/dambreak/console.log"
	``` 

#### Linux
```bash
mkdir -p result/dambreak/ # remove all folders/files in result/dambreak
./build/mps input/dambreak/settings.yml 2> result/dambreak/error.log | tee result/dambreak/console.log # run simulation
```

#### Note
(1). Standard (Error) Output

The output of a c++ program includes standard output and standard error output.
The standard output comes from `std::cout` or `printf()`, 
while the standard error output comes from `std::cerr` or `fprintf(stderr, )`.
By default, both the standard output and the standard error output will be shown in the console.
Changing the output destination is called "redirect", 
and can be accomplished with the `>` command.

For example, the standard output of the command below 
will be written to `result.log`. 
The standard error output will be shown in the console as usual.
```bash
./test.exe > resut.log
```

In the next example, the standard output will be written to `result.log`, 
and the standard error output will be written to `error.log`.
```bash
./test.exe 1> result.log 2> error.log
# "1" can be omitted like below.
# ./test.exe > result.log 2> error.log
```

(2). `tee` command

`tee` command allows us to show the standard output in the console
and save them into a file at the same time.

> it sends the output of a command in two directions (like the letter T)
> 
> <cite>[Microsoft Docs](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.utility/tee-object?view=powershell-7.4&viewFallbackFrom=powershell-7.1)</cite>

Foe example, the standard output of the command below will be shown in the console 
and will be written to `result.log` at the same time.
```bash
./test.exe | tee result.log
```
## VS Code Execution
- Before you begin, install CMake Tools extension.

	![](fig/CMake_Tools.png)

### Build
1. Open Cmake tab.
2. Change `Configure` to `Release`
3. Execute `Build`

	![](fig/release_build.png)

### Execution
- Execute in the same way as the command line.
- Script files are prepared in the `scripts` folder. You can just call it.
- Don't use the `Launch` button on the CMake tab. It will launch the program in a wrong directory, and it won't delete existing files.

#### Windows
```powershell
./scripts/windows.ps1
```

#### Linux
```sh
./scripts/linux.sh
```

#### Note
- You can configure `tasks` to do it easily.

1. Create `.vscode/tasks.json` file.

	![](fig/tasks.png)

1. An example for Windows:
	```json
	{
		"tasks": [
    	{
      	"label": "Execute",
      	"type": "shell",
      	"command": "./scripts/windows.ps1",
      	"options": {
        	"cwd": "${workspaceFolder}"
      	},
      	"group": {
        	"kind": "test",
        	"isDefault": true
      	}
    	},
  	],
  	"version": "2.0.0"
	}
	```

1. Open Command Palette (`Ctrl + Shift + P` or `F1`). Search and execute `Tasks: Run Test Task`. It does the same thing as calling the script in the terminal.

	![](fig/tasks_2.png)

- You can set a keyboard shortcut to run test task. 
  To do so, open Command Palette and move on to `Preferences: Keyboard Shortcuts`.
  Search `Tasks: Run Test Task` and set any keybinding you want. 
  `Ctrl + Shift + T` is a suggestion of the writer.

	![](fig/tasks_3.png)

## Debugging
1. Open Cmake tab.
2. Change `Configure` to `Debug`
3. Execute `Debug`

	![](fig/debug.png)

- Unlike the `launch` button in the previous section, 
  the `debug` button will execute the code in the correct directory. 
  Also, there is no need to delete existing files when debugging. 
  So you can just press it this time.
