//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2006 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/net/imap/IMAPParser.hpp"
#include "vmime/net/imap/IMAPMessage.hpp"
#include "vmime/net/imap/IMAPFolder.hpp"
#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPUtils.hpp"

#include <sstream>
#include <iterator>


namespace vmime {
namespace net {
namespace imap {


//
// IMAPpart
//

class IMAPstructure;

class IMAPpart : public part
{
private:

	friend class vmime::creator;

	IMAPpart(ref <IMAPpart> parent, const int number, const IMAPParser::body_type_mpart* mpart);
	IMAPpart(ref <IMAPpart> parent, const int number, const IMAPParser::body_type_1part* part);

public:

	ref <const structure> getStructure() const;
	ref <structure> getStructure();

	ref <const IMAPpart> getParent() const { return m_parent.acquire(); }

	const mediaType& getType() const { return (m_mediaType); }
	const int getSize() const { return (m_size); }
	const int getNumber() const { return (m_number); }

	ref <const header> getHeader() const
	{
		if (m_header == NULL)
			throw exceptions::unfetched_object();
		else
			return m_header;
	}


	static ref <IMAPpart> create
		(ref <IMAPpart> parent, const int number, const IMAPParser::body* body)
	{
		if (body->body_type_mpart())
		{
			ref <IMAPpart> part = vmime::create <IMAPpart>(parent, number, body->body_type_mpart());
			part->m_structure = vmime::create <IMAPstructure>(part, body->body_type_mpart()->list());

			return part;
		}
		else
		{
			return vmime::create <IMAPpart>(parent, number, body->body_type_1part());
		}
	}


	header& getOrCreateHeader()
	{
		if (m_header != NULL)
			return (*m_header);
		else
			return (*(m_header = vmime::create <header>()));
	}

private:

	ref <IMAPstructure> m_structure;
	weak_ref <IMAPpart> m_parent;
	ref <header> m_header;

	int m_number;
	int m_size;
	mediaType m_mediaType;
};



//
// IMAPstructure
//

class IMAPstructure : public structure
{
public:

	IMAPstructure()
	{
	}

	IMAPstructure(const IMAPParser::body* body)
	{
		m_parts.push_back(IMAPpart::create(NULL, 0, body));
	}

	IMAPstructure(ref <IMAPpart> parent, const std::vector <IMAPParser::body*>& list)
	{
		int number = 0;

		for (std::vector <IMAPParser::body*>::const_iterator
		     it = list.begin() ; it != list.end() ; ++it, ++number)
		{
			m_parts.push_back(IMAPpart::create(parent, number, *it));
		}
	}


	ref <const part> getPartAt(const int x) const
	{
		return m_parts[x];
	}

	ref <part> getPartAt(const int x)
	{
		return m_parts[x];
	}

	const int getPartCount() const
	{
		return m_parts.size();
	}


	static ref <IMAPstructure> emptyStructure()
	{
		return (m_emptyStructure);
	}

private:

	static ref <IMAPstructure> m_emptyStructure;

	std::vector <ref <IMAPpart> > m_parts;
};


ref <IMAPstructure> IMAPstructure::m_emptyStructure = vmime::create <IMAPstructure>();



IMAPpart::IMAPpart(ref <IMAPpart> parent, const int number, const IMAPParser::body_type_mpart* mpart)
	: m_parent(parent), m_header(NULL), m_number(number), m_size(0)
{
	m_mediaType = vmime::mediaType
		("multipart", mpart->media_subtype()->value());
}


IMAPpart::IMAPpart(ref <IMAPpart> parent, const int number, const IMAPParser::body_type_1part* part)
	: m_parent(parent), m_header(NULL), m_number(number), m_size(0)
{
	if (part->body_type_text())
	{
		m_mediaType = vmime::mediaType
			("text", part->body_type_text()->
				media_text()->media_subtype()->value());

		m_size = part->body_type_text()->body_fields()->body_fld_octets()->value();
	}
	else if (part->body_type_msg())
	{
		m_mediaType = vmime::mediaType
			("message", part->body_type_msg()->
				media_message()->media_subtype()->value());
	}
	else
	{
		m_mediaType = vmime::mediaType
			(part->body_type_basic()->media_basic()->media_type()->value(),
			 part->body_type_basic()->media_basic()->media_subtype()->value());

		m_size = part->body_type_basic()->body_fields()->body_fld_octets()->value();
	}

	m_structure = NULL;
}


ref <const structure> IMAPpart::getStructure() const
{
	if (m_structure != NULL)
		return (m_structure);
	else
		return (IMAPstructure::emptyStructure());
}


ref <structure> IMAPpart::getStructure()
{
	if (m_structure != NULL)
		return (m_structure);
	else
		return (IMAPstructure::emptyStructure());
}



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


const int IMAPMessage::getNumber() const
{
	return (m_num);
}


const message::uid IMAPMessage::getUniqueId() const
{
	return (m_uid);
}


const int IMAPMessage::getSize() const
{
	if (m_size == -1)
		throw exceptions::unfetched_object();

	return (m_size);
}


const bool IMAPMessage::isExpunged() const
{
	return (m_expunged);
}


const int IMAPMessage::getFlags() const
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

	p.dynamicCast <IMAPpart>()->getOrCreateHeader().parse(oss.str());
}


void IMAPMessage::extract(ref <const part> p, utility::outputStream& os,
	utility::progressListener* progress, const int start,
	const int length, const bool headerOnly, const bool peek) const
{
	ref <const IMAPFolder> folder = m_folder.acquire();

	IMAPMessage_literalHandler literalHandler(os, progress);

	// Construct section identifier
	std::ostringstream section;

	if (p != NULL)
	{
		ref <const IMAPpart> currentPart = p.dynamicCast <const IMAPpart>();
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

	command << "FETCH " << m_num << " BODY";
	if (peek) command << ".PEEK";
	command << "[";
	command << section.str();
	if (headerOnly) command << ".MIME";   // "MIME" not "HEADER" for parts
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
			m_structure = vmime::create <IMAPstructure>((*it)->body());
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


} // imap
} // net
} // vmime
