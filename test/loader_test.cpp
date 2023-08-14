#include <gtest/gtest.h>
#include "loader.hpp"

TEST(LoaderTest, LoaderTest) {
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
	EXPECT_EQ(intA, 0x10);
	EXPECT_EQ(intB, 123);
	EXPECT_DOUBLE_EQ(doubleA, 1e3);
	EXPECT_DOUBLE_EQ(doubleB, 12.3);
	EXPECT_EQ(boolA, true);
	EXPECT_EQ(boolB, false);
	EXPECT_EQ(stringA, std::string("asdasd"));
	EXPECT_EQ(stringB, std::string("hoge"));
}