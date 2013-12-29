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

#include "vmime/bodyPart.hpp"
#include "vmime/body.hpp"

#include "vmime/contentTypeField.hpp"
#include "vmime/text.hpp"

#include "vmime/utility/random.hpp"

#include "vmime/utility/seekableInputStreamRegionAdapter.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"

#include "vmime/parserHelpers.hpp"

#include "vmime/emptyContentHandler.hpp"
#include "vmime/stringContentHandler.hpp"
#include "vmime/streamContentHandler.hpp"


namespace vmime
{


body::body()
	: m_contents(make_shared <emptyContentHandler>())
{
}


body::~body()
{
}


// static
size_t body::findNextBoundaryPosition
	(shared_ptr <utility::parserInputStreamAdapter> parser, const string& boundary,
	 const size_t position, const size_t end,
	 size_t* boundaryStart, size_t* boundaryEnd)
{
	size_t pos = position;

	while (pos != npos && pos < end)
	{
		pos = parser->findNext(boundary, pos);

		if (pos == npos)
			break;  // not found

		if (pos != 0)
		{
			// Skip transport padding bytes (SPACE or HTAB), if any
			size_t advance = 0;

			while (pos != 0)
			{
				parser->seek(pos - advance - 1);

				const byte_t c = parser->peekByte();

				if (c == ' ' || c == '\t')
					++advance;
				else
					break;
			}

			// Ensure the bytes before boundary are "[LF]--": boundary should be
			// at the beginning of a line, and should start with "--"
			if (pos - advance >= 3)
			{
				parser->seek(pos - advance - 3);

				if (parser->matchBytes("\n--", 3))
				{
					parser->seek(pos + boundary.length());

					const byte_t next = parser->peekByte();

					// Boundary should be followed by a new line or a dash
					if (next == '\r' || next == '\n' || next == '-')
					{
						// Get rid of the "[CR]" just before "[LF]--", if any
						if (pos - advance >= 4)
						{
							parser->seek(pos - advance - 4);

							if (parser->peekByte() == '\r')
								advance++;
						}

						*boundaryStart = pos - advance - 3;
						*boundaryEnd = pos + boundary.length();

						return pos;
					}
				}
			}
		}

		// Boundary is a prefix of another, continue the search
		pos++;
	}

	return pos;
}


void body::parseImpl
	(const parsingContext& /* ctx */,
	 shared_ptr <utility::parserInputStreamAdapter> parser,
	 const size_t position, const size_t end, size_t* newPosition)
{
	removeAllParts();

	m_prologText.clear();
	m_epilogText.clear();

	if (end == position)
	{

		setParsedBounds(position, end);

		if (newPosition)
			*newPosition = end;

		return;
	}

	// Check whether the body is a MIME-multipart.
	// If it is, also get (or try to guess) the boundary separator.
	bool isMultipart = false;
	string boundary;

	shared_ptr <const contentTypeField> ctf =
		m_part->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

	if (ctf)
	{
		const mediaType type = *ctf->getValue <mediaType>();

		if (type.getType() == mediaTypes::MULTIPART)
		{
			isMultipart = true;

			if (ctf->hasBoundary())
			{
				boundary = ctf->getBoundary();
			}
			else
			{
				// No "boundary" parameter specified: we can try to
				// guess it by scanning the body contents...
				size_t pos = position;

				parser->seek(pos);

				if (pos + 2 < end && parser->matchBytes("--", 2))
				{
					pos += 2;
				}
				else
				{
					pos = parser->findNext("\n--", position);

					if ((pos != npos) && (pos + 3 < end))
						pos += 3;  // skip \n--
				}

				if ((pos != npos) && (pos < end))
				{
					parser->seek(pos);

					// Read some bytes after boundary separator
					byte_t buffer[256];
					const size_t bufferLen =
						parser->read(buffer, std::min(end - pos, sizeof(buffer) / sizeof(buffer[0])));

					buffer[sizeof(buffer) / sizeof(buffer[0]) - 1] = '\0';

					// Skip transport padding bytes (SPACE or HTAB), if any
					size_t boundarySkip = 0;

					while (boundarySkip < bufferLen && parserHelpers::isSpace(buffer[boundarySkip]))
						++boundarySkip;

					// Extract boundary from buffer (stop at first CR or LF).
					// We have to stop after a reasonnably long boundary length (100)
					// not to take the whole body contents for a boundary...
					byte_t boundaryBytes[100];
					size_t boundaryLen = 0;

					for (byte_t c = buffer[boundarySkip] ;
					     boundaryLen < bufferLen && boundaryLen < 100 && !(c == '\r' || c == '\n') ;
					     ++boundaryLen, c = buffer[boundarySkip + boundaryLen])
					{
						boundaryBytes[boundaryLen] = c;
					}

					if (boundaryLen >= 1 && boundaryLen < 100)
					{
						// RFC #1521, Page 31:
						// "...the boundary parameter, which consists of 1 to 70
						//  characters from a set of characters known to be very
						//  robust through email gateways, and NOT ending with
						//  white space..."
						while (boundaryLen != 0 &&
						       parserHelpers::isSpace(boundaryBytes[boundaryLen - 1]))
						{
							boundaryLen--;
						}

						if (boundaryLen >= 1)
							boundary = string(boundaryBytes, boundaryBytes + boundaryLen);
					}
				}
			}
		}
 	}

	// This is a multi-part body
	if (isMultipart && !boundary.empty())
	{
		size_t partStart = position;
		size_t pos = position;

		bool lastPart = false;

		// Find the first boundary
		size_t boundaryStart, boundaryEnd;
		pos = findNextBoundaryPosition(parser, boundary, pos, end, &boundaryStart, &boundaryEnd);

		for (int index = 0 ; !lastPart && (pos != npos) && (pos < end) ; ++index)
		{
			size_t partEnd = boundaryStart;

			// Check whether it is the last part (boundary terminated by "--")
			parser->seek(boundaryEnd);

			if (boundaryEnd + 1 < end && parser->matchBytes("--", 2))
			{
				lastPart = true;
				boundaryEnd += 2;
			}

			// RFC #1521, Page 31:
			// "...(If a boundary appears to end with white space, the
			//  white space must be presumed to have been added by a
			//  gateway, and must be deleted.)..."
			parser->seek(boundaryEnd);
			boundaryEnd += parser->skipIf(parserHelpers::isSpaceOrTab, end);

			// End of boundary line
			if (boundaryEnd + 1 < end && parser->matchBytes("\r\n", 2))
			{
				boundaryEnd += 2;
			}
			else if (boundaryEnd < end && parser->peekByte() == '\n')
			{
				++boundaryEnd;
			}

			if (index == 0)
			{
				if (partEnd > partStart)
				{
					vmime::text text;
					text.parse(parser, partStart, partEnd);

					m_prologText = text.getWholeBuffer();
				}
				else
				{
					m_prologText = "";
				}
			}
			else // index > 0
			{
				shared_ptr <bodyPart> part = m_part->createChildPart();

				// End before start may happen on empty bodyparts (directly
				// successive boundaries without even a line-break)
				if (partEnd < partStart)
					std::swap(partStart, partEnd);

				part->parse(parser, partStart, partEnd, NULL);

				m_parts.push_back(part);
			}

			partStart = boundaryEnd;

			// Find the next boundary
			pos = findNextBoundaryPosition
				(parser, boundary, boundaryEnd, end, &boundaryStart, &boundaryEnd);
		}

		m_contents = make_shared <emptyContentHandler>();

		// Last part was not found: recover from missing boundary
		if (!lastPart && pos == npos)
		{
			shared_ptr <bodyPart> part = m_part->createChildPart();

			try
			{
				part->parse(parser, partStart, end);
			}
			catch (std::exception&)
			{
				throw;
			}

			m_parts.push_back(part);
		}
		// Treat remaining text as epilog
		else if (partStart < end)
		{
			vmime::text text;
			text.parse(parser, partStart, end);

			m_epilogText = text.getWholeBuffer();
		}
	}
	// Treat the contents as 'simple' data
	else
	{
		encoding enc;

		shared_ptr <const headerField> cef =
			m_part->getHeader()->findField(fields::CONTENT_TRANSFER_ENCODING);

		if (cef)
		{
			enc = *cef->getValue <encoding>();
		}
		else
		{
			// Defaults to "7bit" (RFC-1521)
			enc = vmime::encoding(encodingTypes::SEVEN_BIT);
		}

		// Extract the (encoded) contents
		const size_t length = end - position;

		shared_ptr <utility::inputStream> contentStream =
			make_shared <utility::seekableInputStreamRegionAdapter>
				(parser->getUnderlyingStream(), position, length);

		m_contents = make_shared <streamContentHandler>(contentStream, length, enc);
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


text body::getActualPrologText(const generationContext& ctx) const
{
	const string& prologText =
		m_prologText.empty()
			? (isRootPart()
				? ctx.getPrologText()
				: NULL_STRING
			  )
			: m_prologText;

	if (prologText.empty())
		return text();
	else
		return text(prologText, vmime::charset("us-ascii"));
}


text body::getActualEpilogText(const generationContext& ctx) const
{
	const string& epilogText =
		m_epilogText.empty()
			? (isRootPart()
				? ctx.getEpilogText()
				: NULL_STRING
			  )
			: m_epilogText;

	if (epilogText.empty())
		return text();
	else
		return text(epilogText, vmime::charset("us-ascii"));
}


void body::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t /* curLinePos */, size_t* newLinePos) const
{
	// MIME-Multipart
	if (getPartCount() != 0)
	{
		string boundary;

		if (!m_part)
		{
			boundary = generateRandomBoundaryString();
		}
		else
		{
			// Use current boundary string, if specified. If no "Content-Type" field is
			// present, or the boundary is not specified, generate a random one
			shared_ptr <contentTypeField> ctf =
				m_part->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

			if (ctf)
			{
				if (ctf->hasBoundary())
				{
					boundary = ctf->getBoundary();
				}
				else
				{
					// No boundary string specified
					boundary = generateRandomBoundaryString();
				}
			}
			else
			{
				// No Content-Type (and no boundary string specified)
				boundary = generateRandomBoundaryString();
			}
		}

		const text prologText = getActualPrologText(ctx);
		const text epilogText = getActualEpilogText(ctx);

		if (!prologText.isEmpty())
		{
			prologText.encodeAndFold(ctx, os, 0,
				NULL, text::FORCE_NO_ENCODING | text::NO_NEW_LINE_SEQUENCE);

			os << CRLF;
		}

		os << "--" << boundary;

		for (size_t p = 0 ; p < getPartCount() ; ++p)
		{
			os << CRLF;

			getPartAt(p)->generate(ctx, os, 0);

			os << CRLF << "--" << boundary;
		}

		os << "--" << CRLF;

		if (!epilogText.isEmpty())
		{
			epilogText.encodeAndFold(ctx, os, 0,
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
		shared_ptr <contentHandler> contents = m_contents->clone();
		contents->setContentTypeHint(getContentType());

		contents->generate(os, getEncoding(), ctx.getMaxLineLength());
	}
}


size_t body::getGeneratedSize(const generationContext& ctx)
{
	// MIME-Multipart
	if (getPartCount() != 0)
	{
		size_t size = 0;

		// Size of parts and boundaries
		for (size_t p = 0 ; p < getPartCount() ; ++p)
		{
			size += 100;  // boundary, CRLF...
			size += getPartAt(p)->getGeneratedSize(ctx);
		}

		// Size of prolog/epilog text
		const text prologText = getActualPrologText(ctx);

		if (!prologText.isEmpty())
		{
			std::ostringstream oss;
			utility::outputStreamAdapter osa(oss);

			prologText.encodeAndFold(ctx, osa, 0,
				NULL, text::FORCE_NO_ENCODING | text::NO_NEW_LINE_SEQUENCE);

			size += oss.str().size();
		}

		const text epilogText = getActualEpilogText(ctx);

		if (!epilogText.isEmpty())
		{
			std::ostringstream oss;
			utility::outputStreamAdapter osa(oss);

			epilogText.encodeAndFold(ctx, osa, 0,
				NULL, text::FORCE_NO_ENCODING | text::NO_NEW_LINE_SEQUENCE);

			size += oss.str().size();
		}

		return size;
	}
	// Simple body
	else
	{
		shared_ptr <utility::encoder::encoder> srcEncoder = m_contents->getEncoding().getEncoder();
		shared_ptr <utility::encoder::encoder> dstEncoder = getEncoding().getEncoder();

		return dstEncoder->getEncodedSize(srcEncoder->getDecodedSize(m_contents->getLength()));
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

	char boundary[2 + 48 + 1] = { 0 };

	boundary[0] = '=';
	boundary[1] = '_';

	// Generate a string of random characters
	unsigned int r = utility::random::getTime();
	unsigned int m = static_cast <unsigned int>(sizeof(unsigned int));

	for (size_t i = 2 ; i < (sizeof(boundary) / sizeof(boundary[0]) - 1) ; ++i)
	{
			boundary[i] = bchars[r & 63];
			r >>= 6;

			if (--m == 0)
			{
				r = utility::random::getNext();
				m = static_cast <unsigned int>(sizeof(unsigned int));
			}
	}

	return (string(boundary));
}


bool body::isValidBoundary(const string& boundary)
{
	static const string validChars("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'()+_,-./:=?");

	const string::const_iterator end = boundary.end();
	bool valid = false;

	if (boundary.length() > 0 && boundary.length() < 70)
	{
		const char last = *(end - 1);

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


void body::setContentType(const mediaType& type, const charset& chset)
{
	shared_ptr <contentTypeField> ctf =
		dynamicCast <contentTypeField>(m_part->getHeader()->ContentType());

	ctf->setValue(type);
	ctf->setCharset(chset);
}


void body::setContentType(const mediaType& type)
{
	m_part->getHeader()->ContentType()->setValue(type);
}


const mediaType body::getContentType() const
{
	shared_ptr <const contentTypeField> ctf =
		m_part->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

	if (ctf)
	{
		return *ctf->getValue <mediaType>();
	}
	else
	{
		// Defaults to "text/plain" (RFC-1521)
		return (mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));
	}
}


void body::setCharset(const charset& chset)
{
	shared_ptr <contentTypeField> ctf =
		m_part->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

	// If a Content-Type field exists, set charset
	if (ctf)
	{
		ctf->setCharset(chset);
	}
	// Else, create a new Content-Type field of default type "text/plain"
	// and set charset on it
	else
	{
		setContentType(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN), chset);
	}
}


const charset body::getCharset() const
{
	const shared_ptr <const contentTypeField> ctf =
		m_part->getHeader()->findField <contentTypeField>(fields::CONTENT_TYPE);

	if (ctf)
	{
		if (ctf->hasCharset())
		{
			return (ctf->getCharset());
		}
		else
		{
			// Defaults to "us-ascii" (RFC-1521)
			return (vmime::charset(charsets::US_ASCII));
		}
	}
	else
	{
		// Defaults to "us-ascii" (RFC-1521)
		return (vmime::charset(charsets::US_ASCII));
	}
}


void body::setEncoding(const encoding& enc)
{
	m_part->getHeader()->ContentTransferEncoding()->setValue(enc);
}


const encoding body::getEncoding() const
{
	shared_ptr <const headerField> cef =
		m_part->getHeader()->findField(fields::CONTENT_TRANSFER_ENCODING);

	if (cef)
	{
		return *cef->getValue <encoding>();
	}
	else
	{
		if (m_contents->isEncoded())
			return m_contents->getEncoding();
	}

	// Defaults to "7bit" (RFC-1521)
	return vmime::encoding(encodingTypes::SEVEN_BIT);
}


void body::setParentPart(bodyPart* parent)
{
	m_part = parent;

	for (std::vector <shared_ptr <bodyPart> >::iterator it = m_parts.begin() ;
	     it != m_parts.end() ; ++it)
	{
		shared_ptr <bodyPart> childPart = *it;
		parent->importChildPart(childPart);
	}
}


bool body::isRootPart() const
{
	return (m_part == NULL || m_part->getParentPart() == NULL);
}


shared_ptr <component> body::clone() const
{
	shared_ptr <body> bdy = make_shared <body>();

	bdy->copyFrom(*this);

	return (bdy);
}


void body::copyFrom(const component& other)
{
	const body& bdy = dynamic_cast <const body&>(other);

	m_prologText = bdy.m_prologText;
	m_epilogText = bdy.m_epilogText;

	m_contents = bdy.m_contents;

	removeAllParts();

	for (size_t p = 0 ; p < bdy.getPartCount() ; ++p)
	{
		shared_ptr <bodyPart> part = m_part->createChildPart();

		part->copyFrom(*bdy.getPartAt(p));

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


const shared_ptr <const contentHandler> body::getContents() const
{
	return (m_contents);
}


void body::setContents(shared_ptr <const contentHandler> contents)
{
	m_contents = contents;
}


void body::setContents(shared_ptr <const contentHandler> contents, const mediaType& type)
{
	m_contents = contents;

	setContentType(type);
}


void body::setContents(shared_ptr <const contentHandler> contents, const mediaType& type, const charset& chset)
{
	m_contents = contents;

	setContentType(type, chset);
}


void body::setContents(shared_ptr <const contentHandler> contents, const mediaType& type,
	const charset& chset, const encoding& enc)
{
	m_contents = contents;

	setContentType(type, chset);
	setEncoding(enc);
}


void body::initNewPart(shared_ptr <bodyPart> part)
{
	// A part can be in only one body at the same time: if part is
	// already attached to a parent part, remove it from the current
	// parent part
	if (part->getParentPart())
		part->getParentPart()->getBody()->removePart(part);

	if (m_part != NULL)
	{
		m_part->importChildPart(part);

		shared_ptr <header> hdr = m_part->getHeader();

		// Check whether we have a boundary string
		shared_ptr <contentTypeField> ctf =
			hdr->findField <contentTypeField>(fields::CONTENT_TYPE);

		if (ctf)
		{
			if (ctf->hasBoundary())
			{
				const string boundary = ctf->getBoundary();

				if (boundary.empty() || !isValidBoundary(boundary))
					ctf->setBoundary(generateRandomBoundaryString());
			}
			else
			{
				// No "boundary" parameter: generate a random one.
				ctf->setBoundary(generateRandomBoundaryString());
			}

			if (ctf->getValue <mediaType>()->getType() != mediaTypes::MULTIPART)
			{
				// Warning: multi-part body but the Content-Type is
				// not specified as "multipart/..."
			}
		}
		else
		{
			// No "Content-Type" field: create a new one and generate
			// a random boundary string.
			ctf = hdr->getField <contentTypeField>(fields::CONTENT_TYPE);

			ctf->setValue(mediaType(mediaTypes::MULTIPART, mediaTypes::MULTIPART_MIXED));
			ctf->setBoundary(generateRandomBoundaryString());
		}
	}
}


void body::appendPart(shared_ptr <bodyPart> part)
{
	initNewPart(part);

	m_parts.push_back(part);
}


void body::insertPartBefore(shared_ptr <bodyPart> beforePart, shared_ptr <bodyPart> part)
{
	initNewPart(part);

	const std::vector <shared_ptr <bodyPart> >::iterator it = std::find
		(m_parts.begin(), m_parts.end(), beforePart);

	if (it == m_parts.end())
		throw exceptions::no_such_part();

	m_parts.insert(it, part);
}


void body::insertPartBefore(const size_t pos, shared_ptr <bodyPart> part)
{
	initNewPart(part);

	m_parts.insert(m_parts.begin() + pos, part);
}


void body::insertPartAfter(shared_ptr <bodyPart> afterPart, shared_ptr <bodyPart> part)
{
	initNewPart(part);

	const std::vector <shared_ptr <bodyPart> >::iterator it = std::find
		(m_parts.begin(), m_parts.end(), afterPart);

	if (it == m_parts.end())
		throw exceptions::no_such_part();

	m_parts.insert(it + 1, part);
}


void body::insertPartAfter(const size_t pos, shared_ptr <bodyPart> part)
{
	initNewPart(part);

	m_parts.insert(m_parts.begin() + pos + 1, part);
}


void body::removePart(shared_ptr <bodyPart> part)
{
	const std::vector <shared_ptr <bodyPart> >::iterator it = std::find
		(m_parts.begin(), m_parts.end(), part);

	if (it == m_parts.end())
		throw exceptions::no_such_part();

	m_parts.erase(it);
}


void body::removePart(const size_t pos)
{
	m_parts.erase(m_parts.begin() + pos);
}


void body::removeAllParts()
{
	m_parts.clear();
}


size_t body::getPartCount() const
{
	return (m_parts.size());
}


bool body::isEmpty() const
{
	return (m_parts.size() == 0);
}


shared_ptr <bodyPart> body::getPartAt(const size_t pos)
{
	return (m_parts[pos]);
}


const shared_ptr <const bodyPart> body::getPartAt(const size_t pos) const
{
	return (m_parts[pos]);
}


const std::vector <shared_ptr <const bodyPart> > body::getPartList() const
{
	std::vector <shared_ptr <const bodyPart> > list;

	list.reserve(m_parts.size());

	for (std::vector <shared_ptr <bodyPart> >::const_iterator it = m_parts.begin() ;
	     it != m_parts.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <shared_ptr <bodyPart> > body::getPartList()
{
	return (m_parts);
}


const std::vector <shared_ptr <component> > body::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	copy_vector(m_parts, list);

	return (list);
}


} // vmime
