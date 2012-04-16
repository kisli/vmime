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

#include "vmime/utility/streamUtils.hpp"

#include <algorithm>  // for std::copy
#include <iterator>   // for std::back_inserter



namespace vmime {
namespace utility {


outputStream& operator<<(outputStream& os, const stream::value_type c)
{
	os.write(&c, 1);
	return (os);
}


outputStream& operator<<(outputStream& os, const string& str)
{
	os.write(str.data(), str.length());
	return (os);
}


stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os)
{
	return bufferedStreamCopy(is, os, 0, NULL);
}


stream::size_type bufferedStreamCopyRange(inputStream& is, outputStream& os,
	const stream::size_type start, const stream::size_type length)
{
	const stream::size_type blockSize =
		std::min(is.getBlockSize(), os.getBlockSize());

	is.skip(start);

	std::vector <stream::value_type> vbuffer(blockSize);

	stream::value_type* buffer = &vbuffer.front();
	stream::size_type total = 0;

	while (!is.eof() && total < length)
	{
		const stream::size_type remaining = std::min(length - total, blockSize);
		const stream::size_type read = is.read(buffer, blockSize);

		if (read != 0)
		{
			os.write(buffer, read);
			total += read;
		}
	}

	return total;
}


stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os,
	const stream::size_type length, progressListener* progress)
{
	const stream::size_type blockSize =
		std::min(is.getBlockSize(), os.getBlockSize());

	std::vector <stream::value_type> vbuffer(blockSize);

	stream::value_type* buffer = &vbuffer.front();
	stream::size_type total = 0;

	if (progress != NULL)
		progress->start(length);

	while (!is.eof())
	{
		const stream::size_type read = is.read(buffer, blockSize);

		if (read != 0)
		{
			os.write(buffer, read);
			total += read;

			if (progress != NULL)
				progress->progress(total, std::max(total, length));
		}
	}

	if (progress != NULL)
		progress->stop(total);

	return (total);
}


} // utility
} // vmime

