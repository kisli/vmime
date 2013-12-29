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

#ifndef VMIME_DISPOSITION_HPP_INCLUDED
#define VMIME_DISPOSITION_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldValue.hpp"

#include <vector>


namespace vmime
{


/** Disposition - from RFC-3798 (basic type).
  */

class VMIME_EXPORT disposition : public headerFieldValue
{
public:

	disposition();
	disposition(const string& actionMode, const string& sendingMode, const string& type, const string& modifier);


	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	disposition& operator=(const disposition& other);

	const std::vector <shared_ptr <component> > getChildComponents();


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
	bool hasModifier(const string& modifier) const;

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

protected:

	// Component parsing & assembling
	void parseImpl
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	void generateImpl
		(const generationContext& ctx,
		 utility::outputStream& os,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_DISPOSITION_HPP_INCLUDED

