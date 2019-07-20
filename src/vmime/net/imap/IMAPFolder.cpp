//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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


#include "vmime/net/imap/IMAPFolder.hpp"

#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPParser.hpp"
#include "vmime/net/imap/IMAPMessage.hpp"
#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPFolderStatus.hpp"
#include "vmime/net/imap/IMAPCommand.hpp"

#include "vmime/message.hpp"

#include "vmime/exception.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"

#include <algorithm>
#include <sstream>


namespace vmime {
namespace net {
namespace imap {


IMAPFolder::IMAPFolder(
	const folder::path& path,
	const shared_ptr <IMAPStore>& store,
	const shared_ptr <folderAttributes>& attribs
)
	: m_store(store),
	  m_connection(store->connection()),
	  m_path(path),
	  m_name(path.isEmpty() ? folder::path::component("") : path.getLastComponent()),
	  m_mode(-1),
	  m_open(false),
	  m_attribs(attribs) {

	store->registerFolder(this);

	m_status = make_shared <IMAPFolderStatus>();
}


IMAPFolder::~IMAPFolder() {

	try {

		shared_ptr <IMAPStore> store = m_store.lock();

		if (store) {

			if (m_open) {
				close(false);
			}

			store->unregisterFolder(this);

		} else if (m_open) {

			m_connection = null;
			onClose();
		}

	} catch (...) {

		// Don't throw in destructor
	}
}


int IMAPFolder::getMode() const {

	if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	return m_mode;
}


const folderAttributes IMAPFolder::getAttributes() {

	// Root folder
	if (m_path.isEmpty()) {

		folderAttributes attribs;
		attribs.setType(folderAttributes::TYPE_CONTAINS_FOLDERS);
		attribs.setFlags(folderAttributes::FLAG_HAS_CHILDREN | folderAttributes::FLAG_NO_OPEN);

		return attribs;

	} else {

		if (!m_attribs) {
			testExistAndGetType();
		}

		return *m_attribs;
	}
}


const folder::path::component IMAPFolder::getName() const {

	return m_name;
}


const folder::path IMAPFolder::getFullPath() const {

	return m_path;
}


void IMAPFolder::open(const int mode, bool failIfModeIsNotAvailable) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	// Ensure this folder is not already open in the same session
	for (std::list <IMAPFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it) {

		if ((*it) != this && (*it)->getFullPath() == m_path) {
			throw exceptions::folder_already_open();
		}
	}

	// Open a connection for this folder
	shared_ptr <IMAPConnection> connection =
		make_shared <IMAPConnection>(store, store->getAuthenticator());

	try {

		connection->connect();

		// Emit the "SELECT" command
		//
		// Example:  C: A142 SELECT INBOX
		//           S: * 172 EXISTS
		//           S: * 1 RECENT
		//           S: * OK [UNSEEN 12] Message 12 is first unseen
		//           S: * OK [UIDVALIDITY 3857529045] UIDs valid
		//           S: * FLAGS (\Answered \Flagged \Deleted \Seen \Draft)
		//           S: * OK [PERMANENTFLAGS (\Deleted \Seen \*)] Limited
		//           S: A142 OK [READ-WRITE] SELECT completed

		std::vector <string> selectParams;

		if (m_connection->hasCapability("CONDSTORE")) {
			selectParams.push_back("CONDSTORE");
		}

		IMAPCommand::SELECT(
			mode == MODE_READ_ONLY,
			IMAPUtils::pathToString(connection->hierarchySeparator(), getFullPath()),
			selectParams
		)->send(connection);

		// Read the response
		scoped_ptr <IMAPParser::response> resp(connection->readResponse());

		if (resp->isBad() || resp->response_done->response_tagged->
				resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

			throw exceptions::command_error("SELECT", resp->getErrorLog(), "bad response");
		}

		auto &respDataList = resp->continue_req_or_response_data;

		for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

			auto *responseData = (*it)->response_data.get();

			if (!responseData) {
				throw exceptions::command_error("SELECT", resp->getErrorLog(), "invalid response");
			}

			// OK Untagged responses: UNSEEN, PERMANENTFLAGS, UIDVALIDITY (optional)
			if (responseData->resp_cond_state) {

				auto *code = responseData->resp_cond_state->resp_text->resp_text_code.get();

				if (code) {

					switch (code->type) {

						case IMAPParser::resp_text_code::NOMODSEQ:

							connection->disableMODSEQ();
							break;

						default:

							break;
					}
				}

			// Untagged responses: FLAGS, EXISTS, RECENT (required)
			} else if (responseData->mailbox_data) {

				switch (responseData->mailbox_data->type) {

					default: break;

					case IMAPParser::mailbox_data::FLAGS: {

						if (!m_attribs) {
							m_attribs = make_shared <folderAttributes>();
						}

						IMAPUtils::mailboxFlagsToFolderAttributes(
							connection,
							*responseData->mailbox_data->mailbox_flag_list,
							*m_attribs
						);

						break;
					}
				}
			}
		}

		processStatusUpdate(resp.get());

		// Check for access mode (read-only or read-write)
		auto *respTextCode = resp->response_done->response_tagged->resp_cond_state->resp_text->resp_text_code.get();

		if (respTextCode) {

			const int openMode =
				(respTextCode->type == IMAPParser::resp_text_code::READ_WRITE)
					? MODE_READ_WRITE
					: MODE_READ_ONLY;

			if (failIfModeIsNotAvailable &&
			    mode == MODE_READ_WRITE && openMode == MODE_READ_ONLY) {

				throw exceptions::operation_not_supported();
			}
		}

		m_connection = connection;
		m_open = true;
		m_mode = mode;

	} catch (std::exception&) {

		throw;
	}
}


void IMAPFolder::close(const bool expunge) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	shared_ptr <IMAPConnection> oldConnection = m_connection;

	// Emit the "CLOSE" command to expunge messages marked
	// as deleted (this is fastest than "EXPUNGE")
	if (expunge) {

		if (m_mode == MODE_READ_ONLY) {
			throw exceptions::operation_not_supported();
		}

		IMAPCommand::CLOSE()->send(oldConnection);
	}

	// Close this folder connection
	oldConnection->disconnect();

	// Now use default store connection
	m_connection = m_store.lock()->connection();

	m_open = false;
	m_mode = -1;

	m_status = make_shared <IMAPFolderStatus>();

	onClose();
}


void IMAPFolder::onClose() {

	for (std::vector <IMAPMessage*>::iterator it = m_messages.begin() ;
	     it != m_messages.end() ; ++it) {

		(*it)->onFolderClosed();
	}

	m_messages.clear();
}


void IMAPFolder::create(const folderAttributes& attribs) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (isOpen()) {
		throw exceptions::illegal_state("Folder is open");
	} else if (exists()) {
		throw exceptions::illegal_state("Folder already exists");
	} else if (!store->isValidFolderName(m_name)) {
		throw exceptions::invalid_folder_name();
	}

	// Emit the "CREATE" command
	//
	// Example:   C: A003 CREATE owatagusiam/
	//            S: A003 OK CREATE completed
	//            C: A004 CREATE owatagusiam/blurdybloop
	//            S: A004 OK CREATE completed

	string mailbox = IMAPUtils::pathToString
		(m_connection->hierarchySeparator(), getFullPath());

	if (attribs.getType() & folderAttributes::TYPE_CONTAINS_FOLDERS) {
		mailbox += m_connection->hierarchySeparator();
	}

	std::vector <string> createParams;

	if (attribs.getSpecialUse() != folderAttributes::SPECIALUSE_NONE) {

		if (!m_connection->hasCapability("CREATE-SPECIAL-USE")) {
			throw exceptions::operation_not_supported();
		}

		// C: t2 CREATE MySpecial (USE (\Drafts \Sent))
		std::ostringstream oss;
		oss << "USE (";

		switch (attribs.getSpecialUse()) {

			case folderAttributes::SPECIALUSE_NONE:      // should not happen
			case folderAttributes::SPECIALUSE_ALL:       oss << "\\All"; break;
			case folderAttributes::SPECIALUSE_ARCHIVE:   oss << "\\Archive"; break;
			case folderAttributes::SPECIALUSE_DRAFTS:    oss << "\\Drafts"; break;
			case folderAttributes::SPECIALUSE_FLAGGED:   oss << "\\Flagged"; break;
			case folderAttributes::SPECIALUSE_JUNK:      oss << "\\Junk"; break;
			case folderAttributes::SPECIALUSE_SENT:      oss << "\\Sent"; break;
			case folderAttributes::SPECIALUSE_TRASH:     oss << "\\Trash"; break;
			case folderAttributes::SPECIALUSE_IMPORTANT: oss << "\\Important"; break;
		}

		oss << ")";

		createParams.push_back(oss.str());
	}

	IMAPCommand::CREATE(mailbox, createParams)->send(m_connection);


	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("CREATE", resp->getErrorLog(), "bad response");
	}

	// Notify folder created
	shared_ptr <events::folderEvent> event =
		make_shared <events::folderEvent>(
			dynamicCast <folder>(shared_from_this()),
			events::folderEvent::TYPE_CREATED,
			m_path, m_path
		);

	notifyFolder(event);
}


void IMAPFolder::destroy() {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	if (isOpen()) {
		throw exceptions::illegal_state("Folder is open");
	}

	const string mailbox = IMAPUtils::pathToString(
		m_connection->hierarchySeparator(), getFullPath()
	);

	IMAPCommand::DELETE(mailbox)->send(m_connection);


	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("DELETE", resp->getErrorLog(), "bad response");
	}

	// Notify folder deleted
	shared_ptr <events::folderEvent> event =
		make_shared <events::folderEvent>(
			dynamicCast <folder>(shared_from_this()),
			events::folderEvent::TYPE_DELETED,
			m_path, m_path
		);

	notifyFolder(event);
}


bool IMAPFolder::exists() {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!isOpen() && !store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	return testExistAndGetType() != -1;
}


int IMAPFolder::testExistAndGetType() {

	// To test whether a folder exists, we simple list it using
	// the "LIST" command, and there should be one unique mailbox
	// with this name...
	//
	// Eg. Test whether '/foo/bar' exists
	//
	//     C: a005 list "" foo/bar
	//     S: * LIST (\NoSelect) "/" foo/bar
	//     S: a005 OK LIST completed
	//
	// ==> OK, exists
	//
	// Test whether '/foo/bar/zap' exists
	//
	//     C: a005 list "" foo/bar/zap
	//     S: a005 OK LIST completed
	//
	// ==> NO, does not exist

	IMAPCommand::LIST(
		"",
		IMAPUtils::pathToString(
			m_connection->hierarchySeparator(),
			getFullPath()
		)
	)->send(m_connection);


	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("LIST", resp->getErrorLog(), "bad response");
	}

	// Check whether the result mailbox list contains this folder
	auto& respDataList = resp->continue_req_or_response_data;

	folderAttributes attribs;
	attribs.setType(-1);

	for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

		if (!(*it)->response_data) {
			throw exceptions::command_error("LIST", resp->getErrorLog(), "invalid response");
		}

		auto *mailboxData = (*it)->response_data->mailbox_data.get();

		// We are only interested in responses of type "LIST"
		if (mailboxData &&
		    mailboxData->type == IMAPParser::mailbox_data::LIST) {

			// Get the folder type/flags at the same time
			IMAPUtils::mailboxFlagsToFolderAttributes(
				m_connection,
				*mailboxData->mailbox_list->mailbox_flag_list,
				attribs
			);
		}
	}

	m_attribs = make_shared <folderAttributes>(attribs);

	return m_attribs->getType();
}


bool IMAPFolder::isOpen() const {

	return m_open;
}


shared_ptr <message> IMAPFolder::getMessage(const size_t num) {

	if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	if (num < 1 || num > m_status->getMessageCount()) {
		throw exceptions::message_not_found();
	}

	return make_shared <IMAPMessage>(dynamicCast <IMAPFolder>(shared_from_this()), num);
}


std::vector <shared_ptr <message> > IMAPFolder::getMessages(const messageSet& msgs) {

	if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	if (msgs.isEmpty()) {
		return std::vector <shared_ptr <message> >();
	}

	std::vector <shared_ptr <message> > messages;

	// Sequence number message set:
	//     C: . FETCH uuuu1,uuuu2,uuuu3 UID
	//     S: * nnnn1 FETCH (UID uuuu1)
	//     S: * nnnn2 FETCH (UID uuuu2)
	//     S: * nnnn3 FETCH (UID uuuu3)
	//     S: . OK FETCH completed

	// UID message set:
	//     C: . UID FETCH uuuu1,uuuu2,uuuu3 UID
	//     S: * nnnn1 FETCH (UID uuuu1)
	//     S: * nnnn2 FETCH (UID uuuu2)
	//     S: * nnnn3 FETCH (UID uuuu3)
	//     S: . OK UID FETCH completed

	std::vector <string> params;
	params.push_back("UID");

	IMAPCommand::FETCH(msgs, params)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("UID FETCH ... UID", resp->getErrorLog(), "bad response");
	}

	// Process the response
	auto &respDataList = resp->continue_req_or_response_data;

	for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

		if (!(*it)->response_data) {
			throw exceptions::command_error("UID FETCH ... UID", resp->getErrorLog(), "invalid response");
		}

		auto *messageData = (*it)->response_data->message_data.get();

		// We are only interested in responses of type "FETCH"
		if (!messageData || messageData->type != IMAPParser::message_data::FETCH) {
			continue;
		}

		// Find UID in message attributes
		const size_t msgNum = messageData->number;
		message::uid msgUID;

		for (auto &att : messageData->msg_att->items) {

			if (att->type == IMAPParser::msg_att_item::UID) {
				msgUID = att->uniqueid->value;
				break;
			}
		}

		if (!msgUID.empty()) {
			shared_ptr <IMAPFolder> thisFolder = dynamicCast <IMAPFolder>(shared_from_this());
			messages.push_back(make_shared <IMAPMessage>(thisFolder, msgNum, msgUID));
		}
	}

	return messages;
}


size_t IMAPFolder::getMessageCount() {

	if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	return m_status->getMessageCount();
}


vmime_uint32 IMAPFolder::getUIDValidity() const {

	if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	return m_status->getUIDValidity();
}


vmime_uint64 IMAPFolder::getHighestModSequence() const {

	if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	return m_status->getHighestModSeq();
}


shared_ptr <folder> IMAPFolder::getFolder(const folder::path::component& name) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	return make_shared <IMAPFolder>(m_path / name, store, shared_ptr <folderAttributes>());
}


std::vector <shared_ptr <folder> > IMAPFolder::getFolders(const bool recursive) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!isOpen() && !store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	// Eg. List folders in '/foo/bar'
	//
	//     C: a005 list "foo/bar" *
	//     S: * LIST (\NoSelect) "/" foo/bar
	//     S: * LIST (\NoInferiors) "/" foo/bar/zap
	//     S: a005 OK LIST completed

	shared_ptr <IMAPCommand> cmd;

	const string pathString = IMAPUtils::pathToString(
		m_connection->hierarchySeparator(), getFullPath()
	);

	if (recursive) {

		cmd = IMAPCommand::LIST(pathString, "*");

	} else {

		cmd = IMAPCommand::LIST(
			pathString.empty()
				? ""
				: (pathString + m_connection->hierarchySeparator()),
			"%"
		);
	}

	cmd->send(m_connection);


	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("LIST", resp->getErrorLog(), "bad response");
	}

	auto &respDataList = resp->continue_req_or_response_data;

	std::vector <shared_ptr <folder> > v;

	for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

		if (!(*it)->response_data) {
			throw exceptions::command_error("LIST", resp->getErrorLog(), "invalid response");
		}

		auto *mailboxData = (*it)->response_data->mailbox_data.get();

		if (!mailboxData || mailboxData->type != IMAPParser::mailbox_data::LIST) {
			continue;
		}

		// Get folder path
		auto &mailbox = mailboxData->mailbox_list->mailbox;

		folder::path path = IMAPUtils::stringToPath(
			mailboxData->mailbox_list->quoted_char, mailbox->name
		);

		if (recursive || m_path.isDirectParentOf(path)) {

			// Append folder to list
			shared_ptr <folderAttributes> attribs = make_shared <folderAttributes>();

			IMAPUtils::mailboxFlagsToFolderAttributes(
				m_connection,
				*mailboxData->mailbox_list->mailbox_flag_list,
				*attribs
			);

			v.push_back(make_shared <IMAPFolder>(path, store, attribs));
		}
	}

	return v;
}


void IMAPFolder::fetchMessages(
	std::vector <shared_ptr <message> >& msg,
	const fetchAttributes& options,
	utility::progressListener* progress
) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	if (msg.empty()) {
		return;
	}

	// Build message numbers list
	std::vector <size_t> list;
	list.reserve(msg.size());

	std::map <size_t, shared_ptr <IMAPMessage> > numberToMsg;

	for (std::vector <shared_ptr <message> >::iterator it = msg.begin() ; it != msg.end() ; ++it) {

		list.push_back((*it)->getNumber());
		numberToMsg[(*it)->getNumber()] = dynamicCast <IMAPMessage>(*it);
	}

	// Send the request
	IMAPUtils::buildFetchCommand(
		m_connection, messageSet::byNumber(list), options
	)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
		resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("FETCH", resp->getErrorLog(), "bad response");
	}

	auto &respDataList = resp->continue_req_or_response_data;

	const size_t total = msg.size();
	size_t current = 0;

	if (progress) {
		progress->start(total);
	}

	try {

		for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

			if (!(*it)->response_data) {
				throw exceptions::command_error("FETCH", resp->getErrorLog(), "invalid response");
			}

			auto *messageData = (*it)->response_data->message_data.get();

			// We are only interested in responses of type "FETCH"
			if (!messageData || messageData->type != IMAPParser::message_data::FETCH) {
				continue;
			}

			// Process fetch response for this message
			const size_t num = messageData->number;

			std::map <size_t, shared_ptr <IMAPMessage> >::iterator msg = numberToMsg.find(num);

			if (msg != numberToMsg.end()) {

				(*msg).second->processFetchResponse(options, *messageData);

				if (progress) {
					progress->progress(++current, total);
				}
			}
		}

	} catch (...) {

		if (progress) {
			progress->stop(total);
		}

		throw;
	}

	if (progress) {
		progress->stop(total);
	}

	processStatusUpdate(resp.get());
}


void IMAPFolder::fetchMessage(const shared_ptr <message>& msg, const fetchAttributes& options) {

	std::vector <shared_ptr <message> > msgs;
	msgs.push_back(msg);

	fetchMessages(msgs, options, /* progress */ NULL);
}


std::vector <shared_ptr <message> > IMAPFolder::getAndFetchMessages(
	const messageSet& msgs,
	const fetchAttributes& attribs
) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	if (msgs.isEmpty()) {
		return std::vector <shared_ptr <message> >();
	}

	// Ensure we also get the UID for each message
	fetchAttributes attribsWithUID(attribs);
	attribsWithUID.add(fetchAttributes::UID);

	// Send the request
	IMAPUtils::buildFetchCommand(m_connection, msgs, attribsWithUID)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
		resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("FETCH", resp->getErrorLog(), "bad response");
	}

	auto &respDataList = resp->continue_req_or_response_data;

	std::vector <shared_ptr <message> > messages;

	for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

		if (!(*it)->response_data) {
			throw exceptions::command_error("FETCH", resp->getErrorLog(), "invalid response");
		}

		auto *messageData = (*it)->response_data->message_data.get();

		// We are only interested in responses of type "FETCH"
		if (!messageData || messageData->type != IMAPParser::message_data::FETCH) {
			continue;
		}

		// Get message number
		const size_t msgNum = messageData->number;

		// Get message UID
		message::uid msgUID;

		for (auto &att : messageData->msg_att->items) {

			if (att->type == IMAPParser::msg_att_item::UID) {
				msgUID = att->uniqueid->value;
				break;
			}
		}

		// Create a new message reference
		shared_ptr <IMAPFolder> thisFolder = dynamicCast <IMAPFolder>(shared_from_this());
		shared_ptr <IMAPMessage> msg = make_shared <IMAPMessage>(thisFolder, msgNum, msgUID);

		messages.push_back(msg);

		// Process fetch response for this message
		msg->processFetchResponse(attribsWithUID, *messageData);
	}

	processStatusUpdate(resp.get());

	return messages;
}


int IMAPFolder::getFetchCapabilities() const {

	return fetchAttributes::ENVELOPE | fetchAttributes::CONTENT_INFO |
	       fetchAttributes::STRUCTURE | fetchAttributes::FLAGS |
	       fetchAttributes::SIZE | fetchAttributes::FULL_HEADER |
	       fetchAttributes::UID | fetchAttributes::IMPORTANCE;
}


shared_ptr <folder> IMAPFolder::getParent() {

	if (m_path.isEmpty()) {

		return null;

	} else {

		return make_shared <IMAPFolder>(
			m_path.getParent(), m_store.lock(), shared_ptr <folderAttributes>()
		);
	}
}


shared_ptr <const store> IMAPFolder::getStore() const {

	return m_store.lock();
}


shared_ptr <store> IMAPFolder::getStore() {

	return m_store.lock();
}


void IMAPFolder::registerMessage(IMAPMessage* msg) {

	m_messages.push_back(msg);
}


void IMAPFolder::unregisterMessage(IMAPMessage* msg) {

	std::vector <IMAPMessage*>::iterator it =
		std::find(m_messages.begin(), m_messages.end(), msg);

	if (it != m_messages.end()) {
		m_messages.erase(it);
	}
}


void IMAPFolder::onStoreDisconnected() {

	m_store.reset();
}


void IMAPFolder::deleteMessages(const messageSet& msgs) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (msgs.isEmpty()) {
		throw exceptions::invalid_argument();
	}

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	} else if (m_mode == MODE_READ_ONLY) {
		throw exceptions::illegal_state("Folder is read-only");
	}

	// Send the request
	IMAPCommand::STORE(
		msgs, message::FLAG_MODE_ADD,
		IMAPUtils::messageFlagList(message::FLAG_DELETED)
	)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
		resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("STORE", resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp.get());
}


void IMAPFolder::setMessageFlags(
	const messageSet& msgs,
	const int flags,
	const int mode
) {

	const std::vector <string> flagList = IMAPUtils::messageFlagList(flags);

	if (!flagList.empty()) {

		// Send the request
		IMAPCommand::STORE(msgs, mode, flagList)->send(m_connection);

		// Get the response
		scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

		if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

			throw exceptions::command_error("STORE", resp->getErrorLog(), "bad response");
		}

		processStatusUpdate(resp.get());
	}
}


messageSet IMAPFolder::addMessage(
	const shared_ptr <vmime::message>& msg,
	const int flags,
	vmime::datetime* date,
	utility::progressListener* progress
) {

	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	msg->generate(ossAdapter);

	const string& str = oss.str();
	utility::inputStreamStringAdapter strAdapter(str);

	return addMessage(strAdapter, str.length(), flags, date, progress);
}


messageSet IMAPFolder::addMessage(
	utility::inputStream& is,
	const size_t size,
	const int flags,
	vmime::datetime* date,
	utility::progressListener* progress
) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	} else if (m_mode == MODE_READ_ONLY) {
		throw exceptions::illegal_state("Folder is read-only");
	}

	// Send the request
	IMAPCommand::APPEND(
		IMAPUtils::pathToString(m_connection->hierarchySeparator(), getFullPath()),
		IMAPUtils::messageFlagList(flags), date, size
	)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	bool ok = false;
	auto &respList = resp->continue_req_or_response_data;

	for (auto it = respList.begin() ; !ok && (it != respList.end()) ; ++it) {

		if ((*it)->continue_req) {
			ok = true;
		}
	}

	if (!ok) {
		throw exceptions::command_error("APPEND", resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp.get());

	// Send message data
	const size_t total = size;
	size_t current = 0;

	if (progress) {
		progress->start(total);
	}

	const size_t blockSize = std::min(
		is.getBlockSize(),
		static_cast <size_t>(m_connection->getSocket()->getBlockSize())
	);

	std::vector <byte_t> vbuffer(blockSize);
	byte_t* buffer = &vbuffer.front();

	while (!is.eof()) {

		// Read some data from the input stream
		const size_t read = is.read(buffer, blockSize);
		current += read;

		// Put read data into socket output stream
		m_connection->sendRaw(buffer, read);

		// Notify progress
		if (progress) {
			progress->progress(current, total);
		}
	}

	m_connection->sendRaw(utility::stringUtils::bytesFromString("\r\n"), 2);

	if (m_connection->getTracer()) {
		m_connection->getTracer()->traceSendBytes(current);
	}

	if (progress) {
		progress->stop(total);
	}

	// Get the response
	scoped_ptr <IMAPParser::response> finalResp(m_connection->readResponse());

	if (finalResp->isBad() || finalResp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("APPEND", resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(finalResp.get());

	auto *respTextCode =
		finalResp->response_done->response_tagged->resp_cond_state->resp_text->resp_text_code.get();

	if (respTextCode && respTextCode->type == IMAPParser::resp_text_code::APPENDUID) {
		return IMAPUtils::buildMessageSet(*respTextCode->uid_set);
	}

	return messageSet::empty();
}


void IMAPFolder::expunge() {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	} else if (m_mode == MODE_READ_ONLY) {
		throw exceptions::illegal_state("Folder is read-only");
	}

	// Send the request
	IMAPCommand::EXPUNGE()->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
		resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("EXPUNGE", resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp.get());
}


void IMAPFolder::rename(const folder::path& newPath) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (m_path.isEmpty() || newPath.isEmpty()) {
		throw exceptions::illegal_operation("Cannot rename root folder");
	} else if (m_path.getSize() == 1 && m_name.getBuffer() == "INBOX") {
		throw exceptions::illegal_operation("Cannot rename 'INBOX' folder");
	} else if (!store->isValidFolderName(newPath.getLastComponent())) {
		throw exceptions::invalid_folder_name();
	}

	// Send the request
	IMAPCommand::RENAME(
		IMAPUtils::pathToString(m_connection->hierarchySeparator(), getFullPath()),
		IMAPUtils::pathToString(m_connection->hierarchySeparator(), newPath)
	)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
		resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("RENAME", resp->getErrorLog(), "bad response");
	}

	// Notify folder renamed
	folder::path oldPath(m_path);

	m_path = newPath;
	m_name = newPath.getLastComponent();

	shared_ptr <events::folderEvent> event =
		make_shared <events::folderEvent>(
			dynamicCast <folder>(shared_from_this()),
			events::folderEvent::TYPE_RENAMED,
			oldPath,
			newPath
		);

	notifyFolder(event);

	// Notify sub-folders
	for (std::list <IMAPFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it) {

		if ((*it) != this && oldPath.isParentOf((*it)->getFullPath())) {

			folder::path oldPath((*it)->m_path);

			(*it)->m_path.renameParent(oldPath, newPath);

			shared_ptr <events::folderEvent> event =
				make_shared <events::folderEvent>(
					dynamicCast <folder>((*it)->shared_from_this()),
					events::folderEvent::TYPE_RENAMED,
					oldPath, (*it)->m_path
				);

			(*it)->notifyFolder(event);
		}
	}

	processStatusUpdate(resp.get());
}


messageSet IMAPFolder::copyMessages(const folder::path& dest, const messageSet& set) {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	} else if (!isOpen()) {
		throw exceptions::illegal_state("Folder not open");
	}

	// Send the request
	IMAPCommand::COPY(
		set,
		IMAPUtils::pathToString(m_connection->hierarchySeparator(), dest)
	)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
		resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("COPY", resp->getErrorLog(), "bad response");
	}

	processStatusUpdate(resp.get());

	auto *respTextCode =
		resp->response_done->response_tagged->resp_cond_state->resp_text->resp_text_code.get();

	if (respTextCode && respTextCode->type == IMAPParser::resp_text_code::COPYUID) {
		return IMAPUtils::buildMessageSet(*respTextCode->uid_set2);
	}

	return messageSet::empty();
}


void IMAPFolder::status(size_t& count, size_t& unseen) {

	count = 0;
	unseen = 0;

	shared_ptr <folderStatus> status = getStatus();

	count = status->getMessageCount();
	unseen = status->getUnseenCount();
}


shared_ptr <folderStatus> IMAPFolder::getStatus() {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	// Build the attributes list
	std::vector <string> attribs;

	attribs.push_back("MESSAGES");
	attribs.push_back("UNSEEN");
	attribs.push_back("UIDNEXT");
	attribs.push_back("UIDVALIDITY");

	if (m_connection->hasCapability("CONDSTORE")) {
		attribs.push_back("HIGHESTMODSEQ");
	}

	// Send the request
	IMAPCommand::STATUS(
		IMAPUtils::pathToString(m_connection->hierarchySeparator(), getFullPath()),
		attribs
	)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("STATUS", resp->getErrorLog(), "bad response");
	}

	auto &respDataList = resp->continue_req_or_response_data;

	for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

		if ((*it)->response_data) {

			auto &responseData = (*it)->response_data;

			if (responseData->mailbox_data &&
				responseData->mailbox_data->type == IMAPParser::mailbox_data::STATUS) {

				shared_ptr <IMAPFolderStatus> status = make_shared <IMAPFolderStatus>();
				status->updateFromResponse(*responseData->mailbox_data);

				m_status->updateFromResponse(*responseData->mailbox_data);

				return status;
			}
		}
	}

	throw exceptions::command_error("STATUS", resp->getErrorLog(), "invalid response");
}


void IMAPFolder::noop() {

	shared_ptr <IMAPStore> store = m_store.lock();

	if (!store) {
		throw exceptions::illegal_state("Store disconnected");
	}

	IMAPCommand::NOOP()->send(m_connection);

	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done->response_tagged->
			resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("NOOP", resp->getErrorLog());
	}

	processStatusUpdate(resp.get());
}


std::vector <size_t> IMAPFolder::getMessageNumbersStartingOnUID(const message::uid& uid) {

	// Send the request
	std::ostringstream uidSearchKey;
	uidSearchKey.imbue(std::locale::classic());
	uidSearchKey << "UID " << uid << ":*";

	std::vector <string> searchKeys;
	searchKeys.push_back(uidSearchKey.str());

	IMAPCommand::SEARCH(searchKeys, /* charset */ NULL)->send(m_connection);

	// Get the response
	scoped_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() ||
	    resp->response_done->response_tagged->resp_cond_state->status != IMAPParser::resp_cond_state::OK) {

		throw exceptions::command_error("SEARCH", resp->getErrorLog(), "bad response");
	}

	auto& respDataList = resp->continue_req_or_response_data;

	std::vector <size_t> seqNumbers;

	for (auto it = respDataList.begin() ; it != respDataList.end() ; ++it) {

		if (!(*it)->response_data) {
			throw exceptions::command_error("SEARCH", resp->getErrorLog(), "invalid response");
		}

		auto *mailboxData = (*it)->response_data->mailbox_data.get();

		// We are only interested in responses of type "SEARCH"
		if (!mailboxData ||
		    mailboxData->type != IMAPParser::mailbox_data::SEARCH) {

			continue;
		}

		for (auto &nzn : mailboxData->search_nz_number_list) {
			seqNumbers.push_back(nzn->value);
		}
	}

	processStatusUpdate(resp.get());

	return seqNumbers;
}


void IMAPFolder::processStatusUpdate(const IMAPParser::response* resp) {

	std::vector <shared_ptr <events::event> > events;

	shared_ptr <IMAPFolderStatus> oldStatus = vmime::clone(m_status);
	int expungedMessageCount = 0;

	// Process tagged response
	if (resp->response_done &&
	    resp->response_done->response_tagged &&
	    resp->response_done->response_tagged->resp_cond_state->resp_text->resp_text_code) {

		m_status->updateFromResponse(
			*resp->response_done->response_tagged->resp_cond_state->resp_text->resp_text_code
		);
	}

	// Process untagged responses
	for (auto it = resp->continue_req_or_response_data.begin() ;
	     it != resp->continue_req_or_response_data.end() ; ++it) {

		if ((*it)->response_data &&
		    (*it)->response_data->resp_cond_state &&
		    (*it)->response_data->resp_cond_state->resp_text->resp_text_code) {

			m_status->updateFromResponse(
				*(*it)->response_data->resp_cond_state->resp_text->resp_text_code
			);

		} else if ((*it)->response_data &&
		           (*it)->response_data->mailbox_data) {

			m_status->updateFromResponse(*(*it)->response_data->mailbox_data);

			// Update folder attributes, if available
			if ((*it)->response_data->mailbox_data->type == IMAPParser::mailbox_data::LIST) {

				folderAttributes attribs;
				IMAPUtils::mailboxFlagsToFolderAttributes(
					m_connection,
					*(*it)->response_data->mailbox_data->mailbox_list->mailbox_flag_list,
					attribs
				);

				m_attribs = make_shared <folderAttributes>(attribs);
			}

		} else if ((*it)->response_data && (*it)->response_data->message_data) {

			auto* msgData = (*it)->response_data->message_data.get();
			const size_t msgNumber = msgData->number;

			if (msgData->type == IMAPParser::message_data::FETCH) {

				// Message changed
				for (std::vector <IMAPMessage*>::iterator mit =
				     m_messages.begin() ; mit != m_messages.end() ; ++mit) {

					if ((*mit)->getNumber() == msgNumber) {
						(*mit)->processFetchResponse(/* options */ 0, *msgData);
					}
				}

				events.push_back(
					make_shared <events::messageChangedEvent>(
						dynamicCast <folder>(shared_from_this()),
						events::messageChangedEvent::TYPE_FLAGS,
						std::vector <size_t>(1, msgNumber)
					)
				);

			} else if (msgData->type == IMAPParser::message_data::EXPUNGE) {

				// A message has been expunged, renumber messages
				for (std::vector <IMAPMessage*>::iterator jt =
				     m_messages.begin() ; jt != m_messages.end() ; ++jt) {

					if ((*jt)->getNumber() == msgNumber) {
						(*jt)->setExpunged();
					} else if ((*jt)->getNumber() > msgNumber) {
						(*jt)->renumber((*jt)->getNumber() - 1);
					}
				}

				events.push_back(
					make_shared <events::messageCountEvent>(
						dynamicCast <folder>(shared_from_this()),
						events::messageCountEvent::TYPE_REMOVED,
						std::vector <size_t>(1, msgNumber)
					)
				);

				expungedMessageCount++;
			}
		}
	}

	// New messages arrived
	if (m_status->getMessageCount() > oldStatus->getMessageCount() - expungedMessageCount) {

		std::vector <size_t> newMessageNumbers;

		for (size_t msgNumber = oldStatus->getMessageCount() - expungedMessageCount ;
		     msgNumber <= m_status->getMessageCount() ; ++msgNumber) {

			newMessageNumbers.push_back(msgNumber);
		}

		events.push_back(
			make_shared <events::messageCountEvent>(
				dynamicCast <folder>(shared_from_this()),
				events::messageCountEvent::TYPE_ADDED,
				newMessageNumbers
			)
		);
	}

	// Dispatch notifications
	for (std::vector <shared_ptr <events::event> >::iterator evit =
	     events.begin() ; evit != events.end() ; ++evit) {

		notifyEvent(*evit);
	}
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

