//
// VMime library (http://vmime.sourceforge.net)
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

#include "vmime/bodyPart.hpp"
#include "vmime/body.hpp"

#include "vmime/options.hpp"

#include "vmime/contentTypeField.hpp"

#include "vmime/utility/random.hpp"

#include "vmime/parserHelpers.hpp"

#include "vmime/emptyContentHandler.hpp"
#include "vmime/stringContentHandler.hpp"


namespace vmime
{


body::body()
	: m_contents(new emptyContentHandler()), m_part(NULL), m_header(NULL)
{
}


body::body(bodyPart* parentPart)
	: m_contents(new emptyContentHandler()),
	  m_part(parentPart), m_header(parentPart != NULL ? parentPart->getHeader() : NULL)
{
}


body::~body()
{
	delete (m_contents);

	removeAllParts();
}


void body::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	removeAllParts();

	// Check whether the body is a MIME-multipart
	bool isMultipart = false;
	string boundary;

	try
	{
		const contentTypeField& ctf = dynamic_cast <contentTypeField&>
			(*m_header->findField(fields::CONTENT_TYPE));

		if (ctf.getValue().getType() == mediaTypes::MULTIPART)
		{
			isMultipart = true;

			try
			{
				boundary = ctf.getBoundary();
			}
			catch (exceptions::no_such_parameter&)
			{
				// No "boundary" parameter specified: we can try to
				// guess it by scanning the body contents...
				string::size_type pos = buffer.find("\n--", position);

				if ((pos != string::npos) && (pos < end))
				{
					pos += 3;

					const string::size_type start = pos;

					char_t c = buffer[pos];
					string::size_type length = 0;

					// We have to stop after a reasonnably long boundary length (100)
					// not to take the whole body contents for a boundary...
					while (pos < end && length < 100 && !(c == '\r' || c == '\n'))
					{
						++length;
						c = buffer[pos++];
					}

					if (pos < end && length < 100)
					{
						// RFC #1521, Page 31:
						// "...the boundary parameter, which consists of 1 to 70
						//  characters from a set of characters known to be very
						//  robust through email gateways, and NOT ending with
						//  white space..."
						while (pos != start && parserHelpers::isSpace(buffer[pos - 1]))
							--pos;

						boundary = string(buffer.begin() + start,
						                  buffer.begin() + pos);
					}
				}
			}
		}
 	}
	catch (exceptions::no_such_field&)
	{
		// No "Content-Type" field...
	}

	// This is a multi-part body
	if (isMultipart && !boundary.empty())
	{
		const string boundarySep("--" + boundary);

		string::size_type partStart = position;
		string::size_type pos = buffer.find(boundarySep, position);

		bool lastPart = false;

		if (pos != string::npos && pos < end)
		{
			m_prologText = string(buffer.begin() + position, buffer.begin() + pos);
		}

		for (int index = 0 ; !lastPart && (pos != string::npos) && (pos < end) ; ++index)
		{
			string::size_type partEnd = pos;

			// Get rid of the [CR]LF just before the boundary string
			if (pos - 1 >= position && buffer[pos - 1] == '\n') --partEnd;
			if (pos - 2 >= position && buffer[pos - 2] == '\r') --partEnd;

			// Check whether it is the last part (boundary terminated by "--")
			pos += boundarySep.length();

			if (pos + 1 < end && buffer[pos] == '-' && buffer[pos + 1] == '-')
			{
				lastPart = true;
				pos += 2;
			}

			// RFC #1521, Page 31:
			// "...(If a boundary appears to end with white space, the
			//  white space must be presumed to have been added by a
			//  gateway, and must be deleted.)..."
			while (pos < end && (buffer[pos] == ' ' || buffer[pos] == '\t'))
				++pos;

			// End of boundary line
			if (pos + 1 < end && buffer[pos] == '\r' && buffer[pos + 1] =='\n')
			{
				pos += 2;
			}
			else if (pos < end && buffer[pos] == '\n')
			{
				++pos;
			}

			if (index > 0)
			{
				bodyPart* part = new bodyPart;

				try
				{
					part->parse(buffer, partStart, partEnd, NULL);
				}
				catch (std::exception&)
				{
					delete (part);
					throw;
				}

				part->m_parent = m_part;

				m_parts.push_back(part);
			}

			partStart = pos;
			pos = buffer.find(boundarySep, partStart);
		}

		setContentsImpl(emptyContentHandler());

		if (partStart < end)
			m_epilogText = string(buffer.begin() + partStart, buffer.begin() + end);
	}
	// Treat the contents as 'simple' data
	else
	{
		// Extract the (encoded) contents
		setContentsImpl(stringContentHandler(buffer, position, end, getEncoding()));
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void body::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type /* curLinePos */, string::size_type* newLinePos) const
{
	// MIME-Multipart
	if (getPartCount() != 0)
	{
		string boundary;

		if (m_header == NULL)
		{
			boundary = generateRandomBoundaryString();
		}
		else
		{
			try
			{
				contentTypeField& ctf = dynamic_cast<contentTypeField&>
					(*m_header->findField(fields::CONTENT_TYPE));

				boundary = ctf.getBoundary();
			}
			catch (exceptions::no_such_field&)
			{
				// Warning: no content-type and no boundary string specified!
				boundary = generateRandomBoundaryString();
			}
			catch (exceptions::no_such_parameter&)
			{
				// Warning: no boundary string specified!
				boundary = generateRandomBoundaryString();
			}
		}

		const string& prologText =
			m_prologText.empty()
				? (isRootPart()
					? options::getInstance()->multipart.getPrologText()
					: NULL_STRING
				  )
				: m_prologText;

		const string& epilogText =
			m_epilogText.empty()
				? (isRootPart()
					? options::getInstance()->multipart.getEpilogText()
					: NULL_STRING
				  )
				: m_epilogText;

		if (!prologText.empty())
		{
			text prolog(word(prologText, getCharset()));

			prolog.encodeAndFold(os, maxLineLength, 0,
				NULL, text::FORCE_NO_ENCODING | text::NO_NEW_LINE_SEQUENCE);

			os << CRLF;
		}

		os << "--" << boundary;

		for (int p = 0 ; p < getPartCount() ; ++p)
		{
			os << CRLF;

			getPartAt(p)->generate(os, maxLineLength, 0);

			os << CRLF << "--" << boundary;
		}

		os << "--" << CRLF;

		if (!epilogText.empty())
		{
			text epilog(word(epilogText, getCharset()));

			epilog.encodeAndFold(os, maxLineLength, 0,
				NULL, text::FORCE_NO_ENCODING | text::NO_NEW_LINE_SEQUENCE);

			os << CRLF;
		}

		if (newLinePos)
			*newLinePos = 0;
	}
	// Simple body
	else
	{
		// Generate the contents
		m_contents->generate(os, getEncoding(), maxLineLength);
	}
}


/*
   RFC #1521, Page 32:
   7.2.1. Multipart: The common syntax

   "...Encapsulation boundaries must not appear within the
   encapsulations, and must be no longer than 70 characters..."


   boundary := 0*69<bchars> bcharsnospace

   bchars := bcharsnospace / " "

   bcharsnospace :=    DIGIT / ALPHA / "'" / "(" / ")" / "+" /"_"
                 / "," / "-" / "." / "/" / ":" / "=" / "?"
*/

const string body::generateRandomBoundaryString()
{
	// 64 characters that can be _safely_ used in a boundary string
	static const char bchars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-+";

	/*
		RFC #1521, Page 19:

		Since the hyphen character ("-") is represented as itself in the
		Quoted-Printable encoding, care must be taken, when encapsulating a
		quoted-printable encoded body in a multipart entity, to ensure that
		the encapsulation boundary does not appear anywhere in the encoded
		body.  (A good strategy is to choose a boundary that includes a
		character sequence such as "=_" which can never appear in a quoted-
		printable body.  See the definition of multipart messages later in
		this document.)
	*/

	string::value_type boundary[2 + 48 + 1] = { 0 };

	boundary[0] = '=';
	boundary[1] = '_';

	// Generate a string of random characters
	unsigned int r = utility::random::getTime();
	unsigned int m = sizeof(unsigned int);

	for (size_t i = 2 ; i < (sizeof(boundary) / sizeof(boundary[0]) - 1) ; ++i)
	{
			boundary[i] = bchars[r & 63];
			r >>= 6;

			if (--m == 0)
			{
				r = utility::random::getNext();
				m = sizeof(unsigned int);
			}
	}

	return (string(boundary));
}


const bool body::isValidBoundary(const string& boundary)
{
	static const string validChars("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'()+_,-./:=?");

	const string::const_iterator end = boundary.end();
	bool valid = false;

	if (boundary.length() > 0 && boundary.length() < 70)
	{
		const string::value_type last = *(end - 1);

		if (!(last == ' ' || last == '\t' || last == '\n'))
		{
			valid = true;

			for (string::const_iterator i = boundary.begin() ; valid && i != end ; ++i)
				valid = (validChars.find_first_of(*i) != string::npos);
		}
	}

	return (valid);
}


//
// Quick-access functions
//

const mediaType body::getContentType() const
{
	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(*m_header->findField(fields::CONTENT_TYPE));

		return (ctf.getValue());
	}
	catch (exceptions::no_such_field&)
	{
		// Defaults to "text/plain" (RFC-1521)
		return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
	}
}


const charset body::getCharset() const
{
	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(*m_header->findField(fields::CONTENT_TYPE));

		const class charset& cs = ctf.getCharset();

		return (cs);
	}
	catch (exceptions::no_such_parameter&)
	{
		// Defaults to "us-ascii" (RFC-1521)
		return (vmime::charset(charsets::US_ASCII));
	}
	catch (exceptions::no_such_field&)
	{
		// Defaults to "us-ascii" (RFC-1521)
		return (vmime::charset(charsets::US_ASCII));
	}
}


const encoding body::getEncoding() const
{
	try
	{
		const contentEncodingField& cef = dynamic_cast<contentEncodingField&>
			(*m_header->findField(fields::CONTENT_TRANSFER_ENCODING));

		return (cef.getValue());
	}
	catch (exceptions::no_such_field&)
	{
		// Defaults to "7bit" (RFC-1521)
		return (vmime::encoding(encodingTypes::SEVEN_BIT));
	}
}


const bool body::isRootPart() const
{
	return (m_part == NULL || m_part->getParentPart() == NULL);
}


body* body::clone() const
{
	body* bdy = new body(NULL);

	bdy->copyFrom(*this);

	return (bdy);
}


void body::copyFrom(const component& other)
{
	const body& bdy = dynamic_cast <const body&>(other);

	m_prologText = bdy.m_prologText;
	m_epilogText = bdy.m_epilogText;

	setContentsImpl(*bdy.m_contents);

	removeAllParts();

	for (int p = 0 ; p < bdy.getPartCount() ; ++p)
	{
		bodyPart* part = bdy.getPartAt(p)->clone();

		part->m_parent = m_part;

		m_parts.push_back(part);
	}
}


body& body::operator=(const body& other)
{
	copyFrom(other);
	return (*this);
}


const string& body::getPrologText() const
{
	return (m_prologText);
}


void body::setPrologText(const string& prologText)
{
	m_prologText = prologText;
}


const string& body::getEpilogText() const
{
	return (m_epilogText);
}


void body::setEpilogText(const string& epilogText)
{
	m_epilogText = epilogText;
}


const contentHandler& body::getContents() const
{
	return (*m_contents);
}


void body::setContents(const contentHandler& contents)
{
	setContentsImpl(contents);
}


void body::initNewPart(bodyPart* part)
{
	part->m_parent = m_part;

	if (m_header != NULL)
	{
		// Check whether we have a boundary string
		try
		{
			contentTypeField& ctf = dynamic_cast<contentTypeField&>
				(*m_header->findField(fields::CONTENT_TYPE));

			try
			{
				const string boundary = ctf.getBoundary();

				if (boundary.empty() || !isValidBoundary(boundary))
					ctf.setBoundary(generateRandomBoundaryString());
			}
			catch (exceptions::no_such_parameter&)
			{
				// No "boundary" parameter: generate a random one.
				ctf.setBoundary(generateRandomBoundaryString());
			}

			if (ctf.getValue().getType() != mediaTypes::MULTIPART)
			{
				// Warning: multi-part body but the Content-Type is
				// not specified as "multipart/..."
			}
		}
		catch (exceptions::no_such_field&)
		{
			// No "Content-Type" field: create a new one and generate
			// a random boundary string.
			contentTypeField& ctf = dynamic_cast<contentTypeField&>
				(*m_header->getField(fields::CONTENT_TYPE));

			ctf.setValue(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED));
			ctf.setBoundary(generateRandomBoundaryString());
		}
	}
}


void body::appendPart(bodyPart* part)
{
	initNewPart(part);

	m_parts.push_back(part);
}


void body::insertPartBefore(bodyPart* beforePart, bodyPart* part)
{
	initNewPart(part);

	const std::vector <bodyPart*>::iterator it = std::find
		(m_parts.begin(), m_parts.end(), beforePart);

	if (it == m_parts.end())
		throw exceptions::no_such_part();

	m_parts.insert(it, part);
}


void body::insertPartBefore(const int pos, bodyPart* part)
{
	initNewPart(part);

	m_parts.insert(m_parts.begin() + pos, part);
}


void body::insertPartAfter(bodyPart* afterPart, bodyPart* part)
{
	initNewPart(part);

	const std::vector <bodyPart*>::iterator it = std::find
		(m_parts.begin(), m_parts.end(), afterPart);

	if (it == m_parts.end())
		throw exceptions::no_such_part();

	m_parts.insert(it + 1, part);
}


void body::insertPartAfter(const int pos, bodyPart* part)
{
	initNewPart(part);

	m_parts.insert(m_parts.begin() + pos + 1, part);
}


void body::removePart(bodyPart* part)
{
	const std::vector <bodyPart*>::iterator it = std::find
		(m_parts.begin(), m_parts.end(), part);

	if (it == m_parts.end())
		throw exceptions::no_such_part();

	delete (*it);

	m_parts.erase(it);
}


void body::removePart(const int pos)
{
	delete (m_parts[pos]);

	m_parts.erase(m_parts.begin() + pos);
}


void body::removeAllParts()
{
	free_container(m_parts);
}


const int body::getPartCount() const
{
	return (m_parts.size());
}


const bool body::isEmpty() const
{
	return (m_parts.size() == 0);
}


bodyPart* body::getPartAt(const int pos)
{
	return (m_parts[pos]);
}


const bodyPart* body::getPartAt(const int pos) const
{
	return (m_parts[pos]);
}


const std::vector <const bodyPart*> body::getPartList() const
{
	std::vector <const bodyPart*> list;

	list.reserve(m_parts.size());

	for (std::vector <bodyPart*>::const_iterator it = m_parts.begin() ;
	     it != m_parts.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <bodyPart*> body::getPartList()
{
	return (m_parts);
}


const std::vector <const component*> body::getChildComponents() const
{
	std::vector <const component*> list;

	copy_vector(m_parts, list);

	return (list);
}


void body::setContentsImpl(const contentHandler& cts)
{
	delete (m_contents);
	m_contents = cts.clone();
}


} // vmime
