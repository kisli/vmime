//
// VMime library (http://www.vmime.org)
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
protected:

	~htmlTextPart();

public:

	htmlTextPart();

	const mediaType getType() const;

	const charset& getCharset() const;
	void setCharset(const charset& ch);

	const contentHandler& getPlainText() const;
	void setPlainText(const contentHandler& plainText);

	const contentHandler& getText() const;
	void setText(const contentHandler& text);

	/** Embedded object (eg: image for &lt;IMG> tag).
	  */
	class embeddedObject
	{
	public:

		embeddedObject(const contentHandler& data, const encoding& enc,
		               const string& id, const mediaType& type);

		/** Return data stored in this embedded object.
		  *
		  * @return stored data
		  */
		const contentHandler& getData() const;

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

		contentHandler* m_data;
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
	const bool hasObject(const string& id) const;

	/** Return the embedded object with the specified identifier.
	  *
	  * @throw exceptions::no_object_found() if no object has been found
	  * @param id object identifier
	  * @return embedded object with the specified identifier
	  */
	const embeddedObject* findObject(const string& id) const;

	/** Return the number of embedded objects.
	  *
	  * @return number of embedded objects
	  */
	const int getObjectCount() const;

	/** Return the embedded object at the specified position.
	  *
	  * @param pos position of the embedded object
	  * @return embedded object at position 'pos'
	  */
	const embeddedObject* getObjectAt(const int pos) const;

	/** Embed an object and returns a string which identifies it.
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
	  *
	  * @param data object data
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	const string addObject(const contentHandler& data, const mediaType& type);

	/** Embed an object and returns a string which identifies it.
	  *
	  * @param data object data
	  * @param enc data encoding
	  * @param type data type
	  * @return an unique object identifier used to identify the new
	  * object among all other embedded objects
	  */
	const string addObject(const contentHandler& data, const encoding& enc, const mediaType& type);

private:

	contentHandler* m_plainText;
	contentHandler* m_text;
	charset m_charset;

	std::vector <embeddedObject*> m_objects;

	void findEmbeddedParts(const bodyPart& part, std::vector <const bodyPart*>& cidParts, std::vector <const bodyPart*>& locParts);
	void addEmbeddedObject(const bodyPart& part, const string& id);

	bool findPlainTextPart(const bodyPart& part, const bodyPart& parent, const bodyPart& textPart);

	const int getPartCount() const;

	void generateIn(bodyPart& message, bodyPart& parent) const;
	void parse(const bodyPart& message, const bodyPart& parent, const bodyPart& textPart);
};


} // vmime


#endif // VMIME_HTMLTEXTPART_HPP_INCLUDED
