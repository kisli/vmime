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

#include "vmime/parameterFactory.hpp"
#include "vmime/exception.hpp"

#include "vmime/standardParams.hpp"


namespace vmime
{


parameterFactory::parameterFactory()
{
	// Register some default names
	registerName <charsetParameter>("charset");
	registerName <dateParameter>("creation-date");
	registerName <dateParameter>("modification-date");
	registerName <dateParameter>("read-date");
}


parameterFactory::~parameterFactory()
{
}


parameterFactory* parameterFactory::getInstance()
{
	static parameterFactory instance;
	return (&instance);
}


parameter* parameterFactory::create
	(const string& name, const string& value)
{
	const string lcName = utility::stringUtils::toLower(name);

	NameMap::const_iterator pos = m_nameMap.find(lcName);
	parameter* param = NULL;

	if (pos != m_nameMap.end())
	{
		param = ((*pos).second)();
	}
	else
	{
		param = registerer <defaultParameter>::creator();
	}

	param->m_name = name;
	if (value != NULL_STRING) param->parse(value);

	return (param);
}


} // vmime
