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
Please ask the authors if you have any questions.

## Roadmap
- Basic MPS simulation code of C++
- input/output


## Contribution
### Coding guide
- C++ standard: C++17
- Do **NOT** use Exceptions
- Macros
	- Defined in `common.hpp`
    - Excessive use should be avoided
	- Always use `rep(i, a, b)` if you need to write `for (int i = a; i < b; i++)`
		- However, **use range-based for `for (auto& e : v)` instead whenever it is possible**

## License
MIT License

## Project status
This project is under development.