/*
 * PathPrinter.cpp
 *
 *  Created on: Jan 3, 2017
 *      Author: bbielaws
 */

#include "PathPrinter.hpp"

#include <iostream>
#include <algorithm>
#include <numeric>

using namespace AJSP;
using namespace std;




void PathPrinter::value(const std::string& value, Parser::Entity)
{
	cout << parser->getCurrentPath() << ": " << value << endl;
}


