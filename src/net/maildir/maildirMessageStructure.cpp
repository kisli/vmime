//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/maildir/maildirMessageStructure.hpp"
#include "vmime/net/maildir/maildirMessagePart.hpp"


namespace vmime {
namespace net {
namespace maildir {


shared_ptr <maildirMessageStructure> maildirMessageStructure::m_emptyStructure = make_shared <maildirMessageStructure>();


maildirMessageStructure::maildirMessageStructure()
{
}


maildirMessageStructure::maildirMessageStructure(shared_ptr <maildirMessagePart> parent, const bodyPart& part)
{
	shared_ptr <maildirMessagePart> mpart = make_shared <maildirMessagePart>(parent, 0, part);
	mpart->initStructure(part);

	m_parts.push_back(mpart);
}


maildirMessageStructure::maildirMessageStructure(shared_ptr <maildirMessagePart> parent, const std::vector <shared_ptr <const vmime::bodyPart> >& list)
{
	for (unsigned int i = 0 ; i < list.size() ; ++i)
	{
		shared_ptr <maildirMessagePart> mpart = make_shared <maildirMessagePart>(parent, i, *list[i]);
		mpart->initStructure(*list[i]);

		m_parts.push_back(mpart);
	}
}


shared_ptr <const messagePart> maildirMessageStructure::getPartAt(const size_t x) const
{
	return m_parts[x];
}


shared_ptr <messagePart> maildirMessageStructure::getPartAt(const size_t x)
{
	return m_parts[x];
}


size_t maildirMessageStructure::getPartCount() const
{
	return m_parts.size();
}


// static
shared_ptr <maildirMessageStructure> maildirMessageStructure::emptyStructure()
{
	return m_emptyStructure;
}


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR
