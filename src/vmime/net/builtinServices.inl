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


// Include registration helpers
#include "vmime/net/serviceRegistration.inl"


#ifndef VMIME_BUILDING_DOC


#if VMIME_HAVE_MESSAGING_PROTO_POP3
	#include "vmime/net/pop3/POP3Store.hpp"
	REGISTER_SERVICE(pop3::POP3Store, pop3, TYPE_STORE);

	#if VMIME_HAVE_TLS_SUPPORT
		#include "vmime/net/pop3/POP3SStore.hpp"
		REGISTER_SERVICE(pop3::POP3SStore, pop3s, TYPE_STORE);
	#endif // VMIME_HAVE_TLS_SUPPORT
#endif


#if VMIME_HAVE_MESSAGING_PROTO_SMTP
	#include "vmime/net/smtp/SMTPTransport.hpp"
	REGISTER_SERVICE(smtp::SMTPTransport, smtp, TYPE_TRANSPORT);

	#if VMIME_HAVE_TLS_SUPPORT
		#include "vmime/net/smtp/SMTPSTransport.hpp"
		REGISTER_SERVICE(smtp::SMTPSTransport, smtps, TYPE_TRANSPORT);
	#endif // VMIME_HAVE_TLS_SUPPORT
#endif


#if VMIME_HAVE_MESSAGING_PROTO_IMAP
	#include "vmime/net/imap/IMAPStore.hpp"
	REGISTER_SERVICE(imap::IMAPStore, imap, TYPE_STORE);

	#if VMIME_HAVE_TLS_SUPPORT
		#include "vmime/net/imap/IMAPSStore.hpp"
		REGISTER_SERVICE(imap::IMAPSStore, imaps, TYPE_STORE);
	#endif // VMIME_HAVE_TLS_SUPPORT
#endif


#if VMIME_HAVE_MESSAGING_PROTO_MAILDIR
	#include "vmime/net/maildir/maildirStore.hpp"
	REGISTER_SERVICE(maildir::maildirStore, maildir, TYPE_STORE);
#endif

#if VMIME_HAVE_MESSAGING_PROTO_SENDMAIL
	#include "vmime/net/sendmail/sendmailTransport.hpp"
	REGISTER_SERVICE(sendmail::sendmailTransport, sendmail, TYPE_TRANSPORT);
#endif


#endif // VMIME_BUILDING_DOC
