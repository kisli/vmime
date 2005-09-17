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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_CONTENTTYPEFIELD_HPP_INCLUDED
#define VMIME_CONTENTTYPEFIELD_HPP_INCLUDED


#include "vmime/parameterizedHeaderField.hpp"
#include "vmime/genericField.hpp"

#include "vmime/mediaType.hpp"
#include "vmime/charset.hpp"


namespace vmime
{


class contentTypeField : public parameterizedHeaderField, public genericField <mediaType>
{
	friend class vmime::creator;  // create ref

protected:

	contentTypeField();
	contentTypeField(contentTypeField&);

public:

	/** Return the value of the "boundary" parameter.
	  *
	  * @return value of the "boundary" parameter
	  */
	const string getBoundary() const;

	/** Set the value of the "boundary" parameter.
	  *
	  * @param boundary new value for the "boundary" parameter
	  */
	void setBoundary(const string& boundary);

	/** Return the value of the "charset" parameter.
	  *
	  * @return value of the "charset" parameter
	  */
	const charset& getCharset() const;

	/** Set the value of the "charset" parameter.
	  *
	  * @param ch new value for the "charset" parameter
	  */
	void setCharset(const charset& ch);

	/** Return the value of the "report-type" parameter (RFC-1892).
	  *
	  * @return value of the "report-type" parameter
	  */
	const string getReportType() const;

	/** Set the value of the "report-type" parameter (RFC-1892).
	  *
	  * @param reportType new value for the "report-type" parameter
	  */
	void setReportType(const string& reportType);
};


} // vmime


#endif // VMIME_CONTENTTYPEFIELD_HPP_INCLUDED
