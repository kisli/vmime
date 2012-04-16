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

#include "vmime/mailbox.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


mailbox::mailbox()
{
}


mailbox::mailbox(const mailbox& mbox)
	: address(), m_name(mbox.m_name), m_email(mbox.m_email)
{
}


mailbox::mailbox(const string& email)
	: m_email(email)
{
}


mailbox::mailbox(const text& name, const string& email)
	: m_name(name), m_email(email)
{
}


/*

 RFC #2822:
 3.4.  ADDRESS SPECIFICATION

mailbox         =       name-addr / addr-spec

name-addr       =       [display-name] angle-addr

angle-addr      =       [CFWS] "<" addr-spec ">" [CFWS] / obs-angle-addr

*/

void mailbox::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pstart;

	// Ignore blank spaces at the beginning
	while (p < pend && parserHelpers::isSpace(*p)) ++p;

	// Current state for parsing machine
	enum States
	{
		State_None,
		State_Name,
		State_Address
	};

	States state = State_Name;   // let's start with name, we will see later (*)

	// Temporary buffers for extracted name and address
	string name;
	string address;
	bool hadBrackets = false;

	while (p < pend)
	{
		if (state == State_Name)
		{
			if (*p == '<')
			{
				state = State_Address;
				continue;
			}

			if (*p == '"') // Quoted string
			{
				++p;

				bool escaped = false;

				while (p < pend)
				{
					if (escaped)
					{
						name += *p;
						escaped = false;
					}
					else if (*p == '\\')
					{
						escaped = true;
					}
					else
					{
						if (*p == '"')
						{
							++p;
							break;
						}
						else
						{
							name += *p;
						}
					}

					++p;
				}
			}
			else
			{
				bool escaped = false;
				int comment = 0;

				while (p < pend)
				{
					if (escaped)
					{
						if (!comment) name += *p;
						escaped = false;
					}
					else if (comment)
					{
						if (*p == '\\')
							escaped = true;
						else if (*p == '(')
							++comment;
						else if (*p == ')')
							--comment;
					}
					else if (*p == '\\')
					{
						escaped = true;
					}
					else if (*p == '(')
					{
						++comment;
					}
					else if (*p == '<')
					{
						// Erase any space between display name and <address>
						string::iterator q = name.end();

						while (q != name.begin() && parserHelpers::isSpace(*(q - 1)))
							--q;

						name.erase(q, name.end());

						break;
					}
					else if (/* parserHelpers::isSpace(*p) || */ *p == '@')
					{
						break;
					}
					else
					{
						name += *p;
					}

					++p;
				}
			}

			if (p < pend && *p == '@')
			{
				// (*) Actually, we were parsing the local-part of an address
				// and not a display name...
				address = name;
				name.clear();

				bool escaped = false;
				int comment = 0;

				while (p < pend)
				{
					if (escaped)
					{
						if (!comment) address += *p;
						escaped = false;
					}
					else if (comment)
					{
						if (*p == '\\')
							escaped = true;
						else if (*p == '(')
							++comment;
						else if (*p == ')')
							--comment;
					}
					else if (*p == '\\')
					{
						escaped = true;
					}
					else if (*p == '(')
					{
						++comment;
					}
					else if (parserHelpers::isSpace(*p))
					{
						break;
					}
					else
					{
						address += *p;
					}

					++p;
				}

				break;
			}
			else
			{
				while (p < pend && parserHelpers::isSpace(*p)) ++p;
				state = State_None;
			}
		}
		else if (state == State_Address)
		{
			// Skip '<' character
			if (*p == '<')
				++p;

			bool escaped = false;
			int comment = 0;

			while (p < pend)
			{
				if (escaped)
				{
					if (!comment) address += *p;
					escaped = false;
				}
				else if (comment)
				{
					if (*p == '\\')
						escaped = true;
					else if (*p == '(')
						++comment;
					else if (*p == ')')
						--comment;
				}
				else if (*p == '(')
				{
					++comment;
				}
				else if (*p == '\\')
				{
					escaped = true;
				}
				else if (*p == '<')
				{
					// If we found a '<' here, it means that the address
					// starts _only_ here...and the stuff we have parsed
					// before belongs actually to the display name!
					name += address;
					address.clear();
				}
				else if (*p == '>')
				{
					hadBrackets = true;
					break;
				}
				else if (!parserHelpers::isSpace(*p))
				{
					address += *p;
				}

				++p;
			}

			break;
		}
		else
		{
			while (p < pend && parserHelpers::isSpace(*p)) ++p;

			if (p < pend)
			{
				//if (*p == '<')
					state = State_Address;
			}
		}
	}

	// Swap name and address when no address was found
	// (email address is mandatory, whereas name is optional).
	if (address.empty() && !name.empty() && !hadBrackets)
	{
		m_email.clear();
		m_email.reserve(name.size());
		m_name.removeAllWords();

		for (string::size_type i = 0 ; i < name.size() ; ++i)
		{
			if (!parserHelpers::isSpace(name[i]))
				m_email += name[i];
		}
	}
	else
	{
		text::decodeAndUnfold(name, &m_name);
		m_email.clear();
		m_email.reserve(address.size());

		for (string::size_type i = 0 ; i < address.size() ; ++i)
		{
			if (!parserHelpers::isSpace(address[i]))
				m_email += address[i];
		}
	}

	setParsedBounds(position, position + (p - pstart));

	if (newPosition)
		*newPosition = position + (p - pstart);
}


void mailbox::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	if (m_name.isEmpty())
	{
		bool newLine = false;

		// No display name is specified, only email address.
		if (curLinePos /* + 2 */ + m_email.length() > maxLineLength)
		{
			os << NEW_LINE_SEQUENCE;
			newLine = true;
		}

		//os << "<" << m_email << ">";
		os << m_email;

		if (newLinePos)
		{
			*newLinePos = curLinePos + m_email.length() /* + 2 */;
			if (newLine) *newLinePos += 1;
		}
	}
	else
	{
		// We have to encode the name:
		//   - if it contains characters in a charset different from "US-ASCII",
		//   - and/or if it contains one or more of these special chars:
		//        CR  LF  TAB  "  ;  ,  <  >  (  )  @  /  ?  .  =  :

		// Check whether there are words that are not "US-ASCII"
		// and/or contain the special chars.
		bool forceEncode = false;

		for (int w = 0 ; !forceEncode && w != m_name.getWordCount() ; ++w)
		{
			if (m_name.getWordAt(w)->getCharset() == charset(charsets::US_ASCII))
			{
				const string& buffer = m_name.getWordAt(w)->getBuffer();

				for (string::const_iterator c = buffer.begin() ;
				     !forceEncode && c != buffer.end() ; ++c)
				{
					switch (*c)
					{
					case '\r':
					case '\n':
					case '\t':
					case ';':
					case ',':
					case '<': case '>':
					case '(': case ')':
					case '@':
					case '/':
					case '?':
					case '.':
					case '=':
					case ':':
					case '"':

						forceEncode = true;
						break;
					}
				}
			}
			else
			{
				forceEncode = true;
			}
		}

		string::size_type pos = curLinePos;
		bool newLine = true;

		m_name.encodeAndFold(os, maxLineLength, pos, &pos,
			text::QUOTE_IF_POSSIBLE | (forceEncode ? text::FORCE_ENCODING : 0));

		if (pos + m_email.length() + 3 > maxLineLength)
		{
			os << NEW_LINE_SEQUENCE;
			newLine = true;
		}

		os << " <" << m_email << ">";

		if (newLinePos)
		{
			*newLinePos = pos + m_email.length() + 3;
			if (newLine) *newLinePos += NEW_LINE_SEQUENCE.length();
		}
	}
}


bool mailbox::operator==(const class mailbox& mailbox) const
{
	return (m_name == mailbox.m_name && m_email == mailbox.m_email);
}


bool mailbox::operator!=(const class mailbox& mailbox) const
{
	return !(*this == mailbox);
}


void mailbox::copyFrom(const component& other)
{
	const mailbox& source = dynamic_cast <const mailbox&>(other);

	m_name = source.m_name;
	m_email = source.m_email;
}


mailbox& mailbox::operator=(const mailbox& other)
{
	copyFrom(other);
	return (*this);
}


ref <component>mailbox::clone() const
{
	return vmime::create <mailbox>(*this);
}


bool mailbox::isEmpty() const
{
	return (m_email.empty());
}


void mailbox::clear()
{
	m_name.removeAllWords();
	m_email.clear();
}


bool mailbox::isGroup() const
{
	return (false);
}


const text& mailbox::getName() const
{
	return (m_name);
}


void mailbox::setName(const text& name)
{
	m_name = name;
}


const string& mailbox::getEmail() const
{
	return (m_email);
}


void mailbox::setEmail(const string& email)
{
	m_email = email;
}


const std::vector <ref <component> > mailbox::getChildComponents()
{
	return std::vector <ref <component> >();
}


} // vmime
