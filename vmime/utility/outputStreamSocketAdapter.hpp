//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2012 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_UTILITY_OUTPUTSTREAMSOCKETADAPTER_HPP_INCLUDED
#define VMIME_UTILITY_OUTPUTSTREAMSOCKETADAPTER_HPP_INCLUDED


#include "vmime/utility/outputStream.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


namespace vmime {
namespace net {
	class socket;  // forward reference
} // net
} // vmime


namespace vmime {
namespace utility {


/** An output stream that is connected to a socket.
  */

class outputStreamSocketAdapter : public outputStream
{
public:

	outputStreamSocketAdapter(net::socket& sok);

	void write(const value_type* const data, const size_type count);
	void flush();

	size_type getBlockSize();

private:

	outputStreamSocketAdapter(const outputStreamSocketAdapter&);

	net::socket& m_socket;
};


} // utility
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES


#endif // VMIME_UTILITY_OUTPUTSTREAMSOCKETADAPTER_HPP_INCLUDED

