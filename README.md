# MPS-Basic

[![Documentation Badge](https://img.shields.io/badge/Documentation-blue)](https://mps-basic.github.io/MPS-Basic/index.html)
![](https://github.com/MPS-Basic/MPS-Basic/actions/workflows/build-cmake.yml/badge.svg)
![](https://github.com/MPS-Basic/MPS-Basic/actions/workflows/doxygen-pages.yml/badge.svg)

Modernized Moving Particle Semi-implicit/Simulation method code written in C++.

> [!NOTE]
> See [Documentation](https://mps-basic.github.io/MPS-Basic/index.html) for more information

## Requirements
### Execution
- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/) (newer than 3.12)
- C++ 17 compiler
- [VTK](https://vtk.org/) (newer than 9.0.0)
- OpenMP 5.0 and above (optional)

### Development
- [Doxygen](https://www.doxygen.nl/) and [Graphviz](https://graphviz.org/) (optional, for building documents)

### Dependencies
- [argparse](https://github.com/p-ranav/argparse)
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
- [fast-cpp-csv-parser](https://github.com/ben-strasser/fast-cpp-csv-parser)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)

#### Development
- [GoogleTest (only for testing)](https://github.com/google/googletest)

## Execution
### Build
1. Generate build system
	```bash
	cmake -S . -B build
	```
1. Execute build
	```bash
	cmake --build build
	```

### Execution
#### Windows
1. Create output directory if not exist
	```powershell
	New-Item -ItemType Directory -Path result/dambreak -Force
	```
1. Remove old output files if exist
	```powershell
	Remove-Item -Path $outputDir/* -Force -Recurse
	```
3. Run simulation
	```powershell
	./build/mps.exe --setting input/dambreak/settings.yml --output result/dambreak 2> result/dambreak/error.log | Tee-Object -FilePath "result/dambreak/console.log"
	```

#### Linux/Mac
1. Create output directory if not exist
	```bash
	mkdir -p result/dambreak/
	```
1. Remove old output files if exist
	```bash
	rm -rf result/dambreak/*
	```
1. Run simulation
	```bash
	./build/mps --setting input/dambreak/settings.yml --output result/dambreak 2> result/dambreak/error.log | tee result/dambreak/console.log
	```

### Checking Results
Result files will be written in `result/dambreak/vtu/***.vtu`.
Open these files in [ParaView](https://www.paraview.org/) to see the result.

## Build documents

> [!NOTE]
> Install dependencies as shown above for the first build
> ```bash
> git submodule update --init doxygen-awesome-css
> ```

```bash
doxygen Doxyfile
```
- You can see the documents in `doxygen/html/index.html`.
- If you want PDF files, you can use `make` command in the `doxygen/latex` directory (LaTeX is required).

## Support
Please ask the authors if you have any questions.

## Contributing
Read [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## License
MIT License

## Project status
This project is under development.
