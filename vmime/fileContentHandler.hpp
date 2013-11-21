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

#ifndef VMIME_FILECONTENTHANDLER_HPP_INCLUDED
#define VMIME_FILECONTENTHANDLER_HPP_INCLUDED


#include "vmime/streamContentHandler.hpp"
#include "vmime/utility/file.hpp"


namespace vmime
{


/** A content handler which obtains its data from a file.
  */

class VMIME_EXPORT fileContentHandler : public streamContentHandler
{
public:

	/** Creates a new empty content handler. No data can be extracted until
	  * a file is attached using setData() function.
	  *
	  * @return a reference to a new content handler
	  */
	fileContentHandler();

	/** Creates a new content handler using a file.
	  *
	  * @param file file from which data will be obtained
	  * @param enc set to anything other than NO_ENCODING if the data contained
	  * in the file is already encoded with the specified encoding
	  *
	  * @return a reference to a new content handler
	  */
	fileContentHandler
		(shared_ptr <utility::file> file,
		 const vmime::encoding& enc = NO_ENCODING);

	~fileContentHandler();

	fileContentHandler(const fileContentHandler& cts);
	fileContentHandler& operator=(const fileContentHandler& cts);

	shared_ptr <contentHandler> clone() const;

	/** Sets the data managed by this content handler.
	  *
	  * @param file file from which data will be obtained
	  * @param enc set to anything other than NO_ENCODING if the data contained
	  * in the file is already encoded with the specified encoding
	  */
	void setData
		(shared_ptr <utility::file> file,
		 const vmime::encoding& enc = NO_ENCODING);

private:

	// Equals to NO_ENCODING if data is not encoded, otherwise this
	// specifies the encoding that have been used to encode the data.
	vmime::encoding m_encoding;

	// Actual data
	shared_ptr <utility::file> m_file;
};


} // vmime


#endif // VMIME_FILECONTENTHANDLER_HPP_INCLUDED
