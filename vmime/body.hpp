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

class body : public component
{
	friend class bodyPart;

private:

	body(bodyPart* parentPart);

public:

	body();
	~body();

	/** Add a part at the end of the list.
	  *
	  * @param part part to append
	  */
	void appendPart(bodyPart* part);

	/** Insert a new part before the specified part.
	  *
	  * @param beforePart part before which the new part will be inserted
	  * @param part part to insert
	  * @throw exceptions::no_such_part if the part is not in the list
	  */
	void insertPartBefore(bodyPart* beforePart, bodyPart* part);

	/** Insert a new part before the specified position.
	  *
	  * @param pos position at which to insert the new part (0 to insert at
	  * the beginning of the list)
	  * @param part part to insert
	  */
	void insertPartBefore(const int pos, bodyPart* part);

	/** Insert a new part after the specified part.
	  *
	  * @param afterPart part after which the new part will be inserted
	  * @param part part to insert
	  * @throw exceptions::no_such_part if the part is not in the list
	  */
	void insertPartAfter(bodyPart* afterPart, bodyPart* part);

	/** Insert a new part after the specified position.
	  *
	  * @param pos position of the part before the new part
	  * @param part part to insert
	  */
	void insertPartAfter(const int pos, bodyPart* part);

	/** Remove the specified part from the list.
	  *
	  * @param part part to remove
	  * @throw exceptions::no_such_part if the part is not in the list
	  */
	void removePart(bodyPart* part);

	/** Remove the part at the specified position.
	  *
	  * @param pos position of the part to remove
	  */
	void removePart(const int pos);

	/** Remove all parts from the list.
	  */
	void removeAllParts();

	/** Return the number of parts in the list.
	  *
	  * @return number of parts
	  */
	const int getPartCount() const;

	/** Tests whether the list of parts is empty.
	  *
	  * @return true if there is no part, false otherwise
	  */
	const bool isEmpty() const;

	/** Return the part at the specified position.
	  *
	  * @param pos position
	  * @return part at position 'pos'
	  */
	bodyPart* getPartAt(const int pos);

	/** Return the part at the specified position.
	  *
	  * @param pos position
	  * @return part at position 'pos'
	  */
	const bodyPart* getPartAt(const int pos) const;

	/** Return the part list.
	  *
	  * @return list of parts
	  */
	const std::vector <const bodyPart*> getPartList() const;

	/** Return the part list.
	  *
	  * @return list of parts
	  */
	const std::vector <bodyPart*> getPartList();

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
	const contentHandler& getContents() const;

	/** Return a modifiable reference to body contents.
	  *
	  * @return body contents
	  */
	contentHandler& getContents();

	/** Set the body contents.
	  *
	  * @param contents new body contents
	  */
	void setContents(const contentHandler& contents);

	/** Return the media type of the data contained in the body contents.
	  * This is a shortcut for getHeader()->ContentType()->getValue()
	  * on the parent part.
	  *
	  * @return media type of body contents
	  */
	const mediaType getContentType() const;

	/** Return the charset of the data contained in the body contents.
	  * This is a shortcut for getHeader()->ContentType()->getCharset()
	  * on the parent part.
	  *
	  * @return charset of body contents
	  */
	const charset getCharset() const;

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
	static const bool isValidBoundary(const string& boundary);

	body* clone() const;
	void copyFrom(const component& other);
	body& operator=(const body& other);

	const std::vector <const component*> getChildComponents() const;

private:

	string m_prologText;
	string m_epilogText;

	contentHandler m_contents;

	bodyPart* m_part;
	header* m_header;

	std::vector <bodyPart*> m_parts;

	const bool isRootPart() const;

	void initNewPart(bodyPart* part);

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_BODY_HPP_INCLUDED
