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

#ifndef VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED
#define VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED


#include "parameterizedHeaderField.hpp"
#include "disposition.hpp"

#include "dateParameter.hpp"
#include "textParameter.hpp"


namespace vmime
{


class contentDispositionField : public parameterizedHeaderField
{
	friend class headerFieldFactory::registerer <contentDispositionField>;

protected:

	contentDispositionField();

public:

	void copyFrom(const headerField& field);

	contentDispositionField& operator=(const disposition& type);

	const disposition& value() const { return (m_value); }
	disposition& value() { return (m_value); }

	const datetime& creationDate() const { return (dynamic_cast<const dateParameter&>(parameters.find("creation-date")).value()); }
	datetime& creationDate() { return (dynamic_cast<dateParameter&>(parameters.get("creation-date")).value()); }

	const datetime& modificationDate() const { return (dynamic_cast<const dateParameter&>(parameters.find("modification-date")).value()); }
	datetime& modificationDate() { return (dynamic_cast<dateParameter&>(parameters.get("modification-date")).value()); }

	const datetime& readDate() const { return (dynamic_cast<const dateParameter&>(parameters.find("read-date")).value()); }
	datetime& readDate() { return (dynamic_cast<dateParameter&>(parameters.get("read-date")).value()); }

	const string& filename() const { return (dynamic_cast<const textParameter&>(parameters.find("filename")).value()); }
	string& filename() { return (dynamic_cast<textParameter&>(parameters.get("filename")).value()); }

	const string& size() const { return (dynamic_cast<const textParameter&>(parameters.find("size")).value()); }
	string& size() { return (dynamic_cast<textParameter&>(parameters.get("size")).value()); }

protected:

	disposition m_value;

	void parseValue(const string& buffer, const string::size_type position, const string::size_type end);
	const string generateValue() const;
};


} // vmime


#endif // VMIME_CONTENTDISPOSITIONFIELD_HPP_INCLUDED
