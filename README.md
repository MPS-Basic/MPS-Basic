# MPS-Basic

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
```bash
cmake -S . -B build # Generate build system
# -S: Source tree. The location of CMakeLists.txt
# -B: Build tree. Directory for storing products for builds
cmake --build build # Execute build
```

> [!NOTE]
> Install dependencies as shown above for the first build
> ```bash
> git submodule update --init eigen
> ```

### Execution
#### Windows
```powershell
New-Item -ItemType Directory -Path result/dambreak -Force # create or clean output directory
./build/mps.exe --setting input/dambreak/settings.yml --output result/dambreak 2> result/dambreak/error.log | Tee-Object -FilePath "result/dambreak/console.log" # run simulation
```

#### Linux/Mac
```bash
mkdir -p result/dambreak/ # create output directory if not exist
rm -rf result/dambreak/* # remove old output files if exit
./build/mps --setting input/dambreak/settings.yml --output result/dambreak 2> result/dambreak/error.log | tee result/dambreak/console.log # run simulation
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
