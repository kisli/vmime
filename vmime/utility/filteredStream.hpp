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

#ifndef VMIME_UTILITY_FILTEREDSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_FILTEREDSTREAM_HPP_INCLUDED


#include "vmime/utility/stream.hpp"


namespace vmime {
namespace utility {


/** A stream whose input is filtered.
  */

class filteredInputStream : public inputStream
{
public:

	/** Return a reference to the stream being filtered.
	  *
	  * @return stream being filtered
	  */
	virtual inputStream& getPreviousInputStream() = 0;
};


/** A stream whose output is filtered.
  */

class filteredOutputStream : public outputStream
{
public:

	/** Return a reference to the stream being filtered.
	  *
	  * @return destination stream for filtered data
	  */
	virtual outputStream& getNextOutputStream() = 0;
};


/** A filtered output stream which replaces "\n."
  * sequences with "\n.." sequences.
  */

class dotFilteredOutputStream : public filteredOutputStream
{
public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param os stream into which write filtered data
	  */
	dotFilteredOutputStream(outputStream& os);

	outputStream& getNextOutputStream();

	void write(const value_type* const data, const size_type count);

private:

        outputStream& m_stream;
	value_type m_previousChar;
};


/** A filtered output stream which replaces CRLF sequences
  * with single LF characters.
  */

class CRLFToLFFilteredOutputStream : public filteredOutputStream
{
public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param os stream into which write filtered data
	  */
	CRLFToLFFilteredOutputStream(outputStream& os);

	outputStream& getNextOutputStream();

	void write(const value_type* const data, const size_type count);

private:

        outputStream& m_stream;
	value_type m_previousChar;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_FILTEREDSTREAM_HPP_INCLUDED

