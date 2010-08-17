//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "tests/testUtils.hpp"


#define VMIME_TEST_SUITE         parameterTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testParseRFC2231)
		VMIME_TEST(testGenerate)
		VMIME_TEST(testGenerateRFC2231)
		VMIME_TEST(testNonStandardEncodedParam)
		VMIME_TEST(testParseNonSignificantWS)
		VMIME_TEST(testEncodeTSpecials)
		VMIME_TEST(testEncodeTSpecialsInRFC2231)
	VMIME_TEST_LIST_END


	// HACK: parameterizedHeaderField constructor is private
	class parameterizedHeaderField : public vmime::parameterizedHeaderField
	{
	public:

		parameterizedHeaderField()
			: headerField("F")
		{
			setValue(vmime::headerFieldFactory::getInstance()->createValue(getName()));
			setValue(vmime::word("X"));
		}
	};


#define FIELD_VALUE(f) (f.getValue()->generate())
#define PARAM_VALUE(p, n) (p.getParameterAt(n)->getValue().generate())
#define PARAM_NAME(p, n) (p.getParameterAt(n)->getName())
#define PARAM_CHARSET(p, n) \
	(p.getParameterAt(n)->getValue().getCharset().generate())
#define PARAM_BUFFER(p, n) \
	(p.getParameterAt(n)->getValue().getBuffer())


	void testParse()
	{
		// Simple parameter
		parameterizedHeaderField p1;
		p1.parse("X; param1=value1;\r\n");

		VASSERT_EQ("1.1", 1, p1.getParameterCount());
		VASSERT_EQ("1.2", "param1", PARAM_NAME(p1, 0));
		VASSERT_EQ("1.3", "value1", PARAM_VALUE(p1, 0));

		// Multi-section parameters (1/2)
		parameterizedHeaderField p2a;
		p2a.parse("X; param1=value1;\r\n"
			  "  param2*0=\"val\";\r\n"
			  "  param2*1=\"ue2\";");

		VASSERT_EQ("2a.1", 2, p2a.getParameterCount());
		VASSERT_EQ("2a.2", "param1", PARAM_NAME(p2a, 0));
		VASSERT_EQ("2a.3", "value1", PARAM_VALUE(p2a, 0));
		VASSERT_EQ("2a.4", "param2", PARAM_NAME(p2a, 1));
		VASSERT_EQ("2a.5", "value2", PARAM_VALUE(p2a, 1));

		// Multi-section parameters (2/2)
		parameterizedHeaderField p2b;
		p2b.parse("X; param1=value1;\r\n"
			  "  param2=\"should be ignored\";\r\n"
			  "  param2*0=\"val\";\r\n"
			  "  param2*1=\"ue2\";");

		VASSERT_EQ("2b.1", 2, p2b.getParameterCount());
		VASSERT_EQ("2b.2", "param1", PARAM_NAME(p2b, 0));
		VASSERT_EQ("2b.3", "value1", PARAM_VALUE(p2b, 0));
		VASSERT_EQ("2b.4", "param2", PARAM_NAME(p2b, 1));
		VASSERT_EQ("2b.5", "value2", PARAM_VALUE(p2b, 1));

		// Extended parameter (charset and language information)
		parameterizedHeaderField p3;
		p3.parse("X; param1*=charset'language'value1;\r\n");

		VASSERT_EQ("3.1", 1, p3.getParameterCount());
		VASSERT_EQ("3.2", "param1", PARAM_NAME(p3, 0));
		VASSERT_EQ("3.3", "charset", PARAM_CHARSET(p3, 0));
		VASSERT_EQ("3.4", "value1", PARAM_BUFFER(p3, 0));

		// Encoded characters in extended parameter values
		parameterizedHeaderField p4;
		p4.parse("X; param1*=a%20value%20with%20multiple%20word%73");  // 0x73 = 's'

		VASSERT_EQ("4.1", 1, p4.getParameterCount());
		VASSERT_EQ("4.2", "param1", PARAM_NAME(p4, 0));
		VASSERT_EQ("4.3", "a value with multiple words", PARAM_VALUE(p4, 0));

		// Invalid encoded character
		parameterizedHeaderField p5;
		p5.parse("X; param1*=test%20value%");

		VASSERT_EQ("5.1", 1, p5.getParameterCount());
		VASSERT_EQ("5.2", "param1", PARAM_NAME(p5, 0));
		VASSERT_EQ("5.3", "test value%", PARAM_VALUE(p5, 0));

		// Spaces before and after '='
		parameterizedHeaderField p6;
		p6.parse("X; param1\t=  \"value1\"");

		VASSERT_EQ("6.1", 1, p6.getParameterCount());
		VASSERT_EQ("6.2", "param1", PARAM_NAME(p6, 0));
		VASSERT_EQ("6.3", "value1", PARAM_VALUE(p6, 0));

		// Quoted strings and escaped chars
		parameterizedHeaderField p7;
		p7.parse("X; param1=\"this is a slash: \\\"\\\\\\\"\"");  // \"\\\"

		VASSERT_EQ("7.1", 1, p7.getParameterCount());
		VASSERT_EQ("7.2", "param1", PARAM_NAME(p7, 0));
		VASSERT_EQ("7.3", "this is a slash: \"\\\"", PARAM_VALUE(p7, 0));
	}

	void testParseRFC2231()
	{
		// Extended parameter with charset specified in more than one
		// section (this is forbidden by RFC, but is should not fail)
		parameterizedHeaderField p1;
		p1.parse("X; param1*0*=charset1'language1'value1;\r\n"
			   "   param1*1*=charset2'language2'value2;");

		VASSERT_EQ("1.1", 1, p1.getParameterCount());
		VASSERT_EQ("1.2", "param1", PARAM_NAME(p1, 0));
		VASSERT_EQ("1.3", "charset1", PARAM_CHARSET(p1, 0));
		VASSERT_EQ("1.4", "value1charset2'language2'value2", PARAM_BUFFER(p1, 0));

		// Charset not specified in the first section (that is not encoded),
		// but specified in the second one (legal)
		parameterizedHeaderField p2;
		p2.parse("X; param1*0=value1;\r\n"
			   "   param1*1*=charset'language'value2;");

		VASSERT_EQ("2.1", 1, p2.getParameterCount());
		VASSERT_EQ("2.2", "param1", PARAM_NAME(p2, 0));
		VASSERT_EQ("2.3", "charset", PARAM_CHARSET(p2, 0));
		VASSERT_EQ("2.4", "value1value2", PARAM_BUFFER(p2, 0));

		// Characters prefixed with '%' in a simple (not extended) section
		// should not be decoded
		parameterizedHeaderField p3;
		p3.parse("X; param1=val%20ue1");

		VASSERT_EQ("3.1", 1, p3.getParameterCount());
		VASSERT_EQ("3.2", "param1", PARAM_NAME(p3, 0));
		VASSERT_EQ("3.3", "val%20ue1", PARAM_VALUE(p3, 0));

		// Multiple sections + charset specified and encoding
		parameterizedHeaderField p4;
		p4.parse("X; param1*0*=charset'language'value1a%20;"
			   "   param1*1*=value1b%20;"
			   "   param1*2=value1c");

		VASSERT_EQ("4.1", 1, p4.getParameterCount());
		VASSERT_EQ("4.2", "param1", PARAM_NAME(p4, 0));
		VASSERT_EQ("4.3", "charset", PARAM_CHARSET(p4, 0));
		VASSERT_EQ("4.4", "value1a value1b value1c", PARAM_BUFFER(p4, 0));

		// No charset specified: defaults to US-ASCII
		parameterizedHeaderField p5;
		p5.parse("X; param1*='language'value1");

		VASSERT_EQ("5.1", 1, p5.getParameterCount());
		VASSERT_EQ("5.2", "param1", PARAM_NAME(p5, 0));
		VASSERT_EQ("5.3", "us-ascii", PARAM_CHARSET(p5, 0));
		VASSERT_EQ("5.4", "value1", PARAM_BUFFER(p5, 0));
	}

	void testGenerate()
	{
		// Simple parameter/value
		parameterizedHeaderField p1;
		p1.appendParameter(vmime::create <vmime::parameter>("param1", "value1"));

		VASSERT_EQ("1", "F: X; param1=value1", p1.generate());

		// Value that needs quoting (1/2)
		parameterizedHeaderField p2a;
		p2a.appendParameter(vmime::create <vmime::parameter>("param1", "value1a;value1b"));

		VASSERT_EQ("2a", "F: X; param1=\"value1a;value1b\"", p2a.generate());

		// Value that needs quoting (2/2)
		parameterizedHeaderField p2b;
		p2b.appendParameter(vmime::create <vmime::parameter>("param1", "va\\lue\"1"));

		VASSERT_EQ("2b", "F: X; param1=\"va\\\\lue\\\"1\"", p2b.generate());
	}

	void testGenerateRFC2231()
	{
		// Extended parameter with charset specifier
		parameterizedHeaderField p1;
		p1.appendParameter(vmime::create <vmime::parameter>("param1",
			vmime::word("value 1\xe9", vmime::charset("charset"))));

#if VMIME_ALWAYS_GENERATE_7BIT_PARAMETER
		VASSERT_EQ("1", "F: X; param1=\"value 1\";param1*=charset''value%201%E9", p1.generate());
#else
		VASSERT_EQ("1", "F: X; param1*=charset''value%201%E9", p1.generate());
#endif

		// Value that spans on multiple lines
		parameterizedHeaderField p2;
		p2.appendParameter(vmime::create <vmime::parameter>("param1",
			vmime::word("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
				    vmime::charset("charset"))));

#if VMIME_ALWAYS_GENERATE_7BIT_PARAMETER
		VASSERT_EQ("2", "F: X; \r\n "
			"param1=abcdefghijklm;\r\n "
			"param1*0*=charset''abc;\r\n "
			"param1*1*=defghijkl;\r\n "
			"param1*2*=mnopqrstu;\r\n "
			"param1*3*=vwxyzABCD;\r\n "
			"param1*4*=EFGHIJKLM;\r\n "
			"param1*5*=NOPQRSTUV;\r\n "
			"param1*6*=WXYZ", p2.generate(25));  // max line length = 25
#else
		VASSERT_EQ("2", "F: X; \r\n "
			"param1*0*=charset''abc;\r\n "
			"param1*1*=defghijkl;\r\n "
			"param1*2*=mnopqrstu;\r\n "
			"param1*3*=vwxyzABCD;\r\n "
			"param1*4*=EFGHIJKLM;\r\n "
			"param1*5*=NOPQRSTUV;\r\n "
			"param1*6*=WXYZ", p2.generate(25));  // max line length = 25
#endif

		// Non-ASCII parameter value
		parameterizedHeaderField p3;
		p3.appendParameter(vmime::create <vmime::parameter>("param1",
			vmime::word("δσσσσσσσσσσσσσσσσσσσσδσδα δσαδσδσαδσαδασδασ δσαδασδσα δσαδασδσα δασδασδασ δασαχφδδσα 2008.doc",
				vmime::charset("utf-8"))));

#if VMIME_ALWAYS_GENERATE_7BIT_PARAMETER
		VASSERT_EQ("3", "F: X; \r\n "
			"param1=\"      2008.doc\";param1*0*=utf-8''%CE%B4%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83;\r\n "
			"param1*1*=%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83;\r\n "
			"param1*2*=%CE%B4%CF%83%CE%B4%CE%B1%20%CE%B4%CF%83%CE%B1%CE%B4%CF%83%CE%B4%CF;\r\n "
			"param1*3*=%83%CE%B1%CE%B4%CF%83%CE%B1%CE%B4%CE%B1%CF%83%CE%B4%CE%B1%CF%83%20;\r\n "
			"param1*4*=%CE%B4%CF%83%CE%B1%CE%B4%CE%B1%CF%83%CE%B4%CF%83%CE%B1%20%CE%B4%CF;\r\n "
			"param1*5*=%83%CE%B1%CE%B4%CE%B1%CF%83%CE%B4%CF%83%CE%B1%20%CE%B4%CE%B1%CF%83;\r\n "
			"param1*6*=%CE%B4%CE%B1%CF%83%CE%B4%CE%B1%CF%83%20%CE%B4%CE%B1%CF%83%CE%B1%CF;\r\n "
			"param1*7*=%87%CF%86%CE%B4%CE%B4%CF%83%CE%B1%202008.doc", p3.generate(80));
#else
		VASSERT_EQ("3", "F: X; param1*0*=utf-8''%CE%B4%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83;\r\n "
			"param1*1*=%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83%CF%83;\r\n "
			"param1*2*=%CE%B4%CF%83%CE%B4%CE%B1%20%CE%B4%CF%83%CE%B1%CE%B4%CF%83%CE%B4%CF;\r\n "
			"param1*3*=%83%CE%B1%CE%B4%CF%83%CE%B1%CE%B4%CE%B1%CF%83%CE%B4%CE%B1%CF%83%20;\r\n "
			"param1*4*=%CE%B4%CF%83%CE%B1%CE%B4%CE%B1%CF%83%CE%B4%CF%83%CE%B1%20%CE%B4%CF;\r\n "
			"param1*5*=%83%CE%B1%CE%B4%CE%B1%CF%83%CE%B4%CF%83%CE%B1%20%CE%B4%CE%B1%CF%83;\r\n "
			"param1*6*=%CE%B4%CE%B1%CF%83%CE%B4%CE%B1%CF%83%20%CE%B4%CE%B1%CF%83%CE%B1%CF;\r\n "
			"param1*7*=%87%CF%86%CE%B4%CE%B4%CF%83%CE%B1%202008.doc", p3.generate(80));
#endif
	}

	void testNonStandardEncodedParam()
	{
		// This syntax is non-standard (expressly prohibited
		// by RFC-2047), but is used by Mozilla:
		//
    		// Content-Type: image/png;
		//    name="=?us-ascii?Q?Logo_VMime=2Epng?="

		parameterizedHeaderField p1;
		p1.parse("image/png; name=\"=?us-ascii?Q?Logo_VMime=2Epng?=\"");

		VASSERT_EQ("1.1", 1, p1.getParameterCount());
		VASSERT_EQ("1.2", "name", PARAM_NAME(p1, 0));
		VASSERT_EQ("1.3", "Logo VMime.png", PARAM_VALUE(p1, 0));

		parameterizedHeaderField p2;
		p2.parse("image/png; name=\"Logo =?us-ascii?Q?VMime=2Epng?=\"");

		VASSERT_EQ("2.1", 1, p2.getParameterCount());
		VASSERT_EQ("2.2", "name", PARAM_NAME(p2, 0));
		VASSERT_EQ("2.3", "Logo VMime.png", PARAM_VALUE(p2, 0));
	}

	// Parse parameters with non-significant whitespaces
	void testParseNonSignificantWS()
	{
		parameterizedHeaderField p1;
		p1.parse(" \t X   \r\n");

		VASSERT_EQ("1.1", "X", FIELD_VALUE(p1));

		parameterizedHeaderField p2;
		p2.parse(" X  ; param1 =  value1 \r\n");

		VASSERT_EQ("2.1", 1, p2.getParameterCount());
		VASSERT_EQ("2.2", "X", FIELD_VALUE(p2));
		VASSERT_EQ("2.3", "param1", PARAM_NAME(p2, 0));
		VASSERT_EQ("2.4", "value1", PARAM_VALUE(p2, 0));
	}

	// Encode "tspecials"
	void testEncodeTSpecials()
	{
		VASSERT_EQ(" 1", "p=\"val(ue\"",  vmime::create <vmime::parameter>("p", "val(ue")->generate());
		VASSERT_EQ(" 2", "p=\"val)ue\"",  vmime::create <vmime::parameter>("p", "val)ue")->generate());
		VASSERT_EQ(" 3", "p=\"val<ue\"",  vmime::create <vmime::parameter>("p", "val<ue")->generate());
		VASSERT_EQ(" 4", "p=\"val>ue\"",  vmime::create <vmime::parameter>("p", "val>ue")->generate());
		VASSERT_EQ(" 5", "p=\"val@ue\"",  vmime::create <vmime::parameter>("p", "val@ue")->generate());
		VASSERT_EQ(" 6", "p=\"val,ue\"",  vmime::create <vmime::parameter>("p", "val,ue")->generate());
		VASSERT_EQ(" 7", "p=\"val;ue\"",  vmime::create <vmime::parameter>("p", "val;ue")->generate());
		VASSERT_EQ(" 8", "p=\"val:ue\"",  vmime::create <vmime::parameter>("p", "val:ue")->generate());
		VASSERT_EQ(" 9", "p=\"val/ue\"",  vmime::create <vmime::parameter>("p", "val/ue")->generate());
		VASSERT_EQ("10", "p=\"val[ue\"",  vmime::create <vmime::parameter>("p", "val[ue")->generate());
		VASSERT_EQ("11", "p=\"val]ue\"",  vmime::create <vmime::parameter>("p", "val]ue")->generate());
		VASSERT_EQ("12", "p=\"val?ue\"",  vmime::create <vmime::parameter>("p", "val?ue")->generate());
		VASSERT_EQ("13", "p=\"val=ue\"",  vmime::create <vmime::parameter>("p", "val=ue")->generate());
		VASSERT_EQ("14", "p=\"val ue\"",  vmime::create <vmime::parameter>("p", "val ue")->generate());
		VASSERT_EQ("15", "p=\"val\tue\"", vmime::create <vmime::parameter>("p", "val\tue")->generate());
	}

	// http://sourceforge.net/projects/vmime/forums/forum/237356/topic/3812278
	void testEncodeTSpecialsInRFC2231()
	{
		VASSERT_EQ("1", "filename*=UTF-8''my_file_name_%C3%B6%C3%A4%C3%BC_%281%29.txt",
			vmime::create <vmime::parameter>("filename", "my_file_name_\xc3\xb6\xc3\xa4\xc3\xbc_(1).txt")->generate());
	}

VMIME_TEST_SUITE_END

