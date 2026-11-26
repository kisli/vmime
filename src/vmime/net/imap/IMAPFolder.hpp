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

#ifndef VMIME_NET_IMAP_IMAPFOLDER_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPFOLDER_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include <vector>
#include <map>
#include <atomic>

#include "vmime/types.hpp"

#include "vmime/net/folder.hpp"

#include "vmime/net/imap/IMAPParser.hpp"
#include "vmime/net/imap/IMAPSearchAttributes.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPStore;
class IMAPMessage;
class IMAPConnection;
class IMAPFolderStatus;


/** IMAP folder implementation.
  */
class VMIME_EXPORT IMAPFolder : public folder {

private:

	friend class IMAPStore;
	friend class IMAPMessage;

	IMAPFolder(const IMAPFolder&);

public:

	IMAPFolder(
		const folder::path& path,
		const shared_ptr <IMAPStore>& store,
		const shared_ptr <folderAttributes>& attribs
	);

	~IMAPFolder();

	int getMode() const;

	folderAttributes getAttributes();

	folder::path::component getName() const;
	folder::path getFullPath() const;

	void open(const int mode, bool failIfModeIsNotAvailable = false);
	void close(const bool expunge);
	void create(const folderAttributes& attribs);

	bool exists();

	void destroy();

	bool isOpen() const;

	shared_ptr <message> getMessage(const size_t num);
	std::vector <shared_ptr <message> > getMessages(const messageSet& msgs);

	std::vector <size_t> getMessageNumbersStartingOnUID(const message::uid& uid);

	/** Return the sequence numbers of messages matching the searchAttributes.
	  *
	  * @param sa the searchAttributes containing search tokens to match messages to
	  * @param charset optional charset name, the string tokens are assumed to be encoded
	  *   in the provided encoding OR need to be in US-ASCII if no charset is provided
	  * @throw exceptions::net_exception if an error occurs
	  *
	  * Quirks: some servers will not accept any encoding other than US-ASCII,
	  * other servers will accept UTF-8 but will fail utf-8
	  */
    std::vector <size_t> getMessageNumbersMatchingSearchAttributes(
        const IMAPSearchAttributes& sa,
        const vmime::charset* charset = nullptr
    );

	/** Return the UIDs of messages matching the searchAttributes.
	  *
	  * @param sa the searchAttributes containing search tokens to match messages to
	  * @param charset optional charset name, the string tokens are assumed to be encoded
	  *   in the provided encoding OR need to be in US-ASCII if no charset is provided
	  * @throw exceptions::net_exception if an error occurs
	  *
	  * Quirks: some servers will not accept any encoding other than US-ASCII,
	  * other servers will accept UTF-8 but will fail utf-8
	  */
    std::vector <message::uid> getMessageUIDsMatchingSearchAttributes(
        const IMAPSearchAttributes& sa,
        const vmime::charset* charset = nullptr
    );

    size_t getMessageCount();

	shared_ptr <folder> getFolder(const folder::path::component& name);
	std::vector <shared_ptr <folder> > getFolders(const bool recursive = false);

	void rename(const folder::path& newPath);

	void deleteMessages(const messageSet& msgs);

	void setMessageFlags(
		const messageSet& msgs,
		const int flags,
		const int mode = message::FLAG_MODE_SET
	);

	messageSet addMessage(
		const shared_ptr <vmime::message>& msg,
		const int flags = -1,
		vmime::datetime* date = NULL,
		utility::progressListener* progress = NULL
	);

	messageSet addMessage(
		utility::inputStream& is,
		const size_t size,
		const int flags = -1,
		vmime::datetime* date = NULL,
		utility::progressListener* progress = NULL
	);

	messageSet copyMessages(const folder::path& dest, const messageSet& msgs);

	void status(size_t& count, size_t& unseen);
	shared_ptr <folderStatus> getStatus();

	void noop();

	/** Possible results of idle(). */
	enum IdleResult {
		IDLE_CHANGES,       /**< The server reported changes in the folder
		                         (the listeners have been notified). */
		IDLE_TIMEOUT,       /**< The delay elapsed without any change. */
		IDLE_INTERRUPTED    /**< idle() was interrupted by a call to
		                         interruptIdle(). */
	};

	/** Wait for changes in this folder, using the IMAP IDLE command
	  * (RFC 2177). This blocks until the server reports a change (new
	  * or expunged messages, flags changes...), the specified delay
	  * elapses, or interruptIdle() is called.
	  *
	  * When the server reports a change, the IDLE command is terminated
	  * and the listeners registered on this folder are notified before
	  * this function returns. They are called from the thread which
	  * runs idle(), and can use the folder to fetch new messages.
	  * Typically, idle() is called in a loop.
	  *
	  * The time-out handler is not used while waiting for changes. It is
	  * used again when terminating the IDLE command.
	  *
	  * Note: interruptIdle() is the only function of this object which
	  * may be called from another thread while idle() is running.
	  *
	  * @param timeout maximum time to wait for changes, in seconds (must
	  * be positive); RFC 2177 asks clients to re-issue the IDLE command
	  * at least every 29 minutes
	  * @return the reason why the function returned
	  * @throw exceptions::invalid_argument if the timeout is not positive
	  * @throw exceptions::illegal_state if the folder is not open
	  * @throw exceptions::operation_not_supported if the server does not
	  * support IDLE
	  * @throw exceptions::command_error if the server rejects the command
	  * or closes the connection
	  * @throw exceptions::net_exception if another error occurs
	  */
	IdleResult idle(const int timeout = 29 * 60);

	/** Interrupt idle(), which will return IDLE_INTERRUPTED. If idle() is
	  * not running, the next call to idle() returns immediately.
	  *
	  * Unlike the other functions of this object, this one may be called
	  * from another thread.
	  */
	void interruptIdle();

	void expunge();

	shared_ptr <folder> getParent();

	shared_ptr <const store> getStore() const;
	shared_ptr <store> getStore();


	void fetchMessages(
		std::vector <shared_ptr <message> >& msg,
		const fetchAttributes& options,
		utility::progressListener* progress = NULL
	);

	void fetchMessage(const shared_ptr <message>& msg, const fetchAttributes& options);

	std::vector <shared_ptr <message> > getAndFetchMessages(
		const messageSet& msgs,
		const fetchAttributes& attribs
	);

	int getFetchCapabilities() const;

	/** Returns the UID validity of the folder for the current session.
	  * If the server is capable of persisting UIDs accross sessions,
	  * this value should never change for a folder. If the UID validity
	  * differs across sessions, then the UIDs obtained during a previous
	  * session may not correspond to the UIDs of the same messages in
	  * this session.
	  *
	  * @return UID validity of the folder
	  */
	vmime_uint32 getUIDValidity() const;

	/** Returns the highest modification sequence of this folder, ie the
	  * modification sequence of the last message that changed in this
	  * folder.
	  *
	  * @return modification sequence, or zero if not supported by
	  * the underlying protocol
	  */
	vmime_uint64 getHighestModSequence() const;

private:

	void registerMessage(IMAPMessage* msg);
	void unregisterMessage(IMAPMessage* msg);

	void onStoreDisconnected();

	void onClose();

	int testExistAndGetType();

	void setMessageFlagsImpl(const string& set, const int flags, const int mode);

	void copyMessagesImpl(const string& set, const folder::path& dest);


	/** Process status updates ("unsolicited responses") contained in the
	  * specified response. Example:
	  *
	  *    C: a006 NOOP
	  *    S: * 930 EXISTS       <-- this is a status update
	  *    S: a006 OK Success
	  *
	  * @param resp parsed IMAP response
	  */
	void processStatusUpdate(const IMAPParser::response* resp);


	weak_ptr <IMAPStore> m_store;
	shared_ptr <IMAPConnection> m_connection;

	folder::path m_path;
	folder::path::component m_name;

	int m_mode;
	bool m_open;

	shared_ptr <folderAttributes> m_attribs;

	shared_ptr <IMAPFolderStatus> m_status;

	std::vector <IMAPMessage*> m_messages;

	std::atomic <bool> m_idleInterrupted;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPFOLDER_HPP_INCLUDED
