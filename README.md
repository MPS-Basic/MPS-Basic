# BasicCode

Modernized Moving Particle Semi-implicit/Simulation method code written in C++.

## installation
### Requirements
- Git
- cmake (newer than 3.1.4)
- C++ 17 compiler

### Build step
First, install dependencies.
```bash
git submodule update --init eigen
```
Second, configure cmake and build.
```bash
mkdir build
cd build
cmake -G "Unix Makefiles" .. # if you want to use other systems you chan change
make # for makefile
```

## Usage


## Support
If you have question, ask authors, please.

## Roadmap
- Basic MPS simulation code of C++
- input/output


## Contributing
### Coding guide
- C++ standard: C++17
- Do **NOT** use Exceptions
- Macros
    - Use macros only where they are literally needed.

## License
MIT Licence

## Project status
This project is under development.