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

#include "vmime/utility/filteredStream.hpp"


namespace vmime {
namespace utility {


// dotFilteredOutputStream

dotFilteredOutputStream::dotFilteredOutputStream(outputStream& os)
	: m_stream(os)
{
}


outputStream& dotFilteredOutputStream::getNextOutputStream()
{
	return (m_stream);
}


void dotFilteredOutputStream::write
	(const value_type* const data, const size_type count)
{
	const value_type* pos = data;
	const value_type* end = data + count;
	const value_type* start = data;

	// Replace "\n." with "\n.."
	while ((pos = std::find(pos, end, '.')) != end)
	{
		const value_type previousChar =
			(pos == data ? m_previousChar : *(pos - 1));

		if (previousChar == '\n')
		{
			m_stream.write(start, pos - data);
			m_stream.write("..", 2);

			start = pos + 1;
		}

		++pos;
	}

	m_stream.write(start, end - start);
	m_previousChar = data[count - 1];
}


// CRLFToLFFilteredOutputStream

CRLFToLFFilteredOutputStream::CRLFToLFFilteredOutputStream(outputStream& os)
	: m_stream(os)
{
}


outputStream& CRLFToLFFilteredOutputStream::getNextOutputStream()
{
	return (m_stream);
}


void CRLFToLFFilteredOutputStream::write
	(const value_type* const data, const size_type count)
{
	const value_type* pos = data;
	const value_type* end = data + count;
	const value_type* start = data;

	// Replace "\r\n" (CRLF) with "\n" (LF)
	while ((pos = std::find(pos, end, '\n')) != end)
	{
		const value_type previousChar =
			(pos == data ? m_previousChar : *(pos - 1));

		if (previousChar == '\r')
		{
			m_stream.write(start, pos - 1 - data);  // do not write \r
			m_stream.write("\n", 1);

			start = pos + 1;
		}

		++pos;
	}

	m_stream.write(start, end - start);
	m_previousChar = data[count - 1];
}


} // utility
} // vmime

