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

#ifndef VMIME_DISPOSITION_HPP_INCLUDED
#define VMIME_DISPOSITION_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"

#include <vector>


namespace vmime
{


/** Disposition - from RFC-3798 (basic type).
  */

class disposition : public component
{
public:

	disposition();
	disposition(const string& actionMode, const string& sendingMode, const string& type, const string& modifier);


	ref <component> clone() const;
	void copyFrom(const component& other);
	disposition& operator=(const disposition& other);

	const std::vector <ref <const component> > getChildComponents() const;


	/** Set the disposition action mode.
	  * See the constants in vmime::dispositionActionModes.
	  *
	  * @param mode disposition action mode
	  */
	void setActionMode(const string& mode);

	/** Return the disposition action mode.
	  * See the constants in vmime::dispositionActionModes.
	  *
	  * @return disposition action mode
	  */
	const string& getActionMode() const;

	/** Set the disposition sending mode.
	  * See the constants in vmime::dispositionSendingModes.
	  *
	  * @param mode disposition sending mode
	  */
	void setSendingMode(const string& mode);

	/** Return the disposition sending mode.
	  * See the constants in vmime::dispositionSendingModes.
	  *
	  * @return disposition sending mode
	  */
	const string& getSendingMode() const;

	/** Set the disposition type.
	  * See the constants in vmime::dispositionTypes.
	  *
	  * @param type disposition type
	  */
	void setType(const string& type);

	/** Return the disposition type.
	  * See the constants in vmime::dispositionTypes.
	  *
	  * @return disposition type
	  */
	const string& getType() const;

	/** Add a disposition modifier if it does not exist.
	  * See the constants in vmime::dispositionModifiers.
	  *
	  * @param modifier modifier to add
	  */
	void addModifier(const string& modifier);

	/** Remove the specified disposition modifier.
	  * See the constants in vmime::dispositionModifiers.
	  *
	  * @param modifier modifier to remove
	  */
	void removeModifier(const string& modifier);

	/** Remove all disposition modifiers.
	  */
	void removeAllModifiers();

	/** Test whether a disposition modifier is set.
	  *
	  * @param modifier modifier to test
	  * @return true if the specified modifier is set, false otherwise
	  */
	const bool hasModifier(const string& modifier) const;

	/** Return the list of modifiers.
	  *
	  * @return list of modifiers
	  */
	const std::vector <string> getModifierList() const;

private:

	string m_actionMode;
	string m_sendingMode;
	string m_type;

	std::vector <string> m_modifiers;

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_DISPOSITION_HPP_INCLUDED

