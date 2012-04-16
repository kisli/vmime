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

#ifndef VMIME_UTILITY_STREAMUTILS_HPP_INCLUDED
#define VMIME_UTILITY_STREAMUTILS_HPP_INCLUDED


#include "vmime/utility/inputStream.hpp"
#include "vmime/utility/outputStream.hpp"

#include "vmime/utility/progressListener.hpp"


namespace vmime {
namespace utility {


/** Copy data from one stream into another stream using a buffered method.
  *
  * @param is input stream (source data)
  * @param os output stream (destination for data)
  * @return number of bytes copied
  */

stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os);

/** Copy data from one stream into another stream using a buffered method
  * and copying only a specified range of data.
  *
  * @param is input stream (source data)
  * @param os output stream (destination for data)
  * @param start number of bytes to ignore before starting copying
  * @param length maximum number of bytes to copy
  * @return number of bytes copied
  */

stream::size_type bufferedStreamCopyRange(inputStream& is, outputStream& os,
	const stream::size_type start, const stream::size_type length);

/** Copy data from one stream into another stream using a buffered method
  * and notify progress state of the operation.
  *
  * @param is input stream (source data)
  * @param os output stream (destination for data)
  * @param length predicted number of bytes to copy
  * @param progress listener to notify
  * @return number of bytes copied
  */

stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os,
	const stream::size_type length, progressListener* progress);


} // utility
} // vmime


#endif // VMIME_UTILITY_STREAMUTILS_HPP_INCLUDED

