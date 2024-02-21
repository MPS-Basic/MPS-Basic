# Basic Code Documentation {#mainpage}

Modernized Moving Particle Semi-implicit/Simulation code written in C++.

## Coding guide
- C++ standard: C++17
- Do **NOT** use Exceptions
- Macros
	- Defined in `common.hpp`
    - Excessive use should be avoided
	- You can use `rep(i, a, b)` if you need to write `for (int i = a; i < b; i++)`
		- However, **use range-based for `for (auto& e : v)` instead whenever it is possible**

## License
MIT License

## Project status
This project is under development.
