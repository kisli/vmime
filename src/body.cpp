//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "bodyPart.hpp"
#include "body.hpp"

#include "options.hpp"

#include "contentTypeField.hpp"
#include "contentEncodingField.hpp"

#include "utility/random.hpp"

#include "parserHelpers.hpp"


namespace vmime
{


body::body(bodyPart& part)
	: parts(*this), m_part(part), m_header(part.header())
{
}


void body::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	parts.clear();

	// Check whether the body is a MIME-multipart
	bool isMultipart = false;
	string boundary;

	try
	{
		const contentTypeField& ctf = dynamic_cast <contentTypeField&>
			(m_header.fields.find(headerField::ContentType));

		if (ctf.value().type() == mediaTypes::MULTIPART)
		{
			isMultipart = true;

			try
			{
				boundary = ctf.boundary();
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
						while (pos != start && isspace(buffer[pos - 1]))
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

				parts.m_parts.push_back(part);
			}

			partStart = pos;
			pos = buffer.find(boundarySep, partStart);
		}

		if (partStart < end)
			m_epilogText = string(buffer.begin() + partStart, buffer.begin() + end);
	}
	// Treat the contents as 'simple' data
	else
	{
		// Extract the (encoded) contents
		m_contents.set(buffer, position, end, encoding());
	}

	if (newPosition)
		*newPosition = end;
}


void body::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type /* curLinePos */, string::size_type* newLinePos) const
{
	// MIME-Multipart
	if (parts.size() != 0)
	{
		string boundary;

		try
		{
			contentTypeField& ctf = dynamic_cast<contentTypeField&>
				(m_header.fields.find(headerField::ContentType));

			boundary = ctf.boundary();
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

		const string& prologText =
			m_prologText.empty()
				? (isRootPart()
					? options::getInstance()->multipart.prologText()
					: NULL_STRING
				  )
				: m_prologText;

		const string& epilogText =
			m_epilogText.empty()
				? (isRootPart()
					? options::getInstance()->multipart.epilogText()
					: NULL_STRING
				  )
				: m_epilogText;

		if (!prologText.empty())
		{
			encodeAndFoldText(os, text(word(prologText, charset())), maxLineLength, 0,
				NULL, encodeAndFoldFlags::forceNoEncoding | encodeAndFoldFlags::noNewLineSequence);

			os << CRLF;
		}

		os << "--" << boundary;

		for (std::vector <bodyPart*>::const_iterator
		     p = parts.m_parts.begin() ; p != parts.m_parts.end() ; ++p)
		{
			os << CRLF;

			(*p)->generate(os, maxLineLength, 0);

			os << CRLF << "--" << boundary;
		}

		os << "--" << CRLF;

		if (!epilogText.empty())
		{
			encodeAndFoldText(os, text(word(epilogText, charset())), maxLineLength, 0,
				NULL, encodeAndFoldFlags::forceNoEncoding | encodeAndFoldFlags::noNewLineSequence);

			os << CRLF;
		}

		if (newLinePos)
			*newLinePos = 0;
	}
	// Simple body
	else
	{
		// Generate the contents
		m_contents.generate(os, encoding(), maxLineLength);
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
	unsigned int r = utility::random::time();
	unsigned int m = sizeof(unsigned int);

	for (size_t i = 2 ; i < (sizeof(boundary) / sizeof(boundary[0]) - 1) ; ++i)
	{
			boundary[i] = bchars[r & 63];
			r >>= 6;

			if (--m == 0)
			{
				r = utility::random::next();
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

const mediaType body::contentType() const
{
	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>(m_header.fields.find(headerField::ContentType));
		return (ctf.value());
	}
	catch (exceptions::no_such_field&)
	{
		// Defaults to "text/plain" (RFC-1521)
		return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
	}
}


const class charset body::charset() const
{
	try
	{
		const contentTypeField& ctf = dynamic_cast<contentTypeField&>(m_header.fields.find(headerField::ContentType));
		const class charset& cs = ctf.charset();

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


const class encoding body::encoding() const
{
	try
	{
		const contentEncodingField& cef = m_header.fields.ContentTransferEncoding();
		return (cef.value());
	}
	catch (exceptions::no_such_field&)
	{
		// Defaults to "7bit" (RFC-1521)
		return (vmime::encoding(encodingTypes::SEVEN_BIT));
	}
}


const bool body::isRootPart() const
{
	return (m_part.parent() == NULL);
}


body& body::operator=(const body& b)
{
	m_prologText = b.m_prologText;
	m_epilogText = b.m_epilogText;

	m_contents = b.m_contents;

	parts = b.parts;

	return (*this);
}


/////////////////////
// Parts container //
/////////////////////


body::partsContainer::partsContainer(class body& body)
	: m_body(body)
{
}


// Part insertion
void body::partsContainer::append(bodyPart* part)
{
	part->m_parent = &(m_body.m_part);

	m_parts.push_back(part);

	// Check whether we have a boundary string
	try
	{
		contentTypeField& ctf = dynamic_cast<contentTypeField&>
			(m_body.m_header.fields.find(headerField::ContentType));

		try
		{
			const string boundary = ctf.boundary();

			if (boundary.empty() || !isValidBoundary(boundary))
				throw exceptions::no_such_parameter("boundary"); // to generate a new one
		}
		catch (exceptions::no_such_parameter&)
		{
			// No "boundary" parameter: generate a random one.
			ctf.boundary() = generateRandomBoundaryString();
		}

		if (ctf.value().type() != mediaTypes::MULTIPART)
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
			(m_body.m_header.fields.get(headerField::ContentType));

		ctf.value() = mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED);
		ctf.boundary() = generateRandomBoundaryString();
	}
}


void body::partsContainer::insert(const iterator it, bodyPart* part)
{
	part->m_parent = &(m_body.m_part);

	m_parts.insert(it.m_iterator, part);
}


// Part removing
void body::partsContainer::remove(const iterator it)
{
	delete (*it.m_iterator);
	m_parts.erase(it.m_iterator);
}


void body::partsContainer::clear()
{
	free_container(m_parts);
}


body::partsContainer::~partsContainer()
{
	clear();
}


body::partsContainer& body::partsContainer::operator=(const partsContainer& c)
{
	std::vector <bodyPart*> parts;

	for (std::vector <bodyPart*>::const_iterator it = c.m_parts.begin() ; it != c.m_parts.end() ; ++it)
	{
		bodyPart* p = (*it)->clone();
		p->m_parent = &(m_body.m_part);

		parts.push_back(p);
	}

	for (std::vector <bodyPart*>::iterator it = m_parts.begin() ; it != m_parts.end() ; ++it)
		delete (*it);

	m_parts.resize(parts.size());
	std::copy(parts.begin(), parts.end(), m_parts.begin());

	return (*this);
}


} // vmime
