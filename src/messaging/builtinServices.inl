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

#define REGISTER_SERVICE(p_class, p_name) \
	vmime::messaging::service::initializer <vmime::messaging::p_class> p_name(#p_name)


#if VMIME_BUILTIN_MESSAGING_PROTO_POP3
	#include "POP3Store.hpp"
	REGISTER_SERVICE(POP3Store, pop3);
#endif


#if VMIME_BUILTIN_MESSAGING_PROTO_SMTP
	#include "SMTPTransport.hpp"
	REGISTER_SERVICE(SMTPTransport, smtp);
#endif


#if VMIME_BUILTIN_MESSAGING_PROTO_IMAP
	#include "IMAPStore.hpp"
	REGISTER_SERVICE(IMAPStore, imap);
#endif


#if VMIME_BUILTIN_MESSAGING_PROTO_MAILDIR
	#include "maildirStore.hpp"
	REGISTER_SERVICE(maildirStore, maildir);
#endif

