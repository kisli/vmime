//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_NET_IMAP_IMAPPARSER_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPPARSER_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/base.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/charset.hpp"
#include "vmime/exception.hpp"

#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/progressListener.hpp"

#include "vmime/utility/encoder/b64Encoder.hpp"
#include "vmime/utility/encoder/qpEncoder.hpp"

#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"

#include "vmime/platform.hpp"

#include "vmime/net/timeoutHandler.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/tracer.hpp"

#include "vmime/net/imap/IMAPTag.hpp"

#include <vector>
#include <stdexcept>
#include <memory>


//#define DEBUG_RESPONSE 1


#if DEBUG_RESPONSE
#   include <iostream>
#endif


/** Make the parsing of a component fail.
  */
#define VIMAP_PARSER_FAIL() \
	{  \
		parser.m_errorResponseLine = makeResponseLine(getComponentName(), line, pos);  \
		return false;  \
	}

/** Make the parsing of a component fail if a condition is not matched.
  * If the specified expression does not resolve to "true", the parsing
  * will fail.
  *
  * @param cond condition expression
  */
#define VIMAP_PARSER_FAIL_UNLESS(cond) \
	if (!(cond))  \
	{  \
		VIMAP_PARSER_FAIL();  \
	}

/** Check for a token and advance.
  * If the token is not matched, parsing will fail.
  *
  * @param type token class
  */
#define VIMAP_PARSER_CHECK(type) \
	VIMAP_PARSER_FAIL_UNLESS(parser.check <type>(line, &pos))

/** Check for an optional token and advance.
  * If the token is not matched, parsing will continue anyway.
  *
  * @param type token class
  */
#define VIMAP_PARSER_TRY_CHECK(type) \
	(parser.check <type>(line, &pos))

/** Get a token and advance.
  * If the token is not matched, parsing will fail.
  *
  * @param type token class
  * @param variable variable which will receive pointer to the retrieved token
  */
#define VIMAP_PARSER_GET(type, variable)  /* raw pointer version */ \
	{  \
		VIMAP_PARSER_FAIL_UNLESS(variable = parser.get <type>(line, &pos));  \
	}

#define VIMAP_PARSER_GET_PTR(type, variable)  /* auto_ptr/shared_ptr version */ \
	{ \
		variable.reset(parser.get <type>(line, &pos));  \
		VIMAP_PARSER_FAIL_UNLESS(variable.get());  \
	}

/** Get an optional token and advance.
  * If the token is not matched, parsing will continue anyway.
  */
#define VIMAP_PARSER_TRY_GET(type, variable)  /* raw pointer version */ \
	(variable = parser.get <type>(line, &pos))

#define VIMAP_PARSER_TRY_GET_PTR(type, variable) /* auto_ptr/shared_ptr version */ \
	(variable.reset(parser.get <type>(line, &pos)), variable.get())

/** Get a token and advance. Token will be pushed back to a vector.
  * If the token is not matched, parsing will fail.
  *
  * @param type token class
  * @param variable variable of type std::vector<> to which the retrieved
  * token will be pushed
  */
#define VIMAP_PARSER_GET_PUSHBACK(type, variable) \
	{  \
		std::auto_ptr <type> v(parser.get <type>(line, &pos));  \
		VIMAP_PARSER_FAIL_UNLESS(v.get());  \
		variable.push_back(v.release());  \
	}

/** Check for a token which takes an argument and advance.
  * If the token is not matched, parsing will fail.
  *
  * @param type token class
  */
#define VIMAP_PARSER_CHECK_WITHARG(type, arg) \
	VIMAP_PARSER_FAIL_UNLESS(parser.checkWithArg <type>(line, &pos, arg))

/** Check for an optional token which takes an argument and advance.
  * If the token is not matched, parsing will continue anyway.
  *
  * @param type token class
  */
#define VIMAP_PARSER_TRY_CHECK_WITHARG(type, arg) \
	(parser.checkWithArg <type>(line, &pos, arg))



namespace vmime {
namespace net {
namespace imap {


#if DEBUG_RESPONSE
	static int IMAPParserDebugResponse_level = 0;
	static std::vector <string> IMAPParserDebugResponse_stack;

	class IMAPParserDebugResponse
	{
	public:

		IMAPParserDebugResponse(const string& name, string& line, const size_t currentPos, const bool &result)
			: m_name(name), m_line(line), m_pos(currentPos), m_result(result)
		{
			++IMAPParserDebugResponse_level;
			IMAPParserDebugResponse_stack.push_back(name);

			for (int i = 0 ; i < IMAPParserDebugResponse_level ; ++i)
				std::cout << "  ";

			std::cout << "ENTER(" << m_name << "), pos=" << m_pos;
			std::cout << std::endl;

			for (std::vector <string>::iterator it = IMAPParserDebugResponse_stack.begin() ;
			     it != IMAPParserDebugResponse_stack.end() ; ++it)
			{
				std::cout << "> " << *it << " ";
			}

			std::cout << std::endl;
			std::cout << string(m_line.begin() + (m_pos < 30 ? 0U : m_pos - 30),
				m_line.begin() + std::min(m_line.length(), m_pos + 30)) << std::endl;

			for (size_t i = (m_pos < 30 ? m_pos : (m_pos - (m_pos - 30))) ; i != 0 ; --i)
				std::cout << " ";

			std::cout << "^" << std::endl;
		}

		~IMAPParserDebugResponse()
		{
			for (int i = 0 ; i < IMAPParserDebugResponse_level ; ++i)
				std::cout << "  ";

			std::cout << "LEAVE(" << m_name << "), result=";
			std::cout << (m_result ? "TRUE" : "FALSE") << ", pos=" << m_pos;
			std::cout << std::endl;

			--IMAPParserDebugResponse_level;
			IMAPParserDebugResponse_stack.pop_back();
		}

	private:

		const string& m_name;
		string& m_line;
		size_t m_pos;
		const bool& m_result;
	};


	#define DEBUG_ENTER_COMPONENT(x, result) \
		IMAPParserDebugResponse dbg(x, line, *currentPos, result)

	#define DEBUG_FOUND(x, y) \
		std::cout << "FOUND: " << x << ": " << y << std::endl;
#else
	#define DEBUG_ENTER_COMPONENT(x, result)
	#define DEBUG_FOUND(x, y)
#endif


class VMIME_EXPORT IMAPParser : public object
{
public:

	IMAPParser()
		: m_tag(), m_socket(), m_progress(NULL), m_strict(false),
		  m_literalHandler(NULL), m_timeoutHandler()
	{
	}


	/** Set the tag currently used by this parser.
	  *
	  * @param tag IMAP command tag
	  */
	void setTag(shared_ptr <IMAPTag> tag)
	{
		m_tag = tag;
	}

	/** Return the tag currently used by this parser.
	  *
	  * @return IMAP command tag
	  */
	shared_ptr <const IMAPTag> getTag() const
	{
		return m_tag.lock();
	}

	/** Set the socket currently used by this parser to receive data
	  * from server.
	  *
	  * @param sok socket
	  */
	void setSocket(shared_ptr <socket> sok)
	{
		m_socket = sok;
	}

	/** Set the timeout handler currently used by this parser.
	  *
	  * @param toh timeout handler
	  */
	void setTimeoutHandler(shared_ptr <timeoutHandler> toh)
	{
		m_timeoutHandler = toh;
	}

	/** Set the tracer currently used by this parser.
	  *
	  * @param tr tracer
	  */
	void setTracer(shared_ptr <tracer> tr)
	{
		m_tracer = tr;
	}

	/** Set whether we operate in strict mode (this may not work
	  * with some servers which are not fully standard-compliant).
	  *
	  * @param strict true to operate in strict mode, or false
	  * to operate in default, relaxed mode
	  */
	void setStrict(const bool strict)
	{
		m_strict = strict;
	}

	/** Return true if the parser operates in strict mode, or
	  * false otherwise.
	  *
	  * @return true if we are in strict mode, false otherwise
	  */
	bool isStrict() const
	{
		return m_strict;
	}



	//
	// literalHandler : literal content handler
	//

	class component;

	class literalHandler
	{
	public:

		virtual ~literalHandler() { }


		// Abstract target class
		class target
		{
		protected:

			target(utility::progressListener* progress) : m_progress(progress) {}
			target(const target&) {}

		public:

			virtual ~target() { }


			utility::progressListener* progressListener() { return (m_progress); }

			virtual void putData(const string& chunk) = 0;

		private:

			utility::progressListener* m_progress;
		};


		// Target: put in a string
		class targetString : public target
		{
		public:

			targetString(utility::progressListener* progress, vmime::string& str)
				: target(progress), m_string(str) { }

			const vmime::string& string() const { return (m_string); }
			vmime::string& string() { return (m_string); }


			void putData(const vmime::string& chunk)
			{
				m_string += chunk;
			}

		private:

			vmime::string& m_string;
		};


		// Target: redirect to an output stream
		class targetStream : public target
		{
		public:

			targetStream(utility::progressListener* progress, utility::outputStream& stream)
				: target(progress), m_stream(stream) { }

			const utility::outputStream& stream() const { return (m_stream); }
			utility::outputStream& stream() { return (m_stream); }


			void putData(const string& chunk)
			{
				m_stream.write(chunk.data(), chunk.length());
			}

		private:

			utility::outputStream& m_stream;
		};


		// Called when the parser needs to know what to do with a literal
		//    . comp: the component in which we are at this moment
		//    . data: data specific to the component (may not be used)
		//
		// Returns :
		//    . == NULL to put the literal into the response
		//    . != NULL to redirect the literal to the specified target

		virtual target* targetFor(const component& comp, const int data) = 0;
	};


	//
	// Base class for a terminal or a non-terminal
	//

	class component
	{
	public:

		component() { }
		virtual ~component() { }

		virtual const string getComponentName() const = 0;

		bool parse(IMAPParser& parser, string& line, size_t* currentPos)
		{
			bool res = false;
			DEBUG_ENTER_COMPONENT(getComponentName(), res);
			res = parseImpl(parser, line, currentPos);
			return res;
		}

		virtual bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) = 0;


		const string makeResponseLine(const string& comp, const string& line,
		                              const size_t pos)
		{
#if DEBUG_RESPONSE
			if (pos > line.length())
				std::cout << "WARNING: component::makeResponseLine(): pos > line.length()" << std::endl;
#endif

			string result(line.substr(0, pos));
			result += "[^]";   // indicates current parser position
			result += line.substr(pos, line.length());
			if (!comp.empty()) result += " [" + comp + "]";

			return (result);
		}
	};


#define COMPONENT_ALIAS(parent, name) \
	class name : public parent \
	{ \
		virtual const string getComponentName() const { return #name; }  \
	public: \
		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) \
		{ \
			return parent::parseImpl(parser, line, currentPos); \
		} \
	}

#define DECLARE_COMPONENT(name) \
	class name : public component  \
	{  \
		virtual const string getComponentName() const { return #name; }  \
	public:


	//
	// Parse one character
	//

	template <char C>
	class one_char : public component
	{
	public:

		const string getComponentName() const
		{
			return string("one_char <") + C + ">";
		}

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			const size_t pos = *currentPos;

			if (pos < line.length() && line[pos] == C)
			{
				*currentPos = pos + 1;
				return true;
			}
			else
			{
				return false;
			}
		}
	};


	//
	// SPACE  ::= <ASCII SP, space, 0x20>
	//

	DECLARE_COMPONENT(SPACE)

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			while (pos < line.length() && (line[pos] == ' ' || line[pos] == '\t'))
				++pos;

			if (pos > *currentPos)
			{
				*currentPos = pos;
				return true;
			}
			else
			{
				return false;
			}
		}
	};


	//
	// CR    ::= <ASCII CR, carriage return, 0x0D>
	// LF    ::= <ASCII LF, line feed, 0x0A>
	// CRLF  ::= CR LF
	//

	DECLARE_COMPONENT(CRLF)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_TRY_CHECK(SPACE);

			if (pos + 1 < line.length() &&
			    line[pos] == 0x0d && line[pos + 1] == 0x0a)
			{
				*currentPos = pos + 2;
				return true;
			}
			else
			{
				return false;
			}
		}
	};


	//
	// SPACE           ::= <ASCII SP, space, 0x20>
	// CTL             ::= <any ASCII control character and DEL, 0x00 - 0x1f, 0x7f>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// ATOM_CHAR       ::= <any CHAR except atom_specials>
	// atom_specials   ::= "(" / ")" / "{" / SPACE / CTL / list_wildcards / quoted_specials
	// list_wildcards  ::= "%" / "*"
	// quoted_specials ::= <"> / "\"
	//
	// tag             ::= 1*<any ATOM_CHAR except "+">    (named "xtag")
	//

	DECLARE_COMPONENT(xtag)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			bool end = false;

			string tagString;
			tagString.reserve(10);

			while (!end && pos < line.length())
			{
				const unsigned char c = line[pos];

				switch (c)
				{
				case '+':
				case '(':
				case ')':
				case '{':
				case 0x20:  // SPACE
				case '%':   // list_wildcards
				case '*':   // list_wildcards
				case '"':   // quoted_specials
				case '\\':  // quoted_specials

					end = true;
					break;

				default:

					if (c <= 0x1f || c >= 0x7f)
						end = true;
					else
					{
						tagString += c;
						++pos;
					}

					break;
				}
			}

			if (tagString == string(*parser.getTag()))
			{
				*currentPos = pos;
				return true;
			}
			else
			{
				// Invalid tag
				return false;
			}
		}
	};


	//
	// digit     ::= "0" / digit_nz
	// digit_nz  ::= "1" / "2" / "3" / "4" / "5" / "6" / "7" / "8" / "9"
	//
	// number    ::= 1*digit
	//               ;; Unsigned 32-bit integer
	//               ;; (0 <= n < 4,294,967,296)
	//

	DECLARE_COMPONENT(number)

		number(const bool nonZero = false)
			: m_nonZero(nonZero), m_value(0)
		{
		}

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			bool valid = true;
			unsigned int val = 0;

			while (valid && pos < line.length())
			{
				const char c = line[pos];

				if (c >= '0' && c <= '9')
				{
					val = (val * 10) + (c - '0');
					++pos;
				}
				else
				{
					valid = false;
				}
			}

			// Check for non-null length (and for non-zero number)
			if (!(m_nonZero && val == 0) && pos != *currentPos)
			{
				m_value = val;
				*currentPos = pos;
				return true;
			}
			else
			{
				return false;
			}
		}

	private:

		const bool m_nonZero;
		unsigned long m_value;

	public:

		unsigned long value() const { return (m_value); }
	};


	// nz_number  ::= digit_nz *digit
	//                ;; Non-zero unsigned 32-bit integer
	//                ;; (0 < n < 4,294,967,296)
	//

	class nz_number : public number
	{
	public:

		nz_number() : number(true)
		{
		}
	};


	//
	// uniqueid    ::= nz_number
	//                 ;; Strictly ascending
	//

	class uniqueid : public nz_number
	{
	public:

		uniqueid() : nz_number()
		{
		}
	};


	// uid-range       = (uniqueid ":" uniqueid)
	//                   ; two uniqueid values and all values
	//                   ; between these two regards of order.
	//                   ; Example: 2:4 and 4:2 are equivalent.

	DECLARE_COMPONENT(uid_range)

		uid_range()
			: m_uniqueid1(NULL), m_uniqueid2(NULL)
		{
		}

		~uid_range()
		{
			delete m_uniqueid1;
			delete m_uniqueid2;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(uniqueid, m_uniqueid1);
			VIMAP_PARSER_CHECK(one_char <','>);
			VIMAP_PARSER_GET(uniqueid, m_uniqueid2);

			*currentPos = pos;

			return true;
		}

	private:

		uniqueid* m_uniqueid1;
		uniqueid* m_uniqueid2;

	public:

		uniqueid* uniqueid1() const { return m_uniqueid1; }
		uniqueid* uniqueid2() const { return m_uniqueid2; }
	};


	//
	// uid-set         = (uniqueid / uid-range) *("," uid-set)
	//

	DECLARE_COMPONENT(uid_set)

		uid_set()
			: m_uniqueid(NULL), m_uid_range(NULL), m_next_uid_set(NULL)
		{
		}

		~uid_set()
		{
			delete m_uniqueid;
			delete m_uid_range;
			delete m_next_uid_set;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			// We have either a 'uid_range' or a 'uniqueid'
			if (!VIMAP_PARSER_TRY_GET(IMAPParser::uid_range, m_uid_range))
				VIMAP_PARSER_GET(IMAPParser::uniqueid, m_uniqueid);

			// And maybe another 'uid-set' following
			if (VIMAP_PARSER_TRY_CHECK(one_char <','>))
				VIMAP_PARSER_GET(IMAPParser::uid_set, m_next_uid_set);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::uniqueid* m_uniqueid;
		IMAPParser::uid_range* m_uid_range;

		IMAPParser::uid_set* m_next_uid_set;

	public:

		IMAPParser::uniqueid* uniqueid() const { return m_uniqueid; }
		IMAPParser::uid_range* uid_range() const { return m_uid_range; }

		IMAPParser::uid_set* next_uid_set() const { return m_next_uid_set; }
	};


	//
	// text       ::= 1*TEXT_CHAR
	//
	// CHAR       ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// TEXT_CHAR  ::= <any CHAR except CR and LF>
	//

	DECLARE_COMPONENT(text)

		text(bool allow8bits = false, const char except = 0)
			: m_allow8bits(allow8bits), m_except(except)
		{
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;
			size_t len = 0;

			if (m_allow8bits || !parser.isStrict())
			{
				const unsigned char except = m_except;

				for (bool end = false ; !end && pos < line.length() ; )
				{
					const unsigned char c = line[pos];

					if (c == 0x00 || c == 0x0d || c == 0x0a || c == except)
					{
						end = true;
					}
					else
					{
						++pos;
						++len;
					}
				}
			}
			else
			{
				const unsigned char except = m_except;

				for (bool end = false ; !end && pos < line.length() ; )
				{
					const unsigned char c = line[pos];

					if (c < 0x01 || c > 0x7f || c == 0x0d || c == 0x0a || c == except)
					{
						end = true;
					}
					else
					{
						++pos;
						++len;
					}
				}
			}

			if (len == 0)
				VIMAP_PARSER_FAIL();

			m_value.resize(len);
			std::copy(line.begin() + *currentPos, line.begin() + pos, m_value.begin());

			*currentPos = pos;

			return true;
		}

	private:

		string m_value;
		const bool m_allow8bits;
		const char m_except;

	public:

		const string& value() const { return (m_value); }
	};


	class text8 : public text
	{
	public:

		text8() : text(true)
		{
		}
	};


	template <char C>
	class text_except : public text
	{
	public:

		text_except() : text(false, C)
		{
		}
	};


	template <char C>
	class text8_except : public text
	{
	public:

		text8_except() : text(true, C)
		{
		}
	};


	//
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	//

	DECLARE_COMPONENT(QUOTED_CHAR)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			const unsigned char c = static_cast <unsigned char>(pos < line.length() ? line[pos] : 0);

			if (c >= 0x01 && c <= 0x7f &&   // 0x01 - 0x7f
			    c != '"' && c != '\\' &&    // quoted_specials
			    c != '\r' && c != '\n')     // CR and LF
			{
				m_value = c;
				*currentPos = pos + 1;
			}
			else if (c == '\\' && pos + 1 < line.length() &&
			         (line[pos + 1] == '"' || line[pos + 1] == '\\'))
			{
				m_value = line[pos + 1];
				*currentPos = pos + 2;
			}
			else
			{
				VIMAP_PARSER_FAIL();
			}

			return true;
		}

	private:

		char m_value;

	public:

		char value() const { return (m_value); }
	};


	//
	// quoted          ::= <"> *QUOTED_CHAR <">
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	//

	DECLARE_COMPONENT(quoted_text)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;
			size_t len = 0;
			bool valid = false;

			m_value.reserve(line.length() - pos);

			for (bool end = false, quoted = false ; !end && pos < line.length() ; )
			{
				const unsigned char c = line[pos];

				if (quoted)
				{
					if (c == '"' || c == '\\')
						m_value += c;
					else
					{
						m_value += '\\';
						m_value += c;
					}

					quoted = false;

					++pos;
					++len;
				}
				else
				{
					if (c == '\\')
					{
						quoted = true;

						++pos;
						++len;
					}
					else if (c == '"')
					{
						valid = true;
						end = true;
					}
					else if (c >= 0x01 && c <= 0x7f &&  // CHAR
					         c != 0x0a && c != 0x0d)    // CR and LF
					{
						m_value += c;

						++pos;
						++len;
					}
					else
					{
						valid = false;
						end = true;
					}
				}
			}

			if (!valid)
				VIMAP_PARSER_FAIL();

			*currentPos = pos;

			return true;
		}

	private:

		string m_value;

	public:

		const string& value() const { return (m_value); }
	};


	//
	// nil  ::= "NIL"
	//

	DECLARE_COMPONENT(NIL)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK_WITHARG(special_atom, "nil");

			*currentPos = pos;

			return true;
		}
	};


	//
	// string          ::= quoted / literal    ----> named 'xstring'
	//
	// nil             ::= "NIL"
	// quoted          ::= <"> *QUOTED_CHAR <">
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// literal         ::= "{" number "}" CRLF *CHAR8
	//                     ;; Number represents the number of CHAR8 octets
	// CHAR8           ::= <any 8-bit octet except NUL, 0x01 - 0xff>
	//

	DECLARE_COMPONENT(xstring)

		xstring(const bool canBeNIL = false, component* comp = NULL, const int data = 0)
			: m_canBeNIL(canBeNIL), m_isNIL(true), m_component(comp), m_data(data)
		{
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (m_canBeNIL &&
			    VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "nil"))
			{
				// NIL
				m_isNIL = true;
			}
			else
			{
				pos = *currentPos;

				m_isNIL = false;

				// quoted ::= <"> *QUOTED_CHAR <">
				if (VIMAP_PARSER_TRY_CHECK(one_char <'"'>))
				{
					shared_ptr <quoted_text> text;
					VIMAP_PARSER_GET_PTR(quoted_text, text);
					VIMAP_PARSER_CHECK(one_char <'"'>);

					if (parser.m_literalHandler != NULL)
					{
						literalHandler::target* target =
							parser.m_literalHandler->targetFor(*m_component, m_data);

						if (target != NULL)
						{
							m_value = "[literal-handler]";

							const size_t length = text->value().length();
							utility::progressListener* progress = target->progressListener();

							if (progress)
							{
								progress->start(length);
							}

							target->putData(text->value());

							if (progress)
							{
								progress->progress(length, length);
								progress->stop(length);
							}

							delete (target);
						}
						else
						{
							m_value = text->value();
						}
					}
					else
					{
						m_value = text->value();
					}

					DEBUG_FOUND("string[quoted]", "<length=" << m_value.length() << ", value='" << m_value << "'>");
				}
				// literal ::= "{" number "}" CRLF *CHAR8
				else
				{
					VIMAP_PARSER_CHECK(one_char <'{'>);

					shared_ptr <number> num;
					VIMAP_PARSER_GET_PTR(number, num);

					const size_t length = num->value();

					VIMAP_PARSER_CHECK(one_char <'}'> );

					VIMAP_PARSER_CHECK(CRLF);


					if (parser.m_literalHandler != NULL)
					{
						literalHandler::target* target =
							parser.m_literalHandler->targetFor(*m_component, m_data);

						if (target != NULL)
						{
							m_value = "[literal-handler]";

							parser.m_progress = target->progressListener();
							parser.readLiteral(*target, length);
							parser.m_progress = NULL;

							delete (target);
						}
						else
						{
							literalHandler::targetString target(NULL, m_value);
							parser.readLiteral(target, length);
						}
					}
					else
					{
						literalHandler::targetString target(NULL, m_value);
						parser.readLiteral(target, length);
					}

					line += parser.readLine();

					DEBUG_FOUND("string[literal]", "<length=" << length << ", value='" << m_value << "'>");
				}
			}

			*currentPos = pos;

			return true;
		}

	private:

		bool m_canBeNIL;
		bool m_isNIL;
		string m_value;

		component* m_component;
		const int m_data;

	public:

		bool isNIL() const { return m_isNIL; }

		const string& value() const { return (m_value); }
		void setValue(const string& val) { m_value = val; }
	};


	//
	// nstring         ::= string / nil
	//

	class nstring : public xstring
	{
	public:

		const string getComponentName() const
		{
			return "nstring";
		}

		nstring(component* comp = NULL, const int data = 0)
			: xstring(true, comp, data)
		{
		}
	};


	//
	// astring ::= atom / string
	//

	DECLARE_COMPONENT(astring)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			shared_ptr <xstring> str;
			VIMAP_PARSER_TRY_GET_PTR(xstring, str);

			if (str)
			{
				m_value = str->value();
			}
			else
			{
				shared_ptr <atom> at;
				VIMAP_PARSER_GET_PTR(atom, at);
				m_value = at->value();
			}

			*currentPos = pos;

			return true;
		}

	private:

		string m_value;

	public:

		const string& value() const { return (m_value); }
	};


	//
	// atom            ::= 1*ATOM_CHAR
	//
	// ATOM_CHAR       ::= <any CHAR except atom_specials>
	// atom_specials   ::= "(" / ")" / "{" / SPACE / CTL / list_wildcards / quoted_specials
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// CTL             ::= <any ASCII control character and DEL, 0x00 - 0x1f, 0x7f>
	// list_wildcards  ::= "%" / "*"
	// quoted_specials ::= <"> / "\"
	// SPACE           ::= <ASCII SP, space, 0x20>
	//

	DECLARE_COMPONENT(atom)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;
			size_t len = 0;

			for (bool end = false ; !end && pos < line.length() ; )
			{
				const unsigned char c = line[pos];

				switch (c)
				{
				case '(':
				case ')':
				case '{':
				case 0x20:  // SPACE
				case '%':   // list_wildcards
				case '*':   // list_wildcards
				case '"':   // quoted_specials
				case '\\':  // quoted_specials

				case '[':
				case ']':   // for "special_atom"

					end = true;
					break;

				default:

					if (c <= 0x1f || c >= 0x7f)
						end = true;
					else
					{
						++pos;
						++len;
					}
				}
			}

			if (len != 0)
			{
				m_value.resize(len);
				std::copy(line.begin() + *currentPos, line.begin() + pos, m_value.begin());

				*currentPos = pos;
			}
			else
			{
				VIMAP_PARSER_FAIL();
			}

			return true;
		}

	private:

		string m_value;

	public:

		const string& value() const { return (m_value); }
	};


	//
	// special atom (eg. "CAPABILITY", "FLAGS", "STATUS"...)
	//
	//  " Except as noted otherwise, all alphabetic characters are case-
	//    insensitive. The use of upper or lower case characters to define
	//    token strings is for editorial clarity only. Implementations MUST
	//    accept these strings in a case-insensitive fashion. "
	//

	class special_atom : public atom
	{
	public:

		const std::string getComponentName() const
		{
			return string("special_atom <") + m_string + ">";
		}

		special_atom(const char* str)
			: m_string(str)   // 'string' must be in lower-case
		{
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!atom::parseImpl(parser, line, &pos))
				return false;

			const char* cmp = value().c_str();
			const char* with = m_string;

			bool ok = true;

			while (ok && *cmp && *with)
			{
				ok = (std::tolower(*cmp, std::locale()) == *with);

				++cmp;
				++with;
			}

			if (!ok || *cmp || *with)
				VIMAP_PARSER_FAIL();

			*currentPos = pos;

			return true;
		}

	private:

		const char* m_string;
	};


	//
	// text_mime2 ::= "=?" <charset> "?" <encoding> "?" <encoded-text> "?="
	//                ;; Syntax defined in [MIME-HDRS]
	//

	DECLARE_COMPONENT(text_mime2)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			shared_ptr <atom> theCharset, theEncoding;
			shared_ptr <text> theText;

			VIMAP_PARSER_CHECK(one_char <'='> );
			VIMAP_PARSER_CHECK(one_char <'?'> );

			VIMAP_PARSER_GET_PTR(atom, theCharset);

			VIMAP_PARSER_CHECK(one_char <'?'> );

			VIMAP_PARSER_GET_PTR(atom, theEncoding);

			VIMAP_PARSER_CHECK(one_char <'?'> );

			VIMAP_PARSER_GET_PTR(text8_except <'?'> , theText);

			VIMAP_PARSER_CHECK(one_char <'?'> );
			VIMAP_PARSER_CHECK(one_char <'='> );

			m_charset = theCharset->value();

			// Decode text
			std::auto_ptr <utility::encoder::encoder> theEncoder;

			if (theEncoding->value()[0] == 'q' || theEncoding->value()[0] == 'Q')
			{
				// Quoted-printable
				theEncoder.reset(new utility::encoder::qpEncoder());
				theEncoder->getProperties()["rfc2047"] = true;
			}
			else if (theEncoding->value()[0] == 'b' || theEncoding->value()[0] == 'B')
			{
				// Base64
				theEncoder.reset(new utility::encoder::b64Encoder());
			}

			if (theEncoder.get())
			{
				utility::inputStreamStringAdapter in(theText->value());
				utility::outputStreamStringAdapter out(m_value);

				theEncoder->decode(in, out);
			}
			// No decoder available
			else
			{
				m_value = theText->value();
			}

			*currentPos = pos;

			return true;
		}

	private:

		vmime::charset m_charset;
		string m_value;

	public:

		const vmime::charset& charset() const { return (m_charset); }
		const string& value() const { return (m_value); }
	};


	// seq-number      = nz-number / "*"
	//                    ; message sequence number (COPY, FETCH, STORE
	//                    ; commands) or unique identifier (UID COPY,
	//                    ; UID FETCH, UID STORE commands).

	DECLARE_COMPONENT(seq_number)

		seq_number()
			: m_number(NULL), m_star(false)
		{
		}

		~seq_number()
		{
			delete m_number;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'*'> ))
			{
				m_star = true;
				m_number = NULL;
			}
			else
			{
				m_star = false;
				VIMAP_PARSER_GET(IMAPParser::number, m_number);
			}

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::number* m_number;
		bool m_star;

	public:

		const IMAPParser::number* number() const { return m_number; }
		bool star() const { return m_star; }
	};


	// seq-range       = seq-number ":" seq-number
	//                    ; two seq-number values and all values between
	//                    ; these two regardless of order.
	//                    ; Example: 2:4 and 4:2 are equivalent and indicate
	//                    ; values 2, 3, and 4.

	DECLARE_COMPONENT(seq_range)

		seq_range()
			: m_first(NULL), m_last(NULL)
		{
		}

		~seq_range()
		{
			delete m_first;
			delete m_last;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(seq_number, m_first);

			VIMAP_PARSER_CHECK(one_char <'*'> );

			VIMAP_PARSER_GET(seq_number, m_last);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::seq_number* m_first;
		IMAPParser::seq_number* m_last;

	public:

		const IMAPParser::seq_number* first() const { return m_first; }
		const IMAPParser::seq_number* last() const { return m_last; }
	};


	// sequence-set    = (seq-number / seq-range) *("," sequence-set)
	//                    ; set of seq-number values, regardless of order.
	//                    ; Servers MAY coalesce overlaps and/or execute the
	//                    ; sequence in any order.
	//                    ; Example: a message sequence number set of
	//                    ; 2,4:7,9,12:* for a mailbox with 15 messages is
	//                    ; equivalent to 2,4,5,6,7,9,12,13,14,15

	DECLARE_COMPONENT(sequence_set)

		sequence_set()
			: m_number(NULL), m_range(NULL), m_nextSet(NULL)
		{
		}

		~sequence_set()
		{
			delete m_number;
			delete m_range;
			delete m_nextSet;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::seq_range, m_range))
				VIMAP_PARSER_GET(IMAPParser::seq_number, m_number);

			if (VIMAP_PARSER_TRY_CHECK(one_char <','> ))
				VIMAP_PARSER_GET(sequence_set, m_nextSet);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::seq_number* m_number;
		IMAPParser::seq_range* m_range;
		IMAPParser::sequence_set* m_nextSet;

	public:

		const IMAPParser::seq_number* seq_number() const { return m_number; }
		const IMAPParser::seq_range* seq_range() const { return m_range; }
		const IMAPParser::sequence_set* next_sequence_set() const { return m_nextSet; }
	};


	// mod-sequence-value  = 1*DIGIT
	//                        ;; Positive unsigned 64-bit integer
	//                        ;; (mod-sequence)
	//                        ;; (1 <= n < 18,446,744,073,709,551,615)

	DECLARE_COMPONENT(mod_sequence_value)

		mod_sequence_value()
			: m_value(0)
		{
		}

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			bool valid = true;
			vmime_uint64 val = 0;

			while (valid && pos < line.length())
			{
				const char c = line[pos];

				if (c >= '0' && c <= '9')
				{
					val = (val * 10) + (c - '0');
					++pos;
				}
				else
				{
					valid = false;
				}
			}

			m_value = val;

			*currentPos = pos;

			return true;
		}

	private:

		vmime_uint64 m_value;

	public:

		vmime_uint64 value() const { return m_value; }
	};


	//
	// flag            ::= "\Answered" / "\Flagged" / "\Deleted" /
	//                     "\Seen" / "\Draft" / flag_keyword / flag_extension
	//
	// flag_extension  ::= "\" atom
	//                     ;; Future expansion.  Client implementations
	//                     ;; MUST accept flag_extension flags.  Server
	//                     ;; implementations MUST NOT generate
	//                     ;; flag_extension flags except as defined by
	//                     ;; future standard or standards-track
	//                     ;; revisions of this specification.
	//
	// flag_keyword    ::= atom
	//

	DECLARE_COMPONENT(flag)

		flag()
			: m_type(UNKNOWN), m_flag_keyword(NULL)
		{
		}

		~flag()
		{
			delete (m_flag_keyword);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'\\'> ))
			{
				if (VIMAP_PARSER_TRY_CHECK(one_char <'*'> ))
				{
					m_type = STAR;
				}
				else
				{
					shared_ptr <atom> at;
					VIMAP_PARSER_GET_PTR(atom, at);

					const string name = utility::stringUtils::toLower(at->value());

					if (name == "answered")
						m_type = ANSWERED;
					else if (name == "flagged")
						m_type = FLAGGED;
					else if (name == "deleted")
						m_type = DELETED;
					else if (name == "seen")
						m_type = SEEN;
					else if (name == "draft")
						m_type = DRAFT;
					else
					{
						m_type = UNKNOWN;
						m_name = name;
					}
				}
			}
			else
			{
				m_type = KEYWORD_OR_EXTENSION;
				VIMAP_PARSER_GET(atom, m_flag_keyword);
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			UNKNOWN,
			ANSWERED,
			FLAGGED,
			DELETED,
			SEEN,
			DRAFT,
			KEYWORD_OR_EXTENSION,
			STAR       // * = custom flags allowed
		};

	private:

		Type m_type;
		string m_name;

		IMAPParser::atom* m_flag_keyword;

	public:

		Type type() const { return (m_type); }
		const string& name() const { return (m_name); }

		const IMAPParser::atom* flag_keyword() const { return (m_flag_keyword); }
	};


	//
	// flag_list ::= "(" #flag ")"
	//

	DECLARE_COMPONENT(flag_list)

		~flag_list()
		{
			for (std::vector <flag*>::iterator it = m_flags.begin() ;
			     it != m_flags.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
			{
				VIMAP_PARSER_GET_PUSHBACK(flag, m_flags);
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <flag*> m_flags;

	public:

		const std::vector <flag*>& flags() const { return (m_flags); }
	};


	//
	// mailbox ::= "INBOX" / astring
	//             ;; INBOX is case-insensitive.  All case variants of
	//             ;; INBOX (e.g. "iNbOx") MUST be interpreted as INBOX
	//             ;; not as an astring.  Refer to section 5.1 for
	//             ;; further semantic details of mailbox names.
	//

	DECLARE_COMPONENT(mailbox)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "inbox"))
			{
				m_type = INBOX;
				m_name = "INBOX";
			}
			else
			{
				m_type = OTHER;

				shared_ptr <astring> astr;
				VIMAP_PARSER_GET_PTR(astring, astr);
				m_name = astr->value();
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			INBOX,
			OTHER
		};

	private:

		Type m_type;
		string m_name;

	public:

		Type type() const { return (m_type); }
		const string& name() const { return (m_name); }
	};


	//
	// mailbox_flag := "\Marked" / "\Noinferiors" /
	//                 "\Noselect" / "\Unmarked" / flag_extension
	//

	DECLARE_COMPONENT(mailbox_flag)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'\\'> ))
			{
				shared_ptr <atom> at;
				VIMAP_PARSER_GET_PTR(atom, at);
				const string name = utility::stringUtils::toLower(at->value());

				m_type = UNKNOWN;  // default

				switch (name[0])
				{
				case 'a':

					if (name == "all")
						m_type = SPECIALUSE_ALL;
					else if (name == "archive")
						m_type = SPECIALUSE_ARCHIVE;

					break;

				case 'd':

					if (name == "drafts")
						m_type = SPECIALUSE_DRAFTS;

					break;

				case 'f':

					if (name == "flagged")
						m_type = SPECIALUSE_FLAGGED;

					break;

				case 'h':

					if (name == "haschildren")
						m_type = HASCHILDREN;
					else if (name == "hasnochildren")
						m_type = HASNOCHILDREN;

					break;

				case 'i':

					if (name == "important")
						m_type = SPECIALUSE_IMPORTANT;

					break;

				case 'j':

					if (name == "junk")
						m_type = SPECIALUSE_JUNK;

					break;

				case 'm':

					if (name == "marked")
						m_type = MARKED;

				case 'n':

					if (name == "noinferiors")
						m_type = NOINFERIORS;
					else if (name == "noselect")
						m_type = NOSELECT;

				case 's':

					if (name == "sent")
						m_type = SPECIALUSE_SENT;

					break;

				case 't':

					if (name == "trash")
						m_type = SPECIALUSE_TRASH;

					break;

				case 'u':

					if (name == "unmarked")
						m_type = UNMARKED;

					break;
				}

				if (m_type == UNKNOWN)
					m_name = "\\" + name;
			}
			else
			{
				shared_ptr <atom> at;
				VIMAP_PARSER_GET_PTR(atom, at);
				const string name = utility::stringUtils::toLower(at->value());

				m_type = UNKNOWN;
				m_name = name;
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			// RFC-3348 - Child Mailbox Extension
			HASCHILDREN,
			HASNOCHILDREN,

			// RFC-6154 - Special-Use Mailboxes
			SPECIALUSE_ALL,
			SPECIALUSE_ARCHIVE,
			SPECIALUSE_DRAFTS,
			SPECIALUSE_FLAGGED,
			SPECIALUSE_JUNK,
			SPECIALUSE_SENT,
			SPECIALUSE_TRASH,
			SPECIALUSE_IMPORTANT,    // draft

			// Standard mailbox flags
			UNKNOWN,
			MARKED,
			NOINFERIORS,
			NOSELECT,
			UNMARKED
		};

	private:

		Type m_type;
		string m_name;

	public:

		Type type() const { return (m_type); }
		const string& name() const { return (m_name); }
	};


	//
	// mailbox_flag_list ::= "(" #(mailbox_flag) ")"
	//

	DECLARE_COMPONENT(mailbox_flag_list)

		~mailbox_flag_list()
		{
			for (std::vector <mailbox_flag*>::iterator it = m_flags.begin() ;
			     it != m_flags.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
			{
				VIMAP_PARSER_GET_PUSHBACK(mailbox_flag, m_flags);
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <mailbox_flag*> m_flags;

	public:

		const std::vector <mailbox_flag*>& flags() const { return (m_flags); }
	};


	//
	// mailbox_list ::= mailbox_flag_list SPACE
	//                  (<"> QUOTED_CHAR <"> / nil) SPACE mailbox
	//

	DECLARE_COMPONENT(mailbox_list)

		mailbox_list()
			: m_mailbox_flag_list(NULL),
			  m_mailbox(NULL), m_quoted_char('\0')
		{
		}

		~mailbox_list()
		{
			delete (m_mailbox_flag_list);
			delete (m_mailbox);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::mailbox_flag_list, m_mailbox_flag_list);

			VIMAP_PARSER_CHECK(SPACE);

			if (!VIMAP_PARSER_TRY_CHECK(NIL))
			{
				VIMAP_PARSER_CHECK(one_char <'"'> );

				shared_ptr <QUOTED_CHAR> qc;
				VIMAP_PARSER_GET_PTR(QUOTED_CHAR, qc);
				m_quoted_char = qc->value();

				VIMAP_PARSER_CHECK(one_char <'"'> );
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::mailbox, m_mailbox);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::mailbox_flag_list* m_mailbox_flag_list;
		IMAPParser::mailbox* m_mailbox;
		char m_quoted_char;

	public:

		const IMAPParser::mailbox_flag_list* mailbox_flag_list() const { return (m_mailbox_flag_list); }
		const IMAPParser::mailbox* mailbox() const { return (m_mailbox); }
		char quoted_char() const { return (m_quoted_char); }
	};


	//
	// auth_type ::= atom
	//               ;; Defined by [IMAP-AUTH]
	//

	DECLARE_COMPONENT(auth_type)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			shared_ptr <atom> at;
			VIMAP_PARSER_GET_PTR(atom, at);
			m_name = utility::stringUtils::toLower(at->value());

			if (m_name == "kerberos_v4")
				m_type = KERBEROS_V4;
			else if (m_name == "gssapi")
				m_type = GSSAPI;
			else if (m_name == "skey")
				m_type = SKEY;
			else
				m_type = UNKNOWN;

			return true;
		}


		enum Type
		{
			UNKNOWN,

			// RFC 1731 - IMAP4 Authentication Mechanisms
			KERBEROS_V4,
			GSSAPI,
			SKEY
		};

	private:

		Type m_type;
		string m_name;

	public:

		Type type() const { return (m_type); }
		const string name() const { return (m_name); }
	};


	//
	// status-att-val  = ("MESSAGES" SP number) /
	//                   ("RECENT" SP number) /
	//                   ("UIDNEXT" SP nz-number) /
	//                   ("UIDVALIDITY" SP nz-number) /
	//                   ("UNSEEN" SP number)
	//
	// IMAP Extension for Conditional STORE (RFC-4551):
	//
	//   status-att-val      =/ "HIGHESTMODSEQ" SP mod-sequence-valzer
	//                          ;; extends non-terminal defined in [IMAPABNF].
	//                          ;; Value 0 denotes that the mailbox doesn't
	//                          ;; support persistent mod-sequences
	//

	DECLARE_COMPONENT(status_att_val)

		status_att_val()
			: m_value(NULL)
		{
		}

		~status_att_val()
		{
			delete m_value;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			// "HIGHESTMODSEQ" SP mod-sequence-valzer
			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "highestmodseq"))
			{
				m_type = HIGHESTMODSEQ;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::mod_sequence_value, m_value);
			}
			else
			{
				if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "messages"))
				{
					m_type = MESSAGES;
				}
				else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "recent"))
				{
					m_type = RECENT;
				}
				else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidnext"))
				{
					m_type = UIDNEXT;
				}
				else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidvalidity"))
				{
					m_type = UIDVALIDITY;
				}
				else
				{
					VIMAP_PARSER_CHECK_WITHARG(special_atom, "unseen");
					m_type = UNSEEN;
				}

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::number, m_value);
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			// Extensions
			HIGHESTMODSEQ,

			// Standard IMAP
			MESSAGES,
			RECENT,
			UIDNEXT,
			UIDVALIDITY,
			UNSEEN
		};

	private:

		Type m_type;
		IMAPParser::component* m_value;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::number* value_as_number() const
		{
			return dynamic_cast <IMAPParser::number *>(m_value);
		}

		const IMAPParser::mod_sequence_value* value_as_mod_sequence_value() const
		{
			return dynamic_cast <IMAPParser::mod_sequence_value *>(m_value);
		}
	};


	// status-att-list = status-att-val *(SP status-att-val)

	DECLARE_COMPONENT(status_att_list)

		~status_att_list()
		{
			for (std::vector <status_att_val*>::iterator it = m_values.begin() ;
			     it != m_values.end() ; ++it)
			{
				delete *it;
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET_PUSHBACK(IMAPParser::status_att_val, m_values);

			while (VIMAP_PARSER_TRY_CHECK(SPACE))
				VIMAP_PARSER_GET_PUSHBACK(IMAPParser::status_att_val, m_values);

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <status_att_val*> m_values;

	public:

		const std::vector <status_att_val*>& values() const { return m_values; }
	};


	//
	// capability ::= "AUTH=" auth_type / atom
	//                ;; New capabilities MUST begin with "X" or be
	//                ;; registered with IANA as standard or standards-track
	//

	DECLARE_COMPONENT(capability)

		capability()
			: m_auth_type(NULL), m_atom(NULL)
		{
		}

		~capability()
		{
			delete (m_auth_type);
			delete (m_atom);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			std::auto_ptr <IMAPParser::atom> at;
			VIMAP_PARSER_GET_PTR(IMAPParser::atom, at);

			string value = at->value();
			const char* str = value.c_str();

			if ((str[0] == 'a' || str[0] == 'A') &&
			    (str[1] == 'u' || str[1] == 'U') &&
			    (str[2] == 't' || str[2] == 'T') &&
			    (str[3] == 'h' || str[3] == 'H') &&
			    (str[4] == '='))
			{
				size_t pos = 5;
				m_auth_type = parser.get <IMAPParser::auth_type>(value, &pos);
			}
			else
			{
				m_atom = at.release();
			}

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::auth_type* m_auth_type;
		IMAPParser::atom* m_atom;

	public:

		const IMAPParser::auth_type* auth_type() const { return (m_auth_type); }
		const IMAPParser::atom* atom() const { return (m_atom); }
	};


	//
	// capability_data ::= "CAPABILITY" SPACE [1#capability SPACE] "IMAP4rev1"
	//                     [SPACE 1#capability]
	//                     ;; IMAP4rev1 servers which offer RFC 1730
	//                     ;; compatibility MUST list "IMAP4" as the first
	//                     ;; capability.
	//

	DECLARE_COMPONENT(capability_data)

		~capability_data()
		{
			for (std::vector <capability*>::iterator it = m_capabilities.begin() ;
			     it != m_capabilities.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK_WITHARG(special_atom, "capability");

			while (VIMAP_PARSER_TRY_CHECK(SPACE))
			{
				capability* cap;

				if (parser.isStrict() || m_capabilities.empty())
				{
					VIMAP_PARSER_GET(capability, cap);
				}
				else
				{
					VIMAP_PARSER_TRY_GET(capability, cap);  // allow SPACE at end of line (Apple iCloud IMAP server)
				}

				if (!cap)
					break;

				m_capabilities.push_back(cap);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <capability*> m_capabilities;

	public:

		const std::vector <capability*>& capabilities() const { return (m_capabilities); }
	};


	//
	// date_day_fixed  ::= (SPACE digit) / 2digit
	//                    ;; Fixed-format version of date_day
	//
	// date_month      ::= "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" /
	//                    "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"
	//
	// date_year       ::= 4digit
	//
	// time            ::= 2digit ":" 2digit ":" 2digit
	//                     ;; Hours minutes seconds
	//
	// zone            ::= ("+" / "-") 4digit
	//                     ;; Signed four-digit value of hhmm representing
	//                     ;; hours and minutes west of Greenwich (that is,
	//                     ;; (the amount that the given time differs from
	//                     ;; Universal Time).  Subtracting the timezone
	//                     ;; from the given time will give the UT form.
	//                     ;; The Universal Time zone is "+0000".
	//
	// date_time       ::= <"> date_day_fixed "-" date_month "-" date_year
	//                     SPACE time SPACE zone <">
	//

	DECLARE_COMPONENT(date_time)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			// <"> date_day_fixed "-" date_month "-" date_year
			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_TRY_CHECK(SPACE);

			shared_ptr <number> nd;
			VIMAP_PARSER_GET_PTR(number, nd);

			VIMAP_PARSER_CHECK(one_char <'-'> );

			shared_ptr <atom> amo;
			VIMAP_PARSER_GET_PTR(atom, amo);

			VIMAP_PARSER_CHECK(one_char <'-'> );

			shared_ptr <number> ny;
			VIMAP_PARSER_GET_PTR(number, ny);

			VIMAP_PARSER_TRY_CHECK(SPACE);

			// 2digit ":" 2digit ":" 2digit
			shared_ptr <number> nh;
			VIMAP_PARSER_GET_PTR(number, nh);

			VIMAP_PARSER_CHECK(one_char <':'> );

			shared_ptr <number> nmi;
			VIMAP_PARSER_GET_PTR(number, nmi);

			VIMAP_PARSER_CHECK(one_char <':'> );

			shared_ptr <number> ns;
			VIMAP_PARSER_GET_PTR(number, ns);

			VIMAP_PARSER_TRY_CHECK(SPACE);

			// ("+" / "-") 4digit
			int sign = 1;

			if (!(VIMAP_PARSER_TRY_CHECK(one_char <'+'> )))
				VIMAP_PARSER_CHECK(one_char <'-'> );

			shared_ptr <number> nz;
			VIMAP_PARSER_GET_PTR(number, nz);

			VIMAP_PARSER_CHECK(one_char <'"'> );


			m_datetime.setHour(static_cast <int>(std::min(std::max(nh->value(), 0ul), 23ul)));
			m_datetime.setMinute(static_cast <int>(std::min(std::max(nmi->value(), 0ul), 59ul)));
			m_datetime.setSecond(static_cast <int>(std::min(std::max(ns->value(), 0ul), 59ul)));

			const int zone = static_cast <int>(nz->value());
			const int zh = zone / 100;   // hour offset
			const int zm = zone % 100;   // minute offset

			m_datetime.setZone(((zh * 60) + zm) * sign);

			m_datetime.setDay(static_cast <int>(std::min(std::max(nd->value(), 1ul), 31ul)));
			m_datetime.setYear(static_cast <int>(ny->value()));

			const string month(utility::stringUtils::toLower(amo->value()));
			int mon = vmime::datetime::JANUARY;

			if (month.length() >= 3)
			{
				switch (month[0])
				{
				case 'j':
				{
					switch (month[1])
					{
					case 'a': mon = vmime::datetime::JANUARY; break;
					case 'u':
					{
						switch (month[2])
						{
						case 'n': mon = vmime::datetime::JUNE; break;
						default:  mon = vmime::datetime::JULY; break;
						}

						break;
					}

					}

					break;
				}
				case 'f': mon = vmime::datetime::FEBRUARY; break;
				case 'm':
				{
					switch (month[2])
					{
					case 'r': mon = vmime::datetime::MARCH; break;
					default:  mon = vmime::datetime::MAY; break;
					}

					break;
				}
				case 'a':
				{
					switch (month[1])
					{
					case 'p': mon = vmime::datetime::APRIL; break;
					default:  mon = vmime::datetime::AUGUST; break;
					}

					break;
				}
				case 's': mon = vmime::datetime::SEPTEMBER; break;
				case 'o': mon = vmime::datetime::OCTOBER; break;
				case 'n': mon = vmime::datetime::NOVEMBER; break;
				case 'd': mon = vmime::datetime::DECEMBER; break;
				}
			}

			m_datetime.setMonth(mon);

			*currentPos = pos;

			return true;
		}

	private:

		vmime::datetime m_datetime;
	};


	//
	// header_fld_name ::= astring
	//

	COMPONENT_ALIAS(astring, header_fld_name);


	//
	// header_list     ::= "(" 1#header_fld_name ")"
	//

	DECLARE_COMPONENT(header_list)

		~header_list()
		{
			for (std::vector <header_fld_name*>::iterator it = m_fld_names.begin() ;
			     it != m_fld_names.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
			{
				VIMAP_PARSER_GET_PUSHBACK(header_fld_name, m_fld_names);
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <header_fld_name*> m_fld_names;

	public:

		const std::vector <header_fld_name*>& fld_names() const { return (m_fld_names); }
	};


	//
	// body_extension  ::= nstring / number / "(" 1#body_extension ")"
	//                     ;; Future expansion.  Client implementations
	//                     ;; MUST accept body_extension fields.  Server
	//                     ;; implementations MUST NOT generate
	//                     ;; body_extension fields except as defined by
	//                     ;; future standard or standards-track
	//                     ;; revisions of this specification.
	//

	DECLARE_COMPONENT(body_extension)

		body_extension()
			: m_nstring(NULL), m_number(NULL)
		{
		}

		~body_extension()
		{
			delete (m_nstring);
			delete (m_number);

			for (std::vector <body_extension*>::iterator it = m_body_extensions.begin() ;
			     it != m_body_extensions.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> ))
			{
				VIMAP_PARSER_GET_PUSHBACK(body_extension, m_body_extensions);

				while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
				{
					VIMAP_PARSER_GET_PUSHBACK(body_extension, m_body_extensions);
					VIMAP_PARSER_TRY_CHECK(SPACE);
				}
			}
			else
			{
				if (!VIMAP_PARSER_TRY_GET(IMAPParser::nstring, m_nstring))
					VIMAP_PARSER_GET(IMAPParser::number, m_number);
			}

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::nstring* m_nstring;
		IMAPParser::number* m_number;

		std::vector <body_extension*> m_body_extensions;

	public:

		IMAPParser::nstring* nstring() const { return (m_nstring); }
		IMAPParser::number* number() const { return (m_number); }

		const std::vector <body_extension*>& body_extensions() const { return (m_body_extensions); }
	};


	//
	// section_text    ::= "HEADER" / "HEADER.FIELDS" [".NOT"]
	//                     SPACE header_list / "TEXT" / "MIME"
	//

	DECLARE_COMPONENT(section_text)

		section_text()
			: m_header_list(NULL)
		{
		}

		~section_text()
		{
			delete (m_header_list);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			// "HEADER.FIELDS" [".NOT"] SPACE header_list
			const bool b1 = VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "header.fields.not");
			const bool b2 = (b1 ? false : VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "header.fields"));

			if (b1 || b2)
			{
				m_type = b1 ? HEADER_FIELDS_NOT : HEADER_FIELDS;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::header_list, m_header_list);
			}
			// "HEADER"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "header"))
			{
				m_type = HEADER;
			}
			// "MIME"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "mime"))
			{
				m_type = MIME;
			}
			// "TEXT"
			else
			{
				m_type = TEXT;

				VIMAP_PARSER_CHECK_WITHARG(special_atom, "text");
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			HEADER,
			HEADER_FIELDS,
			HEADER_FIELDS_NOT,
			MIME,
			TEXT
		};

	private:

		Type m_type;
		IMAPParser::header_list* m_header_list;

	public:

		Type type() const { return (m_type); }
		const IMAPParser::header_list* header_list() const { return (m_header_list); }
	};


	//
	// section         ::= "[" [section_text / (nz_number *["." nz_number]
	//                     ["." (section_text / "MIME")])] "]"
	//

	DECLARE_COMPONENT(section)

		section()
			: m_section_text1(NULL), m_section_text2(NULL)
		{
		}

		~section()
		{
			delete (m_section_text1);
			delete (m_section_text2);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'['> );

			if (!VIMAP_PARSER_TRY_CHECK(one_char <']'> ))
			{
				if (!VIMAP_PARSER_TRY_GET(section_text, m_section_text1))
				{
					shared_ptr <nz_number> num;
					VIMAP_PARSER_GET_PTR(nz_number, num);
					m_nz_numbers.push_back(static_cast <unsigned int>(num->value()));

					while (VIMAP_PARSER_TRY_CHECK(one_char <'.'> ))
					{
						if (VIMAP_PARSER_TRY_GET_PTR(nz_number, num))
						{
							m_nz_numbers.push_back(static_cast <unsigned int>(num->value()));
						}
						else
						{
							VIMAP_PARSER_GET(section_text, m_section_text2);
							break;
						}
					}
				}

				VIMAP_PARSER_CHECK(one_char <']'> );
			}

			*currentPos = pos;

			return true;
		}

	private:

		section_text* m_section_text1;
		section_text* m_section_text2;
		std::vector <unsigned int> m_nz_numbers;

	public:

		const section_text* section_text1() const { return (m_section_text1); }
		const section_text* section_text2() const { return (m_section_text2); }
		const std::vector <unsigned int>& nz_numbers() const { return (m_nz_numbers); }
	};


	//
	// addr_adl        ::= nstring
	//                     ;; Holds route from [RFC-822] route-addr if
	//                     ;; non-NIL
	//
	// addr_host       ::= nstring
	//                     ;; NIL indicates [RFC-822] group syntax.
	//                     ;; Otherwise, holds [RFC-822] domain name
	//
	// addr_mailbox    ::= nstring
	//                     ;; NIL indicates end of [RFC-822] group; if
	//                     ;; non-NIL and addr_host is NIL, holds
	//                     ;; [RFC-822] group name.
	//                     ;; Otherwise, holds [RFC-822] local-part
	//
	// addr_name       ::= nstring
	//                     ;; Holds phrase from [RFC-822] mailbox if
	//                     ;; non-NIL
	//
	// address         ::= "(" addr_name SPACE addr_adl SPACE addr_mailbox
	//                     SPACE addr_host ")"
	//

	DECLARE_COMPONENT(address)

		address()
			: m_addr_name(NULL), m_addr_adl(NULL),
			  m_addr_mailbox(NULL), m_addr_host(NULL)
		{
		}

		~address()
		{
			delete (m_addr_name);
			delete (m_addr_adl);
			delete (m_addr_mailbox);
			delete (m_addr_host);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );
			VIMAP_PARSER_GET(nstring, m_addr_name);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(nstring, m_addr_adl);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(nstring, m_addr_mailbox);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(nstring, m_addr_host);
			VIMAP_PARSER_CHECK(one_char <')'> );

			*currentPos = pos;

			return true;
		}

	private:

		nstring* m_addr_name;
		nstring* m_addr_adl;
		nstring* m_addr_mailbox;
		nstring* m_addr_host;

	public:

		nstring* addr_name() const { return (m_addr_name); }
		nstring* addr_adl() const { return (m_addr_adl); }
		nstring* addr_mailbox() const { return (m_addr_mailbox); }
		nstring* addr_host() const { return (m_addr_host); }
	};


	//
	// address_list ::= "(" 1*address ")" / nil
	//

	DECLARE_COMPONENT(address_list)

		~address_list()
		{
			for (std::vector <address*>::iterator it = m_addresses.begin() ;
			     it != m_addresses.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_CHECK(NIL))
			{
				VIMAP_PARSER_CHECK(one_char <'('> );

				while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
				{
					VIMAP_PARSER_GET_PUSHBACK(address, m_addresses);
					VIMAP_PARSER_TRY_CHECK(SPACE);
				}
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <address*> m_addresses;

	public:

		const std::vector <address*>& addresses() const { return (m_addresses); }
	};


	//
	// env_bcc         ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_bcc);


	//
	// env_cc          ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_cc);


	//
	// env_date        ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_date);


	//
	// env_from        ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_from);


	//
	// env_in_reply_to ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_in_reply_to);


	//
	// env_message_id  ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_message_id);


	//
	// env_reply_to    ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_reply_to);


	//
	// env_sender      ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_sender);


	//
	// env_subject     ::= nstring
	//

	COMPONENT_ALIAS(nstring, env_subject);


	//
	// env_to          ::= "(" 1*address ")" / nil
	//

	COMPONENT_ALIAS(address_list, env_to);


	//
	// envelope ::= "(" env_date SPACE env_subject SPACE env_from
	//              SPACE env_sender SPACE env_reply_to SPACE env_to
	//              SPACE env_cc SPACE env_bcc SPACE env_in_reply_to
	//              SPACE env_message_id ")"
	//

	DECLARE_COMPONENT(envelope)

		envelope()
			: m_env_date(NULL), m_env_subject(NULL),
			  m_env_from(NULL), m_env_sender(NULL), m_env_reply_to(NULL),
			  m_env_to(NULL), m_env_cc(NULL), m_env_bcc(NULL),
			  m_env_in_reply_to(NULL), m_env_message_id(NULL)
		{
		}

		~envelope()
		{
			delete (m_env_date);
			delete (m_env_subject);
			delete (m_env_from);
			delete (m_env_sender);
			delete (m_env_reply_to);
			delete (m_env_to);
			delete (m_env_cc);
			delete (m_env_bcc);
			delete (m_env_in_reply_to);
			delete (m_env_message_id);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			VIMAP_PARSER_GET(IMAPParser::env_date, m_env_date);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_subject, m_env_subject);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_from, m_env_from);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_sender, m_env_sender);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_reply_to, m_env_reply_to);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_to, m_env_to);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_cc, m_env_cc);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_bcc, m_env_bcc);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_in_reply_to, m_env_in_reply_to);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_message_id, m_env_message_id);

			VIMAP_PARSER_CHECK(one_char <')'> );

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::env_date* m_env_date;
		IMAPParser::env_subject* m_env_subject;
		IMAPParser::env_from* m_env_from;
		IMAPParser::env_sender* m_env_sender;
		IMAPParser::env_reply_to* m_env_reply_to;
		IMAPParser::env_to* m_env_to;
		IMAPParser::env_cc* m_env_cc;
		IMAPParser::env_bcc* m_env_bcc;
		IMAPParser::env_in_reply_to* m_env_in_reply_to;
		IMAPParser::env_message_id* m_env_message_id;

	public:

		const IMAPParser::env_date* env_date() const { return (m_env_date); }
		const IMAPParser::env_subject* env_subject() const { return (m_env_subject); }
		const IMAPParser::env_from* env_from() const { return (m_env_from); }
		const IMAPParser::env_sender* env_sender() const { return (m_env_sender); }
		const IMAPParser::env_reply_to* env_reply_to() const { return (m_env_reply_to); }
		const IMAPParser::env_to* env_to() const { return (m_env_to); }
		const IMAPParser::env_cc* env_cc() const { return (m_env_cc); }
		const IMAPParser::env_bcc* env_bcc() const { return (m_env_bcc); }
		const IMAPParser::env_in_reply_to* env_in_reply_to() const { return (m_env_in_reply_to); }
		const IMAPParser::env_message_id* env_message_id() const { return (m_env_message_id); }
	};


	//
	// body_fld_desc   ::= nstring
	//

	COMPONENT_ALIAS(nstring, body_fld_desc);


	//
	// body_fld_id     ::= nstring
	//

	COMPONENT_ALIAS(nstring, body_fld_id);


	//
	// body_fld_md5    ::= nstring
	//

	COMPONENT_ALIAS(nstring, body_fld_md5);


	//
	// body_fld_octets ::= number
	//

	COMPONENT_ALIAS(number, body_fld_octets);


	//
	// body_fld_lines  ::= number
	//

	COMPONENT_ALIAS(number, body_fld_lines);


	//
	// body_fld_enc    ::= (<"> ("7BIT" / "8BIT" / "BINARY" / "BASE64"/
	//                     "QUOTED-PRINTABLE") <">) / string
	//

	class body_fld_enc : public nstring
	{
	public:

		const string getComponentName() const
		{
			return "body_fld_enc";
		}

		body_fld_enc()
		{
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!xstring::parseImpl(parser, line, &pos))
				return false;

			// " When an IMAP4 client sends a FETCH (bodystructure) request
			//   to a server that is running the Exchange Server 2007 IMAP4
			//   service, a corrupted response is sent as a reply "
			//   (see http://support.microsoft.com/kb/975918/en-us)
			//
			// Fail in strict mode
			if (isNIL() && parser.isStrict())
			{
				VIMAP_PARSER_FAIL();
			}

			*currentPos = pos;

			return true;
		}
	};


	//
	// body_fld_param_item ::= string SPACE string
	//

	DECLARE_COMPONENT(body_fld_param_item)

		body_fld_param_item()
			: m_string1(NULL), m_string2(NULL)
		{
		}

		~body_fld_param_item()
		{
			delete (m_string1);
			delete (m_string2);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!parser.isStrict())
			{
				// Some servers send an <atom> instead of a <string> here:
				// eg. ... (CHARSET "X-UNKNOWN") ...
				if (!VIMAP_PARSER_TRY_GET(xstring, m_string1))
				{
					shared_ptr <atom> at;
					VIMAP_PARSER_GET_PTR(atom, at);

					m_string1 = new xstring();
					m_string1->setValue(at->value());
				}
			}
			else
			{
				VIMAP_PARSER_GET(xstring, m_string1);
			}

			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(xstring, m_string2);

			DEBUG_FOUND("body_fld_param_item", "<" << m_string1->value() << ", " << m_string2->value() << ">");

			*currentPos = pos;

			return true;
		}

	private:

		xstring* m_string1;
		xstring* m_string2;

	public:

		const xstring* string1() const { return (m_string1); }
		const xstring* string2() const { return (m_string2); }
	};


	//
	// body_fld_param  ::= "(" 1#(body_fld_param_item) ")" / nil
	//

	DECLARE_COMPONENT(body_fld_param)

		~body_fld_param()
		{
			for (std::vector <body_fld_param_item*>::iterator it = m_items.begin() ;
			     it != m_items.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> ))
			{
				VIMAP_PARSER_GET_PUSHBACK(body_fld_param_item, m_items);

				while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
				{
					VIMAP_PARSER_CHECK(SPACE);
					VIMAP_PARSER_GET_PUSHBACK(body_fld_param_item, m_items);
				}
			}
			else
			{
				VIMAP_PARSER_CHECK(NIL);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <body_fld_param_item*> m_items;

	public:

		const std::vector <body_fld_param_item*>& items() const { return (m_items); }
	};


	//
	// body_fld_dsp    ::= "(" string SPACE body_fld_param ")" / nil
	//

	DECLARE_COMPONENT(body_fld_dsp)

		body_fld_dsp()
			: m_string(NULL), m_body_fld_param(NULL)
		{
		}

		~body_fld_dsp()
		{
			delete (m_string);
			delete (m_body_fld_param);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> ))
			{
				VIMAP_PARSER_GET(xstring, m_string);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(class body_fld_param, m_body_fld_param);
				VIMAP_PARSER_CHECK(one_char <')'> );
			}
			else
			{
				VIMAP_PARSER_CHECK(NIL);
			}

			*currentPos = pos;

			return true;
		}

	private:

		class xstring* m_string;
		class body_fld_param* m_body_fld_param;

	public:

		const class xstring* str() const { return (m_string); }
		const class body_fld_param* body_fld_param() const { return (m_body_fld_param); }
	};


	//
	// body_fld_lang   ::= nstring / "(" 1#string ")"
	//

	DECLARE_COMPONENT(body_fld_lang)

		~body_fld_lang()
		{
			for (std::vector <xstring*>::iterator it = m_strings.begin() ;
			     it != m_strings.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> ))
			{
				VIMAP_PARSER_GET_PUSHBACK(xstring, m_strings);

				while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
				{
					VIMAP_PARSER_CHECK(SPACE);
					VIMAP_PARSER_GET_PUSHBACK(xstring, m_strings);
				}
			}
			else
			{
				VIMAP_PARSER_GET_PUSHBACK(nstring, m_strings);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <xstring*> m_strings;

	public:

		const std::vector <xstring*>& strings() const { return (m_strings); }
	};


	//
	// body_fields     ::= body_fld_param SPACE body_fld_id SPACE
	//                     body_fld_desc SPACE body_fld_enc SPACE
	//                     body_fld_octets
	//

	DECLARE_COMPONENT(body_fields)

		body_fields()
			: m_body_fld_param(NULL), m_body_fld_id(NULL),
			  m_body_fld_desc(NULL), m_body_fld_enc(NULL), m_body_fld_octets(NULL)
		{
		}

		~body_fields()
		{
			delete (m_body_fld_param);
			delete (m_body_fld_id);
			delete (m_body_fld_desc);
			delete (m_body_fld_enc);
			delete (m_body_fld_octets);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::body_fld_param, m_body_fld_param);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_id, m_body_fld_id);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_desc, m_body_fld_desc);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_enc, m_body_fld_enc);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_octets, m_body_fld_octets);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::body_fld_param* m_body_fld_param;
		IMAPParser::body_fld_id* m_body_fld_id;
		IMAPParser::body_fld_desc* m_body_fld_desc;
		IMAPParser::body_fld_enc* m_body_fld_enc;
		IMAPParser::body_fld_octets* m_body_fld_octets;

	public:

		const IMAPParser::body_fld_param* body_fld_param() const { return (m_body_fld_param); }
		const IMAPParser::body_fld_id* body_fld_id() const { return (m_body_fld_id); }
		const IMAPParser::body_fld_desc* body_fld_desc() const { return (m_body_fld_desc); }
		const IMAPParser::body_fld_enc* body_fld_enc() const { return (m_body_fld_enc); }
		const IMAPParser::body_fld_octets* body_fld_octets() const { return (m_body_fld_octets); }
	};


	//
	// media_subtype   ::= string
	//                     ;; Defined in [MIME-IMT]
	//

	COMPONENT_ALIAS(xstring, media_subtype);


	//
	// media_text      ::= <"> "TEXT" <"> SPACE media_subtype
	//                     ;; Defined in [MIME-IMT]
	//

	DECLARE_COMPONENT(media_text)

		media_text()
			: m_media_subtype(NULL)
		{
		}

		~media_text()
		{
			delete (m_media_subtype);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK_WITHARG(special_atom, "text");
			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::media_subtype, m_media_subtype);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::media_subtype* m_media_subtype;

	public:

		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
	};


	//
	// media_message   ::= <"> "MESSAGE" <"> SPACE <"> "RFC822" <">
	//                     ;; Defined in [MIME-IMT]
	//

	DECLARE_COMPONENT(media_message)

		media_message()
			: m_media_subtype(NULL)
		{
		}

		~media_message()
		{
			delete m_media_subtype;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK_WITHARG(special_atom, "message");
			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK(SPACE);

			//VIMAP_PARSER_CHECK(one_char <'"'> );
			//VIMAP_PARSER_CHECK_WITHARG(special_atom, "rfc822");
			//VIMAP_PARSER_CHECK(one_char <'"'> );

			VIMAP_PARSER_GET(IMAPParser::media_subtype, m_media_subtype);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::media_subtype* m_media_subtype;

	public:

		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
	};


	//
	// media_basic     ::= (<"> ("APPLICATION" / "AUDIO" / "IMAGE" /
	//                     "MESSAGE" / "VIDEO") <">) / string)
	//                     SPACE media_subtype
	//                     ;; Defined in [MIME-IMT]

	DECLARE_COMPONENT(media_basic)

		media_basic()
			: m_media_type(NULL), m_media_subtype(NULL)
		{
		}

		~media_basic()
		{
			delete (m_media_type);
			delete (m_media_subtype);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(xstring, m_media_type);

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::media_subtype, m_media_subtype);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::xstring* m_media_type;
		IMAPParser::media_subtype* m_media_subtype;

	public:

		const IMAPParser::xstring* media_type() const { return (m_media_type); }
		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
	};


	//
	// body_ext_1part  ::= body_fld_md5 [SPACE body_fld_dsp
	//                     [SPACE body_fld_lang
	//                     [SPACE 1#body_extension]]]
	//                     ;; MUST NOT be returned on non-extensible
	//                     ;; "BODY" fetch
	//

	DECLARE_COMPONENT(body_ext_1part)

		body_ext_1part()
			: m_body_fld_md5(NULL), m_body_fld_dsp(NULL), m_body_fld_lang(NULL)
		{
		}

		~body_ext_1part()
		{
			delete (m_body_fld_md5);
			delete (m_body_fld_dsp);
			delete (m_body_fld_lang);

			for (std::vector <body_extension*>::iterator it = m_body_extensions.begin() ;
			     it != m_body_extensions.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::body_fld_md5, m_body_fld_md5);

			// [SPACE body_fld_dsp
			if (VIMAP_PARSER_TRY_CHECK(SPACE))
			{
				VIMAP_PARSER_GET(IMAPParser::body_fld_dsp, m_body_fld_dsp);

				// [SPACE body_fld_lang
				if (VIMAP_PARSER_TRY_CHECK(SPACE))
				{
					VIMAP_PARSER_GET(IMAPParser::body_fld_lang, m_body_fld_lang);

					// [SPACE 1#body_extension]
					if (VIMAP_PARSER_TRY_CHECK(SPACE))
					{
						VIMAP_PARSER_GET_PUSHBACK(body_extension, m_body_extensions);

						while (VIMAP_PARSER_TRY_CHECK(SPACE))
							VIMAP_PARSER_GET_PUSHBACK(body_extension, m_body_extensions);
					}
				}
			}

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::body_fld_md5* m_body_fld_md5;
		IMAPParser::body_fld_dsp* m_body_fld_dsp;
		IMAPParser::body_fld_lang* m_body_fld_lang;

		std::vector <body_extension*> m_body_extensions;

	public:

		const IMAPParser::body_fld_md5* body_fld_md5() const { return (m_body_fld_md5); }
		const IMAPParser::body_fld_dsp* body_fld_dsp() const { return (m_body_fld_dsp); }
		const IMAPParser::body_fld_lang* body_fld_lang() const { return (m_body_fld_lang); }

		const std::vector <body_extension*> body_extensions() const { return (m_body_extensions); }
	};


	//
	// body_ext_mpart  ::= body_fld_param
	//                     [SPACE body_fld_dsp SPACE body_fld_lang
	//                     [SPACE 1#body_extension]]
	//                     ;; MUST NOT be returned on non-extensible
	//                     ;; "BODY" fetch

	DECLARE_COMPONENT(body_ext_mpart)

		body_ext_mpart()
			: m_body_fld_param(NULL), m_body_fld_dsp(NULL), m_body_fld_lang(NULL)
		{
		}

		~body_ext_mpart()
		{
			delete (m_body_fld_param);
			delete (m_body_fld_dsp);
			delete (m_body_fld_lang);

			for (std::vector <body_extension*>::iterator it = m_body_extensions.begin() ;
			     it != m_body_extensions.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::body_fld_param, m_body_fld_param);

			// [SPACE body_fld_dsp SPACE body_fld_lang [SPACE 1#body_extension]]
			if (VIMAP_PARSER_TRY_CHECK(SPACE))
			{
				VIMAP_PARSER_GET(IMAPParser::body_fld_dsp, m_body_fld_dsp);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::body_fld_lang, m_body_fld_lang);

				// [SPACE 1#body_extension]
				if (VIMAP_PARSER_TRY_CHECK(SPACE))
				{
					VIMAP_PARSER_GET_PUSHBACK(body_extension, m_body_extensions);

					while (VIMAP_PARSER_TRY_CHECK(SPACE))
						VIMAP_PARSER_GET_PUSHBACK(body_extension, m_body_extensions);
				}
			}

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::body_fld_param* m_body_fld_param;
		IMAPParser::body_fld_dsp* m_body_fld_dsp;
		IMAPParser::body_fld_lang* m_body_fld_lang;

		std::vector <body_extension*> m_body_extensions;

	public:

		const IMAPParser::body_fld_param* body_fld_param() const { return (m_body_fld_param); }
		const IMAPParser::body_fld_dsp* body_fld_dsp() const { return (m_body_fld_dsp); }
		const IMAPParser::body_fld_lang* body_fld_lang() const { return (m_body_fld_lang); }

		const std::vector <body_extension*> body_extensions() const { return (m_body_extensions); }
	};


	//
	// body_type_basic ::= media_basic SPACE body_fields
	//                     ;; MESSAGE subtype MUST NOT be "RFC822"
	//

	DECLARE_COMPONENT(body_type_basic)

		body_type_basic()
			: m_media_basic(NULL), m_body_fields(NULL)
		{
		}

		~body_type_basic()
		{
			delete (m_media_basic);
			delete (m_body_fields);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::media_basic, m_media_basic);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fields, m_body_fields);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::media_basic* m_media_basic;
		IMAPParser::body_fields* m_body_fields;

	public:

		const IMAPParser::media_basic* media_basic() const { return (m_media_basic); }
		const IMAPParser::body_fields* body_fields() const { return (m_body_fields); }
	};


	//
	// body_type_msg   ::= media_message SPACE body_fields SPACE envelope
	//                     SPACE body SPACE body_fld_lines
	//

	class xbody;
	typedef xbody body;

	DECLARE_COMPONENT(body_type_msg)

		body_type_msg()
			: m_media_message(NULL), m_body_fields(NULL),
			  m_envelope(NULL), m_body(NULL), m_body_fld_lines(NULL)
		{
		}

		~body_type_msg()
		{
			delete (m_media_message);
			delete (m_body_fields);
			delete (m_envelope);
			delete (m_body);
			delete (m_body_fld_lines);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::media_message, m_media_message);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fields, m_body_fields);
			VIMAP_PARSER_CHECK(SPACE);

			// BUGFIX: made SPACE optional. This is not standard, but some servers
			// seem to return responses like that...
			VIMAP_PARSER_GET(IMAPParser::envelope, m_envelope);
			VIMAP_PARSER_TRY_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::xbody, m_body);
			VIMAP_PARSER_TRY_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_lines, m_body_fld_lines);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::media_message* m_media_message;
		IMAPParser::body_fields* m_body_fields;
		IMAPParser::envelope* m_envelope;
		IMAPParser::xbody* m_body;
		IMAPParser::body_fld_lines* m_body_fld_lines;

	public:

		const IMAPParser::media_message* media_message() const { return (m_media_message); }
		const IMAPParser::body_fields* body_fields() const { return (m_body_fields); }
		const IMAPParser::envelope* envelope() const { return (m_envelope); }
		const IMAPParser::xbody* body() const { return (m_body); }
		const IMAPParser::body_fld_lines* body_fld_lines() const { return (m_body_fld_lines); }
	};


	//
	// body_type_text  ::= media_text SPACE body_fields SPACE body_fld_lines
	//

	DECLARE_COMPONENT(body_type_text)

		body_type_text()
			: m_media_text(NULL),
			  m_body_fields(NULL), m_body_fld_lines(NULL)
		{
		}

		~body_type_text()
		{
			delete (m_media_text);
			delete (m_body_fields);
			delete (m_body_fld_lines);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::media_text, m_media_text);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fields, m_body_fields);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_lines, m_body_fld_lines);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::media_text* m_media_text;
		IMAPParser::body_fields* m_body_fields;
		IMAPParser::body_fld_lines* m_body_fld_lines;

	public:

		const IMAPParser::media_text* media_text() const { return (m_media_text); }
		const IMAPParser::body_fields* body_fields() const { return (m_body_fields); }
		const IMAPParser::body_fld_lines* body_fld_lines() const { return (m_body_fld_lines); }
	};


	//
	// body_type_1part ::= (body_type_basic / body_type_msg / body_type_text)
	//                     [SPACE body_ext_1part]
	//

	DECLARE_COMPONENT(body_type_1part)

		body_type_1part()
			: m_body_type_basic(NULL), m_body_type_msg(NULL),
			  m_body_type_text(NULL), m_body_ext_1part(NULL)
		{
		}

		~body_type_1part()
		{
			delete (m_body_type_basic);
			delete (m_body_type_msg);
			delete (m_body_type_text);

			delete (m_body_ext_1part);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_type_text, m_body_type_text))
				if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_type_msg, m_body_type_msg))
					VIMAP_PARSER_GET(IMAPParser::body_type_basic, m_body_type_basic);

			if (VIMAP_PARSER_TRY_CHECK(SPACE))
			{
				if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_ext_1part, m_body_ext_1part))
					--pos;
			}

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::body_type_basic* m_body_type_basic;
		IMAPParser::body_type_msg* m_body_type_msg;
		IMAPParser::body_type_text* m_body_type_text;

		IMAPParser::body_ext_1part* m_body_ext_1part;

	public:

		const IMAPParser::body_type_basic* body_type_basic() const { return (m_body_type_basic); }
		const IMAPParser::body_type_msg* body_type_msg() const { return (m_body_type_msg); }
		const IMAPParser::body_type_text* body_type_text() const { return (m_body_type_text); }

		const IMAPParser::body_ext_1part* body_ext_1part() const { return (m_body_ext_1part); }
	};


	//
	// body_type_mpart ::= 1*body SPACE media_subtype
	//                     [SPACE body_ext_mpart]
	//

	DECLARE_COMPONENT(body_type_mpart)

		body_type_mpart()
			: m_media_subtype(NULL), m_body_ext_mpart(NULL)
		{
		}

		~body_type_mpart()
		{
			delete (m_media_subtype);
			delete (m_body_ext_mpart);

			for (std::vector <xbody*>::iterator it = m_list.begin() ;
			     it != m_list.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET_PUSHBACK(xbody, m_list);

			while (true)
			{
				std::auto_ptr <xbody> b;
				VIMAP_PARSER_TRY_GET_PTR(xbody, b);

				if (!b.get())
					break;

				m_list.push_back(b.release());
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::media_subtype, m_media_subtype);

			if (VIMAP_PARSER_TRY_CHECK(SPACE))
				VIMAP_PARSER_GET(IMAPParser::body_ext_mpart, m_body_ext_mpart);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::media_subtype* m_media_subtype;
		IMAPParser::body_ext_mpart* m_body_ext_mpart;

		std::vector <xbody*> m_list;

	public:

		const std::vector <IMAPParser::xbody*>& list() const { return (m_list); }

		const IMAPParser::media_subtype* media_subtype() const { return (m_media_subtype); }
		const IMAPParser::body_ext_mpart* body_ext_mpart() const { return (m_body_ext_mpart); }
	};


	//
	// xbody ::= "(" body_type_1part / body_type_mpart ")"
	//

	DECLARE_COMPONENT(xbody)

		xbody()
			: m_body_type_1part(NULL), m_body_type_mpart(NULL)
		{
		}

		~xbody()
		{
			delete (m_body_type_1part);
			delete (m_body_type_mpart);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_type_mpart, m_body_type_mpart))
				VIMAP_PARSER_GET(IMAPParser::body_type_1part, m_body_type_1part);

			VIMAP_PARSER_CHECK(one_char <')'> );

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::body_type_1part* m_body_type_1part;
		IMAPParser::body_type_mpart* m_body_type_mpart;

	public:

		const IMAPParser::body_type_1part* body_type_1part() const { return (m_body_type_1part); }
		const IMAPParser::body_type_mpart* body_type_mpart() const { return (m_body_type_mpart); }
	};


	// msg_att_item ::= "ENVELOPE" SPACE envelope /
	//                  "FLAGS" SPACE "(" #(flag / "\Recent") ")" /
	//                  "INTERNALDATE" SPACE date_time /
	//                  "RFC822" [".HEADER" / ".TEXT"] SPACE nstring /
	//                  "RFC822.SIZE" SPACE number /
	//                  "BODY" ["STRUCTURE"] SPACE body /
	//                  "BODY" section ["<" number ">"] SPACE nstring /
	//                  "UID" SPACE uniqueid
	//
	// IMAP Extension for Conditional STORE (RFC-4551):
	//
	//   msg_att_item      /= "MODSEQ" SP "(" mod_sequence_value ")"

	DECLARE_COMPONENT(msg_att_item)

		msg_att_item()
			: m_date_time(NULL), m_number(NULL), m_envelope(NULL),
			  m_uniqueid(NULL), m_nstring(NULL), m_body(NULL), m_flag_list(NULL),
			  m_section(NULL), m_mod_sequence_value(NULL)

		{
		}

		~msg_att_item()
		{
			delete (m_date_time);
			delete (m_number);
			delete (m_envelope);
			delete (m_uniqueid);
			delete (m_nstring);
			delete (m_body);
			delete (m_flag_list);
 			delete (m_section);
 			delete m_mod_sequence_value;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			// "ENVELOPE" SPACE envelope
			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "envelope"))
			{
				m_type = ENVELOPE;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::envelope, m_envelope);
			}
			// "FLAGS" SPACE "(" #(flag / "\Recent") ")"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "flags"))
			{
				m_type = FLAGS;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::flag_list, m_flag_list);
			}
			// "INTERNALDATE" SPACE date_time
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "internaldate"))
			{
				m_type = INTERNALDATE;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::date_time, m_date_time);
			}
			// "RFC822" ".HEADER" SPACE nstring
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822.header"))
			{
				m_type = RFC822_HEADER;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::nstring, m_nstring);
			}
			// "RFC822" ".TEXT" SPACE nstring
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822.text"))
			{
				m_type = RFC822_TEXT;

				VIMAP_PARSER_CHECK(SPACE);

				m_nstring = parser.getWithArgs <IMAPParser::nstring>
					(line, &pos, this, RFC822_TEXT);

				VIMAP_PARSER_FAIL_UNLESS(m_nstring);
			}
			// "RFC822.SIZE" SPACE number
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822.size"))
			{
				m_type = RFC822_SIZE;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::number, m_number);
			}
			// "RFC822" SPACE nstring
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822"))
			{
				m_type = RFC822;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::nstring, m_nstring);
			}
			// "BODY" "STRUCTURE" SPACE body
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "bodystructure"))
			{
				m_type = BODY_STRUCTURE;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::body, m_body);
			}
			// "BODY" section ["<" number ">"] SPACE nstring
			// "BODY" SPACE body
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "body"))
			{
				VIMAP_PARSER_TRY_GET(IMAPParser::section, m_section);

				// "BODY" section ["<" number ">"] SPACE nstring
				if (m_section != NULL)
				{
					m_type = BODY_SECTION;

					if (VIMAP_PARSER_TRY_CHECK(one_char <'<'> ))
					{
						VIMAP_PARSER_GET(IMAPParser::number, m_number);
						VIMAP_PARSER_CHECK(one_char <'>'> );
					}

					VIMAP_PARSER_CHECK(SPACE);

					m_nstring = parser.getWithArgs <IMAPParser::nstring>
						(line, &pos, this, BODY_SECTION);

					VIMAP_PARSER_FAIL_UNLESS(m_nstring);
				}
				// "BODY" SPACE body
				else
				{
					m_type = BODY;

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::body, m_body);
				}
			}
			// "MODSEQ" SP "(" mod_sequence_value ")"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "modseq"))
			{
				m_type = MODSEQ;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_CHECK(one_char <'('> );

				VIMAP_PARSER_GET(IMAPParser::mod_sequence_value, m_mod_sequence_value);

				VIMAP_PARSER_CHECK(one_char <')'> );
			}
			// "UID" SPACE uniqueid
			else
			{
				m_type = UID;

				VIMAP_PARSER_CHECK_WITHARG(special_atom, "uid");
				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(uniqueid, m_uniqueid);
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			ENVELOPE,
			FLAGS,
			INTERNALDATE,
			RFC822,
			RFC822_SIZE,
			RFC822_HEADER,
			RFC822_TEXT,
			BODY,
			BODY_SECTION,
			BODY_STRUCTURE,
			UID,
			MODSEQ
		};

	private:

		Type m_type;

		IMAPParser::date_time* m_date_time;
		IMAPParser::number* m_number;
		IMAPParser::envelope* m_envelope;
		IMAPParser::uniqueid* m_uniqueid;
		IMAPParser::nstring* m_nstring;
		IMAPParser::xbody* m_body;
		IMAPParser::flag_list* m_flag_list;
		IMAPParser::section* m_section;
		IMAPParser::mod_sequence_value* m_mod_sequence_value;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::date_time* date_time() const { return (m_date_time); }
		const IMAPParser::number* number() const { return (m_number); }
		const IMAPParser::envelope* envelope() const { return (m_envelope); }
		const IMAPParser::uniqueid* unique_id() const { return (m_uniqueid); }
		const IMAPParser::nstring* nstring() const { return (m_nstring); }
		const IMAPParser::xbody* body() const { return (m_body); }
		const IMAPParser::flag_list* flag_list() const { return (m_flag_list); }
		const IMAPParser::section* section() const { return (m_section); }
		const IMAPParser::mod_sequence_value* mod_sequence_value() { return m_mod_sequence_value; }
	};


	//
	// msg_att ::= "(" 1#(msg_att_item) ")"
	//

	DECLARE_COMPONENT(msg_att)

		~msg_att()
		{
			for (std::vector <msg_att_item*>::iterator it = m_items.begin() ;
			     it != m_items.end() ; ++it)
			{
				delete (*it);
			}
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			m_items.push_back(parser.get <msg_att_item>(line, &pos));

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> ))
			{
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET_PUSHBACK(msg_att_item, m_items);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::vector <msg_att_item*> m_items;

	public:

		const std::vector <msg_att_item*>& items() const { return (m_items); }
	};


	//
	// message_data ::= nz_number SPACE ("EXPUNGE" /
	//                  ("FETCH" SPACE msg_att))
	//

	DECLARE_COMPONENT(message_data)

		message_data()
			: m_number(0), m_msg_att(NULL)
		{
		}

		~message_data()
		{
			delete (m_msg_att);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			std::auto_ptr <nz_number> num;
			VIMAP_PARSER_GET_PTR(nz_number, num);
			m_number = static_cast <unsigned int>(num->value());

			VIMAP_PARSER_CHECK(SPACE);

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "expunge"))
			{
				m_type = EXPUNGE;
			}
			else
			{
				m_type = FETCH;

				VIMAP_PARSER_CHECK_WITHARG(special_atom, "fetch");
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::msg_att, m_msg_att);
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			EXPUNGE,
			FETCH
		};

	private:

		Type m_type;
		unsigned int m_number;
		IMAPParser::msg_att* m_msg_att;

	public:

		Type type() const { return (m_type); }
		unsigned int number() const { return (m_number); }
		const IMAPParser::msg_att* msg_att() const { return (m_msg_att); }
	};


	//
	// resp_text_code ::= "ALERT" / "PARSE" /
	//                    capability-data /
	//                    "PERMANENTFLAGS" SPACE "(" #(flag / "\*") ")" /
	//                    "READ-ONLY" / "READ-WRITE" / "TRYCREATE" /
	//                    "UIDVALIDITY" SPACE nz_number /
	//                    "UNSEEN" SPACE nz_number /
	//                    "UIDNEXT" SPACE nz-number /
	//                    atom [SPACE 1*<any TEXT_CHAR except "]">]
	//
	// IMAP Extension for Conditional STORE (RFC-4551):
	//
	//   resp-text-code      =/ "HIGHESTMODSEQ" SP mod-sequence-value /
	//                          "NOMODSEQ" /
	//                          "MODIFIED" SP set
	//
	// IMAP UIDPLUS Extension (RFC-4315):
	//
	//   resp-text-code      =/ "APPENDUID" SP nz-number SP append-uid /
	//                          "COPYUID" SP nz-number SP uid-set SP uid-set /
	//                          "UIDNOTSTICKY"

	DECLARE_COMPONENT(resp_text_code)

		resp_text_code()
			: m_nz_number(NULL), m_atom(NULL), m_flag_list(NULL),
			  m_text(NULL), m_capability_data(NULL)
		{
		}

		~resp_text_code()
		{
			delete (m_nz_number);
			delete (m_atom);
			delete (m_flag_list);
			delete (m_text);
			delete m_capability_data;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			// "ALERT"
			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "alert"))
			{
				m_type = ALERT;
			}
			// "PARSE"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "parse"))
			{
				m_type = PARSE;
			}
			// capability_data
			else if (VIMAP_PARSER_TRY_GET(IMAPParser::capability_data, m_capability_data))
			{
				m_type = CAPABILITY;
			}
			// "PERMANENTFLAGS" SPACE flag_list
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "permanentflags"))
			{
				m_type = PERMANENTFLAGS;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::flag_list, m_flag_list);
			}
			// "READ-ONLY"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "read-only"))
			{
				m_type = READ_ONLY;
			}
			// "READ-WRITE"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "read-write"))
			{
				m_type = READ_WRITE;
			}
			// "TRYCREATE"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "trycreate"))
			{
				m_type = TRYCREATE;
			}
			// "UIDVALIDITY" SPACE nz_number
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidvalidity"))
			{
				m_type = UIDVALIDITY;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, m_nz_number);
			}
			// "UIDNEXT" SPACE nz_number
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidnext"))
			{
				m_type = UIDNEXT;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, m_nz_number);
			}
			// "UNSEEN" SPACE nz_number
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "unseen"))
			{
				m_type = UNSEEN;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, m_nz_number);
			}
			// "HIGHESTMODSEQ" SP mod-sequence-value
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "highestmodseq"))
			{
				m_type = HIGHESTMODSEQ;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::mod_sequence_value, m_mod_sequence_value);
			}
			// "NOMODSEQ"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "nomodseq"))
			{
				m_type = NOMODSEQ;
			}
			// "MODIFIED" SP sequence-set
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "modified"))
			{
				m_type = MODIFIED;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::sequence_set, m_sequence_set);
			}
			// "APPENDUID" SP nz-number SP append-uid
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "appenduid"))
			{
				m_type = APPENDUID;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, m_nz_number);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::uid_set, m_uid_set);
			}
			// "COPYUID" SP nz-number SP uid-set SP uid-set
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "copyuid"))
			{
				m_type = COPYUID;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, m_nz_number);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::uid_set, m_uid_set);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::uid_set, m_uid_set2);
			}
			// "UIDNOTSTICKY"
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidnotsticky"))
			{
				m_type = UIDNOTSTICKY;
			}
			// atom [SPACE 1*<any TEXT_CHAR except "]">]
			else
			{
				m_type = OTHER;

				VIMAP_PARSER_GET(IMAPParser::atom, m_atom);

				if (VIMAP_PARSER_TRY_CHECK(SPACE))
					VIMAP_PARSER_GET(text_except <']'> , m_text);
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			// Extensions
			HIGHESTMODSEQ,
			NOMODSEQ,
			MODIFIED,
			APPENDUID,
			COPYUID,
			UIDNOTSTICKY,

			// Standard IMAP
			ALERT,
			PARSE,
			CAPABILITY,
			PERMANENTFLAGS,
			READ_ONLY,
			READ_WRITE,
			TRYCREATE,
			UIDVALIDITY,
			UIDNEXT,
			UNSEEN,
			OTHER
		};

	private:

		Type m_type;

		IMAPParser::nz_number* m_nz_number;
		IMAPParser::atom* m_atom;
		IMAPParser::flag_list* m_flag_list;
		IMAPParser::text* m_text;
		IMAPParser::mod_sequence_value* m_mod_sequence_value;
		IMAPParser::sequence_set* m_sequence_set;
		IMAPParser::capability_data* m_capability_data;
		IMAPParser::uid_set* m_uid_set;
		IMAPParser::uid_set* m_uid_set2;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::nz_number* nz_number() const { return (m_nz_number); }
		const IMAPParser::atom* atom() const { return (m_atom); }
		const IMAPParser::flag_list* flag_list() const { return (m_flag_list); }
		const IMAPParser::text* text() const { return (m_text); }
		const IMAPParser::mod_sequence_value* mod_sequence_value() const { return m_mod_sequence_value; }
		const IMAPParser::sequence_set* sequence_set() const { return m_sequence_set; }
		const IMAPParser::capability_data* capability_data() const { return m_capability_data; }
		const IMAPParser::uid_set* uid_set() const { return (m_uid_set); }
		const IMAPParser::uid_set* uid_set2() const { return (m_uid_set2); }
	};


	//
	// resp_text ::= ["[" resp_text_code "]" SPACE] (text_mime2 / text)
	//               ;; text SHOULD NOT begin with "[" or "="

	DECLARE_COMPONENT(resp_text)

		resp_text()
			: m_resp_text_code(NULL)
		{
		}

		~resp_text()
		{
			delete (m_resp_text_code);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'['> ))
			{
				VIMAP_PARSER_GET(IMAPParser::resp_text_code, m_resp_text_code);

				VIMAP_PARSER_CHECK(one_char <']'> );
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			std::auto_ptr <text_mime2> text1;
			VIMAP_PARSER_TRY_GET_PTR(text_mime2, text1);

			if (text1.get())
			{
				m_text = text1->value();
			}
			else
			{
				std::auto_ptr <IMAPParser::text> text2;
				VIMAP_PARSER_TRY_GET_PTR(IMAPParser::text, text2);

				if (text2.get())
				{
					m_text = text2->value();
				}
				else
				{
					// Empty response text
				}
			}

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::resp_text_code* m_resp_text_code;
		string m_text;

	public:

		const IMAPParser::resp_text_code* resp_text_code() const { return (m_resp_text_code); }
		const string& text() const { return (m_text); }
	};


	//
	// continue_req   ::= "+" SPACE (resp_text / base64)
	//

	DECLARE_COMPONENT(continue_req)

		continue_req()
			: m_resp_text(NULL)
		{
		}

		~continue_req()
		{
			delete (m_resp_text);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'+'> );

			if (!parser.isStrict())
			{
				// Some servers do not send SPACE when response text is empty
				if (VIMAP_PARSER_TRY_CHECK(SPACE))
				{
					VIMAP_PARSER_GET(IMAPParser::resp_text, m_resp_text);
				}
				else
				{
					m_resp_text = new IMAPParser::resp_text();  // empty
				}
			}
			else
			{
				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::resp_text, m_resp_text);
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::resp_text* m_resp_text;

	public:

		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
	};


	//
	// resp_cond_state ::= ("OK" / "NO" / "BAD") SPACE resp_text
	//                     ;; Status condition
	//

	DECLARE_COMPONENT(resp_cond_state)

		resp_cond_state()
			: m_resp_text(NULL), m_status(BAD)
		{
		}

		~resp_cond_state()
		{
			delete (m_resp_text);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "ok"))
			{
				m_status = OK;
			}
			else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "no"))
			{
				m_status = NO;
			}
			else
			{
				VIMAP_PARSER_CHECK_WITHARG(special_atom, "bad");
				m_status = BAD;
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_text, m_resp_text);

			*currentPos = pos;

			return true;
		}


		enum Status
		{
			OK,
			NO,
			BAD
		};

	private:

		IMAPParser::resp_text* m_resp_text;
		Status m_status;

	public:

		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
		Status status() const { return (m_status); }
	};


	//
	// resp_cond_bye ::= "BYE" SPACE resp_text
	//

	DECLARE_COMPONENT(resp_cond_bye)

		resp_cond_bye()
			: m_resp_text(NULL)
		{
		}

		~resp_cond_bye()
		{
			delete (m_resp_text);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK_WITHARG(special_atom, "bye");

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_text, m_resp_text);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::resp_text* m_resp_text;

	public:

		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
	};


	//
	// resp_cond_auth  ::= ("OK" / "PREAUTH") SPACE resp_text
	//                     ;; Authentication condition
	//

	DECLARE_COMPONENT(resp_cond_auth)

		resp_cond_auth()
			: m_resp_text(NULL)
		{
		}

		~resp_cond_auth()
		{
			delete (m_resp_text);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "ok"))
			{
				m_cond = OK;
			}
			else
			{
				VIMAP_PARSER_CHECK_WITHARG(special_atom, "preauth");

				m_cond = PREAUTH;
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_text, m_resp_text);

			*currentPos = pos;

			return true;
		}


		enum Condition
		{
			OK,
			PREAUTH
		};

	private:

		Condition m_cond;
		IMAPParser::resp_text* m_resp_text;

	public:

		Condition condition() const { return (m_cond); }
		const IMAPParser::resp_text* resp_text() const { return (m_resp_text); }
	};


	//
	// mailbox_data ::= "FLAGS" SPACE mailbox_flag_list /
	//                  "LIST" SPACE mailbox_list /
	//                  "LSUB" SPACE mailbox_list /
	//                  "MAILBOX" SPACE text /
	//                  "SEARCH" [SPACE 1#nz_number] /
	//                  "STATUS" SPACE mailbox SPACE
	//                    "(" [status-att-list] ")" /
	//                  number SPACE "EXISTS" /
	//                  number SPACE "RECENT"
	//

	DECLARE_COMPONENT(mailbox_data)

		mailbox_data()
			: m_number(NULL), m_mailbox_flag_list(NULL), m_mailbox_list(NULL),
			  m_mailbox(NULL), m_text(NULL), m_status_att_list(NULL)
		{
		}

		~mailbox_data()
		{
			delete (m_number);
			delete (m_mailbox_flag_list);
			delete (m_mailbox_list);
			delete (m_mailbox);
			delete (m_text);

			for (std::vector <nz_number*>::iterator it = m_search_nz_number_list.begin() ;
			     it != m_search_nz_number_list.end() ; ++it)
			{
				delete (*it);
			}

			delete m_status_att_list;
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_GET(IMAPParser::number, m_number))
			{
				VIMAP_PARSER_CHECK(SPACE);

				if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "exists"))
				{
					m_type = EXISTS;
				}
				else
				{
					VIMAP_PARSER_CHECK_WITHARG(special_atom, "recent");

					m_type = RECENT;
				}
			}
			else
			{
				// "FLAGS" SPACE mailbox_flag_list
				if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "flags"))
				{
					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox_flag_list, m_mailbox_flag_list);

					m_type = FLAGS;
				}
				// "LIST" SPACE mailbox_list
				else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "list"))
				{
					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox_list, m_mailbox_list);

					m_type = LIST;
				}
				// "LSUB" SPACE mailbox_list
				else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "lsub"))
				{
					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox_list, m_mailbox_list);

					m_type = LSUB;
				}
				// "MAILBOX" SPACE text
				else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "mailbox"))
				{
					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::text, m_text);

					m_type = MAILBOX;
				}
				// "SEARCH" [SPACE 1#nz_number]
				else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "search"))
				{
					if (VIMAP_PARSER_TRY_CHECK(SPACE))
					{
						VIMAP_PARSER_GET_PUSHBACK(nz_number, m_search_nz_number_list);

						while (VIMAP_PARSER_TRY_CHECK(SPACE))
							VIMAP_PARSER_GET_PUSHBACK(nz_number, m_search_nz_number_list);
					}

					m_type = SEARCH;
				}
				// "STATUS" SPACE mailbox SPACE
				// "(" [status_att_list] ")"
				else
				{
					VIMAP_PARSER_CHECK_WITHARG(special_atom, "status");
					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox, m_mailbox);

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_CHECK(one_char <'('> );
					VIMAP_PARSER_TRY_GET(IMAPParser::status_att_list, m_status_att_list);
					VIMAP_PARSER_CHECK(one_char <')'> );

					m_type = STATUS;
				}
			}

			*currentPos = pos;

			return true;
		}


		enum Type
		{
			FLAGS,
			LIST,
			LSUB,
			MAILBOX,
			SEARCH,
			STATUS,
			EXISTS,
			RECENT
		};

	private:

		Type m_type;

		IMAPParser::number* m_number;
		IMAPParser::mailbox_flag_list* m_mailbox_flag_list;
		IMAPParser::mailbox_list* m_mailbox_list;
		IMAPParser::mailbox* m_mailbox;
		IMAPParser::text* m_text;
		std::vector <nz_number*> m_search_nz_number_list;
		IMAPParser::status_att_list* m_status_att_list;

	public:

		Type type() const { return (m_type); }

		const IMAPParser::number* number() const { return (m_number); }
		const IMAPParser::mailbox_flag_list* mailbox_flag_list() const { return (m_mailbox_flag_list); }
		const IMAPParser::mailbox_list* mailbox_list() const { return (m_mailbox_list); }
		const IMAPParser::mailbox* mailbox() const { return (m_mailbox); }
		const IMAPParser::text* text() const { return (m_text); }
		const std::vector <nz_number*>& search_nz_number_list() const { return (m_search_nz_number_list); }
		const IMAPParser::status_att_list* status_att_list() const { return m_status_att_list; }
	};


	//
	// response_data  ::= "*" SPACE (resp_cond_state / resp_cond_bye /
	//                    mailbox_data / message_data / capability_data) CRLF
	//

	DECLARE_COMPONENT(response_data)

		response_data()
			: m_resp_cond_state(NULL), m_resp_cond_bye(NULL),
			  m_mailbox_data(NULL), m_message_data(NULL), m_capability_data(NULL)
		{
		}

		~response_data()
		{
			delete (m_resp_cond_state);
			delete (m_resp_cond_bye);
			delete (m_mailbox_data);
			delete (m_message_data);
			delete (m_capability_data);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'*'> );
			VIMAP_PARSER_CHECK(SPACE);

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::resp_cond_state, m_resp_cond_state))
				if (!VIMAP_PARSER_TRY_GET(IMAPParser::resp_cond_bye, m_resp_cond_bye))
					if (!VIMAP_PARSER_TRY_GET(IMAPParser::mailbox_data, m_mailbox_data))
						if (!VIMAP_PARSER_TRY_GET(IMAPParser::message_data, m_message_data))
							VIMAP_PARSER_GET(IMAPParser::capability_data, m_capability_data);

			if (!parser.isStrict())
			{
				// Allow SPACEs at end of line
				while (VIMAP_PARSER_TRY_CHECK(SPACE))
					;
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::resp_cond_state* m_resp_cond_state;
		IMAPParser::resp_cond_bye* m_resp_cond_bye;
		IMAPParser::mailbox_data* m_mailbox_data;
		IMAPParser::message_data* m_message_data;
		IMAPParser::capability_data* m_capability_data;

	public:

		const IMAPParser::resp_cond_state* resp_cond_state() const { return (m_resp_cond_state); }
		const IMAPParser::resp_cond_bye* resp_cond_bye() const { return (m_resp_cond_bye); }
		const IMAPParser::mailbox_data* mailbox_data() const { return (m_mailbox_data); }
		const IMAPParser::message_data* message_data() const { return (m_message_data); }
		const IMAPParser::capability_data* capability_data() const { return (m_capability_data); }
	};


	DECLARE_COMPONENT(continue_req_or_response_data)

		continue_req_or_response_data()
			: m_continue_req(NULL), m_response_data(NULL)
		{
		}

		~continue_req_or_response_data()
		{
			delete (m_continue_req);
			delete (m_response_data);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::continue_req, m_continue_req))
				VIMAP_PARSER_GET(IMAPParser::response_data, m_response_data);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::continue_req* m_continue_req;
		IMAPParser::response_data* m_response_data;

	public:

		const IMAPParser::continue_req* continue_req() const { return (m_continue_req); }
		const IMAPParser::response_data* response_data() const { return (m_response_data); }
	};


	//
	// response_fatal ::= "*" SPACE resp_cond_bye CRLF
	//                    ;; Server closes connection immediately
	//

	DECLARE_COMPONENT(response_fatal)

		response_fatal()
			: m_resp_cond_bye(NULL)
		{
		}

		~response_fatal()
		{
			delete (m_resp_cond_bye);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'*'> );
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_cond_bye, m_resp_cond_bye);

			if (!parser.isStrict())
			{
				// Allow SPACEs at end of line
				while (VIMAP_PARSER_TRY_CHECK(SPACE))
					;
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::resp_cond_bye* m_resp_cond_bye;

	public:

		const IMAPParser::resp_cond_bye* resp_cond_bye() const { return (m_resp_cond_bye); }
	};


	//
	// response_tagged ::= tag SPACE resp_cond_state CRLF
	//

	DECLARE_COMPONENT(response_tagged)

		response_tagged()
			: m_resp_cond_state(NULL)
		{
		}

		~response_tagged()
		{
			delete (m_resp_cond_state);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(IMAPParser::xtag);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::resp_cond_state, m_resp_cond_state);

			if (!parser.isStrict())
			{
				// Allow SPACEs at end of line
				while (VIMAP_PARSER_TRY_CHECK(SPACE))
					;
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::resp_cond_state* m_resp_cond_state;

	public:

		const IMAPParser::resp_cond_state* resp_cond_state() const { return (m_resp_cond_state); }
	};


	//
	// response_done ::= response_tagged / response_fatal
	//

	DECLARE_COMPONENT(response_done)

		response_done()
			: m_response_tagged(NULL), m_response_fatal(NULL)
		{
		}

		~response_done()
		{
			delete (m_response_tagged);
			delete (m_response_fatal);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::response_tagged, m_response_tagged))
				VIMAP_PARSER_GET(IMAPParser::response_fatal, m_response_fatal);

			*currentPos = pos;

			return true;
		}

	private:

		IMAPParser::response_tagged* m_response_tagged;
		IMAPParser::response_fatal* m_response_fatal;

	public:

		const IMAPParser::response_tagged* response_tagged() const { return (m_response_tagged); }
		const IMAPParser::response_fatal* response_fatal() const { return (m_response_fatal); }
	};


	//
	// response ::= *(continue_req / response_data) response_done
	//

	DECLARE_COMPONENT(response)

		response()
			: m_response_done(NULL)
		{
		}

		~response()
		{
			for (std::vector <IMAPParser::continue_req_or_response_data*>::iterator
			     it = m_continue_req_or_response_data.begin() ;
			     it != m_continue_req_or_response_data.end() ; ++it)
			{
				delete (*it);
			}

			delete (m_response_done);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;
			string curLine = line;
			bool partial = false;  // partial response

			IMAPParser::continue_req_or_response_data* resp = NULL;

			while ((resp = parser.get <IMAPParser::continue_req_or_response_data>(curLine, &pos)))
			{
				m_continue_req_or_response_data.push_back(resp);

				// Partial response (continue_req)
				if (resp->continue_req())
				{
					partial = true;
					break;
				}

				// We have read a CRLF, read another line
				curLine = parser.readLine();
				pos = 0;
			}

			if (!partial)
			{
				m_response_done = parser.get <IMAPParser::response_done>(curLine, &pos);
				VIMAP_PARSER_FAIL_UNLESS(m_response_done);
			}

			*currentPos = pos;

			return true;
		}


		bool isBad() const
		{
			if (!response_done())  // incomplete (partial) response
				return (true);

			if (response_done()->response_fatal())
				return (true);

			if (response_done()->response_tagged()->resp_cond_state()->
				status() == IMAPParser::resp_cond_state::BAD)
			{
				return (true);
			}

			return (false);
		}

		void setErrorLog(const string& errorLog)
		{
			m_errorLog = errorLog;
		}

		const string& getErrorLog() const
		{
			return m_errorLog;
		}

	private:

		std::vector <IMAPParser::continue_req_or_response_data*> m_continue_req_or_response_data;
		IMAPParser::response_done* m_response_done;

		string m_errorLog;

	public:

		const std::vector <IMAPParser::continue_req_or_response_data*>& continue_req_or_response_data() const { return (m_continue_req_or_response_data); }
		const IMAPParser::response_done* response_done() const { return (m_response_done); }
	};


	//
	// greeting ::= "*" SPACE (resp_cond_auth / resp_cond_bye) CRLF
	//

	DECLARE_COMPONENT(greeting)

		greeting()
			: m_resp_cond_auth(NULL), m_resp_cond_bye(NULL)
		{
		}

		~greeting()
		{
			delete (m_resp_cond_auth);
			delete (m_resp_cond_bye);
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'*'> );
			VIMAP_PARSER_CHECK(SPACE);

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::resp_cond_auth, m_resp_cond_auth))
				VIMAP_PARSER_GET(IMAPParser::resp_cond_bye, m_resp_cond_bye);

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}

		void setErrorLog(const string& errorLog)
		{
			m_errorLog = errorLog;
		}

		const string& getErrorLog() const
		{
			return m_errorLog;
		}

	private:

		IMAPParser::resp_cond_auth* m_resp_cond_auth;
		IMAPParser::resp_cond_bye* m_resp_cond_bye;

		string m_errorLog;

	public:

		const IMAPParser::resp_cond_auth* resp_cond_auth() const { return (m_resp_cond_auth); }
		const IMAPParser::resp_cond_bye* resp_cond_bye() const { return (m_resp_cond_bye); }
	};



	//
	// The main functions used to parse a response
	//

	response* readResponse(literalHandler* lh = NULL)
	{
		size_t pos = 0;
		string line = readLine();

		m_literalHandler = lh;
		response* resp = get <response>(line, &pos);
		m_literalHandler = NULL;

		if (!resp)
			throw exceptions::invalid_response("", m_errorResponseLine);

		resp->setErrorLog(lastLine());

		return (resp);
	}


	greeting* readGreeting()
	{
		size_t pos = 0;
		string line = readLine();

		greeting* greet = get <greeting>(line, &pos);

		if (!greet)
			throw exceptions::invalid_response("", m_errorResponseLine);

		greet->setErrorLog(lastLine());

		return greet;
	}


	/** Parse a token and advance.
	  * If the token has been parsed successfully, a raw pointer to it
	  * will be returned. The caller is responsible to free the memory.
	  *
	  * @param TYPE token type (class)
	  * @param line line which is currently being parsed
	  * @param currentPos current position in the line (will be updated
	  * when the function returns)
	  * @return a raw pointer to the parsed token, or NULL otherwise
	  */
	template <class TYPE>
	TYPE* get(string& line, size_t* currentPos)
	{
		component* resp = new TYPE;
		return internalGet <TYPE>(resp, line, currentPos);
	}

	/** Parse a token which takes 2 arguments and advance.
	  * If the token has been parsed successfully, a raw pointer to it
	  * will be returned. The caller is responsible to free the memory.
	  *
	  * @param TYPE token type (class)
	  * @param ARG1_TYPE type of argument #1 (class)
	  * @param ARG2_TYPE type of argument #2 (class)
	  * @param line line which is currently being parsed
	  * @param currentPos current position in the line (will be updated
	  * when the function returns)
	  * @param arg1 value of argument 1 to be passed to the token
	  * @param arg2 value of argument 2 to be passed to the token
	  * @return a raw pointer to the parsed token, or NULL otherwise
	  */
	template <class TYPE, class ARG1_TYPE, class ARG2_TYPE>
	TYPE* getWithArgs(string& line, size_t* currentPos,
	                  ARG1_TYPE arg1, ARG2_TYPE arg2)
	{
		component* resp = new TYPE(arg1, arg2);
		return internalGet <TYPE>(resp, line, currentPos);
	}

private:

	template <class TYPE>
	TYPE* internalGet(component* resp, string& line, size_t* currentPos)
	{
		const size_t oldPos = *currentPos;

		if (!resp->parse(*this, line, currentPos))
		{
			*currentPos = oldPos;

			delete (resp);

			return NULL;
		}

		return static_cast <TYPE*>(resp);
	}

	const string lastLine() const
	{
		// Remove blanks and new lines at the end of the line.
		string line(m_lastLine);

		string::const_iterator it = line.end();
		int count = 0;

		while (it != line.begin())
		{
			const unsigned char c = *(it - 1);

			if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
				break;

			++count;
			--it;
		}

		line.resize(line.length() - count);

		return (line);
	}

public:

	/** Check for a token and advance.
	  *
	  * @param TYPE token type (class)
	  * @param line line which is currently being parsed
	  * @param currentPos current position in the line (will be updated
	  * when the function returns)
	  * @return true if the token has been parsed, or false otherwise
	  */
	template <class TYPE>
	bool check(string& line, size_t* currentPos)
	{
		const size_t oldPos = *currentPos;

		TYPE term;

		if (!term.parse(*this, line, currentPos))
		{
			*currentPos = oldPos;
			return false;
		}
		else
		{
			return true;
		}
	}

	/** Check for a token which takes an argument and advance.
	  *
	  * @param TYPE token type (class)
	  * @param ARG_TYPE argument type (class)
	  * @param line line which is currently being parsed
	  * @param currentPos current position in the line (will be updated
	  * when the function returns)
	  * @param arg argument to be passed to the token
	  * @return true if the token has been parsed, or false otherwise
	  */
	template <class TYPE, class ARG_TYPE>
	bool checkWithArg(string& line, size_t* currentPos, const ARG_TYPE arg)
	{
		const size_t oldPos = *currentPos;

		TYPE term(arg);
		
		if (!term.parse(*this, line, currentPos))
		{
			*currentPos = oldPos;
			return false;
		}
		else
		{
			return true;
		}
	}


private:

	weak_ptr <IMAPTag> m_tag;
	weak_ptr <socket> m_socket;
	shared_ptr <tracer> m_tracer;

	utility::progressListener* m_progress;

	bool m_strict;

	literalHandler* m_literalHandler;

	weak_ptr <timeoutHandler> m_timeoutHandler;


	string m_buffer;

	string m_lastLine;
	string m_errorResponseLine;

public:

	/** Read a line from the input buffer. The function blocks until a
	  * complete line is read from the buffer. Position in input buffer
	  * will be updated.
	  *
	  * @return next line
	  */
	const string readLine()
	{
		size_t pos;

		while ((pos = m_buffer.find('\n')) == string::npos)
		{
			read();
		}

		string line;
		line.resize(pos + 1);
		std::copy(m_buffer.begin(), m_buffer.begin() + pos + 1, line.begin());

		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + pos + 1);

		m_lastLine = line;

#if DEBUG_RESPONSE
		std::cout << std::endl << "Read line:" << std::endl << line << std::endl;
#endif

		if (m_tracer)
		{
			string::size_type len = line.length();
			while (len != 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) --len;
			m_tracer->traceReceive(line.substr(0, len));
		}

		return (line);
	}

	/** Fill in the input buffer with data available from the socket stream.
	  * The function blocks until some data is available.
	  */
	void read()
	{
		string receiveBuffer;

		shared_ptr <timeoutHandler> toh = m_timeoutHandler.lock();
		shared_ptr <socket> sok = m_socket.lock();

		if (toh)
			toh->resetTimeOut();

		while (receiveBuffer.empty())
		{
			// Check whether the time-out delay is elapsed
			if (toh && toh->isTimeOut())
			{
				if (!toh->handleTimeOut())
					throw exceptions::operation_timed_out();
			}

			// We have received data: reset the time-out counter
			sok->receive(receiveBuffer);

			if (receiveBuffer.empty())   // buffer is empty
			{
				if (sok->getStatus() & socket::STATUS_WANT_WRITE)
					sok->waitForWrite();
				else
					sok->waitForRead();

				continue;
			}

			// We have received data ...
			if (toh)
				toh->resetTimeOut();
		}

		m_buffer += receiveBuffer;
	}


	void readLiteral(literalHandler::target& buffer, size_t count)
	{
		size_t len = 0;
		string receiveBuffer;

		shared_ptr <timeoutHandler> toh = m_timeoutHandler.lock();
		shared_ptr <socket> sok = m_socket.lock();

		if (m_progress)
			m_progress->start(count);

		if (toh)
			toh->resetTimeOut();

		if (!m_buffer.empty())
		{
			if (m_buffer.length() > count)
			{
				buffer.putData(string(m_buffer.begin(), m_buffer.begin() + count));
				m_buffer.erase(m_buffer.begin(), m_buffer.begin() + count);
				len = count;
			}
			else
			{
				len += m_buffer.length();
				buffer.putData(m_buffer);
				m_buffer.clear();
			}
		}

		while (len < count)
		{
			// Check whether the time-out delay is elapsed
			if (toh && toh->isTimeOut())
			{
				if (!toh->handleTimeOut())
					throw exceptions::operation_timed_out();

				toh->resetTimeOut();
			}

			// Receive data from the socket
			sok->receive(receiveBuffer);

			if (receiveBuffer.empty())   // buffer is empty
			{
				if (sok->getStatus() & socket::STATUS_WANT_WRITE)
					sok->waitForWrite();
				else
					sok->waitForRead();

				continue;
			}

			// We have received data: reset the time-out counter
			if (toh)
				toh->resetTimeOut();

			if (len + receiveBuffer.length() > count)
			{
				const size_t remaining = count - len;

				// Get the needed amount of data
				buffer.putData(string(receiveBuffer.begin(), receiveBuffer.begin() + remaining));

				// Put the remaining data into the internal response buffer
				receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + remaining);
				m_buffer += receiveBuffer;

				len = count;
			}
			else
			{
				buffer.putData(receiveBuffer);
				len += receiveBuffer.length();
			}

			// Notify progress
			if (m_progress)
				m_progress->progress(len, count);
		}

		if (m_tracer)
			m_tracer->traceReceiveBytes(count);

		if (m_progress)
			m_progress->stop(count);
	}
};


} // imap
} // net
} // vmime


#undef VIMAP_PARSER_FAIL
#undef VIMAP_PARSER_FAIL_UNLESS
#undef VIMAP_PARSER_CHECK
#undef VIMAP_PARSER_TRY_CHECK
#undef VIMAP_PARSER_GET
#undef VIMAP_PARSER_GET_PTR
#undef VIMAP_PARSER_TRY_GET
#undef VIMAP_PARSER_TRY_GET_PTR
#undef VIMAP_PARSER_GET_PUSHBACK
#undef VIMAP_PARSER_CHECK_WITHARG
#undef VIMAP_PARSER_TRY_CHECK_WITHARG


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPPARSER_HPP_INCLUDED
