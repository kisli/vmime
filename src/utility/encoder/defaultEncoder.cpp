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

#include "vmime/utility/encoder/defaultEncoder.hpp"

#include "vmime/utility/streamUtils.hpp"


namespace vmime {
namespace utility {
namespace encoder {


defaultEncoder::defaultEncoder()
{
}


utility::stream::size_type defaultEncoder::encode(utility::inputStream& in,
	utility::outputStream& out, utility::progressListener* progress)
{
	in.reset();  // may not work...

	// No encoding performed
	utility::stream::size_type res = 0;

	if (progress)
		res = utility::bufferedStreamCopy(in, out, 0, progress);
	else
		res = utility::bufferedStreamCopy(in, out);

	return res;
}


utility::stream::size_type defaultEncoder::decode(utility::inputStream& in,
	utility::outputStream& out, utility::progressListener* progress)
{
	in.reset();  // may not work...

	// No decoding performed
	utility::stream::size_type res = 0;

	if (progress)
		res = utility::bufferedStreamCopy(in, out, 0, progress);
	else
		res = utility::bufferedStreamCopy(in, out);

	return res;
}


} // encoder
} // utility
} // vmime
