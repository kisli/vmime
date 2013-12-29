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


#include "vmime/net/maildir/maildirFormat.hpp"
#include "vmime/net/maildir/maildirStore.hpp"

#include "vmime/net/maildir/format/kmailMaildirFormat.hpp"
#include "vmime/net/maildir/format/courierMaildirFormat.hpp"

#include "vmime/utility/file.hpp"


namespace vmime {
namespace net {
namespace maildir {


const utility::file::path::component maildirFormat::TMP_DIR("tmp", vmime::charset(vmime::charsets::US_ASCII));
const utility::file::path::component maildirFormat::CUR_DIR("cur", vmime::charset(vmime::charsets::US_ASCII));
const utility::file::path::component maildirFormat::NEW_DIR("new", vmime::charset(vmime::charsets::US_ASCII));


//
// maildirFormat::context
//

maildirFormat::context::context(shared_ptr <maildirStore> store)
	: m_store(store)
{
}


shared_ptr <maildirStore> maildirFormat::context::getStore() const
{
	return constCast <maildirStore>(m_store.lock());
}


//
// maildirFormat
//

maildirFormat::maildirFormat(shared_ptr <context> ctx)
	: m_context(ctx)
{
}


shared_ptr <maildirFormat::context> maildirFormat::getContext()
{
	return m_context;
}


shared_ptr <const maildirFormat::context> maildirFormat::getContext() const
{
	return m_context;
}


// static
shared_ptr <maildirFormat> maildirFormat::detect(shared_ptr <maildirStore> store)
{
	shared_ptr <context> ctx = make_shared <context>(store);

	// Try Courier format
	shared_ptr <maildirFormat> fmt = make_shared <format::courierMaildirFormat>(ctx);

	if (fmt->supports())
		return fmt;

	// Default is KMail format
	return make_shared <format::kmailMaildirFormat>(ctx);
}


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

