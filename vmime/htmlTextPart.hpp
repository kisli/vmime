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

class htmlTextPart : public textPart
{
public:

	htmlTextPart();
	~htmlTextPart();

	const mediaType getType() const;

	const charset& getCharset() const;
	void setCharset(const charset& ch);

	const ref <const contentHandler> getPlainText() const;
	void setPlainText(ref <contentHandler> plainText);

	const ref <const contentHandler> getText() const;
	void setText(ref <contentHandler> text);

	/** Embedded object (eg: image for &lt;IMG> tag).
	  */
	class embeddedObject : public object
	{
	public:

		embeddedObject(ref <contentHandler> data, const encoding& enc,
		               const string& id, const mediaType& type);

		/** Return data stored in this embedded object.
		  *
		  * @return stored data
		  */
		const ref <const contentHandler> getData() const;

		/** Return the encoding used for data in this
		  * embedded object.
		  *
		  * @return data encoding
		  */
		const vmime::encoding& getEncoding() const;

		/** Return the identifier of this embedded object.
		  *
		  * @return object identifier
		  */
		const string& getId() const;

		/** Return the content type of data stored in
		  * this embedded object.
		  *
		  * @return data type
		  */
		const mediaType& getType() const;

	private:

		ref <contentHandler> m_data;
		encoding m_encoding;
		string m_id;
		mediaType m_type;
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
	  * @throw exceptions::no_object_found() if no object has been found
	  * @param id object identifier
	  * @return embedded object with the specified identifier
	  */
	const ref <const embeddedObject> findObject(const string& id) const;

	/** Return the number of embedded objects.
	  *
	  * @return number of embedded objects
	  */
	int getObjectCount() const;

	/** Return the embedded object at the specified position.
	  *
	  * @param pos position of the embedded object
	  * @return embedded object at position 'pos'
	  */
	const ref <const embeddedObject> getObjectAt(const int pos) const;

	/** Embed an object and returns a string which identifies it.
	  * The returned identifier is suitable for use in the 'src' attribute
	  * of an <img> tag.
	  *
	  * \deprecated Use the addObject() methods which take a 'contentHandler'
	  * parameter type instead.
	  *
	  * @param data object data
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	const string addObject(const string& data, const mediaType& type);

	/** Embed an object and returns a string which identifies it.
	  * The returned identifier is suitable for use in the 'src' attribute
	  * of an <img> tag.
	  *
	  * @param data object data
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	const string addObject(ref <contentHandler> data, const mediaType& type);

	/** Embed an object and returns a string which identifies it.
	  * The returned identifier is suitable for use in the 'src' attribute
	  * of an <img> tag.
	  *
	  * @param data object data
	  * @param enc data encoding
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	const string addObject(ref <contentHandler> data, const encoding& enc, const mediaType& type);


	int getPartCount() const;

	void generateIn(ref <bodyPart> message, ref <bodyPart> parent) const;
	void parse(ref <const bodyPart> message, ref <const bodyPart> parent, ref <const bodyPart> textPart);

private:

	ref <contentHandler> m_plainText;
	ref <contentHandler> m_text;
	charset m_charset;

	std::vector <ref <embeddedObject> > m_objects;

	void findEmbeddedParts(const bodyPart& part, std::vector <ref <const bodyPart> >& cidParts, std::vector <ref <const bodyPart> >& locParts);
	void addEmbeddedObject(const bodyPart& part, const string& id);

	bool findPlainTextPart(const bodyPart& part, const bodyPart& parent, const bodyPart& textPart);

	static const string cleanId(const string& id);
};


} // vmime


#endif // VMIME_HTMLTEXTPART_HPP_INCLUDED
