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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/imap/IMAPFolderStatus.hpp"


namespace vmime {
namespace net {
namespace imap {


IMAPFolderStatus::IMAPFolderStatus()
	: m_count(0),
	  m_unseen(0),
	  m_recent(0),
	  m_uidValidity(0),
	  m_uidNext(0),
	  m_highestModSeq(0)
{
}


unsigned int IMAPFolderStatus::getMessageCount() const
{
	return m_count;
}


unsigned int IMAPFolderStatus::getUnseenCount() const
{
	return m_unseen;
}


unsigned int IMAPFolderStatus::getRecentCount() const
{
	return m_recent;
}


vmime_uint32 IMAPFolderStatus::getUIDValidity() const
{
	return m_uidValidity;
}


vmime_uint32 IMAPFolderStatus::getUIDNext() const
{
	return m_uidNext;
}


vmime_uint64 IMAPFolderStatus::getHighestModSeq() const
{
	return m_highestModSeq;
}


void IMAPFolderStatus::updateFromResponse(const IMAPParser::mailbox_data* resp)
{
	if (resp->type() == IMAPParser::mailbox_data::STATUS)
	{
		const IMAPParser::status_att_list* statusAttList = resp->status_att_list();

		for (std::vector <IMAPParser::status_att_val*>::const_iterator
			 jt = statusAttList->values().begin() ; jt != statusAttList->values().end() ; ++jt)
		{
			switch ((*jt)->type())
			{
			case IMAPParser::status_att_val::MESSAGES:

				m_count = (*jt)->value_as_number()->value();
				break;

			case IMAPParser::status_att_val::UNSEEN:

				m_unseen = (*jt)->value_as_number()->value();
				break;

			case IMAPParser::status_att_val::RECENT:

				m_recent = (*jt)->value_as_number()->value();
				break;

			case IMAPParser::status_att_val::UIDNEXT:

				m_uidNext = (*jt)->value_as_number()->value();
				break;

			case IMAPParser::status_att_val::UIDVALIDITY:

				m_uidValidity = (*jt)->value_as_number()->value();
				break;

			case IMAPParser::status_att_val::HIGHESTMODSEQ:

				m_highestModSeq = (*jt)->value_as_mod_sequence_value()->value();
				break;
			}
		}
	}
	else if (resp->type() == IMAPParser::mailbox_data::EXISTS)
	{
		m_count = resp->number()->value();
	}
	else if (resp->type() == IMAPParser::mailbox_data::RECENT)
	{
		m_recent = resp->number()->value();
	}
}


void IMAPFolderStatus::updateFromResponse(const IMAPParser::resp_text_code* resp)
{
	switch (resp->type())
	{
	case IMAPParser::resp_text_code::UIDVALIDITY:

		m_uidValidity = resp->nz_number()->value();
		break;

	case IMAPParser::resp_text_code::UIDNEXT:

		m_uidNext = resp->nz_number()->value();
		break;

	case IMAPParser::resp_text_code::UNSEEN:

		m_unseen = resp->nz_number()->value();
		break;

	case IMAPParser::resp_text_code::HIGHESTMODSEQ:

		m_highestModSeq = resp->mod_sequence_value()->value();
		break;

	case IMAPParser::resp_text_code::NOMODSEQ:

		m_highestModSeq = 0;
		break;

	default:

		break;
	}
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP
