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

#ifndef VMIME_DEFAULTPARAMETER_HPP_INCLUDED
#define VMIME_DEFAULTPARAMETER_HPP_INCLUDED


#include "vmime/parameter.hpp"
#include "vmime/parameterFactory.hpp"

#include "vmime/word.hpp"


namespace vmime
{


/** Default parameter implementation (with support for RFC-2231).
  */

class defaultParameter : public parameter
{
	friend class vmime::creator;  // create ref

protected:

	defaultParameter();

public:

	defaultParameter& operator=(const defaultParameter& other);

	const word& getValue() const;
	word& getValue();

	void setValue(const word& value);
	void setValue(const component& value);

	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;

private:

	void parse(const std::vector <valueChunk>& chunks);

	const ref <const component> getValueImp() const;
	const ref <component> getValueImp();


	ref <word> m_value;
};


} // vmime


#endif // VMIME_DEFAULTPARAMETER_HPP_INCLUDED
