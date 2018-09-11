/*
 * PathPrinter.hpp
 *
 *  Created on: Jan 3, 2017
 *      Author: bbielaws
 */

#ifndef PATHPRINTER_HPP_
#define PATHPRINTER_HPP_

#include "AJSP.hpp"
#include "PathConstructor.hpp"

class PathPrinter: public AJSP::Listener
{
	public:
		PathPrinter(AJSP::Parser* p): parser(p) {}
		virtual ~PathPrinter() {}
		virtual void value(const std::string& value, AJSP::Parser::Entity entity);

	private:
		AJSP::Parser*   parser;
};

#endif /* PATHPRINTER_HPP_ */
