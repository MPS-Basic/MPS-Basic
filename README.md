# MPS-Basic

[![Documentation Badge](https://img.shields.io/badge/Documentation-blue)](https://mps-basic.github.io/MPS-Basic/index.html)
![](https://github.com/MPS-Basic/MPS-Basic/actions/workflows/build-cmake.yml/badge.svg)
![](https://github.com/MPS-Basic/MPS-Basic/actions/workflows/doxygen-pages.yml/badge.svg)

Modernized Moving Particle Semi-implicit/Simulation method code written in C++.

> [!NOTE]
> See [Documentation](https://mps-basic.github.io/MPS-Basic/index.html) for more information

## Requirements
### Execution
- Git
- cmake (newer than 3.9)
- C++ 17 compiler
- OpenMP 5.0 and above (optional)

### Development
- Doxygen and Graphviz (optional, for building documents)

### Dependencies
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)

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

> [!NOTE]
> Install dependencies as shown above for the first build
> ```bash
> git submodule update --init eigen
> ```

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

## I/O
### Input
To be written.

### Output
- The results are written in the following formats:
	- `result/prof`: Profile data
	- `result/vtu`: VTK data

#### Profile data
- The profile data is in the following format:

```prof
0  // time
627  // number of particles
3 -0.1 -0.1 0 0 0 0  // type x y z u v w
3 -0.1 -0.075 0 0 0 0
3 -0.1 -0.05 0 0 0 0
...
```


## Build documents
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
