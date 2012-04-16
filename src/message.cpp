//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/message.hpp"
#include "vmime/options.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"

#include <sstream>


namespace vmime
{


message::message()
{
}


void message::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	// We override this function to change the default value for the
	// "maxLineLength" parameter. So, the user will just have to call
	// message::generate() without any argument to use the maximum line
	// length specified in vmime::options...
	bodyPart::generate(os, maxLineLength, curLinePos, newLinePos);
}


const string message::generate(const string::size_type maxLineLength,
	const string::size_type curLinePos) const
{
	std::ostringstream oss;
	utility::outputStreamAdapter adapter(oss);

	generate(adapter, maxLineLength, curLinePos, NULL);

	return (oss.str());
}



void message::generate
	(ref <utility::outputStream> os,
	 const string::size_type maxLineLength,
	 const string::size_type curLinePos,
	 string::size_type* newLinePos) const
{
	bodyPart::generate(os, maxLineLength, curLinePos, newLinePos);
}


} // vmime

