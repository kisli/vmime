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

#ifndef VMIME_CONTENTTYPEFIELD_HPP_INCLUDED
#define VMIME_CONTENTTYPEFIELD_HPP_INCLUDED


#include "parameterizedHeaderField.hpp"

#include "mediaType.hpp"
#include "charset.hpp"

#include "textParameter.hpp"
#include "charsetParameter.hpp"


namespace vmime
{


class contentTypeField : public parameterizedHeaderField
{
	friend class headerFieldFactory::registerer <contentTypeField>;

protected:

	contentTypeField();

public:

	void copyFrom(const headerField& field);

	contentTypeField& operator=(const mediaType& type);

	const mediaType& value() const { return (m_value); }
	mediaType& value() { return (m_value); }

	const string& boundary() const { return (dynamic_cast<const textParameter&>(parameters.find("boundary")).value()); }
	string& boundary() { return (dynamic_cast<textParameter&>(parameters.get("boundary")).value()); }

	const class charset& charset() const { return (dynamic_cast<const charsetParameter&>(parameters.find("charset")).value()); }
	class charset& charset() { return (dynamic_cast<charsetParameter&>(parameters.get("charset")).value()); }

protected:

	mediaType m_value;

	void parseValue(const string& buffer, const string::size_type position, const string::size_type end);
	const string generateValue() const;
};


} // vmime


#endif // VMIME_CONTENTTYPEFIELD_HPP_INCLUDED
