/*
 * main.cpp
 *
 *  Created on: Jan 2, 2017
 *      Author: bbielaws
 */

#ifdef AJSP_TESTS

#include <fstream>
#include <iostream>

#include "AJSP.hpp"
#include "Printer.hpp"
#include "PathPrinter.hpp"
#include "VectorCollector.hpp"
#include "MapCollector.hpp"
#include <utility>

using namespace std;
using namespace AJSP;


struct Result
{
		Result(AJSP::Parser::Result result, uint32_t offset): result(result), offset(offset) {}
		AJSP::Parser::Result result;
		uint32_t offset;

		bool operator==(const Result& other) const
				{
			if (result == Parser::Result::DONE and other.result == Parser::Result::DONE)
				return true;

			if (result != other.result)
				return false;

			if (offset != other.offset)
				return false;

			return true;
				}

		bool operator!=(const Result& other) const {return !(*this == other);}

		std::string toString() const
		{
			return string(Parser::getResultDescription(result)) + " @ " + to_string(offset);
		}
};

std::string loadFile(const std::string& filename)
{
	std::string result;
	std::ifstream ifs (filename, std::ifstream::in);
	if (!ifs.good())
		return result;

	result.reserve(4096);

	while (ifs.good())
	{
		result += ifs.get();
	}

	return result;
}

Result parseFile(const std::string& filename)
{
	AJSP::Parser p;
	PathPrinter pp(&p);

	p.setListener(&pp);

	std::ifstream ifs (filename, std::ifstream::in);

	if (!ifs.good())
	{
		return Result(Parser::Result::INVALID_INTERNAL_STATE, 0xFFFFFFFF);
	}

	while (ifs.good())
	{
		auto r = p.parse(ifs.get());
		if (r != Parser::Result::OK)
			return Result(r, p.getCurrentOffset());
	};

	ifs.close();

	return Result(Parser::Result::OK, 0);
}

vector<pair<string, Result>> tests =
{
		{"jsonExamples/weatherExample.json", Result(Parser::Result::DONE, 0)},

		//object tests
		{"jsonExamples/emptyObject.json", Result(Parser::Result::DONE, 0)},
		{"jsonExamples/objectJustKey.json", Result(Parser::Result::IC_OBJECT_VALUE_EXPECTED, 9)},
		{"jsonExamples/objectWrongClosing.json", Result(Parser::Result::IC_OBJECT_KEY_OR_END_EXPECTED, 1)},
		{"jsonExamples/objectWithColon.json", Result(Parser::Result::IC_OBJECT_KEY_OR_END_EXPECTED, 1)},
		{"jsonExamples/objectMissingColon.json", Result(Parser::Result::IC_OBJECT_COLON_EXPECTED, 7)},
		{"jsonExamples/string.json", Result(Parser::Result::DONE, 0)},

		//array tests
		{"jsonExamples/arrayNumericSimple.json", Result(Parser::Result::DONE, 0)},
		{"jsonExamples/emptyArray.json", Result(Parser::Result::DONE, 0)},
		{"jsonExamples/arrayWrongClosing.json", Result(Parser::Result::IC_ARRAY_VALUE_OR_END_EXPECTED, 1)},
		{"jsonExamples/arrayLeadingComma.json", Result(Parser::Result::IC_ARRAY_VALUE_OR_END_EXPECTED, 1)},
		{"jsonExamples/arrayTrailingComma.json", Result(Parser::Result::IC_ARRAY_VALUE_EXPECTED, 3)},
};

class PP: public PathPrinter
{
	public:
		PP(AJSP::Parser* p): PathPrinter(p) {}
		void done() {cntr++;}
		uint32_t getCntr() const {return cntr;}
	private:
		uint32_t cntr = 0;
};

void test1()
{
	AJSP::Parser p;
	PP pp(&p);
	p.setListener(&pp);

	cout << "Multiple object test..." << endl;

	const char data[] = "{'test':'test'} ['test'] 'test'";

	for (auto& c: data)
	{
		p.parse(c);
	}

	if (pp.getCntr() == 3)
	{
		cout << "OK" << endl;
	}
	else
	{
		cout << "Failed..." << endl;
	}
}

void testVectorCollector()
{

	auto l = [](const std::string& s){return s.find("/root/main/") != std::string::npos;};

	cout << "VectorCollector test..." << endl;
	VectorCollector vc(l);
	std::string json = loadFile("jsonExamples/weatherExample.json");

	for (const auto& c: json)
	{
		vc.parse(c);
	}

	for (const auto& v: vc.getValues())
	{
		cout << v.key << ": " << v.value << endl;
	}
}

void testMapCollector()
{

	auto l = [](const std::string& s, const std::string& v){return s.find("/root/main/") != std::string::npos;};

	cout << "MapCollector test..." << endl;
	MapCollector mc(l);
	std::string json = loadFile("jsonExamples/weatherExample.json");

	for (const auto& c: json)
	{
		mc.parse(c);
	}

	for (const auto& c: json)
	{
	    mc.parse(c);
	}

	for (const auto& v: mc.getValues())
	{
		cout << v.first << ": " << v.second << endl;
	}
}

int main(int argc, char* argv[])
{
	cout << "-----------------------------------------------------" << endl;
	test1();
	cout << "-----------------------------------------------------" << endl;
	testVectorCollector();
	cout << "-----------------------------------------------------" << endl;
	testMapCollector();
	cout << "-----------------------------------------------------" << endl;

	for (const auto& p: tests)
	{
		auto r = parseFile(p.first);

		cout << "-----------------------------------------------------" << endl;
		cout << "Testing file: " << p.first << endl;
		if (r != p.second)
		{
			cout << "Expected: " << p.second.toString() << endl;
			cout << "Result:   " << r.toString() << endl;
		}
		else
			cout << "OK" << endl;
	}

	return 0;
}

#endif