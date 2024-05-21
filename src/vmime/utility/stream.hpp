//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_UTILITY_STREAM_HPP_INCLUDED
#define VMIME_UTILITY_STREAM_HPP_INCLUDED


#include <sstream>

#include "vmime/config.hpp"
#include "vmime/types.hpp"
#include "vmime/base.hpp"


namespace vmime {
namespace utility {


/** Base class for input/output stream.
  */
class VMIME_EXPORT stream : public object {
public:
	stream() = default;
	/* Presence of move-ctor/move-asg inhibits default copy-ctor/copy-asg */
	stream(stream &&) = default;
	stream &operator=(stream &&) = default;

	virtual ~stream() { }

	/** Return the preferred maximum block size when reading
	  * from or writing to this stream.
	  *
	  * @return block size, in bytes
	  */
	virtual size_t getBlockSize();
};


} // utility
} // vmime


#endif // VMIME_UTILITY_STREAM_HPP_INCLUDED
