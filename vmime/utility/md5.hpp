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

#ifndef VMIME_UTILITY_MD5_HPP_INCLUDED
#define VMIME_UTILITY_MD5_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/config.hpp"


namespace vmime {
namespace utility {


class md5
{
public:

	md5();
	md5(const vmime_uint8* const in, const unsigned long length);
	md5(const string& in);

public:

	const string hex();
	const vmime_uint8* hash();

	void update(const vmime_uint8* data, unsigned long len);
	void update(const string& in);

protected:

	void init();
	void transformHelper();
	void transform();
	void finalize();

	vmime_uint32 m_hash[4];

	unsigned long m_byteCount;
	vmime_uint8 m_block[64];

	bool m_finalized;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_MD5_HPP_INCLUDED
