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

#include "vmime/misc/importanceHelper.hpp"
#include "vmime/exception.hpp"


namespace vmime {
namespace misc {


void importanceHelper::resetImportance(ref <message> msg)
{
	resetImportanceHeader(msg->getHeader());
}


void importanceHelper::resetImportanceHeader(ref <header> hdr)
{
	try
	{
		ref <headerField> fld = hdr->findField("X-Priority");
		hdr->removeField(fld);
	}
	catch (exceptions::no_such_field)
	{
		// Ignore
	}

	try
	{
		ref <headerField> fld = hdr->findField("Importance");
		hdr->removeField(fld);
	}
	catch (exceptions::no_such_field)
	{
		// Ignore
	}
}


const importanceHelper::Importance importanceHelper::getImportance(ref <const message> msg)
{
	return getImportanceHeader(msg->getHeader());
}


const importanceHelper::Importance importanceHelper::getImportanceHeader(ref <const header> hdr)
{
	// Try "X-Priority" field
	try
	{
		const ref <const defaultField> fld = hdr->findField("X-Priority").dynamicCast <const defaultField>();
		const string value = fld->getValue();

		int n = IMPORTANCE_NORMAL;

		std::istringstream iss(value);
		iss >> n;

		Importance i = IMPORTANCE_NORMAL;

		switch (n)
		{
		case 1: i = IMPORTANCE_HIGHEST; break;
		case 2: i = IMPORTANCE_HIGH; break;
		case 3: i = IMPORTANCE_NORMAL; break;
		case 4: i = IMPORTANCE_LOW; break;
		case 5: i = IMPORTANCE_LOWEST; break;
		}

		return (i);
	}
	catch (exceptions::no_such_field)
	{
		// Try "Importance" field
		try
		{
			const ref <const defaultField> fld = hdr->findField("Importance").dynamicCast <const defaultField>();
			const string value = utility::stringUtils::toLower(utility::stringUtils::trim(fld->getValue()));

			if (value == "low")
				return (IMPORTANCE_LOWEST);
			else if (value == "high")
				return (IMPORTANCE_HIGHEST);
			else
				return (IMPORTANCE_NORMAL);
		}
		catch (exceptions::no_such_field)
		{
			// Default
			return (IMPORTANCE_NORMAL);
		}
	}

	// Should not go here...
	return (IMPORTANCE_NORMAL);
}


void importanceHelper::setImportance(ref <message> msg, const Importance i)
{
	setImportanceHeader(msg->getHeader(), i);
}


void importanceHelper::setImportanceHeader(ref <header> hdr, const Importance i)
{
	// "X-Priority:" Field
	ref <defaultField> fld = hdr->getField("X-Priority").dynamicCast <defaultField>();

	switch (i)
	{
	case IMPORTANCE_HIGHEST: fld->setValue("1 (Highest)"); break;
	case IMPORTANCE_HIGH:    fld->setValue("2 (High)"); break;
	default:
	case IMPORTANCE_NORMAL:  fld->setValue("3 (Normal)"); break;
	case IMPORTANCE_LOW:     fld->setValue("4 (Low)"); break;
	case IMPORTANCE_LOWEST:  fld->setValue("5 (Lowest)"); break;
	}

	// "Importance:" Field
	fld = hdr->getField("Importance").dynamicCast <defaultField>();

	switch (i)
	{
	case IMPORTANCE_HIGHEST:
	case IMPORTANCE_HIGH:

		fld->setValue("high");
		break;

	default:
	case IMPORTANCE_NORMAL:

		fld->setValue("normal");
		break;

	case IMPORTANCE_LOWEST:
	case IMPORTANCE_LOW:

		fld->setValue("low");
		break;
	}
}


} // misc
} // vmime
