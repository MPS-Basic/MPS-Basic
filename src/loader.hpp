#pragma once
#include "common.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace loader {

using namespace std;

enum class VariableType { Int, Double, Bool, String };

class VariableData {
private:
	int* pint       = nullptr;
	double* pdouble = nullptr;
	bool* pbool     = nullptr;
	string* pstring = nullptr;

public:
	const VariableType type;
	bool hasLoaded = false;

	VariableData(int* p) : type(VariableType::Int) {
		pint = p;
	}

	VariableData(double* p) : type(VariableType::Double) {
		pdouble = p;
	}

	VariableData(bool* p) : type(VariableType::Bool) {
		pbool = p;
	}

	VariableData(string* p) : type(VariableType::String) {
		pstring = p;
	}

	void set(int v) {
		*pint = v;
	}

	void set(double v) {
		*pdouble = v;
	}

	void set(bool v) {
		*pbool = v;
	}

	void set(string v) {
		*pstring = v;
	}
};

constexpr char* IDENTIFIER_REGEX = "[a-zA-Z0-9_]+";

class Loader {
private:
	map<string, VariableData> vars;

public:
	Loader() {
	}

	template <class T>
	auto addDefinition(string name, T* pointer)
	    -> decltype(VariableData(pointer), void()) {
		if (!regex_match(name, regex(IDENTIFIER_REGEX))) {
			cerr << "invalid identifier: " + name + "; it must match " +
			            IDENTIFIER_REGEX
			     << endl;
			exit(-1);
		}
		if (vars.count(name) > 0) {
			cerr << "identifier " + name + " has already been defined" << endl;
			exit(-1);
		}
		vars.insert(name, VariableData(pointer));
	}

	void load(string path) {
		for (auto& v : vars) {
			v.second.hasLoaded = false;
		}
		ifstream in(path);
		if (in.fail()) {
			cerr << "could not load file: " + path << endl;
			exit(-1);
		}
		string line;
		while (getline(in, line)) {
			parseLine(line);
		}
	}

private:
	void parseLine(string line) {
		auto commentRegex = regex("\\s*#.*");
		if (regex_match(line, commentRegex)) {
			return; // skipping the comment
		}
		smatch match;
		auto definitionRegex =
		    regex("\\s*("s + IDENTIFIER_REGEX + ")\\s+(.+?)\\s*");
		if (regex_match(line, match, definitionRegex)) {
			string name  = match[1];
			string value = match[2];
			if (vars.count(name) == 0) {
				cout << "[WARN] " + name +
				            " is found, but not defined. skipping..."
				     << endl;
				return;
			}
			auto data = vars[name];
			switch (data.type) {
			case VariableType::Int:
				data.set(parseInt(value));
				break;
			case VariableType::Double:
				data.set(parseDouble(value));
				break;
			case VariableType::Bool:
				data.set(parseBool(value));
				break;
			case VariableType::String:
				data.set(parseString(value));
				break;
			}
		}
	}

	int parseInt(string str) {
		return stoi(str);
	}

	double parseDouble(string str) {
		return stod(str);
	}

	bool parseBool(string str) {
		if (str == "true")
			return true;
		if (str == "false")
			return false;
		cerr << "could not parse " + str +
		            " as a bool; use only \"true\" or \"false\""
		     << endl;
		exit(-1);
	}

	string parseString(string str) {
		ostringstream res;
		auto len = str.length();
		string quotationError =
		    "strings must be enclosed in double quotations: " + str;
		string invalidEscapeError = "an invalid escape sequence found: " + str;
		if (len < 2) {
			cerr << quotationError << endl;
			exit(-1);
		}
		bool quotationOpened = false;
		bool quotationClosed = false;
		rep(i, 0, len) {
			if (i == 0 && str[i] == '"') {
				quotationOpened = true;
				continue;
			}
			if (i == len - 1 && str[i] == '"') {
				quotationClosed = true;
				continue;
			}
			if (str[i] == '\\') {
				if (i == len - 1) {
					cerr << invalidEscapeError << endl;
					exit(-1);
				}
				i++;
				switch (str[i]) {
				case 't':
					res << "\t";
					break;
				case 'n':
					res << "\n";
					break;
				case '\\':
					res << "\\";
					break;
				case '"':
					res << "\"";
					break;
				default:
					cerr << invalidEscapeError << endl;
					exit(-1);
					break;
				}
				continue;
			}
			res << str[i];
		}
	}
};

} // namespace loader

// expose data
using VariableType = loader::VariableType;
using VariableData = loader::VariableData;
using Loader       = loader::Loader;