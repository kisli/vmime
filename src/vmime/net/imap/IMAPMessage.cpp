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


#include "vmime/net/imap/IMAPParser.hpp"
#include "vmime/net/imap/IMAPMessage.hpp"
#include "vmime/net/imap/IMAPFolder.hpp"
#include "vmime/net/imap/IMAPFolderStatus.hpp"
#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/imap/IMAPMessageStructure.hpp"
#include "vmime/net/imap/IMAPMessagePart.hpp"
#include "vmime/net/imap/IMAPMessagePartContentHandler.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"

#include <sstream>
#include <iterator>
#include <typeinfo>


namespace vmime {
namespace net {
namespace imap {


#ifndef VMIME_BUILDING_DOC

//
// IMAPMessage_literalHandler
//

class IMAPMessage_literalHandler : public IMAPParser::literalHandler
{
public:

	IMAPMessage_literalHandler(utility::outputStream& os, utility::progressListener* progress)
		: m_os(os), m_progress(progress)
	{
	}

	target* targetFor(const IMAPParser::component& comp, const int /* data */)
	{
		if (typeid(comp) == typeid(IMAPParser::msg_att_item))
		{
			const int type = static_cast
				<const IMAPParser::msg_att_item&>(comp).type();

			if (type == IMAPParser::msg_att_item::BODY_SECTION ||
			    type == IMAPParser::msg_att_item::RFC822_TEXT)
			{
				return new targetStream(m_progress, m_os);
			}
		}

		return (NULL);
	}

private:

	utility::outputStream& m_os;
	utility::progressListener* m_progress;
};

#endif // VMIME_BUILDING_DOC



//
// IMAPMessage
//


IMAPMessage::IMAPMessage(shared_ptr <IMAPFolder> folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1U), m_flags(FLAG_UNDEFINED),
	  m_expunged(false), m_modseq(0), m_structure(null)
{
	folder->registerMessage(this);
}


IMAPMessage::IMAPMessage(shared_ptr <IMAPFolder> folder, const int num, const uid& uid)
	: m_folder(folder), m_num(num), m_size(-1), m_flags(FLAG_UNDEFINED),
	  m_expunged(false), m_uid(uid), m_modseq(0), m_structure(null)
{
	folder->registerMessage(this);
}


IMAPMessage::~IMAPMessage()
{
	shared_ptr <IMAPFolder> folder = m_folder.lock();

	if (folder)
		folder->unregisterMessage(this);
}


void IMAPMessage::onFolderClosed()
{
	m_folder.reset();
}


int IMAPMessage::getNumber() const
{
	return (m_num);
}


const message::uid IMAPMessage::getUID() const
{
	return m_uid;
}


vmime_uint64 IMAPMessage::getModSequence() const
{
	return m_modseq;
}


size_t IMAPMessage::getSize() const
{
	if (m_size == -1U)
		throw exceptions::unfetched_object();

	return (m_size);
}


bool IMAPMessage::isExpunged() const
{
	return (m_expunged);
}


int IMAPMessage::getFlags() const
{
	if (m_flags == FLAG_UNDEFINED)
		throw exceptions::unfetched_object();

	return (m_flags);
}


shared_ptr <const messageStructure> IMAPMessage::getStructure() const
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


shared_ptr <messageStructure> IMAPMessage::getStructure()
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


shared_ptr <const header> IMAPMessage::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (m_header);
}


void IMAPMessage::extract
	(utility::outputStream& os,
	 utility::progressListener* progress,
     const size_t start, const size_t length,
     const bool peek) const
{
	shared_ptr <const IMAPFolder> folder = m_folder.lock();

	if (!folder)
		throw exceptions::folder_not_found();

	extractImpl(null, os, progress, start, length,
		EXTRACT_HEADER | EXTRACT_BODY | (peek ? EXTRACT_PEEK : 0));
}


void IMAPMessage::extractPart
	(shared_ptr <const messagePart> p,
	 utility::outputStream& os,
	 utility::progressListener* progress,
	 const size_t start, const size_t length,
	 const bool peek) const
{
	shared_ptr <const IMAPFolder> folder = m_folder.lock();

	if (!folder)
		throw exceptions::folder_not_found();

	extractImpl(p, os, progress, start, length,
		EXTRACT_HEADER | EXTRACT_BODY | (peek ? EXTRACT_PEEK : 0));
}


void IMAPMessage::fetchPartHeader(shared_ptr <messagePart> p)
{
	shared_ptr <IMAPFolder> folder = m_folder.lock();

	if (!folder)
		throw exceptions::folder_not_found();

	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	extractImpl(p, ossAdapter, NULL, 0, -1, EXTRACT_HEADER | EXTRACT_PEEK);

	dynamicCast <IMAPMessagePart>(p)->getOrCreateHeader().parse(oss.str());
}


void IMAPMessage::fetchPartHeaderForStructure(shared_ptr <messageStructure> str)
{
	for (size_t i = 0, n = str->getPartCount() ; i < n ; ++i)
	{
		shared_ptr <messagePart> part = str->getPartAt(i);

		// Fetch header of current part
		fetchPartHeader(part);

		// Fetch header of sub-parts
		fetchPartHeaderForStructure(part->getStructure());
	}
}


void IMAPMessage::extractImpl
	(shared_ptr <const messagePart> p,
	 utility::outputStream& os,
	 utility::progressListener* progress,
	 const size_t start, const size_t length,
	 const int extractFlags) const
{
	shared_ptr <const IMAPFolder> folder = m_folder.lock();

	IMAPMessage_literalHandler literalHandler(os, progress);

	// Construct section identifier
	std::ostringstream section;
	section.imbue(std::locale::classic());

	if (p != NULL)
	{
		shared_ptr <const IMAPMessagePart> currentPart = dynamicCast <const IMAPMessagePart>(p);
		std::vector <int> numbers;

		numbers.push_back(currentPart->getNumber());
		currentPart = currentPart->getParent();

		while (currentPart != NULL)
		{
			numbers.push_back(currentPart->getNumber());
			currentPart = currentPart->getParent();
		}

		numbers.erase(numbers.end() - 1);

		for (std::vector <int>::reverse_iterator it = numbers.rbegin() ; it != numbers.rend() ; ++it)
		{
			if (it != numbers.rbegin()) section << ".";
			section << (*it + 1);
		}
	}

	// Build the body descriptor for FETCH
	/*
	   BODY[]               header + body
	   BODY.PEEK[]          header + body (peek)
	   BODY[HEADER]         header
	   BODY.PEEK[HEADER]    header (peek)
	   BODY[TEXT]           body
	   BODY.PEEK[TEXT]      body (peek)
	*/
	std::ostringstream bodyDesc;
	bodyDesc.imbue(std::locale::classic());

	bodyDesc << "BODY";

	if (extractFlags & EXTRACT_PEEK)
		bodyDesc << ".PEEK";

	bodyDesc << "[";

	if (section.str().empty())
	{
		// header + body
		if ((extractFlags & EXTRACT_HEADER) && (extractFlags & EXTRACT_BODY))
			bodyDesc << "";
		// body only
		else if (extractFlags & EXTRACT_BODY)
			bodyDesc << "TEXT";
		// header only
		else if (extractFlags & EXTRACT_HEADER)
			bodyDesc << "HEADER";
	}
	else
	{
		bodyDesc << section.str();

		// header + body
		if ((extractFlags & EXTRACT_HEADER) && (extractFlags & EXTRACT_BODY))
			throw exceptions::operation_not_supported();
		// header only
		else if (extractFlags & EXTRACT_HEADER)
			bodyDesc << ".MIME";   // "MIME" not "HEADER" for parts
	}

	bodyDesc << "]";

	if (start != 0 || length != static_cast <size_t>(-1))
		bodyDesc << "<" << start << "." << length << ">";

	std::vector <std::string> fetchParams;
	fetchParams.push_back(bodyDesc.str());

	// Send the request
	IMAPCommand::FETCH(
		m_uid.empty() ? messageSet::byNumber(m_num) : messageSet::byUID(m_uid),
		fetchParams
	)->send(constCast <IMAPFolder>(folder)->m_connection);

	// Get the response
	std::auto_ptr <IMAPParser::response> resp
		(constCast <IMAPFolder>(folder)->m_connection->readResponse(&literalHandler));

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("FETCH",
			resp->getErrorLog(), "bad response");
	}


	if (extractFlags & EXTRACT_BODY)
	{
		// TODO: update the flags (eg. flag "\Seen" may have been set)
	}
}


int IMAPMessage::processFetchResponse
	(const fetchAttributes& options, const IMAPParser::message_data* msgData)
{
	shared_ptr <IMAPFolder> folder = m_folder.lock();

	// Get message attributes
	const std::vector <IMAPParser::msg_att_item*> atts = msgData->msg_att()->items();
	int changes = 0;

	for (std::vector <IMAPParser::msg_att_item*>::const_iterator
	     it = atts.begin() ; it != atts.end() ; ++it)
	{
		switch ((*it)->type())
		{
		case IMAPParser::msg_att_item::FLAGS:
		{
			int flags = IMAPUtils::messageFlagsFromFlags((*it)->flag_list());

			if (m_flags != flags)
			{
				m_flags = flags;
				changes |= events::messageChangedEvent::TYPE_FLAGS;
			}

			break;
		}
		case IMAPParser::msg_att_item::UID:
		{
			m_uid = (*it)->unique_id()->value();
			break;
		}
		case IMAPParser::msg_att_item::MODSEQ:
		{
			m_modseq = (*it)->mod_sequence_value()->value();
			break;
		}
		case IMAPParser::msg_att_item::ENVELOPE:
		{
			if (!options.has(fetchAttributes::FULL_HEADER))
			{
				const IMAPParser::envelope* env = (*it)->envelope();
				shared_ptr <vmime::header> hdr = getOrCreateHeader();

				// Date
				hdr->Date()->setValue(env->env_date()->value());

				// Subject
				text subject;
				text::decodeAndUnfold(env->env_subject()->value(), &subject);

				hdr->Subject()->setValue(subject);

				// From
				mailboxList from;
				IMAPUtils::convertAddressList(*(env->env_from()), from);

				if (!from.isEmpty())
					hdr->From()->setValue(*(from.getMailboxAt(0)));

				// To
				mailboxList to;
				IMAPUtils::convertAddressList(*(env->env_to()), to);

				hdr->To()->setValue(to.toAddressList());

				// Sender
				mailboxList sender;
				IMAPUtils::convertAddressList(*(env->env_sender()), sender);

				if (!sender.isEmpty())
					hdr->Sender()->setValue(*(sender.getMailboxAt(0)));

				// Reply-to
				mailboxList replyTo;
				IMAPUtils::convertAddressList(*(env->env_reply_to()), replyTo);

				if (!replyTo.isEmpty())
					hdr->ReplyTo()->setValue(*(replyTo.getMailboxAt(0)));

				// Cc
				mailboxList cc;
				IMAPUtils::convertAddressList(*(env->env_cc()), cc);

				if (!cc.isEmpty())
					hdr->Cc()->setValue(cc.toAddressList());

				// Bcc
				mailboxList bcc;
				IMAPUtils::convertAddressList(*(env->env_bcc()), bcc);

				if (!bcc.isEmpty())
					hdr->Bcc()->setValue(bcc.toAddressList());
			}

			break;
		}
		case IMAPParser::msg_att_item::BODY_STRUCTURE:
		{
			m_structure = make_shared <IMAPMessageStructure>((*it)->body());
			break;
		}
		case IMAPParser::msg_att_item::RFC822_HEADER:
		{
			getOrCreateHeader()->parse((*it)->nstring()->value());
			break;
		}
		case IMAPParser::msg_att_item::RFC822_SIZE:
		{
			m_size = static_cast <size_t>((*it)->number()->value());
			break;
		}
		case IMAPParser::msg_att_item::BODY_SECTION:
		{
			if (!options.has(fetchAttributes::FULL_HEADER))
			{
				if ((*it)->section()->section_text1() &&
				    (*it)->section()->section_text1()->type()
				        == IMAPParser::section_text::HEADER_FIELDS)
				{
					header tempHeader;
					tempHeader.parse((*it)->nstring()->value());

					vmime::header& hdr = *getOrCreateHeader();
					std::vector <shared_ptr <headerField> > fields = tempHeader.getFieldList();

					for (std::vector <shared_ptr <headerField> >::const_iterator jt = fields.begin() ;
					     jt != fields.end() ; ++jt)
					{
						hdr.appendField(vmime::clone(*jt));
					}
				}
			}

			break;
		}
		case IMAPParser::msg_att_item::INTERNALDATE:
		case IMAPParser::msg_att_item::RFC822:
		case IMAPParser::msg_att_item::RFC822_TEXT:
		case IMAPParser::msg_att_item::BODY:
		{
			break;
		}

		}
	}

	return changes;
}


shared_ptr <header> IMAPMessage::getOrCreateHeader()
{
	if (m_header != NULL)
		return (m_header);
	else
		return (m_header = make_shared <header>());
}


void IMAPMessage::setFlags(const int flags, const int mode)
{
	shared_ptr <IMAPFolder> folder = m_folder.lock();

	if (!folder)
		throw exceptions::folder_not_found();

	if (!m_uid.empty())
		folder->setMessageFlags(messageSet::byUID(m_uid), flags, mode);
	else
		folder->setMessageFlags(messageSet::byNumber(m_num), flags, mode);
}


void IMAPMessage::constructParsedMessage
	(shared_ptr <bodyPart> parentPart, shared_ptr <messageStructure> str, int level)
{
	if (level == 0)
	{
		shared_ptr <messagePart> part = str->getPartAt(0);

		// Copy header
		shared_ptr <const header> hdr = part->getHeader();
		parentPart->getHeader()->copyFrom(*hdr);

		// Initialize body
		parentPart->getBody()->setContents
			(make_shared <IMAPMessagePartContentHandler>
				(dynamicCast <IMAPMessage>(shared_from_this()),
				 part, parentPart->getBody()->getEncoding()));

		constructParsedMessage(parentPart, part->getStructure(), 1);
	}
	else
	{
		for (size_t i = 0, n = str->getPartCount() ; i < n ; ++i)
		{
			shared_ptr <messagePart> part = str->getPartAt(i);

			shared_ptr <bodyPart> childPart = make_shared <bodyPart>();

			// Copy header
			shared_ptr <const header> hdr = part->getHeader();
			childPart->getHeader()->copyFrom(*hdr);

			// Initialize body
			childPart->getBody()->setContents
				(make_shared <IMAPMessagePartContentHandler>
					(dynamicCast <IMAPMessage>(shared_from_this()),
					 part, childPart->getBody()->getEncoding()));

			// Add child part
			parentPart->getBody()->appendPart(childPart);

			// Construct sub parts
			constructParsedMessage(childPart, part->getStructure(), ++level);
		}
	}
}


shared_ptr <vmime::message> IMAPMessage::getParsedMessage()
{
	// Fetch structure
	shared_ptr <messageStructure> structure;

	try
	{
		structure = getStructure();
	}
	catch (exceptions::unfetched_object&)
	{
		std::vector <shared_ptr <message> > msgs;
		msgs.push_back(dynamicCast <IMAPMessage>(shared_from_this()));

		m_folder.lock()->fetchMessages
			(msgs, fetchAttributes(fetchAttributes::STRUCTURE), /* progress */ NULL);

		structure = getStructure();
	}

	// Fetch header for each part
	fetchPartHeaderForStructure(structure);

	// Construct message from structure
	shared_ptr <vmime::message> msg = make_shared <vmime::message>();

	constructParsedMessage(msg, structure);

	return msg;
}


void IMAPMessage::renumber(const int number)
{
	m_num = number;
}


void IMAPMessage::setExpunged()
{
	m_expunged = true;
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

