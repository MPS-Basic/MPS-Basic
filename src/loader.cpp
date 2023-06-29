#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using std::string;

enum class VariableType { Int, Double, Bool, String };

class VariableData {
private:
	int* pint = nullptr;
	double* pdouble = nullptr;
	bool* pbool = nullptr;
	string* pstring = nullptr;

public:
	VariableData() {
	}

	void set(string name, int* p) {
		pint = p;
	}
};

class Loader {
private:
public:
	Loader() {
		//
	}

	void addDefinition(string name, int* pointer);
};
