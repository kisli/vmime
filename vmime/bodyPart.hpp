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

#ifndef VMIME_BODYPART_HPP_INCLUDED
#define VMIME_BODYPART_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"

#include "vmime/header.hpp"
#include "vmime/body.hpp"


namespace vmime
{


/** A MIME part.
  */

class VMIME_EXPORT bodyPart : public component
{
	friend class body;

public:

	bodyPart();

	/** Return the header section of this part.
	  *
	  * @return header section
	  */
	const shared_ptr <const header> getHeader() const;

	/** Return the header section of this part.
	  *
	  * @return header section
	  */
	shared_ptr <header> getHeader();

	/** Replaces the header section of this part.
	  *
	  * @param header the new header of this part
	  */
	void setHeader(shared_ptr <header> header);

	/** Return the body section of this part.
	  *
	  * @return body section
	  */
	const shared_ptr <const body> getBody() const;

	/** Return the body section of this part.
	  *
	  * @return body section
	  */
	shared_ptr <body> getBody();

	/** Replaces the body section of this part.
	  *
	  * @param body new body section
	  */
	void setBody(shared_ptr <body> body);

	/** Return the parent part of this part.
	  *
	  * @return parent part or NULL if not known
	  */
	bodyPart* getParentPart();

	/** Return the parent part of this part (const version).
	  *
	  * @return parent part or NULL if not known
	  */
	const bodyPart* getParentPart() const;


	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	bodyPart& operator=(const bodyPart& other);

	const std::vector <shared_ptr <component> > getChildComponents();

	size_t getGeneratedSize(const generationContext& ctx);

private:

	shared_ptr <header> m_header;
	mutable shared_ptr <body> m_body;

	// We can't use a weak_ptr<> here as the parent part may
	// have been allocated on the stack
	bodyPart* m_parent;

protected:

	/** Creates and returns a new part and set this part as its
	  * parent. The newly created sub-part should then be added
	  * to this part by calling getBody()->appendPart(). Called
	  * by the body class.
	  *
	  * @return child part
	  */
	shared_ptr <bodyPart> createChildPart();

	/** Detach the specified part from its current parent part (if
	  * any) and attach it to this part by setting this part as its
	  * new parent. The sub-part should then be added to this part
	  * by calling getBody()->appendPart(). Called by body class.
	  *
	  * @param part child part to attach
	  */
	void importChildPart(shared_ptr <bodyPart> part);

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


#endif // VMIME_BODYPART_HPP_INCLUDED
