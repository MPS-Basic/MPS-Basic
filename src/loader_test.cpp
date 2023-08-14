#include "loader.hpp"

// tentative test code; should be replaced later
int main() {
    using namespace std;
    Loader loader;
    int intA;
    int intB;
    double doubleA;
    double doubleB;
    bool boolA;
    bool boolB;
    string stringA;
    string stringB;
    loader.addDefinition("intA", &intA);
    loader.addDefinition("intB", &intB, 123);
    loader.addDefinition("doubleA", &doubleA);
    loader.addDefinition("doubleB", &doubleB, 12.3);
    loader.addDefinition("boolA", &boolA);
    loader.addDefinition("boolB", &boolB, true);
    loader.addDefinition("stringA", &stringA);
    loader.addDefinition("stringB", &stringB, "hoge"s);
    string content = "# comment line\n"
                     "intA\t\t0x10\n"
                     "\t \tdoubleA 1e3\n"
                     "boolA true\n"
                     "   #comment2\n"
                     "\n"
                     "   \t   \n"
                     "boolB       false\n"
                     "stringA    \"asdasd\"\n";
	istringstream in(content);
    loader.load(in);
    cout << intA << endl;
    cout << intB << endl;
    cout << doubleA << endl;
    cout << doubleB << endl;
    cout << boolA << endl;
    cout << boolB << endl;
    cout << stringA << endl;
    cout << stringB << endl;
    return 0;
}
