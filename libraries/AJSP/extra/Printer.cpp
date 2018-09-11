/*
 * Printer.cpp
 *
 *  Created on: Jan 3, 2017
 *      Author: bbielaws
 */

#include "Printer.hpp"
#include <iostream>

using namespace AJSP;
using namespace std;

std::string getIndent(int indentLevel)
{
	return std::string(indentLevel, '\t');
}

void PrinterListener::arrayStart()
{
	cout << getIndent(indent) << "[" << endl;
	indent++;
}

void PrinterListener::arrayEnd()
{
	indent--;
	cout  << getIndent(indent) << "]" << endl;
}

void PrinterListener::objectStart()
{
	cout << getIndent(indent) << "{" << endl;;
	indent++;
}

void PrinterListener::objectEnd()
{
	indent--;
	cout << getIndent(indent) << "}" << endl;
}

void PrinterListener::key(const std::string& key)
{
	cout << getIndent(indent) << key << ": ";
}

void PrinterListener::value(const std::string& value)
{
	cout << getIndent(indent) << value << ", " << endl;
}

void PrinterListener::done()
{
	cout << "\nDONE!\n";
}





