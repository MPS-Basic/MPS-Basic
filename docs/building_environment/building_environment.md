# Building Environment

[TOC]

This page explains how to build environment for executing MPS-Basic.

## What You Need
There are 5 things you need.

Component | Explanation | Examples
--- | --- | ---
Editor/IDE | to write (, build and execute) programs. | Vim, Emacs, Visual Studio Code, Visual Studio, ...
Compiler | to compile the program | Clang, GCC, ...
Libraries | what you include at the beginning of the code | C++ Standard Library (like `<vector>`), OpenMP, ...
Build System | automatically build executable files based on setting files | Make, MSBuild, Ninja, ...
CMake | automatically generate input files for user defined Build System | ---

### Editor/IDE
The simplest editors, such as Vim and Emacs, specialize in editing programs.
To build and run a program, you need to open a terminal in a separate window.
In contrast, IDEs have advanced features that are not limited to program editing
and can automatically build and run programs.
IDE stands for Integrated Development Environment.

Examples:
- Simple editors
  - Vim
  - Emacs
- IDEs
  - Visual Studio Code
  - Xcode
  - Eclipse

Simple editors are so simple that it's inconvenient (unless you're a pro),
while IDEs are so sophisticated that it's difficult for beginners to use.
Visual Studio Code is somewhere in between.
VS Code allows you to open a terminal within it's window
and to introduce extensions to automate the build and execution of programs.
***If you're new, use Visual Studio Code***.

### Compiler and Libraries
Sophisticated functionalities in `c++` such as `std::string`,  `std::vector`,
and many more are provided as a C++ Standard Library.
And you have to include the library at the beginning of the code to use it,
like `#include <string>` or `#include <vector>`.
Also, if you want to use OpenMP to parallelize your code, you need a library for that.
So if you want to use such functionalities in your program,
you have to download libraries in addition to the compiler.

Commonly used libraries are usually downloaded at the same time as the compiler,
and the compiler and the libraries are automatically linked
(meaning the compiler knows which libraries to use).
But sometimes you have to let the compiler know the location of libraries
when compiler doesn't come with libraries or you want to use additional libraries
that doesn't come with the compiler.

### Build System and CMake
#### Why Use Build System?
When we have a single source file `main.cpp`, the compile is quite easy.
```bash
g++ main.cpp -o main
```
This command creates executable file named `main`.

Things get complicated when we have more files.
Let's assume we have multiple source files (`main.cpp`, `file1.cpp` and `file2.cpp`),
and header files (`file1.hpp` and `file2.hpp`).
In this case, our command will be:
```bash
g++ -c main.cpp -o main.o
g++ -c file1.cpp -o file1.o
g++ -c file2.cpp -o file2.o
g++ main.o file1.o file2.o -o main
```
Now this is complicated.
If we want to add new files or change compiler options,
then it'll be something I don't want to imagine.

That's why we use a build system.
One of the most widely used build systems is `Make`.
@note
Other build systems besides `Make` include `MSBuild` (Microsoft Build Engine) and `Ninja`.

Let's see how we compile the code using `Make`.
First, you need to prepare a file named `Makefile`:
```Makefile
SRC = main.cpp file1.cpp file2.cpp # source files
OBJ = $(SRC:.cpp=.o) # object files

CXX = g++ # compiler
CXXFLAGS = -Wall -std=c++11 # compile options (flags)

main: $(OBJ)
  $(CXX) $(CXXFLAGS) -o main $(OBJ) # command to generate exe file

# How the object files are related
main.o: main.cpp file1.hpp file2.hpp
file1.o: file1.cpp file1.hpp
file2.o: file2.cpp file2.hpp

# clean
clean:
  rm -f $(OBJ) main
```
You don't have to understand this, but I hope you get the idea that
you specify all the settings (like compiler and options),
and `Make` generates the command for you.
Now that you have your `Makefile`, all you have to do is:
```bash
make
```
Thats's it.

The advantages of using make can be summarized as follows.
- You don't have to type complicated compile command.
- When you have new files, you can just add them to `Makefile`.
- When you want to change compile options, you can just change `Makefile`.

@remark
Plus, `Make` only re-compile files that have been changed.
This makes compile process much faster when there are many files to deal with.

#### Why Use CMake?
So it's better to use a Build System such as `Make` to compile.
Then why use `CMake`?

There are some problems when using a specific build system.
- There are multiple build systems, and they require their own setting files.
  Your co-worker may want to use `MSBuild` or `Ninja` instead of `Make`,
  and do you want to prepare setting files for all of them?
- Input files for build systems are usually complicated,
  and they use their own syntax that you have to remember.
  Like the above `Makefile` example looked difficult, right?
So using a specific build system is problematic especially when you are working
as a team.

Now `CMake` solves these problems.
`CMake` is a tool for generating input files that user-defined build system
will use to generate executable file.
To use `CMake`, you need a file named `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.10)

project(MyProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(main main.cpp file1.cpp file2.cpp)
```
That's it! How easy it is compared to the above example of `Makefile`!
Now what you do is:
```bash
cmake -S . -B build
```
Argument `-S` specifies the location of `CMakeLists.txt`,
and `-B` specifies the location where the input files for build system will be generated.
`CMake Generator` is responsible for generating input files for build system,
so you specify which generator you want to use based on your build system using `-G` option.
For example,
```bash
cmake -S . -B build -G "Unix Makefiles"
```
will create input files that `Make` uses (which is `Makefile`).
Likewise,
```bash
cmake -S . -B build -G "Visual Studio 17 2022"
```
generates Visual Studio 2022 project files that `MSBuild` uses, and
```bash
cmake -S . -B build -G "Ninja"
```
generates `build.ninja` files for `Ninja` to use.

Now you can see that by introducing `cmake` you can make your project platform-independent,
meaning your project can be built in Windows, Mac, Linux, or any other platforms,
just by a single and easy `CMakeLists.txt` file.

#### Workflow
Finally let's see how we compile the code using `CMake` and build system.
Here, let's assume we use `Make` as a build system.
```bash
cmake -S . -B build -G "Unix Makefiles" # Generate build/Makefile
cd build # Move down to build directory
make # Use Makefile and generate exe file
```
If you want to use `MSBuild`, `Ninja` or any other build system,
you just change the `-G` option at the first command and the third command as appropriate.

@tips
`-G` flag is optional. If not provided, `CMake` will use default generator which is determined
based on your platform.
Type `cmake --help` to see your default generator.


A simpler approach would be to do this:
```bash
cmake -S . -B build -G "Unix Makefiles"
cmake --build build
```
This time we didn't call `Make`, but `CMake` calls it for us based on the
input files generated in `build` directory.
This way, you don't have to remember the commands to call the build system
you want to use, and cmake handles it all for you.

## Detailed Information for Each Platform
- \subpage windows
- \subpage mac
- \subpage linux