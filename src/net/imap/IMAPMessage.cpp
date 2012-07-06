//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/net/imap/IMAPParser.hpp"
#include "vmime/net/imap/IMAPMessage.hpp"
#include "vmime/net/imap/IMAPFolder.hpp"
#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/imap/IMAPStructure.hpp"
#include "vmime/net/imap/IMAPPart.hpp"
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


IMAPMessage::IMAPMessage(ref <IMAPFolder> folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_flags(FLAG_UNDEFINED),
	  m_expunged(false), m_structure(NULL)
{
	folder->registerMessage(this);
}


IMAPMessage::~IMAPMessage()
{
	ref <IMAPFolder> folder = m_folder.acquire();

	if (folder)
		folder->unregisterMessage(this);
}


void IMAPMessage::onFolderClosed()
{
	m_folder = NULL;
}


int IMAPMessage::getNumber() const
{
	return (m_num);
}


const message::uid IMAPMessage::getUniqueId() const
{
	return (m_uid);
}


int IMAPMessage::getSize() const
{
	if (m_size == -1)
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


ref <const structure> IMAPMessage::getStructure() const
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


ref <structure> IMAPMessage::getStructure()
{
	if (m_structure == NULL)
		throw exceptions::unfetched_object();

	return m_structure;
}


ref <const header> IMAPMessage::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (m_header);
}


void IMAPMessage::extract(utility::outputStream& os, utility::progressListener* progress,
                          const int start, const int length, const bool peek) const
{
	ref <const IMAPFolder> folder = m_folder.acquire();

	if (!folder)
		throw exceptions::folder_not_found();

	extract(NULL, os, progress, start, length, false, peek);
}


void IMAPMessage::extractPart
	(ref <const part> p, utility::outputStream& os, utility::progressListener* progress,
	 const int start, const int length, const bool peek) const
{
	ref <const IMAPFolder> folder = m_folder.acquire();

	if (!folder)
		throw exceptions::folder_not_found();

	extract(p, os, progress, start, length, false, peek);
}


void IMAPMessage::fetchPartHeader(ref <part> p)
{
	ref <IMAPFolder> folder = m_folder.acquire();

	if (!folder)
		throw exceptions::folder_not_found();

	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	extract(p, ossAdapter, NULL, 0, -1, true, true);

	p.dynamicCast <IMAPPart>()->getOrCreateHeader().parse(oss.str());
}


void IMAPMessage::fetchPartHeaderForStructure(ref <structure> str)
{
	for (int i = 0, n = str->getPartCount() ; i < n ; ++i)
	{
		ref <class part> part = str->getPartAt(i);

		// Fetch header of current part
		fetchPartHeader(part);

		// Fetch header of sub-parts
		fetchPartHeaderForStructure(part->getStructure());
	}
}


void IMAPMessage::extract(ref <const part> p, utility::outputStream& os,
	utility::progressListener* progress, const int start,
	const int length, const bool headerOnly, const bool peek) const
{
	ref <const IMAPFolder> folder = m_folder.acquire();

	IMAPMessage_literalHandler literalHandler(os, progress);

	// Construct section identifier
	std::ostringstream section;
	section.imbue(std::locale::classic());

	if (p != NULL)
	{
		ref <const IMAPPart> currentPart = p.dynamicCast <const IMAPPart>();
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

	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	command << "FETCH " << m_num << " BODY";
	if (peek) command << ".PEEK";
	command << "[";

	if (section.str().empty())
	{
		if (headerOnly)
			command << "HEADER";
	}
	else
	{
		command << section.str();
		if (headerOnly) command << ".MIME";   // "MIME" not "HEADER" for parts
	}

	command << "]";

	if (start != 0 || length != -1)
		command << "<" << start << "." << length << ">";

	// Send the request
	folder.constCast <IMAPFolder>()->m_connection->send(true, command.str(), true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp
		(folder.constCast <IMAPFolder>()->m_connection->readResponse(&literalHandler));

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("FETCH",
			folder.constCast <IMAPFolder>()->m_connection->getParser()->lastLine(), "bad response");
	}


	if (!headerOnly)
	{
		// TODO: update the flags (eg. flag "\Seen" may have been set)
	}
}


void IMAPMessage::fetch(ref <IMAPFolder> msgFolder, const int options)
{
	ref <IMAPFolder> folder = m_folder.acquire();

	if (folder != msgFolder)
		throw exceptions::folder_not_found();

	// Send the request
	std::vector <int> list;
	list.push_back(m_num);

	const string command = IMAPUtils::buildFetchRequest(list, options);

	folder->m_connection->send(true, command, true);

	// Get the response
	utility::auto_ptr <IMAPParser::response> resp(folder->m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
		resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("FETCH",
			folder->m_connection->getParser()->lastLine(), "bad response");
	}

	const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
		resp->continue_req_or_response_data();

	for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
	     it = respDataList.begin() ; it != respDataList.end() ; ++it)
	{
		if ((*it)->response_data() == NULL)
		{
			throw exceptions::command_error("FETCH",
				folder->m_connection->getParser()->lastLine(), "invalid response");
		}

		const IMAPParser::message_data* messageData =
			(*it)->response_data()->message_data();

		// We are only interested in responses of type "FETCH"
		if (messageData == NULL || messageData->type() != IMAPParser::message_data::FETCH)
			continue;

		if (static_cast <int>(messageData->number()) != m_num)
			continue;

		// Process fetch response for this message
		processFetchResponse(options, messageData->msg_att());
	}
}


void IMAPMessage::processFetchResponse
	(const int options, const IMAPParser::msg_att* msgAtt)
{
	ref <IMAPFolder> folder = m_folder.acquire();

	// Get message attributes
	const std::vector <IMAPParser::msg_att_item*> atts =
		msgAtt->items();

	int flags = 0;

	for (std::vector <IMAPParser::msg_att_item*>::const_iterator
	     it = atts.begin() ; it != atts.end() ; ++it)
	{
		switch ((*it)->type())
		{
		case IMAPParser::msg_att_item::FLAGS:
		{
			flags |= IMAPUtils::messageFlagsFromFlags((*it)->flag_list());
			break;
		}
		case IMAPParser::msg_att_item::UID:
		{
			std::ostringstream oss;
			oss.imbue(std::locale::classic());

			oss << folder->m_uidValidity << ":" << (*it)->unique_id()->value();

			m_uid = oss.str();
			break;
		}
		case IMAPParser::msg_att_item::ENVELOPE:
		{
			if (!(options & folder::FETCH_FULL_HEADER))
			{
				const IMAPParser::envelope* env = (*it)->envelope();
				ref <vmime::header> hdr = getOrCreateHeader();

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

				hdr->To()->setValue(to);

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
					hdr->Cc()->setValue(cc);

				// Bcc
				mailboxList bcc;
				IMAPUtils::convertAddressList(*(env->env_bcc()), bcc);

				if (!bcc.isEmpty())
					hdr->Bcc()->setValue(bcc);
			}

			break;
		}
		case IMAPParser::msg_att_item::BODY_STRUCTURE:
		{
			m_structure = vmime::create <IMAPStructure>((*it)->body());
			break;
		}
		case IMAPParser::msg_att_item::RFC822_HEADER:
		{
			getOrCreateHeader()->parse((*it)->nstring()->value());
			break;
		}
		case IMAPParser::msg_att_item::RFC822_SIZE:
		{
			m_size = (*it)->number()->value();
			break;
		}
		case IMAPParser::msg_att_item::BODY_SECTION:
		{
			if (!(options & folder::FETCH_FULL_HEADER))
			{
				if ((*it)->section()->section_text1() &&
				    (*it)->section()->section_text1()->type()
				        == IMAPParser::section_text::HEADER_FIELDS)
				{
					header tempHeader;
					tempHeader.parse((*it)->nstring()->value());

					vmime::header& hdr = *getOrCreateHeader();
					std::vector <ref <headerField> > fields = tempHeader.getFieldList();

					for (std::vector <ref <headerField> >::const_iterator jt = fields.begin() ;
					     jt != fields.end() ; ++jt)
					{
						hdr.appendField((*jt)->clone().dynamicCast <headerField>());
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

	if (options & folder::FETCH_FLAGS)
		m_flags = flags;
}


ref <header> IMAPMessage::getOrCreateHeader()
{
	if (m_header != NULL)
		return (m_header);
	else
		return (m_header = vmime::create <header>());
}


void IMAPMessage::setFlags(const int flags, const int mode)
{
	ref <IMAPFolder> folder = m_folder.acquire();

	if (!folder)
		throw exceptions::folder_not_found();
	else if (folder->m_mode == folder::MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	// Build the request text
	std::ostringstream command;
	command.imbue(std::locale::classic());

	command << "STORE " << m_num;

	switch (mode)
	{
	case FLAG_MODE_ADD:    command << " +FLAGS"; break;
	case FLAG_MODE_REMOVE: command << " -FLAGS"; break;
	default:
	case FLAG_MODE_SET:    command << " FLAGS"; break;
	}

	if (m_flags == FLAG_UNDEFINED)   // Update local flags only if they
		command << ".SILENT ";      // have been fetched previously
	else
		command << " ";

	std::vector <string> flagList;

	if (flags & FLAG_REPLIED) flagList.push_back("\\Answered");
	if (flags & FLAG_MARKED) flagList.push_back("\\Flagged");
	if (flags & FLAG_DELETED) flagList.push_back("\\Deleted");
	if (flags & FLAG_SEEN) flagList.push_back("\\Seen");
	if (flags & FLAG_DRAFT) flagList.push_back("\\Draft");

	if (!flagList.empty())
	{
		command << "(";

		if (flagList.size() >= 2)
		{
			std::copy(flagList.begin(), flagList.end() - 1,
			          std::ostream_iterator <string>(command, " "));
		}

		command << *(flagList.end() - 1) << ")";

		// Send the request
		folder->m_connection->send(true, command.str(), true);

		// Get the response
		utility::auto_ptr <IMAPParser::response> resp(folder->m_connection->readResponse());

		if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
		{
			throw exceptions::command_error("STORE",
				folder->m_connection->getParser()->lastLine(), "bad response");
		}

		// Update the local flags for this message
		if (m_flags != FLAG_UNDEFINED)
		{
			const std::vector <IMAPParser::continue_req_or_response_data*>& respDataList =
				resp->continue_req_or_response_data();

			int newFlags = 0;

			for (std::vector <IMAPParser::continue_req_or_response_data*>::const_iterator
			     it = respDataList.begin() ; it != respDataList.end() ; ++it)
			{
				if ((*it)->response_data() == NULL)
					continue;

				const IMAPParser::message_data* messageData =
					(*it)->response_data()->message_data();

				// We are only interested in responses of type "FETCH"
				if (messageData == NULL || messageData->type() != IMAPParser::message_data::FETCH)
					continue;

				// Get message attributes
				const std::vector <IMAPParser::msg_att_item*> atts =
					messageData->msg_att()->items();

				for (std::vector <IMAPParser::msg_att_item*>::const_iterator
				     it = atts.begin() ; it != atts.end() ; ++it)
				{
					if ((*it)->type() == IMAPParser::msg_att_item::FLAGS)
						newFlags |= IMAPUtils::messageFlagsFromFlags((*it)->flag_list());
				}
			}

			m_flags = newFlags;
		}

		// Notify message flags changed
		std::vector <int> nums;
		nums.push_back(m_num);

		events::messageChangedEvent event
			(folder, events::messageChangedEvent::TYPE_FLAGS, nums);

		for (std::list <IMAPFolder*>::iterator it = folder->m_store.acquire()->m_folders.begin() ;
		     it != folder->m_store.acquire()->m_folders.end() ; ++it)
		{
			if ((*it)->getFullPath() == folder->m_path)
				(*it)->notifyMessageChanged(event);
		}
	}
}


void IMAPMessage::constructParsedMessage(ref <bodyPart> parentPart, ref <structure> str, int level)
{
	if (level == 0)
	{
		ref <class part> part = str->getPartAt(0);

		// Copy header
		ref <const header> hdr = part->getHeader();
		parentPart->getHeader()->copyFrom(*hdr);

		// Initialize body
		parentPart->getBody()->setContents
			(vmime::create <IMAPMessagePartContentHandler>
				(thisRef().dynamicCast <IMAPMessage>(),
				 part, parentPart->getBody()->getEncoding()));

		constructParsedMessage(parentPart, part->getStructure(), 1);
	}
	else
	{
		for (int i = 0, n = str->getPartCount() ; i < n ; ++i)
		{
			ref <class part> part = str->getPartAt(i);

			ref <bodyPart> childPart = vmime::create <bodyPart>();

			// Copy header
			ref <const header> hdr = part->getHeader();
			childPart->getHeader()->copyFrom(*hdr);

			// Initialize body
			childPart->getBody()->setContents
				(vmime::create <IMAPMessagePartContentHandler>
					(thisRef().dynamicCast <IMAPMessage>(),
					 part, childPart->getBody()->getEncoding()));

			// Add child part
			parentPart->getBody()->appendPart(childPart);

			// Construct sub parts
			constructParsedMessage(childPart, part->getStructure(), ++level);
		}
	}
}


ref <vmime::message> IMAPMessage::getParsedMessage()
{
	// Fetch structure
	ref <structure> structure = NULL;

	try
	{
		structure = getStructure();
	}
	catch (exceptions::unfetched_object&)
	{
		fetch(m_folder.acquire(), IMAPFolder::FETCH_STRUCTURE);
		structure = getStructure();
	}

	// Fetch header for each part
	fetchPartHeaderForStructure(structure);

	// Construct message from structure
	ref <vmime::message> msg = vmime::create <vmime::message>();

	constructParsedMessage(msg, structure);

	return msg;
}


} // imap
} // net
} // vmime

