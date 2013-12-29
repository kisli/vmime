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

#ifndef VMIME_BODY_HPP_INCLUDED
#define VMIME_BODY_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"

#include "vmime/header.hpp"

#include "vmime/mediaType.hpp"
#include "vmime/charset.hpp"
#include "vmime/encoding.hpp"

#include "vmime/contentHandler.hpp"


namespace vmime
{


class bodyPart;


/** Body section of a MIME part.
  */

class VMIME_EXPORT body : public component
{
	friend class bodyPart;

public:

	body();
	~body();

	/** Add a part at the end of the list.
	  *
	  * @param part part to append
	  */
	void appendPart(shared_ptr <bodyPart> part);

	/** Insert a new part before the specified part.
	  *
	  * @param beforePart part before which the new part will be inserted
	  * @param part part to insert
	  * @throw exceptions::no_such_part if the part is not in the list
	  */
	void insertPartBefore(shared_ptr <bodyPart> beforePart, shared_ptr <bodyPart> part);

	/** Insert a new part before the specified position.
	  *
	  * @param pos position at which to insert the new part (0 to insert at
	  * the beginning of the list)
	  * @param part part to insert
	  */
	void insertPartBefore(const size_t pos, shared_ptr <bodyPart> part);

	/** Insert a new part after the specified part.
	  *
	  * @param afterPart part after which the new part will be inserted
	  * @param part part to insert
	  * @throw exceptions::no_such_part if the part is not in the list
	  */
	void insertPartAfter(shared_ptr <bodyPart> afterPart, shared_ptr <bodyPart> part);

	/** Insert a new part after the specified position.
	  *
	  * @param pos position of the part before the new part
	  * @param part part to insert
	  */
	void insertPartAfter(const size_t pos, shared_ptr <bodyPart> part);

	/** Remove the specified part from the list.
	  *
	  * @param part part to remove
	  * @throw exceptions::no_such_part if the part is not in the list
	  */
	void removePart(shared_ptr <bodyPart> part);

	/** Remove the part at the specified position.
	  *
	  * @param pos position of the part to remove
	  */
	void removePart(const size_t pos);

	/** Remove all parts from the list.
	  */
	void removeAllParts();

	/** Return the number of parts in the list.
	  *
	  * @return number of parts
	  */
	size_t getPartCount() const;

	/** Tests whether the list of parts is empty.
	  *
	  * @return true if there is no part, false otherwise
	  */
	bool isEmpty() const;

	/** Return the part at the specified position.
	  *
	  * @param pos position
	  * @return part at position 'pos'
	  */
	shared_ptr <bodyPart> getPartAt(const size_t pos);

	/** Return the part at the specified position.
	  *
	  * @param pos position
	  * @return part at position 'pos'
	  */
	const shared_ptr <const bodyPart> getPartAt(const size_t pos) const;

	/** Return the part list.
	  *
	  * @return list of parts
	  */
	const std::vector <shared_ptr <const bodyPart> > getPartList() const;

	/** Return the part list.
	  *
	  * @return list of parts
	  */
	const std::vector <shared_ptr <bodyPart> > getPartList();

	/** Return the prolog text.
	  *
	  * @return prolog text
	  */
	const string& getPrologText() const;

	/** Set the prolog text.
	  *
	  * @param prologText new prolog text
	  */
	void setPrologText(const string& prologText);

	/** Return the epilog text.
	  *
	  * @return epilog text
	  */
	const string& getEpilogText() const;

	/** Set the epilog text.
	  *
	  * @param epilogText new epilog text
	  */
	void setEpilogText(const string& epilogText);

	/** Return a read-only reference to body contents.
	  *
	  * @return read-only body contents
	  */
	const shared_ptr <const contentHandler> getContents() const;

	/** Set the body contents.
	  *
	  * @param contents new body contents
	  */
	void setContents(shared_ptr <const contentHandler> contents);

	/** Set the body contents and type.
	  *
	  * @param contents new body contents
	  * @param type type of contents
	  */
	void setContents(shared_ptr <const contentHandler> contents, const mediaType& type);

	/** Set the body contents, type and charset.
	  *
	  * @param contents new body contents
	  * @param type type of contents
	  * @param chset charset of contents
	  */
	void setContents(shared_ptr <const contentHandler> contents, const mediaType& type, const charset& chset);

	/** Set the body contents, type, charset and encoding.
	  *
	  * @param contents new body contents
	  * @param type type of contents
	  * @param chset charset of contents
	  * @param enc contents encoding
	  */
	void setContents(shared_ptr <const contentHandler> contents, const mediaType& type,
		const charset& chset, const encoding& enc);

	/** Set the MIME type and charset of contents.
	  * If a charset is defined, it will not be modified.
	  *
	  * @param type MIME media type of contents
	  * @param chset charset of contents
	  */
	void setContentType(const mediaType& type, const charset& chset);

	/** Set the MIME type of contents.
	  *
	  * @param type MIME media type of contents
	  */
	void setContentType(const mediaType& type);

	/** Return the media type of the data contained in the body contents.
	  * This is a shortcut for getHeader()->ContentType()->getValue()
	  * on the parent part.
	  *
	  * @return media type of body contents
	  */
	const mediaType getContentType() const;

	/** Set the charset of contents.
	  * If the type is not set, it will be set to default "text/plain" type.
	  *
	  * @param chset charset of contents
	  */
	void setCharset(const charset& chset);

	/** Return the charset of the data contained in the body contents.
	  * This is a shortcut for getHeader()->ContentType()->getCharset()
	  * on the parent part.
	  *
	  * @return charset of body contents
	  */
	const charset getCharset() const;

	/** Set the output encoding of contents.
	  * Contents will be encoded (or re-encoded) when this node is being generated.
	  *
	  * @param enc encoding of contents
	  */
	void setEncoding(const encoding& enc);

	/** Return the encoding used to encode the body contents.
	  * This is a shortcut for getHeader()->ContentTransferEncoding()->getValue()
	  * on the parent part.
	  *
	  * @return encoding of body contents
	  */
	const encoding getEncoding() const;

	/** Generate a new random boundary string.
	  *
	  * @return randomly generated boundary string
	  */
	static const string generateRandomBoundaryString();

	/** Test a boundary string for validity (as defined in RFC #1521, page 19).
	  *
	  * @param boundary boundary string to test
	  * @return true if the boundary string is valid, false otherwise
	  */
	static bool isValidBoundary(const string& boundary);

	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	body& operator=(const body& other);

	const std::vector <shared_ptr <component> > getChildComponents();

	size_t getGeneratedSize(const generationContext& ctx);

private:

	text getActualPrologText(const generationContext& ctx) const;
	text getActualEpilogText(const generationContext& ctx) const;

	void setParentPart(bodyPart* parent);


	string m_prologText;
	string m_epilogText;

	shared_ptr <const contentHandler> m_contents;

	bodyPart* m_part;

	std::vector <shared_ptr <bodyPart> > m_parts;

	bool isRootPart() const;

	void initNewPart(shared_ptr <bodyPart> part);

protected:

	/** Finds the next boundary position in the parsing buffer.
	  *
	  * @param parser parser object
	  * @param boundary boundary string (without "--" nor CR/LF)
	  * @param position start position
	  * @param end end position
	  * @param boundaryStart will hold the start position of the boundary (including any
	  * CR/LF and "--" before the boundary)
	  * @param boundaryEnd will hold the end position of the boundary (position just
	  * before the CRLF or "--" which follows)
	  * @return the position of the boundary string, or npos if not found
	  */
	size_t findNextBoundaryPosition
		(shared_ptr <utility::parserInputStreamAdapter> parser, const string& boundary,
		 const size_t position, const size_t end,
		 size_t* boundaryStart, size_t* boundaryEnd);

	// Component parsing & assembling
	void parseImpl
		(const parsingContext& ctx,
		 shared_ptr <utility::parserInputStreamAdapter> parser,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	void generateImpl
		(const generationContext& ctx,
		 utility::outputStream& os,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_BODY_HPP_INCLUDED
