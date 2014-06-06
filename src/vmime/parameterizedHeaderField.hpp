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

#ifndef VMIME_PARAMETERIZEDHEADERFIELD_HPP_INCLUDED
#define VMIME_PARAMETERIZEDHEADERFIELD_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldFactory.hpp"
#include "vmime/parameter.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


/** A header field that can also contain parameters (name=value pairs).
  * Parameters can be created using vmime::parameterFactory.
  */

class VMIME_EXPORT parameterizedHeaderField : public headerField
{
	friend class headerFieldFactory;

protected:

	// Protected constructor to prevent the user from creating
	// new objects without using 'headerFieldFactory'
	parameterizedHeaderField();

public:

	~parameterizedHeaderField();

	void copyFrom(const component& other);
	parameterizedHeaderField& operator=(const parameterizedHeaderField& other);

	/** Checks whether (at least) one parameter with this name exists.
	  * Parameter name is case-insensitive.
	  *
	  * @param paramName parameter name
	  * @return true if at least one parameter with the specified name
	  * exists, or false otherwise
	  */
	bool hasParameter(const string& paramName) const;

	/** Find the first parameter that matches the specified name. Parameter name
	  * is case-insensitive. If no parameter is found, NULL is returned.
	  *
	  * @param paramName parameter name
	  * @return first parameter with the specified name, or NULL if
	  * no parameter with this name exists
	  */
	shared_ptr <parameter> findParameter(const string& paramName) const;

	/** Find the first parameter that matches the specified name.
	  * Parameter name is case-insensitive.
	  * If no parameter is found, one will be created and inserted into
	  * the parameter list.
	  *
	  * @param paramName parameter name
	  * @return first parameter with the specified name or a new field
	  * if no parameter is found
	  */
	shared_ptr <parameter> getParameter(const string& paramName);

	/** Add a parameter at the end of the list.
	  *
	  * @param param parameter to append
	  */
	void appendParameter(shared_ptr <parameter> param);

	/** Insert a new parameter before the specified parameter.
	  *
	  * @param beforeParam parameter before which the new parameter will be inserted
	  * @param param parameter to insert
	  * @throw std::out_of_range if the parameter is not in the list
	  */
	void insertParameterBefore(shared_ptr <parameter> beforeParam, shared_ptr <parameter> param);

	/** Insert a new parameter before the specified position.
	  *
	  * @param pos position at which to insert the new parameter (0 to insert at
	  * the beginning of the list)
	  * @param param parameter to insert
	  * @throw std::out_of_range if the position is out of range
	  */
	void insertParameterBefore(const size_t pos, shared_ptr <parameter> param);

	/** Insert a new parameter after the specified parameter.
	  *
	  * @param afterParam parameter after which the new parameter will be inserted
	  * @param param parameter to insert
	  * @throw std::out_of_range if the parameter is not in the list
	  */
	void insertParameterAfter(shared_ptr <parameter> afterParam, shared_ptr <parameter> param);

	/** Insert a new parameter after the specified position.
	  *
	  * @param pos position of the parameter before the new parameter
	  * @param param parameter to insert
	  * @throw std::out_of_range if the position is out of range
	  */
	void insertParameterAfter(const size_t pos, shared_ptr <parameter> param);

	/** Remove the specified parameter from the list.
	  *
	  * @param param parameter to remove
	  * @throw std::out_of_range if the parameter is not in the list
	  */
	void removeParameter(shared_ptr <parameter> param);

	/** Remove the parameter at the specified position.
	  *
	  * @param pos position of the parameter to remove
	  */
	void removeParameter(const size_t pos);

	/** Remove all parameters from the list.
	  */
	void removeAllParameters();

	/** Return the number of parameters in the list.
	  *
	  * @return number of parameters
	  */
	size_t getParameterCount() const;

	/** Tests whether the list of parameters is empty.
	  *
	  * @return true if there is no parameter, false otherwise
	  */
	bool isEmpty() const;

	/** Return the parameter at the specified position.
	  *
	  * @param pos position
	  * @return parameter at position 'pos'
	  */
	const shared_ptr <parameter> getParameterAt(const size_t pos);

	/** Return the parameter at the specified position.
	  *
	  * @param pos position
	  * @return parameter at position 'pos'
	  */
	const shared_ptr <const parameter> getParameterAt(const size_t pos) const;

	/** Return the parameter list.
	  *
	  * @return list of parameters
	  */
	const std::vector <shared_ptr <const parameter> > getParameterList() const;

	/** Return the parameter list.
	  *
	  * @return list of parameters
	  */
	const std::vector <shared_ptr <parameter> > getParameterList();

	const std::vector <shared_ptr <component> > getChildComponents();

private:

	std::vector <shared_ptr <parameter> > m_params;

protected:

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


#endif // VMIME_PARAMETERIZEDHEADERFIELD_HPP_INCLUDED
