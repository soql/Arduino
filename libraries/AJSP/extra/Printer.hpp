/*
 * Printer.hpp
 *
 *  Created on: Jan 3, 2017
 *      Author: bbielaws
 */

#ifndef PRINTER_HPP_
#define PRINTER_HPP_

#include "AJSP.hpp"

class PrinterListener: public AJSP::Listener
{
	public:
		PrinterListener(AJSP::Parser* p): parser(p) {}
		virtual ~PrinterListener() {}

		virtual void arrayStart();
		virtual void arrayEnd();

		virtual void objectStart();
		virtual void objectEnd();

		virtual void key(const std::string& key);
		virtual void value(const std::string& value);

		virtual void done();

	private:
		int indent = 1;
		AJSP::Parser* parser;
};




#endif /* PRINTER_HPP_ */
