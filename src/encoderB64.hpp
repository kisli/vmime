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

#ifndef VMIME_ENCODERB64_HPP_INCLUDED
#define VMIME_ENCODERB64_HPP_INCLUDED


#include "encoder.hpp"


namespace vmime
{


/** Base64 encoder.
  */

class encoderB64 : public encoder
{
public:

	encoderB64();

	const utility::stream::size_type encode(utility::inputStream& in, utility::outputStream& out);
	const utility::stream::size_type decode(utility::inputStream& in, utility::outputStream& out);

	const std::vector <string> getAvailableProperties() const;

protected:

	static const unsigned char sm_alphabet[];
	static const unsigned char sm_decodeMap[256];
};


} // vmime


#endif // VMIME_ENCODERB64_HPP_INCLUDED
