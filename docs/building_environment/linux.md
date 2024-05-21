\page linux Linux

[TOC]

There are two choices to make.
- Compiler/libraries
- Build system

Contrary to [Windows](@ref windows), Compiler and Libraries are combined in one choice.

## Simple Answer
- Use [Clang][Clang] as a compiler
- Use [Ninja][Ninja] as a build system

### Steps to Build Environment

First install CMake, Clang and Ninja.

For Ubuntu 22.04 and later, please run the following command:
```bash
sudo apt install cmake clang libomp-dev ninja-build
```

(If you're using VSCode,) add the following to ```.vscode/settings.json```.
Create one if you don't have.
This will make CMake to use Ninja as a generator
(meaning VSCode will add ```-G "Ninja"``` to the command)
```json
"cmake.generator": "Ninja",
```

Now you can build and execute following [Execution page](execution.md).
Make sure you choose ```Clang``` as a compiler in the CMake tab in Visual Studio Code.

## Detailed Explanations for Compiler and Libraries
- [Clang][Clang]: ***Recommended***
- [GCC][GCC]
- [OneAPI][OneAPI]
- [AOCC][AOCC]

To enable multi-threading, OpenMP version 5 is required.
[OpenMP official site](https://www.openmp.org/resources/openmp-compilers-tools/) provides a list of compilers that support OpenMP 5.

### Clang
[Clang][Clang] is easy to install, and recommended in this MPS-Basic project.
To enable OpenMP, OpenMP runtime (`libomp-dev` in debian/Ubuntu) is required.

### GCC
[GCC][GCC] is a traditional compiler that is widely installed.
However, OpenMP support is not as good as Clang.
Since this project uses new features of OpenMP 5.0, we recommend Clang.

### OneAPI
[OneAPI][OneAPI] (formerly known as Intel compiler) is a suite of tools for Intel CPUs.
OpenMP 5.0 is supported from version 2021.1.

### AOCC
[AOCC][AOCC] (AMD Optimizing C/C++ Compiler) is a compiler for AMD CPUs.
OpenMP 5.0 is supported from version 4.2.

## Detailed Explanations for Build System
There are mainly three build systems that Windows users can choose.
- [Ninja][Ninja]: ***Recommended***
- [Make][Make]

### Ninja
[Ninja][Ninja] is easy to install and easy to use.
We recommend beginners to use this build system since it's simple and fast.

### Make
[Make][Make] is a traditional build system that is widely installed.
It's also easy to use, but `-jX` (`X`: the number of concurrent jobs) option is required to build in parallel.

[Clang]: https://clang.llvm.org/
[Ninja]: https://ninja-build.org/
[Make]: https://www.gnu.org/software/make/
[GCC]: https://gcc.gnu.org/
[OneAPI]: https://www.intel.com/content/www/us/en/developer/tools/oneapi/toolkits.html
[AOCC]: https://www.amd.com/en/developer/aocc.html