//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "tests/parser/testUtils.hpp"

using namespace unitpp;


namespace
{
	// HACK: parameterizedHeaderField constructor is private
	class parameterizedHeaderField : public vmime::parameterizedHeaderField
	{
	private:

		vmime::ref <vmime::typeAdapter <vmime::string> > m_value;

	public:

		parameterizedHeaderField()
			: headerField("F"),
			  m_value(vmime::create <vmime::typeAdapter <vmime::string> >("X"))
		{
		}

		const vmime::component& getValue() const { return *m_value; }
		vmime::component& getValue() { return *m_value; }

		void setValue(const vmime::component&) { /* Do nothing */ }

		const vmime::ref <const vmime::component> getValueImp() const { return m_value; }
		vmime::ref <vmime::component> getValueImp() { return m_value; }
	};


#define PARAM_VALUE(p, n) (p.getParameterAt(n)->getValue().generate())
#define PARAM_NAME(p, n) (p.getParameterAt(n)->getName())
#define PARAM_CHARSET(p, n) ( \
	(p.getParameterAt(n).staticCast <vmime::defaultParameter>())->getValue().getCharset().generate())
#define PARAM_BUFFER(p, n) ( \
	(p.getParameterAt(n).staticCast <vmime::defaultParameter>())->getValue().getBuffer())


	class parameterTest : public suite
	{
		void testParse()
		{
			// Simple parameter
			parameterizedHeaderField p1;
			p1.parse("X; param1=value1;\r\n");

			assert_eq("1.1", 1, p1.getParameterCount());
			assert_eq("1.2", "param1", PARAM_NAME(p1, 0));
			assert_eq("1.3", "value1", PARAM_VALUE(p1, 0));

			// Multi-section parameters (1/2)
			parameterizedHeaderField p2a;
			p2a.parse("X; param1=value1;\r\n"
			          "  param2*0=\"val\";\r\n"
			          "  param2*1=\"ue2\";");

			assert_eq("2a.1", 2, p2a.getParameterCount());
			assert_eq("2a.2", "param1", PARAM_NAME(p2a, 0));
			assert_eq("2a.3", "value1", PARAM_VALUE(p2a, 0));
			assert_eq("2a.4", "param2", PARAM_NAME(p2a, 1));
			assert_eq("2a.5", "value2", PARAM_VALUE(p2a, 1));

			// Multi-section parameters (2/2)
			parameterizedHeaderField p2b;
			p2b.parse("X; param1=value1;\r\n"
			          "  param2=\"should be ignored\";\r\n"
			          "  param2*0=\"val\";\r\n"
			          "  param2*1=\"ue2\";");

			assert_eq("2b.1", 2, p2b.getParameterCount());
			assert_eq("2b.2", "param1", PARAM_NAME(p2b, 0));
			assert_eq("2b.3", "value1", PARAM_VALUE(p2b, 0));
			assert_eq("2b.4", "param2", PARAM_NAME(p2b, 1));
			assert_eq("2b.5", "value2", PARAM_VALUE(p2b, 1));

			// Extended parameter (charset and language information)
			parameterizedHeaderField p3;
			p3.parse("X; param1*=charset'language'value1;\r\n");

			assert_eq("3.1", 1, p3.getParameterCount());
			assert_eq("3.2", "param1", PARAM_NAME(p3, 0));
			assert_eq("3.3", "charset", PARAM_CHARSET(p3, 0));
			assert_eq("3.4", "value1", PARAM_BUFFER(p3, 0));

			// Encoded characters in extended parameter values
			parameterizedHeaderField p4;
			p4.parse("X; param1*=a%20value%20with%20multiple%20word%73");  // 0x73 = 's'

			assert_eq("4.1", 1, p4.getParameterCount());
			assert_eq("4.2", "param1", PARAM_NAME(p4, 0));
			assert_eq("4.3", "a value with multiple words", PARAM_VALUE(p4, 0));

			// Invalid encoded character
			parameterizedHeaderField p5;
			p5.parse("X; param1*=test%20value%");

			assert_eq("5.1", 1, p5.getParameterCount());
			assert_eq("5.2", "param1", PARAM_NAME(p5, 0));
			assert_eq("5.3", "test value%", PARAM_VALUE(p5, 0));

			// Spaces before and after '='
			parameterizedHeaderField p6;
			p6.parse("X; param1\t=  \"value1\"");

			assert_eq("6.1", 1, p6.getParameterCount());
			assert_eq("6.2", "param1", PARAM_NAME(p6, 0));
			assert_eq("6.3", "value1", PARAM_VALUE(p6, 0));

			// Quoted strings and escaped chars
			parameterizedHeaderField p7;
			p7.parse("X; param1=\"this is a slash: \\\"\\\\\\\"\"");  // \"\\\"

			assert_eq("7.1", 1, p7.getParameterCount());
			assert_eq("7.2", "param1", PARAM_NAME(p7, 0));
			assert_eq("7.3", "this is a slash: \"\\\"", PARAM_VALUE(p7, 0));

			// Extended parameter with charset specified in more than one
			// section (this is forbidden by RFC, but is should not fail)
			parameterizedHeaderField p8;
			p8.parse("X; param1*0*=charset1'language1'value1;\r\n"
			         "   param1*1*=charset2'language2'value2;");

			assert_eq("8.1", 1, p8.getParameterCount());
			assert_eq("8.2", "param1", PARAM_NAME(p8, 0));
			assert_eq("8.3", "charset1", PARAM_CHARSET(p8, 0));
			assert_eq("8.4", "value1charset2'language2'value2", PARAM_BUFFER(p8, 0));

			// Charset not specified in the first section (that is not encoded),
			// but specified in the second one (legal)
			parameterizedHeaderField p9;
			p9.parse("X; param1*0=value1;\r\n"
			         "   param1*1*=charset'language'value2;");

			assert_eq("9.1", 1, p9.getParameterCount());
			assert_eq("9.2", "param1", PARAM_NAME(p9, 0));
			assert_eq("9.3", "charset", PARAM_CHARSET(p9, 0));
			assert_eq("9.4", "value1value2", PARAM_BUFFER(p9, 0));

			// Characters prefixed with '%' in a simple (not extended) section
			// should not be decoded
			parameterizedHeaderField p10;
			p10.parse("X; param1=val%20ue1");

			assert_eq("10.1", 1, p10.getParameterCount());
			assert_eq("10.2", "param1", PARAM_NAME(p10, 0));
			assert_eq("10.3", "val%20ue1", PARAM_VALUE(p10, 0));

			// Multiple sections + charset specified and encoding
			parameterizedHeaderField p11;
			p11.parse("X; param1*0*=charset'language'value1a%20;"
			          "   param1*1*=value1b%20;"
			          "   param1*2=value1c");

			assert_eq("11.1", 1, p11.getParameterCount());
			assert_eq("11.2", "param1", PARAM_NAME(p11, 0));
			assert_eq("11.3", "charset", PARAM_CHARSET(p11, 0));
			assert_eq("11.4", "value1a value1b value1c", PARAM_BUFFER(p11, 0));

			// No charset specified: defaults to US-ASCII
			parameterizedHeaderField p12;
			p12.parse("X; param1*='language'value1");

			assert_eq("12.1", 1, p12.getParameterCount());
			assert_eq("12.2", "param1", PARAM_NAME(p12, 0));
			assert_eq("12.3", "us-ascii", PARAM_CHARSET(p12, 0));
			assert_eq("12.4", "value1", PARAM_BUFFER(p12, 0));
		}

		void testGenerate()
		{
			// Simple parameter/value
			parameterizedHeaderField p1;
			p1.appendParameter(vmime::parameterFactory::getInstance()->create("param1", "value1"));

			assert_eq("1", "F: X; param1=value1", p1.generate());

			// Value that needs quoting (1/2)
			parameterizedHeaderField p2a;
			p2a.appendParameter(vmime::parameterFactory::getInstance()->create("param1", "value1a;value1b"));

			assert_eq("2a", "F: X; param1=\"value1a;value1b\"", p2a.generate());

			// Value that needs quoting (2/2)
			parameterizedHeaderField p2b;
			p2b.appendParameter(vmime::parameterFactory::getInstance()->create("param1", "va\\lue\"1"));

			assert_eq("2b", "F: X; param1=\"va\\\\lue\\\"1\"", p2b.generate());

			// Extended parameter with charset specifier
			parameterizedHeaderField p3;
			p3.appendParameter(vmime::parameterFactory::getInstance()->create("param1",
				vmime::word("value 1\xe9", vmime::charset("charset"))));

			assert_eq("3", "F: X; param1=\"value 1\";param1*=charset''value%201%E9", p3.generate());

			// Value that spans on multiple lines
			parameterizedHeaderField p4;
			p4.appendParameter(vmime::parameterFactory::getInstance()->create("param1",
				vmime::word("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
				            vmime::charset("charset"))));

			assert_eq("4", "F: X; \r\n "
				"param1=abcdefghijklm;\r\n "
				"param1*0*=charset''abc;\r\n "
				"param1*1*=defghijkl;\r\n "
				"param1*2*=mnopqrstu;\r\n "
				"param1*3*=vwxyzABCD;\r\n "
				"param1*4*=EFGHIJKLM;\r\n "
				"param1*5*=NOPQRSTUV;\r\n "
				"param1*6*=WXYZ", p4.generate(25));  // max line length = 25
		}

	public:

		parameterTest() : suite("vmime::path")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &parameterTest::testParse));
			add("Generate", testcase(this, "Generate", &parameterTest::testGenerate));

			suite::main().add("vmime::parameter", this);
		}

	};

	parameterTest* theTest = new parameterTest();
}
