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

#ifndef VMIME_HTMLTEXTPART_HPP_INCLUDED
#define VMIME_HTMLTEXTPART_HPP_INCLUDED


#include "vmime/textPart.hpp"
#include "vmime/messageId.hpp"
#include "vmime/encoding.hpp"

#include "vmime/contentHandler.hpp"


namespace vmime
{


/** Text part of type 'text/html'.
  */

class VMIME_EXPORT htmlTextPart : public textPart
{
public:

	htmlTextPart();
	~htmlTextPart();

	const mediaType getType() const;

	const charset& getCharset() const;
	void setCharset(const charset& ch);

	shared_ptr <const contentHandler> getPlainText() const;
	void setPlainText(shared_ptr <contentHandler> plainText);

	const shared_ptr <const contentHandler> getText() const;
	void setText(shared_ptr <contentHandler> text);

	/** Embedded object (eg: image for &lt;IMG> tag).
	  */
	class VMIME_EXPORT embeddedObject : public object
	{
	public:

		/** The ways embedded objects can be referenced. */
		enum ReferenceType
		{
			REFERENCED_BY_ID,         /**< Referenced by Content-Id. */
			REFERENCED_BY_LOCATION    /**< Referenced by Content-Location. */
		};

		/** Constructs an embedded object.
		  *
		  * @param data content of the object
		  * @param enc encoding of the data
		  * @param id object identifier
		  * @param type object content type
		  * @param refType reference type
		  * @return a reference to a new embedded object
		  */
		embeddedObject(shared_ptr <contentHandler> data, const encoding& enc,
		               const string& id, const mediaType& type,
		               const ReferenceType refType);

		/** Return data stored in this embedded object.
		  *
		  * @return stored data
		  */
		shared_ptr <const contentHandler> getData() const;

		/** Return the encoding used for data in this
		  * embedded object.
		  *
		  * @return data encoding
		  */
		const vmime::encoding getEncoding() const;

		/** Returns the identifier of this embedded object (either a
		  * unique ID or a location).
		  *
		  * @return object identifier
		  */
		const string getId() const;

		/** Return the identifier used to reference this embedded object
		  * in a text document (for example, you can use the result as
		  * the "src" attribute of an &lt;img> tag).
		  *
		  * @return object reference identifier
		  */
		const string getReferenceId() const;

		/** Return the content type of data stored in
		  * this embedded object.
		  *
		  * @return data type
		  */
		const mediaType getType() const;

		/** Returns the way this object is referenced.
		  *
		  * @return reference type (see ReferenceType enum)
		  */
		ReferenceType getReferenceType() const;

		/** Returns whether this object matches the specified identifier.
		  *
		  * @param id identifier to test
		  * @return true if the specified identifier references this
		  * object, or false otherwise
		  */
		bool matchesId(const string& id) const;

	private:

		static const string cleanId(const string& id);

		shared_ptr <contentHandler> m_data;
		encoding m_encoding;
		string m_id;
		mediaType m_type;
		ReferenceType m_refType;
	};


	/** Test the existence of an embedded object given its identifier.
	  *
	  * @param id object identifier
	  * @return true if an object with this identifier exists,
	  * false otherwise
	  */
	bool hasObject(const string& id) const;

	/** Return the embedded object with the specified identifier.
	  *
	  * @param id object identifier
	  * @return embedded object with the specified identifier, or NULL if
	  * no object has been found
	  */
	shared_ptr <const embeddedObject> findObject(const string& id) const;

	/** Return the number of embedded objects.
	  *
	  * @return number of embedded objects
	  */
	size_t getObjectCount() const;

	/** Return the embedded object at the specified position.
	  *
	  * @param pos position of the embedded object
	  * @return embedded object at position 'pos'
	  */
	shared_ptr <const embeddedObject> getObjectAt(const size_t pos) const;

	/** Embed an object and returns a string which identifies it.
	  * The returned identifier is suitable for use in the 'src' attribute
	  * of an &lt;img> tag.
	  *
	  * \deprecated Use the addObject() methods which take a 'contentHandler'
	  * parameter type instead.
	  *
	  * @param data object data
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	shared_ptr <const embeddedObject> addObject(const string& data, const mediaType& type);

	/** Embed an object and returns a string which identifies it.
	  * The returned identifier is suitable for use in the 'src' attribute
	  * of an &lt;img> tag.
	  *
	  * @param data object data
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	shared_ptr <const embeddedObject> addObject(shared_ptr <contentHandler> data, const mediaType& type);

	/** Embed an object and returns a string which identifies it.
	  * The returned identifier is suitable for use in the 'src' attribute
	  * of an &lt;img> tag.
	  *
	  * @param data object data
	  * @param enc data encoding
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	shared_ptr <const embeddedObject> addObject(shared_ptr <contentHandler> data, const encoding& enc, const mediaType& type);


	size_t getPartCount() const;

	void generateIn(shared_ptr <bodyPart> message, shared_ptr <bodyPart> parent) const;
	void parse(shared_ptr <const bodyPart> message, shared_ptr <const bodyPart> parent, shared_ptr <const bodyPart> textPart);

private:

	shared_ptr <contentHandler> m_plainText;
	shared_ptr <contentHandler> m_text;
	charset m_charset;

	std::vector <shared_ptr <embeddedObject> > m_objects;

	void findEmbeddedParts(const bodyPart& part, std::vector <shared_ptr <const bodyPart> >& cidParts, std::vector <shared_ptr <const bodyPart> >& locParts);
	void addEmbeddedObject(const bodyPart& part, const string& id, const embeddedObject::ReferenceType refType);

	bool findPlainTextPart(const bodyPart& part, const bodyPart& parent, const bodyPart& textPart);
};


} // vmime


#endif // VMIME_HTMLTEXTPART_HPP_INCLUDED
