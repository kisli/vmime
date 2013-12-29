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


IMAPFolderStatus::IMAPFolderStatus(const IMAPFolderStatus& other)
	: folderStatus(),
	  m_count(other.m_count),
	  m_unseen(other.m_unseen),
	  m_recent(other.m_recent),
	  m_uidValidity(other.m_uidValidity),
	  m_uidNext(other.m_uidNext),
	  m_highestModSeq(other.m_highestModSeq)
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


shared_ptr <folderStatus> IMAPFolderStatus::clone() const
{
	return make_shared <IMAPFolderStatus>(*this);
}


bool IMAPFolderStatus::updateFromResponse(const IMAPParser::mailbox_data* resp)
{
	bool changed = false;

	if (resp->type() == IMAPParser::mailbox_data::STATUS)
	{
		const IMAPParser::status_att_list* statusAttList = resp->status_att_list();

		for (std::vector <IMAPParser::status_att_val*>::const_iterator
			 jt = statusAttList->values().begin() ; jt != statusAttList->values().end() ; ++jt)
		{
			switch ((*jt)->type())
			{
			case IMAPParser::status_att_val::MESSAGES:
			{
				const unsigned int count =
					static_cast <unsigned int>((*jt)->value_as_number()->value());

				if (m_count != count)
				{
					m_count = count;
					changed = true;
				}

				break;
			}
			case IMAPParser::status_att_val::UNSEEN:
			{
				const unsigned int unseen =
					static_cast <unsigned int>((*jt)->value_as_number()->value());

				if (m_unseen != unseen)
				{
					m_unseen = unseen;
					changed = true;
				}

				break;
			}
			case IMAPParser::status_att_val::RECENT:
			{
				const unsigned int recent =
					static_cast <unsigned int>((*jt)->value_as_number()->value());

				if (m_recent != recent)
				{
					m_recent = recent;
					changed = true;
				}

				break;
			}
			case IMAPParser::status_att_val::UIDNEXT:
			{
				const vmime_uint32 uidNext =
					static_cast <vmime_uint32>((*jt)->value_as_number()->value());

				if (m_uidNext != uidNext)
				{
					m_uidNext = uidNext;
					changed = true;
				}

				break;
			}
			case IMAPParser::status_att_val::UIDVALIDITY:
			{
				const vmime_uint32 uidValidity =
					static_cast <vmime_uint32>((*jt)->value_as_number()->value());

				if (m_uidValidity != uidValidity)
				{
					m_uidValidity = uidValidity;
					changed = true;
				}

				break;
			}
			case IMAPParser::status_att_val::HIGHESTMODSEQ:
			{
				const vmime_uint64 highestModSeq =
					static_cast <vmime_uint64>((*jt)->value_as_mod_sequence_value()->value());

				if (m_highestModSeq != highestModSeq)
				{
					m_highestModSeq = highestModSeq;
					changed = true;
				}

				break;
			}

			}
		}
	}
	else if (resp->type() == IMAPParser::mailbox_data::EXISTS)
	{
		const unsigned int count =
			static_cast <unsigned int>(resp->number()->value());

		if (m_count != count)
		{
			m_count = count;
			changed = true;
		}
	}
	else if (resp->type() == IMAPParser::mailbox_data::RECENT)
	{
		const unsigned int recent =
			static_cast <unsigned int>(resp->number()->value());

		if (m_recent != recent)
		{
			m_recent = recent;
			changed = true;
		}
	}

	return changed;
}


bool IMAPFolderStatus::updateFromResponse(const IMAPParser::resp_text_code* resp)
{
	bool changed = false;

	switch (resp->type())
	{
	case IMAPParser::resp_text_code::UIDVALIDITY:
	{
		const vmime_uint32 uidValidity =
			static_cast <vmime_uint32>(resp->nz_number()->value());

		if (m_uidValidity != uidValidity)
		{
			m_uidValidity = uidValidity;
			changed = true;
		}

		break;
	}
	case IMAPParser::resp_text_code::UIDNEXT:
	{
		const vmime_uint32 uidNext =
			static_cast <vmime_uint32>(resp->nz_number()->value());

		if (m_uidNext != uidNext)
		{
			m_uidNext = uidNext;
			changed = true;
		}

		break;
	}
	case IMAPParser::resp_text_code::UNSEEN:
	{
		const unsigned int unseen =
			static_cast <unsigned int>(resp->nz_number()->value());

		if (m_unseen != unseen)
		{
			m_unseen = unseen;
			changed = true;
		}

		break;
	}
	case IMAPParser::resp_text_code::HIGHESTMODSEQ:
	{
		const vmime_uint64 highestModSeq =
			static_cast <vmime_uint64>(resp->mod_sequence_value()->value());

		if (m_highestModSeq != highestModSeq)
		{
			m_highestModSeq = highestModSeq;
			changed = true;
		}

		break;
	}
	case IMAPParser::resp_text_code::NOMODSEQ:
	{
		if (m_highestModSeq != 0)
		{
			m_highestModSeq = 0;
			changed = true;
		}

		break;
	}
	default:

		break;
	}

	return changed;
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP
