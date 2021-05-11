//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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
#define VIMAP_PARSER_GET(type, variable) \
	{ \
		variable.reset(parser.get <type>(line, &pos));  \
		VIMAP_PARSER_FAIL_UNLESS(variable.get());  \
	}

/** Get an optional token and advance.
  * If the token is not matched, parsing will continue anyway.
  */
#define VIMAP_PARSER_TRY_GET(type, variable) \
	(variable.reset(parser.get <type>(line, &pos)), variable.get())

/** Get an optional token and advance. If found, token will be pushed back
  * to a vector. If the token is not matched, stopInstr will be executed.
  *
  * @param type token class
  * @param variable variable of type std::vector<> to which the retrieved
  * token will be pushed
  * @param stopInstr instruction to execute if token is not found
  */
#define VIMAP_PARSER_TRY_GET_PUSHBACK_OR_ELSE(type, variable, stopInstr) \
	{  \
		std::unique_ptr <type> v;  \
		try  \
		{  \
			v.reset(parser.get <type>(line, &pos));  \
			if (!v)  \
			{  \
				stopInstr;  \
			}  \
			variable.push_back(std::move(v));  \
		}  \
		catch (...)  \
		{  \
			throw;  \
		}  \
	}

/** Get a token and advance. Token will be pushed back to a vector.
  * If the token is not matched, parsing will fail.
  *
  * @param type token class
  * @param variable variable of type std::vector<> to which the retrieved
  * token will be pushed
  */
#define VIMAP_PARSER_GET_PUSHBACK(type, variable) \
	VIMAP_PARSER_TRY_GET_PUSHBACK_OR_ELSE(type, variable, VIMAP_PARSER_FAIL())


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

	class IMAPParserDebugResponse {

	public:

		IMAPParserDebugResponse(
			const string& name,
			string& line,
			const size_t currentPos,
			const bool &result
		)
			: m_name(name),
			  m_line(line),
			  m_pos(currentPos),
			  m_result(result) {

			++IMAPParserDebugResponse_level;
			IMAPParserDebugResponse_stack.push_back(name);

			for (int i = 0 ; i < IMAPParserDebugResponse_level ; ++i) {
				std::cout << "  ";
			}

			std::cout << "ENTER(" << m_name << "), pos=" << m_pos;
			std::cout << std::endl;

			for (std::vector <string>::iterator it = IMAPParserDebugResponse_stack.begin() ;
			     it != IMAPParserDebugResponse_stack.end() ; ++it) {

				std::cout << "> " << *it << " ";
			}

			std::cout << std::endl;
			std::cout << string(m_line.begin() + (m_pos < 30 ? 0U : m_pos - 30),
				m_line.begin() + std::min(m_line.length(), m_pos + 30)) << std::endl;

			for (size_t i = (m_pos < 30 ? m_pos : (m_pos - (m_pos - 30))) ; i != 0 ; --i) {
				std::cout << " ";
			}

			std::cout << "^" << std::endl;
		}

		~IMAPParserDebugResponse() {

			for (int i = 0 ; i < IMAPParserDebugResponse_level ; ++i) {
				std::cout << "  ";
			}

			std::cout << "LEAVE(" << m_name << "), result=";
			std::cout << (m_result ? "TRUE" : "FALSE") << ", pos=" << m_pos;
			std::cout << std::endl;

			--IMAPParserDebugResponse_level;
			IMAPParserDebugResponse_stack.pop_back();
		}

	private:

		const string m_name;
		string m_line;
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


class VMIME_EXPORT IMAPParser : public object {

public:

	IMAPParser()
		: m_progress(NULL),
		  m_strict(false),
		  m_literalHandler(NULL) {

	}

	~IMAPParser() {

		for (auto it = m_pendingResponses.begin() ; it != m_pendingResponses.end() ; ++it) {
			delete it->second;
		}
	}


	/** Set the socket currently used by this parser to receive data
	  * from server.
	  *
	  * @param sok socket
	  */
	void setSocket(const shared_ptr <socket>& sok) {

		m_socket = sok;
	}

	/** Set the timeout handler currently used by this parser.
	  *
	  * @param toh timeout handler
	  */
	void setTimeoutHandler(const shared_ptr <timeoutHandler>& toh) {

		m_timeoutHandler = toh;
	}

	/** Set the tracer currently used by this parser.
	  *
	  * @param tr tracer
	  */
	void setTracer(const shared_ptr <tracer>& tr) {

		m_tracer = tr;
	}

	/** Set whether we operate in strict mode (this may not work
	  * with some servers which are not fully standard-compliant).
	  *
	  * @param strict true to operate in strict mode, or false
	  * to operate in default, relaxed mode
	  */
	void setStrict(const bool strict) {

		m_strict = strict;
	}

	/** Return true if the parser operates in strict mode, or
	  * false otherwise.
	  *
	  * @return true if we are in strict mode, false otherwise
	  */
	bool isStrict() const {

		return m_strict;
	}



	//
	// literalHandler : literal content handler
	//

	class component;

	class literalHandler {

	public:

		virtual ~literalHandler() { }


		// Abstract target class
		class target {

		protected:

			target(utility::progressListener* progress) : m_progress(progress) {}
			target(const target&) {}

		public:

			virtual ~target() { }


			utility::progressListener* progressListener() { return (m_progress); }

			virtual void putData(const string& chunk) = 0;

			virtual size_t getBytesWritten() const = 0;

		private:

			utility::progressListener* m_progress;
		};


		// Target: put in a string
		class targetString : public target {

		public:

			targetString(utility::progressListener* progress, vmime::string& str)
				: target(progress), m_string(str), m_bytesWritten(0) { }

			const vmime::string& string() const { return (m_string); }
			vmime::string& string() { return (m_string); }


			void putData(const vmime::string& chunk) {

				m_string += chunk;
				m_bytesWritten += chunk.length();
			}

			size_t getBytesWritten() const {

				return m_bytesWritten;
			}

		private:

			vmime::string& m_string;
			size_t m_bytesWritten;
		};


		// Target: redirect to an output stream
		class targetStream : public target {

		public:

			targetStream(
				utility::progressListener* progress,
				utility::outputStream& stream
			)
				: target(progress),
				  m_stream(stream),
				  m_bytesWritten(0) {

			}

			const utility::outputStream& stream() const { return (m_stream); }
			utility::outputStream& stream() { return (m_stream); }


			void putData(const string& chunk) {

				m_stream.write(chunk.data(), chunk.length());
				m_bytesWritten += chunk.length();
			}

			size_t getBytesWritten() const {

				return m_bytesWritten;
			}

		private:

			utility::outputStream& m_stream;
			size_t m_bytesWritten;
		};


		// Called when the parser needs to know what to do with a literal
		//    . comp: the component in which we are at this moment
		//    . data: data specific to the component (may not be used)
		//
		// Returns :
		//    . == NULL to put the literal into the response
		//    . != NULL to redirect the literal to the specified target

		virtual shared_ptr <target> targetFor(const component& comp, const int data) = 0;
	};


	//
	// Base class for a terminal or a non-terminal
	//

	class component {

	public:

		component() { }
		virtual ~component() { }

		virtual const string getComponentName() const = 0;

		bool parse(IMAPParser& parser, string& line, size_t* currentPos) {

			bool res = false;
			DEBUG_ENTER_COMPONENT(getComponentName(), res);
			res = parseImpl(parser, line, currentPos);
			return res;
		}

		virtual bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) = 0;


		const string makeResponseLine(
			const string& comp,
			const string& line,
			const size_t pos
		) {

#if DEBUG_RESPONSE
			if (pos > line.length()) {
				std::cout << "WARNING: component::makeResponseLine(): pos > line.length()" << std::endl;
			}
#endif

			string result(line.substr(0, pos));
			result += "[^]";   // indicates current parser position
			result += line.substr(pos, line.length());
			if (!comp.empty()) result += " [" + comp + "]";

			return (result);
		}
	};


#define COMPONENT_ALIAS(parent, name) \
	class name : public parent { \
		virtual const string getComponentName() const { return #name; }  \
	public: \
		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) { \
			return parent::parseImpl(parser, line, currentPos); \
		} \
	}

#define DECLARE_COMPONENT(name) \
	class name : public component {  \
		virtual const string getComponentName() const { return #name; }  \
	public:


	//
	// Parse one character
	//

	template <char C>
	class one_char : public component {

	public:

		const string getComponentName() const {

			return string("one_char <") + C + ">";
		}

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos) {

			const size_t pos = *currentPos;

			if (pos < line.length() && line[pos] == C) {
				*currentPos = pos + 1;
				return true;
			} else {
				return false;
			}
		}
	};


	//
	// SPACE  ::= <ASCII SP, space, 0x20>
	//

	DECLARE_COMPONENT(SPACE)

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			while (pos < line.length() && (line[pos] == ' ' || line[pos] == '\t')) {
				++pos;
			}

			if (pos > *currentPos) {
				*currentPos = pos;
				return true;
			} else {
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_TRY_CHECK(SPACE);

			if (pos + 1 < line.length() &&
			    line[pos] == 0x0d && line[pos + 1] == 0x0a) {

				*currentPos = pos + 2;
				return true;

			} else {

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

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			bool end = false;

			tagString.reserve(10);

			while (!end && pos < line.length()) {

				const unsigned char c = line[pos];

				switch (c) {

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

						if (c <= 0x1f || c >= 0x7f) {
							end = true;
						} else {
							tagString += c;
							++pos;
						}

						break;
				}
			}

			*currentPos = pos;
			return true;
		}

		string tagString;
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
			: value(0),
			  m_nonZero(nonZero) {

		}

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			bool valid = true;
			unsigned int val = 0;

			while (valid && pos < line.length()) {

				const char c = line[pos];

				if (c >= '0' && c <= '9') {
					val = (val * 10) + (c - '0');
					++pos;
				} else {
					valid = false;
				}
			}

			// Check for non-null length (and for non-zero number)
			if (!(m_nonZero && val == 0) && pos != *currentPos) {
				value = val;
				*currentPos = pos;
				return true;
			} else {
				return false;
			}
		}


		unsigned long value;

	private:

		const bool m_nonZero;
	};


	// nz_number  ::= digit_nz *digit
	//                ;; Non-zero unsigned 32-bit integer
	//                ;; (0 < n < 4,294,967,296)
	//

	class nz_number : public number {

	public:

		nz_number()
			: number(true) {

		}
	};


	//
	// uniqueid    ::= nz_number
	//                 ;; Strictly ascending
	//

	class uniqueid : public nz_number {

	public:

		uniqueid()
			: nz_number() {

		}
	};


	// uid-range       = (uniqueid ":" uniqueid)
	//                   ; two uniqueid values and all values
	//                   ; between these two regards of order.
	//                   ; Example: 2:4 and 4:2 are equivalent.

	DECLARE_COMPONENT(uid_range)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(uniqueid, uniqueid1);
			VIMAP_PARSER_CHECK(one_char <':'>);
			VIMAP_PARSER_GET(uniqueid, uniqueid2);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <uniqueid> uniqueid1;
		std::unique_ptr <uniqueid> uniqueid2;
	};


	//
	// uid-set         = (uniqueid / uid-range) *("," uid-set)
	//

	DECLARE_COMPONENT(uid_set)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			// We have either a 'uid_range' or a 'uniqueid'
			if (!VIMAP_PARSER_TRY_GET(IMAPParser::uid_range, uid_range)) {
				VIMAP_PARSER_GET(IMAPParser::uniqueid, uniqueid);
			}

			// And maybe another 'uid-set' following
			if (VIMAP_PARSER_TRY_CHECK(one_char <','>)) {
				VIMAP_PARSER_GET(IMAPParser::uid_set, next_uid_set);
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::uniqueid> uniqueid;
		std::unique_ptr <IMAPParser::uid_range> uid_range;

		std::unique_ptr <IMAPParser::uid_set> next_uid_set;
	};


	//
	// text       ::= 1*TEXT_CHAR
	//
	// CHAR       ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	// TEXT_CHAR  ::= <any CHAR except CR and LF>
	//

	DECLARE_COMPONENT(text)

		text(bool allow8bits = false, const char except = 0)
			: m_allow8bits(allow8bits),
			  m_except(except) {
		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;
			size_t len = 0;

			if (m_allow8bits || !parser.isStrict()) {

				const unsigned char except = m_except;

				for (bool end = false ; !end && pos < line.length() ; ) {

					const unsigned char c = line[pos];

					if (c == 0x00 || c == 0x0d || c == 0x0a || c == except) {
						end = true;
					} else {
						++pos;
						++len;
					}
				}

			} else {

				const unsigned char except = m_except;

				for (bool end = false ; !end && pos < line.length() ; ) {

					const unsigned char c = line[pos];

					if (c < 0x01 || c > 0x7f || c == 0x0d || c == 0x0a || c == except) {
						end = true;
					} else {
						++pos;
						++len;
					}
				}
			}

			if (len == 0) {
				VIMAP_PARSER_FAIL();
			}

			value.resize(len);
			std::copy(line.begin() + *currentPos, line.begin() + pos, value.begin());

			*currentPos = pos;

			return true;
		}


		string value;

	private:

		const bool m_allow8bits;
		const char m_except;
	};


	class text8 : public text {

	public:

		text8() : text(true) {

		}
	};


	template <char C>
	class text_except : public text {

	public:

		text_except() : text(false, C) {

		}
	};


	template <char C>
	class text8_except : public text {

	public:

		text8_except() : text(true, C) {

		}
	};


	//
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	//

	DECLARE_COMPONENT(QUOTED_CHAR)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			const unsigned char c = static_cast <unsigned char>(pos < line.length() ? line[pos] : 0);

			if (c >= 0x01 && c <= 0x7f &&   // 0x01 - 0x7f
			    c != '"' && c != '\\' &&    // quoted_specials
			    c != '\r' && c != '\n') {   // CR and LF

				value = c;
				*currentPos = pos + 1;

			} else if (c == '\\' && pos + 1 < line.length() &&
			           (line[pos + 1] == '"' || line[pos + 1] == '\\')) {

				value = line[pos + 1];
				*currentPos = pos + 2;

			} else {

				VIMAP_PARSER_FAIL();
			}

			return true;
		}


		char value;
	};


	//
	// quoted          ::= <"> *QUOTED_CHAR <">
	// QUOTED_CHAR     ::= <any TEXT_CHAR except quoted_specials> / "\" quoted_specials
	// quoted_specials ::= <"> / "\"
	// TEXT_CHAR       ::= <any CHAR except CR and LF>
	// CHAR            ::= <any 7-bit US-ASCII character except NUL, 0x01 - 0x7f>
	//

	DECLARE_COMPONENT(quoted_text)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;
			size_t len = 0;
			bool valid = false;

			value.reserve(line.length() - pos);

			for (bool end = false, quoted = false ; !end && pos < line.length() ; ) {

				const unsigned char c = line[pos];

				if (quoted) {

					if (c == '"' || c == '\\') {
						value += c;
					} else {
						value += '\\';
						value += c;
					}

					quoted = false;

					++pos;
					++len;

				} else {

					if (c == '\\') {

						quoted = true;

						++pos;
						++len;

					} else if (c == '"') {

						valid = true;
						end = true;

					} else if (c >= 0x01 && c <= 0x7f &&  // CHAR
					           c != 0x0a && c != 0x0d) {  // CR and LF

						value += c;

						++pos;
						++len;

					} else {

						valid = false;
						end = true;
					}
				}
			}

			if (!valid) {
				VIMAP_PARSER_FAIL();
			}

			*currentPos = pos;

			return true;
		}


		string value;
	};


	//
	// nil  ::= "NIL"
	//

	DECLARE_COMPONENT(NIL)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

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

		xstring(
			const bool canBeNIL = false,
			component* comp = NULL,
			const int data = 0
		)
			: isNIL(true),
			  m_canBeNIL(canBeNIL),
			  m_component(comp),
			  m_data(data) {

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (m_canBeNIL &&
			    VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "nil")) {

				// NIL
				isNIL = true;

			} else {

				pos = *currentPos;

				isNIL = false;

				// quoted ::= <"> *QUOTED_CHAR <">
				if (VIMAP_PARSER_TRY_CHECK(one_char <'"'>)) {

					shared_ptr <quoted_text> text;
					VIMAP_PARSER_GET(quoted_text, text);
					VIMAP_PARSER_CHECK(one_char <'"'>);

					if (parser.m_literalHandler != NULL) {

						shared_ptr <literalHandler::target> target =
							parser.m_literalHandler->targetFor(*m_component, m_data);

						if (target != NULL) {

							value = "[literal-handler]";

							const size_t length = text->value.length();
							utility::progressListener* progress = target->progressListener();

							if (progress) {
								progress->start(length);
							}

							target->putData(text->value);

							if (progress) {
								progress->progress(length, length);
								progress->stop(length);
							}

						} else {

							value = text->value;
						}

					} else {

						value = text->value;
					}

					DEBUG_FOUND("string[quoted]", "<length=" << value.length() << ", value='" << value << "'>");

				// literal ::= "{" number "}" CRLF *CHAR8
				} else if (VIMAP_PARSER_TRY_CHECK(one_char <'{'>)) {

					shared_ptr <number> num;
					VIMAP_PARSER_GET(number, num);

					const size_t length = num->value;

					VIMAP_PARSER_CHECK(one_char <'}'> );

					VIMAP_PARSER_CHECK(CRLF);


					if (parser.m_literalHandler != NULL) {

						shared_ptr <literalHandler::target> target =
							parser.m_literalHandler->targetFor(*m_component, m_data);

						if (target != NULL) {

							value = "[literal-handler]";

							parser.m_progress = target->progressListener();
							parser.readLiteral(*target, length);
							parser.m_progress = NULL;

						} else {

							literalHandler::targetString target(NULL, value);
							parser.readLiteral(target, length);
						}

					} else {

						literalHandler::targetString target(NULL, value);
						parser.readLiteral(target, length);
					}

					line += parser.readLine();

					DEBUG_FOUND("string[literal]", "<length=" << length << ", value='" << value << "'>");

				} else {

					VIMAP_PARSER_FAIL();
				}
			}

			*currentPos = pos;

			return true;
		}


		bool isNIL;
		string value;

	private:

		bool m_canBeNIL;

		component* m_component;
		const int m_data;
	};


	//
	// nstring         ::= string / nil
	//

	class nstring : public xstring {

	public:

		const string getComponentName() const {

			return "nstring";
		}

		nstring(component* comp = NULL, const int data = 0)
			: xstring(true, comp, data) {

		}
	};


	//
	// astring         = 1*ASTRING-CHAR / string
	//
	// ASTRING-CHAR    = ATOM-CHAR / resp-specials
	//
	// ATOM-CHAR       = <any CHAR except atom-specials>
	//
	// atom-specials   = "(" / ")" / "{" / SP / CTL / list-wildcards /
	//                   quoted-specials / resp-specials
	//
	// list-wildcards  = "%" / "*"
	//
	// quoted-specials = DQUOTE / "\"
	//
	// resp-specials   = "]"
	//
	DECLARE_COMPONENT(astring)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			std::unique_ptr <xstring> str;
			VIMAP_PARSER_TRY_GET(xstring, str);

			if (str) {

				value = str->value;

			} else {

				value.reserve(line.length() - pos);

				for (bool end = false ; !end && pos < line.length() ; ) {

					const unsigned char c = line[pos];

					if (!parser.isStrict() || (c >= 0x01 && c <= 0x7f)) {  // CHAR or any byte in non-strict mode

						if (c == '(' ||
						    c == ')' ||
						    c == '{' ||
						    c == 0x20 ||  // SP
						    c == 0x0a || c == 0x0d ||  // CR and LF
						    c <= 0x1f || c == 0x7f ||  // CTL
						    c == '%' || c == '*' ||  // list-wildcards
						    c == '"' || c == '\\' ||  // quoted-specials
						    (parser.isStrict() && c == ']')) {  // resp-specials

							end = true;

						} else {

							value += c;

							++pos;
						}

					} else {

						end = true;
					}
				}
			}

			*currentPos = pos;

			return true;
		}


		string value;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;
			size_t len = 0;

			for (bool end = false ; !end && pos < line.length() ; ) {

				const unsigned char c = line[pos];

				switch (c) {

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

						if (c <= 0x1f || c >= 0x7f) {
							end = true;
						} else {
							++pos;
							++len;
						}
					}
			}

			if (len != 0) {

				value.resize(len);
				std::copy(line.begin() + *currentPos, line.begin() + pos, value.begin());

				*currentPos = pos;

			} else {

				VIMAP_PARSER_FAIL();
			}

			return true;
		}


		string value;
	};


	//
	// special atom (eg. "CAPABILITY", "FLAGS", "STATUS"...)
	//
	//  " Except as noted otherwise, all alphabetic characters are case-
	//    insensitive. The use of upper or lower case characters to define
	//    token strings is for editorial clarity only. Implementations MUST
	//    accept these strings in a case-insensitive fashion. "
	//

	class special_atom : public atom {

	public:

		const std::string getComponentName() const {

			return string("special_atom <") + m_string + ">";
		}

		special_atom(const char* str)
			: m_string(str) {  // 'string' must be in lower-case

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!atom::parseImpl(parser, line, &pos)) {
				return false;
			}

			const char* cmp = value.c_str();
			const char* with = m_string;

			bool ok = true;

			while (ok && *cmp && *with) {

				ok = (std::tolower(*cmp, std::locale()) == *with);

				++cmp;
				++with;
			}

			if (!ok || *cmp || *with) {
				VIMAP_PARSER_FAIL();
			}

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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			std::unique_ptr <atom> theCharset, theEncoding;
			std::unique_ptr <text> theText;

			VIMAP_PARSER_CHECK(one_char <'='> );
			VIMAP_PARSER_CHECK(one_char <'?'> );

			VIMAP_PARSER_GET(atom, theCharset);

			VIMAP_PARSER_CHECK(one_char <'?'> );

			VIMAP_PARSER_GET(atom, theEncoding);

			VIMAP_PARSER_CHECK(one_char <'?'> );

			VIMAP_PARSER_GET(text8_except <'?'> , theText);

			VIMAP_PARSER_CHECK(one_char <'?'> );
			VIMAP_PARSER_CHECK(one_char <'='> );

			charset = theCharset->value;

			// Decode text
			scoped_ptr <utility::encoder::encoder> theEncoder;

			if (theEncoding->value[0] == 'q' || theEncoding->value[0] == 'Q') {

				// Quoted-printable
				theEncoder.reset(new utility::encoder::qpEncoder());
				theEncoder->getProperties()["rfc2047"] = true;

			} else if (theEncoding->value[0] == 'b' || theEncoding->value[0] == 'B') {

				// Base64
				theEncoder.reset(new utility::encoder::b64Encoder());
			}

			if (theEncoder.get()) {

				utility::inputStreamStringAdapter in(theText->value);
				utility::outputStreamStringAdapter out(value);

				theEncoder->decode(in, out);

			// No decoder available
			} else {

				value = theText->value;
			}

			*currentPos = pos;

			return true;
		}


		vmime::charset charset;
		string value;
	};


	// seq-number      = nz-number / "*"
	//                    ; message sequence number (COPY, FETCH, STORE
	//                    ; commands) or unique identifier (UID COPY,
	//                    ; UID FETCH, UID STORE commands).

	DECLARE_COMPONENT(seq_number)

		seq_number()
			: star(false) {

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'*'> )) {

				star = true;
				number.reset();

			} else {

				star = false;
				VIMAP_PARSER_GET(IMAPParser::number, number);
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::number> number;
		bool star;
	};


	// seq-range       = seq-number ":" seq-number
	//                    ; two seq-number values and all values between
	//                    ; these two regardless of order.
	//                    ; Example: 2:4 and 4:2 are equivalent and indicate
	//                    ; values 2, 3, and 4.

	DECLARE_COMPONENT(seq_range)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(seq_number, first);

			VIMAP_PARSER_CHECK(one_char <'*'> );

			VIMAP_PARSER_GET(seq_number, last);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::seq_number> first;
		std::unique_ptr <IMAPParser::seq_number> last;
	};


	// sequence-set    = (seq-number / seq-range) *("," sequence-set)
	//                    ; set of seq-number values, regardless of order.
	//                    ; Servers MAY coalesce overlaps and/or execute the
	//                    ; sequence in any order.
	//                    ; Example: a message sequence number set of
	//                    ; 2,4:7,9,12:* for a mailbox with 15 messages is
	//                    ; equivalent to 2,4,5,6,7,9,12,13,14,15

	DECLARE_COMPONENT(sequence_set)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::seq_range, range)) {
				VIMAP_PARSER_GET(IMAPParser::seq_number, number);
			}

			if (VIMAP_PARSER_TRY_CHECK(one_char <','> )) {
				VIMAP_PARSER_GET(sequence_set, nextSet);
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::seq_number> number;
		std::unique_ptr <IMAPParser::seq_range> range;
		std::unique_ptr <IMAPParser::sequence_set> nextSet;
	};


	// mod-sequence-value  = 1*DIGIT
	//                        ;; Positive unsigned 64-bit integer
	//                        ;; (mod-sequence)
	//                        ;; (1 <= n < 18,446,744,073,709,551,615)

	DECLARE_COMPONENT(mod_sequence_value)

		mod_sequence_value()
			: value(0) {

		}

		bool parseImpl(IMAPParser& /* parser */, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			bool valid = true;
			vmime_uint64 val = 0;

			while (valid && pos < line.length()) {

				const char c = line[pos];

				if (c >= '0' && c <= '9') {
					val = (val * 10) + (c - '0');
					++pos;
				} else {
					valid = false;
				}
			}

			value = val;

			*currentPos = pos;

			return true;
		}


		vmime_uint64 value;
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
			: type(UNKNOWN) {

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'\\'> )) {

				if (VIMAP_PARSER_TRY_CHECK(one_char <'*'> )) {

					type = STAR;

				} else {

					shared_ptr <atom> at;
					VIMAP_PARSER_GET(atom, at);

					const string tname = utility::stringUtils::toLower(at->value);

					if (tname == "answered") {
						type = ANSWERED;
					} else if (tname == "flagged") {
						type = FLAGGED;
					} else if (tname == "deleted") {
						type = DELETED;
					} else if (tname == "seen") {
						type = SEEN;
					} else if (tname == "draft") {
						type = DRAFT;
					} else {
						type = UNKNOWN;
						name = tname;
					}
				}

			} else {

				if (!parser.isStrict() && VIMAP_PARSER_TRY_CHECK(one_char <'*'> )) {
					type = STAR;  // map "*" to "\*"
				} else {
					type = KEYWORD_OR_EXTENSION;
					VIMAP_PARSER_GET(atom, flag_keyword);
				}
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
			UNKNOWN,
			ANSWERED,
			FLAGGED,
			DELETED,
			SEEN,
			DRAFT,
			KEYWORD_OR_EXTENSION,
			STAR       // * = custom flags allowed
		};


		Type type;
		string name;

		std::unique_ptr <IMAPParser::atom> flag_keyword;
	};


	//
	// flag_list ::= "(" #flag ")"
	//

	DECLARE_COMPONENT(flag_list)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
				VIMAP_PARSER_GET_PUSHBACK(flag, flags);
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <flag>> flags;
	};


	//
	// mailbox ::= "INBOX" / astring
	//             ;; INBOX is case-insensitive.  All case variants of
	//             ;; INBOX (e.g. "iNbOx") MUST be interpreted as INBOX
	//             ;; not as an astring.  Refer to section 5.1 for
	//             ;; further semantic details of mailbox names.
	//

	DECLARE_COMPONENT(mailbox)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "inbox")) {

				type = INBOX;
				name = "INBOX";

			} else {

				type = OTHER;

				std::unique_ptr <astring> astr;
				VIMAP_PARSER_GET(astring, astr);
				name = astr->value;
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
			INBOX,
			OTHER
		};


		Type type;
		string name;
	};


	//
	// mailbox_flag := "\Marked" / "\Noinferiors" /
	//                 "\Noselect" / "\Unmarked" / flag_extension
	//

	DECLARE_COMPONENT(mailbox_flag)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'\\'> )) {

				std::unique_ptr <atom> at;
				VIMAP_PARSER_GET(atom, at);

				const string tname = utility::stringUtils::toLower(at->value);

				type = UNKNOWN;  // default

				switch (tname[0]) {

					case 'a':

						if (tname == "all") {
							type = SPECIALUSE_ALL;
						} else if (tname == "archive") {
							type = SPECIALUSE_ARCHIVE;
						}

						break;

					case 'd':

						if (tname == "drafts") {
							type = SPECIALUSE_DRAFTS;
						}

						break;

					case 'f':

						if (tname == "flagged") {
							type = SPECIALUSE_FLAGGED;
						}

						break;

					case 'h':

						if (tname == "haschildren") {
							type = HASCHILDREN;
						} else if (tname == "hasnochildren") {
							type = HASNOCHILDREN;
						}

						break;

					case 'i':

						if (tname == "important") {
							type = SPECIALUSE_IMPORTANT;
						}

						break;

					case 'j':

						if (tname == "junk") {
							type = SPECIALUSE_JUNK;
						}

						break;

					case 'm':

						if (tname == "marked") {
							type = MARKED;
						}

						break;

					case 'n':

						if (tname == "noinferiors") {
							type = NOINFERIORS;
						} else if (tname == "noselect") {
							type = NOSELECT;
						}

						break;

					case 's':

						if (tname == "sent") {
							type = SPECIALUSE_SENT;
						}

						break;

					case 't':

						if (tname == "trash") {
							type = SPECIALUSE_TRASH;
						}

						break;

					case 'u':

						if (tname == "unmarked") {
							type = UNMARKED;
						}

						break;
				}

				if (type == UNKNOWN) {
					name = "\\" + tname;
				}

			} else {

				std::unique_ptr <atom> at;
				VIMAP_PARSER_GET(atom, at);
				const string tname = utility::stringUtils::toLower(at->value);

				type = UNKNOWN;
				name = tname;
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
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


		Type type;
		string name;
	};


	//
	// mailbox_flag_list ::= "(" #(mailbox_flag) ")"
	//

	DECLARE_COMPONENT(mailbox_flag_list)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
				VIMAP_PARSER_GET_PUSHBACK(mailbox_flag, flags);
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <mailbox_flag>> flags;
	};


	//
	// mailbox_list ::= mailbox_flag_list SPACE
	//                  (<"> QUOTED_CHAR <"> / nil) SPACE mailbox
	//

	DECLARE_COMPONENT(mailbox_list)

		mailbox_list()
			: quoted_char('\0') {

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::mailbox_flag_list, mailbox_flag_list);

			VIMAP_PARSER_CHECK(SPACE);

			if (!VIMAP_PARSER_TRY_CHECK(NIL)) {

				VIMAP_PARSER_CHECK(one_char <'"'> );

				std::unique_ptr <QUOTED_CHAR> qc;
				VIMAP_PARSER_GET(QUOTED_CHAR, qc);
				quoted_char = qc->value;

				VIMAP_PARSER_CHECK(one_char <'"'> );
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::mailbox, mailbox);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::mailbox_flag_list> mailbox_flag_list;
		std::unique_ptr <IMAPParser::mailbox> mailbox;

		char quoted_char;
	};


	//
	// auth_type ::= atom
	//               ;; Defined by [IMAP-AUTH]
	//

	DECLARE_COMPONENT(auth_type)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			std::unique_ptr <atom> at;
			VIMAP_PARSER_GET(atom, at);

			name = utility::stringUtils::toLower(at->value);

			if (name == "kerberos_v4") {
				type = KERBEROS_V4;
			} else if (name == "gssapi") {
				type = GSSAPI;
			} else if (name == "skey") {
				type = SKEY;
			} else {
				type = UNKNOWN;
			}

			return true;
		}


		enum Type {
			UNKNOWN,

			// RFC 1731 - IMAP4 Authentication Mechanisms
			KERBEROS_V4,
			GSSAPI,
			SKEY
		};


		Type type;
		string name;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			// "HIGHESTMODSEQ" SP mod-sequence-valzer
			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "highestmodseq")) {

				type = HIGHESTMODSEQ;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::mod_sequence_value, value);

			} else {

				if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "messages")) {
					type = MESSAGES;
				} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "recent")) {
					type = RECENT;
				} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidnext")) {
					type = UIDNEXT;
				} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidvalidity")) {
					type = UIDVALIDITY;
				} else {
					VIMAP_PARSER_CHECK_WITHARG(special_atom, "unseen");
					type = UNSEEN;
				}

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::number, value);
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
			// Extensions
			HIGHESTMODSEQ,

			// Standard IMAP
			MESSAGES,
			RECENT,
			UIDNEXT,
			UIDVALIDITY,
			UNSEEN
		};


		Type type;
		std::unique_ptr <IMAPParser::component> value;


		const IMAPParser::number* value_as_number() const {
			return dynamic_cast <IMAPParser::number *>(value.get());
		}

		const IMAPParser::mod_sequence_value* value_as_mod_sequence_value() const {
			return dynamic_cast <IMAPParser::mod_sequence_value *>(value.get());
		}
	};


	// status-att-list = status-att-val *(SP status-att-val)

	DECLARE_COMPONENT(status_att_list)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET_PUSHBACK(IMAPParser::status_att_val, values);

			while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
				VIMAP_PARSER_GET_PUSHBACK(IMAPParser::status_att_val, values);
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <status_att_val>> values;
	};


	//
	// capability ::= "AUTH=" auth_type / atom
	//                ;; New capabilities MUST begin with "X" or be
	//                ;; registered with IANA as standard or standards-track
	//

	DECLARE_COMPONENT(capability)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::atom, atom);

			string value = atom->value;
			const char* str = value.c_str();

			if ((str[0] == 'a' || str[0] == 'A') &&
			    (str[1] == 'u' || str[1] == 'U') &&
			    (str[2] == 't' || str[2] == 'T') &&
			    (str[3] == 'h' || str[3] == 'H') &&
			    (str[4] == '=')) {

				size_t pos = 5;
				auth_type.reset(parser.get <IMAPParser::auth_type>(value, &pos));

				atom.reset();
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::auth_type> auth_type;
		std::unique_ptr <IMAPParser::atom> atom;
	};


	//
	// capability_data ::= "CAPABILITY" SPACE [1#capability SPACE] "IMAP4rev1"
	//                     [SPACE 1#capability]
	//                     ;; IMAP4rev1 servers which offer RFC 1730
	//                     ;; compatibility MUST list "IMAP4" as the first
	//                     ;; capability.
	//

	DECLARE_COMPONENT(capability_data)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK_WITHARG(special_atom, "capability");

			while (VIMAP_PARSER_TRY_CHECK(SPACE)) {

				std::unique_ptr <capability> cap;

				if (parser.isStrict() || capabilities.empty()) {
					VIMAP_PARSER_GET(capability, cap);
				} else {
					VIMAP_PARSER_TRY_GET(capability, cap);  // allow SPACE at end of line (Apple iCloud IMAP server)
				}

				if (!cap) {
					break;
				}

				capabilities.push_back(std::move(cap));
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <capability>> capabilities;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			// <"> date_day_fixed "-" date_month "-" date_year
			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_TRY_CHECK(SPACE);

			shared_ptr <number> nd;
			VIMAP_PARSER_GET(number, nd);

			VIMAP_PARSER_CHECK(one_char <'-'> );

			shared_ptr <atom> amo;
			VIMAP_PARSER_GET(atom, amo);

			VIMAP_PARSER_CHECK(one_char <'-'> );

			shared_ptr <number> ny;
			VIMAP_PARSER_GET(number, ny);

			VIMAP_PARSER_TRY_CHECK(SPACE);

			// 2digit ":" 2digit ":" 2digit
			shared_ptr <number> nh;
			VIMAP_PARSER_GET(number, nh);

			VIMAP_PARSER_CHECK(one_char <':'> );

			shared_ptr <number> nmi;
			VIMAP_PARSER_GET(number, nmi);

			VIMAP_PARSER_CHECK(one_char <':'> );

			shared_ptr <number> ns;
			VIMAP_PARSER_GET(number, ns);

			VIMAP_PARSER_TRY_CHECK(SPACE);

			// ("+" / "-") 4digit
			int sign = 1;

			if (!(VIMAP_PARSER_TRY_CHECK(one_char <'+'> ))) {
				VIMAP_PARSER_CHECK(one_char <'-'> );
			}

			shared_ptr <number> nz;
			VIMAP_PARSER_GET(number, nz);

			VIMAP_PARSER_CHECK(one_char <'"'> );


			m_datetime.setHour(static_cast <int>(std::min(std::max(nh->value, 0ul), 23ul)));
			m_datetime.setMinute(static_cast <int>(std::min(std::max(nmi->value, 0ul), 59ul)));
			m_datetime.setSecond(static_cast <int>(std::min(std::max(ns->value, 0ul), 59ul)));

			const int zone = static_cast <int>(nz->value);
			const int zh = zone / 100;   // hour offset
			const int zm = zone % 100;   // minute offset

			m_datetime.setZone(((zh * 60) + zm) * sign);

			m_datetime.setDay(static_cast <int>(std::min(std::max(nd->value, 1ul), 31ul)));
			m_datetime.setYear(static_cast <int>(ny->value));

			const string month(utility::stringUtils::toLower(amo->value));
			int mon = vmime::datetime::JANUARY;

			if (month.length() >= 3) {

				switch (month[0]) {

					case 'j': {

						switch (month[1]) {

							case 'a': mon = vmime::datetime::JANUARY; break;
							case 'u': {

								switch (month[2]) {

									case 'n': mon = vmime::datetime::JUNE; break;
									default:  mon = vmime::datetime::JULY; break;
								}

								break;
							}

						}

						break;
					}
					case 'f': mon = vmime::datetime::FEBRUARY; break;
					case 'm': {

						switch (month[2]) {
							case 'r': mon = vmime::datetime::MARCH; break;
							default:  mon = vmime::datetime::MAY; break;
						}

						break;
					}
					case 'a':
					{
						switch (month[1]) {
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
				VIMAP_PARSER_GET_PUSHBACK(header_fld_name, fld_names);
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <header_fld_name>> fld_names;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> )) {

				VIMAP_PARSER_GET_PUSHBACK(body_extension, body_extensions);

				while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
					VIMAP_PARSER_GET_PUSHBACK(body_extension, body_extensions);
					VIMAP_PARSER_TRY_CHECK(SPACE);
				}

			} else {

				if (!VIMAP_PARSER_TRY_GET(IMAPParser::nstring, nstring)) {
					VIMAP_PARSER_GET(IMAPParser::number, number);
				}
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::nstring> nstring;
		std::unique_ptr <IMAPParser::number> number;

		std::vector <std::unique_ptr <body_extension>> body_extensions;
	};


	//
	// section_text    ::= "HEADER" / "HEADER.FIELDS" [".NOT"]
	//                     SPACE header_list / "TEXT" / "MIME"
	//

	DECLARE_COMPONENT(section_text)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			// "HEADER.FIELDS" [".NOT"] SPACE header_list
			const bool b1 = VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "header.fields.not");
			const bool b2 = (b1 ? false : VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "header.fields"));

			if (b1 || b2) {

				type = b1 ? HEADER_FIELDS_NOT : HEADER_FIELDS;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::header_list, header_list);

			// "HEADER"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "header")) {

				type = HEADER;

			// "MIME"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "mime")) {

				type = MIME;

			// "TEXT"
			} else {

				type = TEXT;

				VIMAP_PARSER_CHECK_WITHARG(special_atom, "text");
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
			HEADER,
			HEADER_FIELDS,
			HEADER_FIELDS_NOT,
			MIME,
			TEXT
		};


		Type type;
		std::unique_ptr <IMAPParser::header_list> header_list;
	};


	//
	// section         ::= "[" [section_text / (nz_number *["." nz_number]
	//                     ["." (section_text / "MIME")])] "]"
	//

	DECLARE_COMPONENT(section)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'['> );

			if (!VIMAP_PARSER_TRY_CHECK(one_char <']'> )) {

				if (!VIMAP_PARSER_TRY_GET(section_text, section_text1)) {

					std::unique_ptr <nz_number> num;
					VIMAP_PARSER_GET(nz_number, num);
					nz_numbers.push_back(static_cast <unsigned int>(num->value));

					while (VIMAP_PARSER_TRY_CHECK(one_char <'.'> )) {

						if (VIMAP_PARSER_TRY_GET(nz_number, num)) {
							nz_numbers.push_back(static_cast <unsigned int>(num->value));
						} else {
							VIMAP_PARSER_GET(section_text, section_text2);
							break;
						}
					}
				}

				VIMAP_PARSER_CHECK(one_char <']'> );
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <section_text> section_text1;
		std::unique_ptr <section_text> section_text2;
		std::vector <unsigned int> nz_numbers;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );
			VIMAP_PARSER_GET(nstring, addr_name);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(nstring, addr_adl);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(nstring, addr_mailbox);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(nstring, addr_host);
			VIMAP_PARSER_CHECK(one_char <')'> );

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <nstring> addr_name;
		std::unique_ptr <nstring> addr_adl;
		std::unique_ptr <nstring> addr_mailbox;
		std::unique_ptr <nstring> addr_host;
	};


	//
	// address_list ::= "(" 1*address ")" / nil
	//

	DECLARE_COMPONENT(address_list)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_CHECK(NIL)) {

				VIMAP_PARSER_CHECK(one_char <'('> );

				while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
					VIMAP_PARSER_GET_PUSHBACK(address, addresses);
					VIMAP_PARSER_TRY_CHECK(SPACE);
				}
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <address>> addresses;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			VIMAP_PARSER_GET(IMAPParser::env_date, env_date);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_subject, env_subject);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_from, env_from);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_sender, env_sender);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_reply_to, env_reply_to);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_to, env_to);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_cc, env_cc);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_bcc, env_bcc);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_in_reply_to, env_in_reply_to);
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::env_message_id, env_message_id);

			VIMAP_PARSER_CHECK(one_char <')'> );

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::env_date> env_date;
		std::unique_ptr <IMAPParser::env_subject> env_subject;
		std::unique_ptr <IMAPParser::env_from> env_from;
		std::unique_ptr <IMAPParser::env_sender> env_sender;
		std::unique_ptr <IMAPParser::env_reply_to> env_reply_to;
		std::unique_ptr <IMAPParser::env_to> env_to;
		std::unique_ptr <IMAPParser::env_cc> env_cc;
		std::unique_ptr <IMAPParser::env_bcc> env_bcc;
		std::unique_ptr <IMAPParser::env_in_reply_to> env_in_reply_to;
		std::unique_ptr <IMAPParser::env_message_id> env_message_id;
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

	class body_fld_enc : public nstring {

	public:

		const string getComponentName() const {

			return "body_fld_enc";
		}

		body_fld_enc() {

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!xstring::parseImpl(parser, line, &pos)) {
				return false;
			}

			// " When an IMAP4 client sends a FETCH (bodystructure) request
			//   to a server that is running the Exchange Server 2007 IMAP4
			//   service, a corrupted response is sent as a reply "
			//   (see http://support.microsoft.com/kb/975918/en-us)
			//
			// Fail in strict mode
			if (isNIL && parser.isStrict()) {
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!parser.isStrict()) {

				// Some servers send an <atom> instead of a <string> here:
				// eg. ... (CHARSET "X-UNKNOWN") ...
				if (!VIMAP_PARSER_TRY_GET(xstring, string1)) {

					std::unique_ptr <atom> at;
					VIMAP_PARSER_GET(atom, at);

					string1.reset(new xstring());
					string1->value = at->value;
				}

			} else {

				VIMAP_PARSER_GET(xstring, string1);
			}

			VIMAP_PARSER_CHECK(SPACE);

			if (!parser.isStrict()) {

				// In non-strict mode, allow NIL in value
				std::unique_ptr <nstring> nstr;
				VIMAP_PARSER_GET(nstring, nstr);

				string2.reset(new xstring());
				string2->value = nstr->value;

			} else {

				VIMAP_PARSER_GET(xstring, string2);
			}

			DEBUG_FOUND("body_fld_param_item", "<" << string1->value << ", " << string2->value << ">");

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <xstring> string1;
		std::unique_ptr <xstring> string2;
	};


	//
	// body_fld_param  ::= "(" 1#(body_fld_param_item) ")" / nil
	//

	DECLARE_COMPONENT(body_fld_param)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> )) {

				bool isNIL = false;

				if (!parser.isStrict()) {

					// In non-strict mode, allow "()" instead of "NIL"
					if (VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
						isNIL = true;
					}
				}

				if (!isNIL) {

					VIMAP_PARSER_GET_PUSHBACK(body_fld_param_item, items);

					while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
						VIMAP_PARSER_CHECK(SPACE);
						VIMAP_PARSER_GET_PUSHBACK(body_fld_param_item, items);
					}
				}

			} else {

				VIMAP_PARSER_CHECK(NIL);
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <body_fld_param_item>> items;
	};


	//
	// body_fld_dsp    ::= "(" string SPACE body_fld_param ")" / nil
	//

	DECLARE_COMPONENT(body_fld_dsp)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> )) {

				VIMAP_PARSER_GET(xstring, m_string);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(class body_fld_param, m_body_fld_param);
				VIMAP_PARSER_CHECK(one_char <')'> );

			} else {

				VIMAP_PARSER_CHECK(NIL);
			}

			*currentPos = pos;

			return true;
		}

	private:

		std::unique_ptr <class xstring> m_string;
		std::unique_ptr <class body_fld_param> m_body_fld_param;

	public:

		const class xstring* str() const { return m_string.get(); }
		const class body_fld_param* body_fld_param() const { return m_body_fld_param.get(); }
	};


	//
	// body_fld_lang   ::= nstring / "(" 1#string ")"
	//

	DECLARE_COMPONENT(body_fld_lang)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'('> )) {

				VIMAP_PARSER_GET_PUSHBACK(xstring, strings);

				while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
					VIMAP_PARSER_CHECK(SPACE);
					VIMAP_PARSER_GET_PUSHBACK(xstring, strings);
				}

			} else {

				VIMAP_PARSER_GET_PUSHBACK(nstring, strings);
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <xstring>> strings;
	};


	//
	// body_fields     ::= body_fld_param SPACE body_fld_id SPACE
	//                     body_fld_desc SPACE body_fld_enc SPACE
	//                     body_fld_octets
	//

	DECLARE_COMPONENT(body_fields)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::body_fld_param, body_fld_param);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_id, body_fld_id);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_desc, body_fld_desc);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_enc, body_fld_enc);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_octets, body_fld_octets);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::body_fld_param> body_fld_param;
		std::unique_ptr <IMAPParser::body_fld_id> body_fld_id;
		std::unique_ptr <IMAPParser::body_fld_desc> body_fld_desc;
		std::unique_ptr <IMAPParser::body_fld_enc> body_fld_enc;
		std::unique_ptr <IMAPParser::body_fld_octets> body_fld_octets;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK_WITHARG(special_atom, "text");
			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::media_subtype, media_subtype);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::media_subtype> media_subtype;
	};


	//
	// media_message   ::= <"> "MESSAGE" <"> SPACE <"> "RFC822" <">
	//                     ;; Defined in [MIME-IMT]
	//

	DECLARE_COMPONENT(media_message)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK_WITHARG(special_atom, "message");
			VIMAP_PARSER_CHECK(one_char <'"'> );
			VIMAP_PARSER_CHECK(SPACE);

			//VIMAP_PARSER_CHECK(one_char <'"'> );
			//VIMAP_PARSER_CHECK_WITHARG(special_atom, "rfc822");
			//VIMAP_PARSER_CHECK(one_char <'"'> );

			VIMAP_PARSER_GET(IMAPParser::media_subtype, media_subtype);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::media_subtype> media_subtype;
	};


	//
	// media_basic     ::= (<"> ("APPLICATION" / "AUDIO" / "IMAGE" /
	//                     "MESSAGE" / "VIDEO") <">) / string)
	//                     SPACE media_subtype
	//                     ;; Defined in [MIME-IMT]

	DECLARE_COMPONENT(media_basic)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(xstring, media_type);

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::media_subtype, media_subtype);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::xstring> media_type;
		std::unique_ptr <IMAPParser::media_subtype> media_subtype;
	};


	//
	// body_ext_1part  ::= body_fld_md5 [SPACE body_fld_dsp
	//                     [SPACE body_fld_lang
	//                     [SPACE 1#body_extension]]]
	//                     ;; MUST NOT be returned on non-extensible
	//                     ;; "BODY" fetch
	//

	DECLARE_COMPONENT(body_ext_1part)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos)
		{
			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::body_fld_md5, body_fld_md5);

			// [SPACE body_fld_dsp
			if (VIMAP_PARSER_TRY_CHECK(SPACE)) {

				VIMAP_PARSER_GET(IMAPParser::body_fld_dsp, body_fld_dsp);

				// [SPACE body_fld_lang
				if (VIMAP_PARSER_TRY_CHECK(SPACE)) {

					VIMAP_PARSER_GET(IMAPParser::body_fld_lang, body_fld_lang);

					// [SPACE 1#body_extension]
					if (VIMAP_PARSER_TRY_CHECK(SPACE)) {

						VIMAP_PARSER_GET_PUSHBACK(body_extension, body_extensions);

						while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
							VIMAP_PARSER_GET_PUSHBACK(body_extension, body_extensions);
						}
					}
				}
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::body_fld_md5> body_fld_md5;
		std::unique_ptr <IMAPParser::body_fld_dsp> body_fld_dsp;
		std::unique_ptr <IMAPParser::body_fld_lang> body_fld_lang;

		std::vector <std::unique_ptr <body_extension>> body_extensions;
	};


	//
	// body_ext_mpart  ::= body_fld_param
	//                     [SPACE body_fld_dsp [SPACE body_fld_lang
	//                     [SPACE 1#body_extension]]]
	//                     ;; MUST NOT be returned on non-extensible
	//                     ;; "BODY" fetch

	DECLARE_COMPONENT(body_ext_mpart)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::body_fld_param, body_fld_param);

			// [SPACE body_fld_dsp [SPACE body_fld_lang [SPACE 1#body_extension]]]
			if (VIMAP_PARSER_TRY_CHECK(SPACE)) {

				VIMAP_PARSER_GET(IMAPParser::body_fld_dsp, body_fld_dsp);

				if (VIMAP_PARSER_TRY_CHECK(SPACE)) {

					VIMAP_PARSER_GET(IMAPParser::body_fld_lang, body_fld_lang);

					// [SPACE 1#body_extension]
					if (VIMAP_PARSER_TRY_CHECK(SPACE)) {

						VIMAP_PARSER_GET_PUSHBACK(body_extension, body_extensions);

						while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
							VIMAP_PARSER_GET_PUSHBACK(body_extension, body_extensions);
						}
					}
				}
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::body_fld_param> body_fld_param;
		std::unique_ptr <IMAPParser::body_fld_dsp> body_fld_dsp;
		std::unique_ptr <IMAPParser::body_fld_lang> body_fld_lang;

		std::vector <std::unique_ptr <body_extension>> body_extensions;
	};


	//
	// body_type_basic ::= media_basic SPACE body_fields
	//                     ;; MESSAGE subtype MUST NOT be "RFC822"
	//

	DECLARE_COMPONENT(body_type_basic)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::media_basic, media_basic);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fields, body_fields);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::media_basic> media_basic;
		std::unique_ptr <IMAPParser::body_fields> body_fields;
	};


	//
	// body_type_msg   ::= media_message SPACE body_fields SPACE envelope
	//                     SPACE body SPACE body_fld_lines
	//

	class xbody;
	typedef xbody body;

	DECLARE_COMPONENT(body_type_msg)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::media_message, media_message);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fields, body_fields);
			VIMAP_PARSER_CHECK(SPACE);

			// BUGFIX: made SPACE optional. This is not standard, but some servers
			// seem to return responses like that...
			VIMAP_PARSER_GET(IMAPParser::envelope, envelope);
			VIMAP_PARSER_TRY_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::xbody, body);
			VIMAP_PARSER_TRY_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_lines, body_fld_lines);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::media_message> media_message;
		std::unique_ptr <IMAPParser::body_fields> body_fields;
		std::unique_ptr <IMAPParser::envelope> envelope;
		std::unique_ptr <IMAPParser::xbody> body;
		std::unique_ptr <IMAPParser::body_fld_lines> body_fld_lines;
	};


	//
	// body_type_text  ::= media_text SPACE body_fields SPACE body_fld_lines
	//

	DECLARE_COMPONENT(body_type_text)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::media_text, media_text);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fields, body_fields);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::body_fld_lines, body_fld_lines);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::media_text> media_text;
		std::unique_ptr <IMAPParser::body_fields> body_fields;
		std::unique_ptr <IMAPParser::body_fld_lines> body_fld_lines;
	};


	//
	// body_type_1part ::= (body_type_basic / body_type_msg / body_type_text)
	//                     [SPACE body_ext_1part]
	//

	DECLARE_COMPONENT(body_type_1part)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_type_text, body_type_text)) {
				if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_type_msg, body_type_msg)) {
					VIMAP_PARSER_GET(IMAPParser::body_type_basic, body_type_basic);
				}
			}

			if (VIMAP_PARSER_TRY_CHECK(SPACE)) {
				if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_ext_1part, body_ext_1part)) {
					--pos;
				}
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::body_type_basic> body_type_basic;
		std::unique_ptr <IMAPParser::body_type_msg> body_type_msg;
		std::unique_ptr <IMAPParser::body_type_text> body_type_text;

		std::unique_ptr <IMAPParser::body_ext_1part> body_ext_1part;
	};


	//
	// body_type_mpart ::= 1*body SPACE media_subtype
	//                     [SPACE body_ext_mpart]
	//

	DECLARE_COMPONENT(body_type_mpart)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET_PUSHBACK(xbody, list);

			while (true) {
				VIMAP_PARSER_TRY_GET_PUSHBACK_OR_ELSE(xbody, list, break);
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::media_subtype, media_subtype);

			if (VIMAP_PARSER_TRY_CHECK(SPACE)) {
				VIMAP_PARSER_GET(IMAPParser::body_ext_mpart, body_ext_mpart);
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::media_subtype> media_subtype;
		std::unique_ptr <IMAPParser::body_ext_mpart> body_ext_mpart;

		std::vector <std::unique_ptr <xbody>> list;
	};


	//
	// xbody ::= "(" body_type_1part / body_type_mpart ")"
	//

	DECLARE_COMPONENT(xbody)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::body_type_mpart, body_type_mpart)) {
				VIMAP_PARSER_GET(IMAPParser::body_type_1part, body_type_1part);
			}

			VIMAP_PARSER_CHECK(one_char <')'> );

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::body_type_1part> body_type_1part;
		std::unique_ptr <IMAPParser::body_type_mpart> body_type_mpart;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			// "ENVELOPE" SPACE envelope
			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "envelope")) {

				type = ENVELOPE;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::envelope, envelope);

			// "FLAGS" SPACE "(" #(flag / "\Recent") ")"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "flags")) {

				type = FLAGS;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::flag_list, flag_list);

			// "INTERNALDATE" SPACE date_time
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "internaldate")) {

				type = INTERNALDATE;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::date_time, date_time);

			// "RFC822" ".HEADER" SPACE nstring
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822.header")) {

				type = RFC822_HEADER;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::nstring, nstring);

			// "RFC822" ".TEXT" SPACE nstring
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822.text")) {

				type = RFC822_TEXT;

				VIMAP_PARSER_CHECK(SPACE);

				nstring.reset(parser.getWithArgs <IMAPParser::nstring>(line, &pos, this, RFC822_TEXT));

				VIMAP_PARSER_FAIL_UNLESS(nstring);

			// "RFC822.SIZE" SPACE number
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822.size")) {

				type = RFC822_SIZE;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::number, number);

			// "RFC822" SPACE nstring
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "rfc822")) {

				type = RFC822;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::nstring, nstring);

			// "BODY" "STRUCTURE" SPACE body
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "bodystructure")) {

				type = BODY_STRUCTURE;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::body, body);

			// "BODY" section ["<" number ">"] SPACE nstring
			// "BODY" SPACE body
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "body")) {

				VIMAP_PARSER_TRY_GET(IMAPParser::section, section);

				// "BODY" section ["<" number ">"] SPACE nstring
				if (section != NULL) {

					type = BODY_SECTION;

					if (VIMAP_PARSER_TRY_CHECK(one_char <'<'> )) {
						VIMAP_PARSER_GET(IMAPParser::number, number);
						VIMAP_PARSER_CHECK(one_char <'>'> );
					}

					VIMAP_PARSER_CHECK(SPACE);

					nstring.reset(parser.getWithArgs <IMAPParser::nstring>(line, &pos, this, BODY_SECTION));

					VIMAP_PARSER_FAIL_UNLESS(nstring);

				// "BODY" SPACE body
				} else {

					type = BODY;

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::body, body);
				}

			// "MODSEQ" SP "(" mod_sequence_value ")"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "modseq")) {

				type = MODSEQ;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_CHECK(one_char <'('> );

				VIMAP_PARSER_GET(IMAPParser::mod_sequence_value, mod_sequence_value);

				VIMAP_PARSER_CHECK(one_char <')'> );

			// "UID" SPACE uniqueid
			} else {

				type = UID;

				VIMAP_PARSER_CHECK_WITHARG(special_atom, "uid");
				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::uniqueid, uniqueid);
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
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


		Type type;

		std::unique_ptr <IMAPParser::date_time> date_time;
		std::unique_ptr <IMAPParser::number> number;
		std::unique_ptr <IMAPParser::envelope> envelope;
		std::unique_ptr <IMAPParser::uniqueid> uniqueid;
		std::unique_ptr <IMAPParser::nstring> nstring;
		std::unique_ptr <IMAPParser::xbody> body;
		std::unique_ptr <IMAPParser::flag_list> flag_list;
		std::unique_ptr <IMAPParser::section> section;
		std::unique_ptr <IMAPParser::mod_sequence_value> mod_sequence_value;
	};


	//
	// msg_att ::= "(" 1#(msg_att_item) ")"
	//

	DECLARE_COMPONENT(msg_att)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'('> );

			items.push_back(std::move(std::unique_ptr <msg_att_item>(parser.get <msg_att_item>(line, &pos))));

			while (!VIMAP_PARSER_TRY_CHECK(one_char <')'> )) {
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET_PUSHBACK(msg_att_item, items);
			}

			*currentPos = pos;

			return true;
		}


		std::vector <std::unique_ptr <msg_att_item>> items;
	};


	//
	// message_data ::= nz_number SPACE ("EXPUNGE" /
	//                  ("FETCH" SPACE msg_att))
	//

	DECLARE_COMPONENT(message_data)

		message_data()
			: number(0) {

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			scoped_ptr <nz_number> num;
			VIMAP_PARSER_GET(nz_number, num);
			number = static_cast <unsigned int>(num->value);

			VIMAP_PARSER_CHECK(SPACE);

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "expunge")) {

				type = EXPUNGE;

			} else {

				type = FETCH;

				VIMAP_PARSER_CHECK_WITHARG(special_atom, "fetch");
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::msg_att, msg_att);
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
			EXPUNGE,
			FETCH
		};


		Type type;
		unsigned int number;
		std::unique_ptr <IMAPParser::msg_att> msg_att;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			// "ALERT"
			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "alert")) {

				type = ALERT;

			// "PARSE"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "parse")) {

				type = PARSE;

			// capability_data
			} else if (VIMAP_PARSER_TRY_GET(IMAPParser::capability_data, capability_data)) {

				type = CAPABILITY;

			// "PERMANENTFLAGS" SPACE flag_list
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "permanentflags")) {

				type = PERMANENTFLAGS;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::flag_list, flag_list);

			// "READ-ONLY"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "read-only")) {

				type = READ_ONLY;

			// "READ-WRITE"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "read-write")) {

				type = READ_WRITE;

			// "TRYCREATE"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "trycreate")) {

				type = TRYCREATE;

			// "UIDVALIDITY" SPACE nz_number
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidvalidity")) {

				type = UIDVALIDITY;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, nz_number);

			// "UIDNEXT" SPACE nz_number
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidnext")) {

				type = UIDNEXT;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, nz_number);

			// "UNSEEN" SPACE nz_number
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "unseen")) {

				type = UNSEEN;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, nz_number);

			// "HIGHESTMODSEQ" SP mod-sequence-value
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "highestmodseq")) {

				type = HIGHESTMODSEQ;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::mod_sequence_value, mod_sequence_value);

			// "NOMODSEQ"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "nomodseq")) {

				type = NOMODSEQ;

			// "MODIFIED" SP sequence-set
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "modified")) {

				type = MODIFIED;

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::sequence_set, sequence_set);

			// "APPENDUID" SP nz-number SP append-uid
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "appenduid")) {

				type = APPENDUID;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, nz_number);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::uid_set, uid_set);

			// "COPYUID" SP nz-number SP uid-set SP uid-set
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "copyuid")) {

				type = COPYUID;

				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::nz_number, nz_number);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::uid_set, uid_set);
				VIMAP_PARSER_CHECK(SPACE);
				VIMAP_PARSER_GET(IMAPParser::uid_set, uid_set2);

			// "UIDNOTSTICKY"
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "uidnotsticky")) {

				type = UIDNOTSTICKY;

			// atom [SPACE 1*<any TEXT_CHAR except "]">]
			} else {

				type = OTHER;

				VIMAP_PARSER_GET(IMAPParser::atom, atom);

				if (VIMAP_PARSER_TRY_CHECK(SPACE)) {
					VIMAP_PARSER_GET(text_except <']'> , text);
				}
			}

			*currentPos = pos;

			return true;
		}


		enum Type {
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


		Type type;

		std::unique_ptr <IMAPParser::nz_number> nz_number;
		std::unique_ptr <IMAPParser::atom> atom;
		std::unique_ptr <IMAPParser::flag_list> flag_list;
		std::unique_ptr <IMAPParser::text> text;
		std::unique_ptr <IMAPParser::mod_sequence_value> mod_sequence_value;
		std::unique_ptr <IMAPParser::sequence_set> sequence_set;
		std::unique_ptr <IMAPParser::capability_data> capability_data;
		std::unique_ptr <IMAPParser::uid_set> uid_set;
		std::unique_ptr <IMAPParser::uid_set> uid_set2;
	};


	//
	// resp_text ::= ["[" resp_text_code "]" SPACE] (text_mime2 / text)
	//               ;; text SHOULD NOT begin with "[" or "="

	DECLARE_COMPONENT(resp_text)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK(one_char <'['> )) {

				VIMAP_PARSER_GET(IMAPParser::resp_text_code, resp_text_code);

				VIMAP_PARSER_CHECK(one_char <']'> );
				VIMAP_PARSER_TRY_CHECK(SPACE);
			}

			std::unique_ptr <text_mime2> text1;
			VIMAP_PARSER_TRY_GET(text_mime2, text1);

			if (text1.get()) {

				text = text1->value;

			} else {

				std::unique_ptr <IMAPParser::text> text2;
				VIMAP_PARSER_TRY_GET(IMAPParser::text, text2);

				if (text2.get()) {
					text = text2->value;
				} else {
					// Empty response text
				}
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::resp_text_code> resp_text_code;
		string text;
	};


	//
	// continue_req   ::= "+" SPACE (resp_text / base64)
	//

	DECLARE_COMPONENT(continue_req)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'+'> );

			if (!parser.isStrict()) {

				// Some servers do not send SPACE when response text is empty
				if (VIMAP_PARSER_TRY_CHECK(SPACE)) {
					VIMAP_PARSER_GET(IMAPParser::resp_text, resp_text);
				} else {
					resp_text.reset(new IMAPParser::resp_text());  // empty
				}

			} else {

				VIMAP_PARSER_CHECK(SPACE);

				VIMAP_PARSER_GET(IMAPParser::resp_text, resp_text);
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::resp_text> resp_text;
	};


	//
	// resp_cond_state ::= ("OK" / "NO" / "BAD") SPACE resp_text
	//                     ;; Status condition
	//

	DECLARE_COMPONENT(resp_cond_state)

		resp_cond_state()
			: status(BAD) {

		}

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "ok")) {
				status = OK;
			} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "no")) {
				status = NO;
			} else {
				VIMAP_PARSER_CHECK_WITHARG(special_atom, "bad");
				status = BAD;
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_text, resp_text);

			*currentPos = pos;

			return true;
		}


		enum Status {
			OK,
			NO,
			BAD
		};


		std::unique_ptr <IMAPParser::resp_text> resp_text;
		Status status;
	};


	//
	// resp_cond_bye ::= "BYE" SPACE resp_text
	//

	DECLARE_COMPONENT(resp_cond_bye)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK_WITHARG(special_atom, "bye");

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_text, resp_text);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::resp_text> resp_text;
	};


	//
	// resp_cond_auth  ::= ("OK" / "PREAUTH") SPACE resp_text
	//                     ;; Authentication condition
	//

	DECLARE_COMPONENT(resp_cond_auth)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "ok")) {
				condition = OK;
			} else {
				VIMAP_PARSER_CHECK_WITHARG(special_atom, "preauth");
				condition = PREAUTH;
			}

			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_text, resp_text);

			*currentPos = pos;

			return true;
		}


		enum Condition {
			OK,
			PREAUTH
		};


		Condition condition;
		std::unique_ptr <IMAPParser::resp_text> resp_text;
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

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (VIMAP_PARSER_TRY_GET(IMAPParser::number, number)) {

				VIMAP_PARSER_CHECK(SPACE);

				if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "exists")) {
					type = EXISTS;
				} else {
					VIMAP_PARSER_CHECK_WITHARG(special_atom, "recent");
					type = RECENT;
				}

			} else {

				// "FLAGS" SPACE mailbox_flag_list
				if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "flags")) {

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox_flag_list, mailbox_flag_list);

					type = FLAGS;

				// "LIST" SPACE mailbox_list
				} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "list")) {

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox_list, mailbox_list);

					type = LIST;

				// "LSUB" SPACE mailbox_list
				} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "lsub")) {

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox_list, mailbox_list);

					type = LSUB;

				// "MAILBOX" SPACE text
				} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "mailbox")) {

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::text, text);

					type = MAILBOX;

				// "SEARCH" [SPACE 1#nz_number]
				} else if (VIMAP_PARSER_TRY_CHECK_WITHARG(special_atom, "search")) {

					if (VIMAP_PARSER_TRY_CHECK(SPACE)) {

						VIMAP_PARSER_GET_PUSHBACK(nz_number, search_nz_number_list);

						if (!parser.isStrict()) {

							// Allow extra SPACEs at end of line
							while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
								VIMAP_PARSER_TRY_GET_PUSHBACK_OR_ELSE(nz_number, search_nz_number_list, { break; });
							}

						} else {

							while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
								VIMAP_PARSER_GET_PUSHBACK(nz_number, search_nz_number_list);
							}
						}
					}

					type = SEARCH;

				// "STATUS" SPACE mailbox SPACE
				// "(" [status_att_list] ")"
				} else {

					VIMAP_PARSER_CHECK_WITHARG(special_atom, "status");
					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_GET(IMAPParser::mailbox, mailbox);

					VIMAP_PARSER_CHECK(SPACE);

					VIMAP_PARSER_CHECK(one_char <'('> );
					VIMAP_PARSER_TRY_GET(IMAPParser::status_att_list, status_att_list);
					VIMAP_PARSER_CHECK(one_char <')'> );

					type = STATUS;
				}
			}

			*currentPos = pos;

			return true;
		}


		enum Type {

			FLAGS,
			LIST,
			LSUB,
			MAILBOX,
			SEARCH,
			STATUS,
			EXISTS,
			RECENT
		};


		Type type;

		std::unique_ptr <IMAPParser::number> number;
		std::unique_ptr <IMAPParser::mailbox_flag_list> mailbox_flag_list;
		std::unique_ptr <IMAPParser::mailbox_list> mailbox_list;
		std::unique_ptr <IMAPParser::mailbox> mailbox;
		std::unique_ptr <IMAPParser::text> text;
		std::vector <std::unique_ptr <nz_number>> search_nz_number_list;
		std::unique_ptr <IMAPParser::status_att_list> status_att_list;
	};


	//
	// response_data  ::= "*" SPACE (resp_cond_state / resp_cond_bye /
	//                    mailbox_data / message_data / capability_data) CRLF
	//

	DECLARE_COMPONENT(response_data)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'*'> );
			VIMAP_PARSER_CHECK(SPACE);

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::resp_cond_state, resp_cond_state)) {
				if (!VIMAP_PARSER_TRY_GET(IMAPParser::resp_cond_bye, resp_cond_bye)) {
					if (!VIMAP_PARSER_TRY_GET(IMAPParser::mailbox_data, mailbox_data)) {
						if (!VIMAP_PARSER_TRY_GET(IMAPParser::message_data, message_data)) {
							VIMAP_PARSER_GET(IMAPParser::capability_data, capability_data);
						}
					}
				}
			}

			if (!parser.isStrict()) {

				// Allow SPACEs at end of line
				while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
					;
				}
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::resp_cond_state> resp_cond_state;
		std::unique_ptr <IMAPParser::resp_cond_bye> resp_cond_bye;
		std::unique_ptr <IMAPParser::mailbox_data> mailbox_data;
		std::unique_ptr <IMAPParser::message_data> message_data;
		std::unique_ptr <IMAPParser::capability_data> capability_data;
	};


	DECLARE_COMPONENT(continue_req_or_response_data)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::continue_req, continue_req)) {
				VIMAP_PARSER_GET(IMAPParser::response_data, response_data);
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::continue_req> continue_req;
		std::unique_ptr <IMAPParser::response_data> response_data;
	};


	//
	// response_fatal ::= "*" SPACE resp_cond_bye CRLF
	//                    ;; Server closes connection immediately
	//

	DECLARE_COMPONENT(response_fatal)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'*'> );
			VIMAP_PARSER_CHECK(SPACE);

			VIMAP_PARSER_GET(IMAPParser::resp_cond_bye, resp_cond_bye);

			if (!parser.isStrict()) {

				// Allow SPACEs at end of line
				while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
					;
				}
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::resp_cond_bye> resp_cond_bye;
	};


	//
	// response_tagged ::= tag SPACE resp_cond_state CRLF
	//

	DECLARE_COMPONENT(response_tagged)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_GET(IMAPParser::xtag, tag);
			VIMAP_PARSER_CHECK(SPACE);
			VIMAP_PARSER_GET(IMAPParser::resp_cond_state, resp_cond_state);

			if (!parser.isStrict()) {

				// Allow SPACEs at end of line
				while (VIMAP_PARSER_TRY_CHECK(SPACE)) {
					;
				}
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::xtag> tag;
		std::unique_ptr <IMAPParser::resp_cond_state> resp_cond_state;
	};


	//
	// response_done ::= response_tagged / response_fatal
	//

	DECLARE_COMPONENT(response_done)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::response_tagged, response_tagged)) {
				VIMAP_PARSER_GET(IMAPParser::response_fatal, response_fatal);
			}

			*currentPos = pos;

			return true;
		}


		std::unique_ptr <IMAPParser::response_tagged> response_tagged;
		std::unique_ptr <IMAPParser::response_fatal> response_fatal;
	};


	//
	// response ::= *(continue_req / response_data) response_done
	//

	DECLARE_COMPONENT(response)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;
			string curLine = line;
			bool partial = false;  // partial response

			IMAPParser::continue_req_or_response_data* resp = NULL;

			while ((resp = parser.get <IMAPParser::continue_req_or_response_data>(curLine, &pos))) {

				continue_req_or_response_data.push_back(
					std::move(
						std::unique_ptr <IMAPParser::continue_req_or_response_data>(resp)
					)
				);

				// Partial response (continue_req)
				if (resp->continue_req) {
					partial = true;
					break;
				}

				// We have read a CRLF, read another line
				curLine = parser.readLine();
				pos = 0;
			}

			if (!partial) {
				response_done.reset(parser.get <IMAPParser::response_done>(curLine, &pos));
				VIMAP_PARSER_FAIL_UNLESS(response_done);
			}

			*currentPos = pos;

			return true;
		}


		bool isBad() const {

			if (!response_done) {  // incomplete (partial) response
				return true;
			}

			if (response_done->response_fatal) {
				return true;
			}

			if (response_done->response_tagged->resp_cond_state->status == IMAPParser::resp_cond_state::BAD) {

				return true;
			}

			return false;
		}

		void setErrorLog(const string& errorLog) {

			m_errorLog = errorLog;
		}

		const string& getErrorLog() const {

			return m_errorLog;
		}


		std::vector <std::unique_ptr <IMAPParser::continue_req_or_response_data>> continue_req_or_response_data;
		std::unique_ptr <IMAPParser::response_done> response_done;

	private:

		string m_errorLog;
	};


	//
	// greeting ::= "*" SPACE (resp_cond_auth / resp_cond_bye) CRLF
	//

	DECLARE_COMPONENT(greeting)

		bool parseImpl(IMAPParser& parser, string& line, size_t* currentPos) {

			size_t pos = *currentPos;

			VIMAP_PARSER_CHECK(one_char <'*'> );
			VIMAP_PARSER_CHECK(SPACE);

			if (!VIMAP_PARSER_TRY_GET(IMAPParser::resp_cond_auth, resp_cond_auth)) {
				VIMAP_PARSER_GET(IMAPParser::resp_cond_bye, resp_cond_bye);
			}

			VIMAP_PARSER_CHECK(CRLF);

			*currentPos = pos;

			return true;
		}

		void setErrorLog(const string& errorLog) {

			m_errorLog = errorLog;
		}

		const string& getErrorLog() const {

			return m_errorLog;
		}


		std::unique_ptr <IMAPParser::resp_cond_auth> resp_cond_auth;
		std::unique_ptr <IMAPParser::resp_cond_bye> resp_cond_bye;

	private:

		string m_errorLog;
	};



	//
	// The main functions used to parse a response
	//

	response* readResponse(const IMAPTag& tag, literalHandler* lh = NULL) {

		while (true) {

			auto it = m_pendingResponses.find(std::string(tag));

			if (it != m_pendingResponses.end()) {
				auto* resp = it->second;
				m_pendingResponses.erase(it);
				return resp;
			}

			size_t pos = 0;
			string line = readLine();

			m_literalHandler = lh;
			response* resp = get <response>(line, &pos);
			m_literalHandler = NULL;

			if (!resp) {
				throw exceptions::invalid_response("", m_errorResponseLine);
			}

			resp->setErrorLog(lastLine());

			// If there is a continue_req, return the response immediately
			for (auto &respData : resp->continue_req_or_response_data) {
				if (respData->continue_req) {
					return resp;
				}
			}

			// Else, return response if the tag is the one we expect
			if (resp->response_done && resp->response_done->response_tagged &&
			    resp->response_done->response_tagged->tag) {

				if (tag == resp->response_done->response_tagged->tag->tagString) {
					return resp;
				} else {
					// Not our response tag, cache it for later
					m_pendingResponses[resp->response_done->response_tagged->tag->tagString] = resp;
				}
			} else {
				// Untagged response
				return resp;
			}
		}
	}


	greeting* readGreeting() {

		size_t pos = 0;
		string line = readLine();

		greeting* greet = get <greeting>(line, &pos);

		if (!greet) {
			throw exceptions::invalid_response("", m_errorResponseLine);
		}

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
	TYPE* get(string& line, size_t* currentPos) {

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
	TYPE* getWithArgs(string& line, size_t* currentPos, ARG1_TYPE arg1, ARG2_TYPE arg2) {

		component* resp = new TYPE(arg1, arg2);
		return internalGet <TYPE>(resp, line, currentPos);
	}

private:

	template <class TYPE>
	TYPE* internalGet(component* resp, string& line, size_t* currentPos) {

		const size_t oldPos = *currentPos;

		if (!resp->parse(*this, line, currentPos)) {

			*currentPos = oldPos;

			delete resp;

			return NULL;
		}

		return static_cast <TYPE*>(resp);
	}

	const string lastLine() const {

		// Remove blanks and new lines at the end of the line.
		string line(m_lastLine);

		string::const_iterator it = line.end();
		int count = 0;

		while (it != line.begin()) {

			const unsigned char c = *(it - 1);

			if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r')) {
				break;
			}

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
	bool check(string& line, size_t* currentPos) {

		const size_t oldPos = *currentPos;

		TYPE term;

		if (!term.parse(*this, line, currentPos)) {
			*currentPos = oldPos;
			return false;
		} else {
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
	bool checkWithArg(string& line, size_t* currentPos, const ARG_TYPE arg) {

		const size_t oldPos = *currentPos;

		TYPE term(arg);

		if (!term.parse(*this, line, currentPos)) {
			*currentPos = oldPos;
			return false;
		} else {
			return true;
		}
	}


private:

	weak_ptr <socket> m_socket;
	shared_ptr <tracer> m_tracer;

	utility::progressListener* m_progress;

	bool m_strict;

	literalHandler* m_literalHandler;

	weak_ptr <timeoutHandler> m_timeoutHandler;


	string m_buffer;

	string m_lastLine;
	string m_errorResponseLine;

	std::map <std::string, response*> m_pendingResponses;

public:

	/** Read a line from the input buffer. The function blocks until a
	  * complete line is read from the buffer. Position in input buffer
	  * will be updated.
	  *
	  * @return next line
	  */
	const string readLine() {

		size_t pos;

		while ((pos = m_buffer.find('\n')) == string::npos) {
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

		if (m_tracer) {
			string::size_type len = line.length();
			while (len != 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) --len;
			m_tracer->traceReceive(line.substr(0, len));
		}

		return (line);
	}

	/** Fill in the input buffer with data available from the socket stream.
	  * The function blocks until some data is available.
	  */
	void read() {

		string receiveBuffer;

		shared_ptr <timeoutHandler> toh = m_timeoutHandler.lock();
		shared_ptr <socket> sok = m_socket.lock();

		if (!sok)
			throw exceptions::illegal_state("Store disconnected");

		if (toh) {
			toh->resetTimeOut();
		}

		while (receiveBuffer.empty()) {

			// Check whether the time-out delay is elapsed
			if (toh && toh->isTimeOut()) {
				if (!toh->handleTimeOut()) {
					throw exceptions::operation_timed_out();
				}
			}

			// We have received data: reset the time-out counter
			sok->receive(receiveBuffer);

			if (receiveBuffer.empty()) {   // buffer is empty

				if (sok->getStatus() & socket::STATUS_WANT_WRITE) {
					sok->waitForWrite();
				} else {
					sok->waitForRead();
				}

				continue;
			}

			// We have received data ...
			if (toh) {
				toh->resetTimeOut();
			}
		}

		m_buffer += receiveBuffer;
	}


	void readLiteral(literalHandler::target& buffer, size_t count) {

		size_t len = 0;
		string receiveBuffer;

		shared_ptr <timeoutHandler> toh = m_timeoutHandler.lock();
		shared_ptr <socket> sok = m_socket.lock();

		if (!sok)
			throw exceptions::illegal_state("Store disconnected");

		if (m_progress) {
			m_progress->start(count);
		}

		if (toh) {
			toh->resetTimeOut();
		}

		if (!m_buffer.empty()) {

			if (m_buffer.length() > count) {

				buffer.putData(string(m_buffer.begin(), m_buffer.begin() + count));
				m_buffer.erase(m_buffer.begin(), m_buffer.begin() + count);
				len = count;

			} else {

				len += m_buffer.length();
				buffer.putData(m_buffer);
				m_buffer.clear();
			}
		}

		while (len < count) {

			// Check whether the time-out delay is elapsed
			if (toh && toh->isTimeOut()) {

				if (!toh->handleTimeOut()) {
					throw exceptions::operation_timed_out();
				}

				toh->resetTimeOut();
			}

			// Receive data from the socket
			sok->receive(receiveBuffer);

			if (receiveBuffer.empty()) {  // buffer is empty

				if (sok->getStatus() & socket::STATUS_WANT_WRITE) {
					sok->waitForWrite();
				} else {
					sok->waitForRead();
				}

				continue;
			}

			// We have received data: reset the time-out counter
			if (toh) {
				toh->resetTimeOut();
			}

			if (len + receiveBuffer.length() > count) {

				const size_t remaining = count - len;

				// Get the needed amount of data
				buffer.putData(string(receiveBuffer.begin(), receiveBuffer.begin() + remaining));

				// Put the remaining data into the internal response buffer
				receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + remaining);
				m_buffer += receiveBuffer;

				len = count;

			} else {

				buffer.putData(receiveBuffer);
				len += receiveBuffer.length();
			}

			// Notify progress
			if (m_progress) {
				m_progress->progress(len, count);
			}
		}

		if (m_tracer) {
			m_tracer->traceReceiveBytes(count);
		}

		if (m_progress) {
			m_progress->stop(count);
		}
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
#undef VIMAP_PARSER_GET
#undef VIMAP_PARSER_TRY_GET
#undef VIMAP_PARSER_TRY_GET
#undef VIMAP_PARSER_GET_PUSHBACK
#undef VIMAP_PARSER_CHECK_WITHARG
#undef VIMAP_PARSER_TRY_CHECK_WITHARG


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPPARSER_HPP_INCLUDED
