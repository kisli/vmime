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

#include "vmime/parameter.hpp"
#include "vmime/parserHelpers.hpp"

#include "vmime/text.hpp"
#include "vmime/encoding.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"


namespace vmime
{


parameter::parameter(const string& name)
	: m_name(name), m_value(make_shared <word>())
{
}


parameter::parameter(const string& name, const word& value)
	: m_name(name), m_value(make_shared <word>(value))
{
}


parameter::parameter(const string& name, const string& value)
	: m_name(name), m_value(make_shared <word>(value))
{
}


parameter::parameter(const parameter&)
	: component()
{
}


shared_ptr <component> parameter::clone() const
{
	shared_ptr <parameter> p = make_shared <parameter>(m_name);
	p->copyFrom(*this);

	return (p);
}


void parameter::copyFrom(const component& other)
{
	const parameter& param = dynamic_cast <const parameter&>(other);

	m_name = param.m_name;
	m_value->copyFrom(*param.m_value);
}


parameter& parameter::operator=(const parameter& other)
{
	copyFrom(other);
	return (*this);
}


const string& parameter::getName() const
{
	return m_name;
}


const word& parameter::getValue() const
{
	return *m_value;
}


void parameter::setValue(const component& value)
{
	std::ostringstream oss;
	utility::outputStreamAdapter vos(oss);

	value.generate(vos);

	setValue(word(oss.str(), vmime::charsets::US_ASCII));
}


void parameter::setValue(const word& value)
{
	*m_value = value;
}


void parameter::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	m_value->setBuffer(string(buffer.begin() + position, buffer.begin() + end));

	if (ctx.getInternationalizedEmailSupport())
		m_value->setCharset(charset(charsets::UTF_8));
	else
		m_value->setCharset(charset(charsets::US_ASCII));

	if (newPosition)
		*newPosition = end;
}


void parameter::parse(const parsingContext& ctx, const std::vector <valueChunk>& chunks)
{
	bool foundCharsetChunk = false;

	charset ch(charsets::US_ASCII);
	string lang;

	std::ostringstream value;
	value.imbue(std::locale::classic());

	for (std::vector <valueChunk>::size_type i = 0 ; i < chunks.size() ; ++i)
	{
		const valueChunk& chunk = chunks[i];

		// Decode following data
		if (chunk.encoded)
		{
			const size_t len = chunk.data.length();
			size_t pos = 0;

			// If this is the first encoded chunk, extract charset
			// and language information
			if (!foundCharsetChunk)
			{
				// Eg. "us-ascii'en'This%20is%20even%20more%20"
				size_t q = chunk.data.find_first_of('\'');

				if (q != string::npos)
				{
					const string chs = chunk.data.substr(0, q);

					if (!chs.empty())
						ch = charset(chs);

					++q;
					pos = q;
				}

				q = chunk.data.find_first_of('\'', pos);

				if (q != string::npos)
				{
					// Extract language
					lang = chunk.data.substr(pos, q - pos);

					++q;
					pos = q;
				}

				foundCharsetChunk = true;
			}

			for (size_t i = pos ; i < len ; ++i)
			{
				const char c = chunk.data[i];

				if (c == '%' && i + 2 < len)
				{
					unsigned int v = 0;

					// First char
					switch (chunk.data[i + 1])
					{
					case 'a': case 'A': v += 10; break;
					case 'b': case 'B': v += 11; break;
					case 'c': case 'C': v += 12; break;
					case 'd': case 'D': v += 13; break;
					case 'e': case 'E': v += 14; break;
					case 'f': case 'F': v += 15; break;
					default: // assume 0-9

						v += (chunk.data[i + 1] - '0');
						break;
					}

					v *= 16;

					// Second char
					switch (chunk.data[i + 2])
					{
					case 'a': case 'A': v += 10; break;
					case 'b': case 'B': v += 11; break;
					case 'c': case 'C': v += 12; break;
					case 'd': case 'D': v += 13; break;
					case 'e': case 'E': v += 14; break;
					case 'f': case 'F': v += 15; break;
					default: // assume 0-9

						v += (chunk.data[i + 2] - '0');
						break;
					}

					value << static_cast <char>(v);

					i += 2; // skip next 2 chars
				}
				else
				{
					value << c;
				}
			}
		}
		// Simply copy data, as it is not encoded
		else
		{
			// This syntax is non-standard (expressly prohibited
			// by RFC-2047), but is used by Mozilla:
			//
    		// Content-Type: image/png;
			//    name="=?us-ascii?Q?Logo_VMime=2Epng?="

			// Using 'vmime::text' to parse the data is safe even
			// if the data is not encoded, because it can recover
			// from parsing errors.
			vmime::text t;
			t.parse(ctx, chunk.data);

			if (t.getWordCount() != 0)
			{
				value << t.getWholeBuffer();

				if (!foundCharsetChunk)
				{
					// This is still wrong. Each word can have it's own charset, and can
					// be mixed (eg. iso-8859-1 and iso-2022-jp), but very unlikely. Real
					// fix is to have parameters store a vmime::text instead of a
					// vmime::word in m_value. But that changes the interface.
					for (size_t i = 0 ; i < t.getWordCount() ; ++i)
					{
						if (t.getWordAt(i)->getCharset() != ch && ch == charsets::US_ASCII)
						{
							ch = t.getWordAt(i)->getCharset();
							break;
						}
					}
				}
			}
		}
	}

	m_value->setBuffer(value.str());
	m_value->setCharset(ch);
	m_value->setLanguage(lang);
}


void parameter::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t curLinePos, size_t* newLinePos) const
{
	const string& name = m_name;
	const string& value = m_value->getBuffer();

	// For compatibility with implementations that do not understand RFC-2231,
	// we may also generate a normal "7bit/us-ascii" parameter
	generationContext::EncodedParameterValueModes
		genMode = ctx.getEncodedParameterValueMode();

#if VMIME_ALWAYS_GENERATE_7BIT_PARAMETER
	genMode = generationContext::PARAMETER_VALUE_RFC2231_AND_RFC2047;
#endif

	// [By Eugene A. Shatokhin]
	// Note that if both the normal "7bit/us-ascii" value and the extended
	// value are present, the latter can be ignored by mail processing systems.
	// This may lead to annoying problems, for example, with strange names of
	// attachments with all but 7-bit ascii characters removed, etc. To avoid
	// this, I would suggest not to create "7bit/us-ascii" value if the extended
	// value is to be generated.

	// A stream for a temporary storage
	string sevenBitBuffer;
	utility::outputStreamStringAdapter sevenBitStream(sevenBitBuffer);

	size_t pos = curLinePos;

	if (pos + name.length() + 10 + value.length() > ctx.getMaxLineLength())
	{
		sevenBitStream << NEW_LINE_SEQUENCE;
		pos = NEW_LINE_SEQUENCE_LENGTH;
	}

	bool needQuoting = false;
	bool needQuotedPrintable = false;
	size_t valueLength = 0;

	// Use worst-case length name.length()+2 for 'name=' part of line
	for (size_t i = 0 ; (i < value.length()) && (pos + name.length() + 2 + valueLength < ctx.getMaxLineLength() - 4) ; ++i, ++valueLength)
	{
		switch (value[i])
		{
		// Characters that need to be quoted _and_ escaped
		case '"':
		case '\\':
		// Other characters that need quoting
		case ' ':
		case '\t':
		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '/':
		case '[':
		case ']':
		case '?':
		case '=':

			needQuoting = true;
			break;

		default:

			if (!parserHelpers::isAscii(value[i]))
			{
				needQuotedPrintable = true;
				needQuoting = true;
			}

			break;
		}
	}

	const bool cutValue = (valueLength != value.length());  // has the value been cut?

	if (needQuoting)
	{
		sevenBitStream << name << "=\"";
		pos += name.length() + 2;
	}
	else
	{
		sevenBitStream << name << "=";
		pos += name.length() + 1;
	}

	// Check whether there is a recommended encoding for this charset.
	// If so, the whole buffer will be encoded. Else, the number of
	// 7-bit (ASCII) bytes in the input will be used to determine if
	// we need to encode the whole buffer.
	encoding recommendedEnc;
	const bool alwaysEncode = m_value->getCharset().getRecommendedEncoding(recommendedEnc);
	bool extended = alwaysEncode;

	if ((needQuotedPrintable || cutValue || !m_value->getLanguage().empty()) &&
	    genMode != generationContext::PARAMETER_VALUE_NO_ENCODING)
	{
		// Send the name in quoted-printable, so outlook express et.al.
		// will understand the real filename
		size_t oldLen = sevenBitBuffer.length();
		m_value->generate(sevenBitStream);
		pos += sevenBitBuffer.length() - oldLen;
		extended = true;		// also send with RFC-2231 encoding
	}
	else
	{
		// Do not chop off this value, but just add the complete name as one header line.
		for (size_t i = 0, n = value.length(), curValueLength = 0 ;
		     i < n && curValueLength < valueLength ; ++i)
		{
			const char_t c = value[i];

			if (/* needQuoting && */ (c == '"' || c == '\\'))  // 'needQuoting' is implicit
			{
				sevenBitStream << '\\' << value[i];  // escape 'x' with '\x'
				pos += 2;
			}
			else if (parserHelpers::isAscii(c))
			{
				sevenBitStream << value[i];
				++pos;
				++curValueLength;
			}
			else
			{
				extended = true;
			}
		}

	} // !needQuotedPrintable

	if (needQuoting)
	{
		sevenBitStream << '"';
		++pos;
	}

	if (genMode == generationContext::PARAMETER_VALUE_RFC2047_ONLY ||
	    genMode == generationContext::PARAMETER_VALUE_RFC2231_AND_RFC2047)
	{
		os << sevenBitBuffer;
	}

	// Also generate an extended parameter if the value contains 8-bit characters
	// or is too long for a single line
	if ((extended || cutValue) &&
		genMode != generationContext::PARAMETER_VALUE_NO_ENCODING &&
	    genMode != generationContext::PARAMETER_VALUE_RFC2047_ONLY)
	{

		if (genMode == generationContext::PARAMETER_VALUE_RFC2231_AND_RFC2047)
		{
			os << ';';
			++pos;
		}
		else
		{
			// The data output to 'sevenBitBuffer' will be discarded in this case
			pos = curLinePos;
		}

		/* RFC-2231
		 * ========
		 *
		 * Content-Type: message/external-body; access-type=URL;
		 *    URL*0="ftp://";
		 *    URL*1="cs.utk.edu/pub/moore/bulk-mailer/bulk-mailer.tar"
		 *
		 * Content-Type: application/x-stuff;
		 *    title*=us-ascii'en-us'This%20is%20%2A%2A%2Afun%2A%2A%2A
		 *
		 * Content-Type: application/x-stuff;
		 *    title*0*=us-ascii'en'This%20is%20even%20more%20
		 *    title*1*=%2A%2A%2Afun%2A%2A%2A%20
		 *    title*2="isn't it!"
		 */

		// Check whether there is enough space for the first section:
		// parameter name, section identifier, charset and separators
		// + at least 5 characters for the value
		const size_t firstSectionLength =
			  name.length() + 4 /* *0*= */ + 2 /* '' */
			+ m_value->getCharset().getName().length();

		if (pos + firstSectionLength + 5 >= ctx.getMaxLineLength())
		{
			os << NEW_LINE_SEQUENCE;
			pos = NEW_LINE_SEQUENCE_LENGTH;
		}

		// Split text into multiple sections that fit on one line
		int sectionCount = 0;
		std::vector <string> sectionText;

		string currentSection;
		size_t currentSectionLength = firstSectionLength;

		for (size_t i = 0 ; i < value.length() ; ++i)
		{
			// Check whether we should start a new line (taking into
			// account the next character will be encoded = worst case)
			if (currentSectionLength + 3 >= ctx.getMaxLineLength())
			{
				sectionText.push_back(currentSection);
				sectionCount++;

				currentSection.clear();
				currentSectionLength = NEW_LINE_SEQUENCE_LENGTH
					+ name.length() + 6;
			}

			// Output next character
			const char_t c = value[i];
			bool encode = false;

			switch (c)
			{
			// special characters
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '%':
			case '"':
			case ';':
			case ',':
			case '(':
			case ')':
			case '<':
			case '>':
			case '@':
			case ':':
			case '/':
			case '[':
			case ']':
			case '?':
			case '=':

				encode = true;
				break;

			default:

				encode = (!parserHelpers::isPrint(c) ||
				          !parserHelpers::isAscii(c) ||
				          alwaysEncode);

				break;
			}

			if (encode)  // need encoding
			{
				const int h1 = static_cast <unsigned char>(c) / 16;
				const int h2 = static_cast <unsigned char>(c) % 16;

				currentSection += '%';
				currentSection += "0123456789ABCDEF"[h1];
				currentSection += "0123456789ABCDEF"[h2];

				pos += 3;
				currentSectionLength += 3;
			}
			else
			{
				currentSection += value[i];

				++pos;
				++currentSectionLength;
			}
		}

		if (!currentSection.empty())
		{
			sectionText.push_back(currentSection);
			sectionCount++;
		}

		// Output sections
		for (int sectionNumber = 0 ; sectionNumber < sectionCount ; ++sectionNumber)
		{
			os << name;

			if (sectionCount != 1) // no section specifier when only a single one
			{
				os << '*';
				os << sectionNumber;
			}

			os << "*=";

			if (sectionNumber == 0)
			{
				os << m_value->getCharset().getName();
				os << '\'' << /* No language */ '\'';
			}

			os << sectionText[sectionNumber];

			if (sectionNumber + 1 < sectionCount)
			{
				os << ';';
				os << NEW_LINE_SEQUENCE;
				pos = NEW_LINE_SEQUENCE_LENGTH;
			}
		}
	}
	else if (!(genMode == generationContext::PARAMETER_VALUE_RFC2047_ONLY ||
	           genMode == generationContext::PARAMETER_VALUE_RFC2231_AND_RFC2047))
	{
		// The value does not contain 8-bit characters and
		// is short enough for a single line.
		// "7bit/us-ascii" will suffice in this case.

		// Output what has been stored in temporary buffer so far
		os << sevenBitBuffer;
	}

	if (newLinePos)
		*newLinePos = pos;
}


const std::vector <shared_ptr <component> > parameter::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	list.push_back(m_value);

	return list;
}


} // vmime

