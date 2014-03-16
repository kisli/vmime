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

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <locale>
#include <clocale>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "example6_tracer.hpp"
#include "example6_authenticator.hpp"
#include "example6_certificateVerifier.hpp"
#include "example6_timeoutHandler.hpp"


// Global session object
static vmime::shared_ptr <vmime::net::session> g_session
	= vmime::make_shared <vmime::net::session>();


/** Returns the messaging protocols supported by VMime.
  *
  * @param type service type (vmime::net::service::TYPE_STORE or
  * vmime::net::service::TYPE_TRANSPORT)
  */
static const std::string findAvailableProtocols(const vmime::net::service::Type type)
{
	vmime::shared_ptr <vmime::net::serviceFactory> sf =
		vmime::net::serviceFactory::getInstance();

	std::ostringstream res;
	int count = 0;

	for (int i = 0 ; i < sf->getServiceCount() ; ++i)
	{
		const vmime::net::serviceFactory::registeredService& serv = *sf->getServiceAt(i);

		if (serv.getType() == type)
		{
			if (count != 0)
				res << ", ";

			res << serv.getName();
			++count;
		}
	}

	return res.str();
}


// Exception helper
static std::ostream& operator<<(std::ostream& os, const vmime::exception& e)
{
	os << "* vmime::exceptions::" << e.name() << std::endl;
	os << "    what = " << e.what() << std::endl;

	// More information for special exceptions
	if (dynamic_cast <const vmime::exceptions::command_error*>(&e))
	{
		const vmime::exceptions::command_error& cee =
			dynamic_cast <const vmime::exceptions::command_error&>(e);

		os << "    command = " << cee.command() << std::endl;
		os << "    response = " << cee.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::invalid_response*>(&e))
	{
		const vmime::exceptions::invalid_response& ir =
			dynamic_cast <const vmime::exceptions::invalid_response&>(e);

		os << "    response = " << ir.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::connection_greeting_error*>(&e))
	{
		const vmime::exceptions::connection_greeting_error& cgee =
			dynamic_cast <const vmime::exceptions::connection_greeting_error&>(e);

		os << "    response = " << cgee.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::authentication_error*>(&e))
	{
		const vmime::exceptions::authentication_error& aee =
			dynamic_cast <const vmime::exceptions::authentication_error&>(e);

		os << "    response = " << aee.response() << std::endl;
	}

	if (dynamic_cast <const vmime::exceptions::filesystem_exception*>(&e))
	{
		const vmime::exceptions::filesystem_exception& fse =
			dynamic_cast <const vmime::exceptions::filesystem_exception&>(e);

		os << "    path = " << vmime::platform::getHandler()->
			getFileSystemFactory()->pathToString(fse.path()) << std::endl;
	}

	if (e.other() != NULL)
		os << *e.other();

	return os;
}


/** Print the MIME structure of a message on the standard output.
  *
  * @param s structure object
  * @param level current depth
  */
static void printStructure(vmime::shared_ptr <const vmime::net::messageStructure> s, const int level = 0)
{
	for (int i = 0 ; i < s->getPartCount() ; ++i)
	{
		vmime::shared_ptr <const vmime::net::messagePart> part = s->getPartAt(i);

		for (int j = 0 ; j < level * 2 ; ++j)
			std::cout << " ";

		std::cout << (part->getNumber() + 1) << ". "
				<< part->getType().generate()
				<< " [" << part->getSize() << " byte(s)]"
				<< std::endl;

		printStructure(part->getStructure(), level + 1);
	}
}


static const vmime::string getFolderPathString(vmime::shared_ptr <vmime::net::folder> f)
{
	const vmime::string n = f->getName().getBuffer();

	if (n.empty()) // root folder
	{
		return "/";
	}
	else
	{
		vmime::shared_ptr <vmime::net::folder> p = f->getParent();
		return getFolderPathString(p) + n + "/";
	}
}


/** Print folders and sub-folders on the standard output.
  *
  * @param folder current folder
  */
static void printFolders(vmime::shared_ptr <vmime::net::folder> folder, const int level = 0)
{
	for (int j = 0 ; j < level * 2 ; ++j)
		std::cout << " ";

	std::cout << getFolderPathString(folder) << std::endl;

	std::vector <vmime::shared_ptr <vmime::net::folder> > subFolders = folder->getFolders(false);

	for (unsigned int i = 0 ; i < subFolders.size() ; ++i)
		printFolders(subFolders[i], level + 1);
}


/** Print a menu on the standard output.
  *
  * @param choices menu choices
  */
static unsigned int printMenu(const std::vector <std::string>& choices)
{
	std::cout << std::endl;

	for (unsigned int i = 0 ; i < choices.size() ; ++i)
		std::cout << "   " << (i + 1) << ". " << choices[i] << std::endl;

	std::cout << std::endl;
	std::cout << "   Your choice? [1-" << choices.size() << "] ";
	std::cout.flush();

	std::string line;
	std::getline(std::cin, line);

	std::istringstream iss(line);

	unsigned int choice = 0;
	iss >> choice;

	std::cout << std::endl;

	if (choice < 1 || choice > choices.size())
		return 0;
	else
		return choice;
}


/** Send a message interactively.
  */
static void sendMessage()
{
	try
	{
		// Request user to enter an URL
		std::cout << "Enter an URL to connect to transport service." << std::endl;
		std::cout << "Available protocols: " << findAvailableProtocols(vmime::net::service::TYPE_TRANSPORT) << std::endl;
		std::cout << "(eg. smtp://myserver.com, sendmail://localhost)" << std::endl;
		std::cout << "> ";
		std::cout.flush();

		vmime::string urlString;
		std::getline(std::cin, urlString);

		vmime::utility::url url(urlString);

		vmime::shared_ptr <vmime::net::transport> tr;

		if (url.getUsername().empty() || url.getPassword().empty())
			tr = g_session->getTransport(url, vmime::make_shared <interactiveAuthenticator>());
		else
			tr = g_session->getTransport(url);

#if VMIME_HAVE_TLS_SUPPORT

		// Enable TLS support if available
		tr->setProperty("connection.tls", true);

		// Set the time out handler
		tr->setTimeoutHandlerFactory(vmime::make_shared <timeoutHandlerFactory>());

		// Set the object responsible for verifying certificates, in the
		// case a secured connection is used (TLS/SSL)
		tr->setCertificateVerifier
			(vmime::make_shared <interactiveCertificateVerifier>());

#endif // VMIME_HAVE_TLS_SUPPORT

		// You can also set some properties (see example7 to know the properties
		// available for each service). For example, for SMTP:
		if (!url.getUsername().empty() || !url.getPassword().empty())
			tr->setProperty("options.need-authentication", true);

		// Trace communication between client and server
		vmime::shared_ptr <std::ostringstream> traceStream = vmime::make_shared <std::ostringstream>();
		tr->setTracerFactory(vmime::make_shared <myTracerFactory>(traceStream));

		// Information about the mail
		std::cout << "Enter email of the expeditor (eg. me@somewhere.com): ";
		std::cout.flush();

		vmime::string fromString;
		std::getline(std::cin, fromString);

		vmime::mailbox from(fromString);
		vmime::mailboxList to;

		for (bool cont = true ; cont ; )
		{
			std::cout << "Enter email of the recipient (empty to stop): ";
			std::cout.flush();

			vmime::string toString;
			std::getline(std::cin, toString);

			cont = (toString.size() != 0);

			if (cont)
				to.appendMailbox(vmime::make_shared <vmime::mailbox>(toString));
		}

		std::cout << "Enter message data, including headers (end with '.' on a single line):" << std::endl;

		std::ostringstream data;

		for (bool cont = true ; cont ; )
		{
			std::string line;
			std::getline(std::cin, line);

			if (line == ".")
				cont = false;
			else
				data << line << "\r\n";
		}

		// Connect to server
		tr->connect();

		// Send the message
		vmime::string msgData = data.str();
		vmime::utility::inputStreamStringAdapter vis(msgData);

		tr->send(from, to, vis, msgData.length());

		// Note: you could also write this:
		//     vmime::message msg;
		//     ...
		//     tr->send(&msg);

		// Display connection log
		std::cout << std::endl;
		std::cout << "Connection Trace:" << std::endl;
		std::cout << "=================" << std::endl;
		std::cout << traceStream->str();

		tr->disconnect();
	}
	catch (vmime::exception& e)
	{
		std::cerr << std::endl;
		std::cerr << e << std::endl;
		throw;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl;
		std::cerr << "std::exception: " << e.what() << std::endl;
		throw;
	}
}


/** Connect to a message store interactively.
  */
static void connectStore()
{
	try
	{
		// Request user to enter an URL
		std::cout << "Enter an URL to connect to store service." << std::endl;
		std::cout << "Available protocols: " << findAvailableProtocols(vmime::net::service::TYPE_STORE) << std::endl;
		std::cout << "(eg. pop3://user:pass@myserver.com, imap://myserver.com:123)" << std::endl;
		std::cout << "> ";
		std::cout.flush();

		vmime::string urlString;
		std::getline(std::cin, urlString);

		vmime::utility::url url(urlString);

		// If no authenticator is given in argument to getStore(), a default one
		// is used. Its behaviour is to get the user credentials from the
		// session properties "auth.username" and "auth.password".
		vmime::shared_ptr <vmime::net::store> st;

		if (url.getUsername().empty() || url.getPassword().empty())
			st = g_session->getStore(url, vmime::make_shared <interactiveAuthenticator>());
		else
			st = g_session->getStore(url);

#if VMIME_HAVE_TLS_SUPPORT

		// Enable TLS support if available
		st->setProperty("connection.tls", true);

		// Set the time out handler
		st->setTimeoutHandlerFactory(vmime::make_shared <timeoutHandlerFactory>());

		// Set the object responsible for verifying certificates, in the
		// case a secured connection is used (TLS/SSL)
		st->setCertificateVerifier
			(vmime::make_shared <interactiveCertificateVerifier>());

#endif // VMIME_HAVE_TLS_SUPPORT

		// Trace communication between client and server
		vmime::shared_ptr <std::ostringstream> traceStream = vmime::make_shared <std::ostringstream>();
		st->setTracerFactory(vmime::make_shared <myTracerFactory>(traceStream));

		// Connect to the mail store
		st->connect();

		// Display some information about the connection
		vmime::shared_ptr <vmime::net::connectionInfos> ci = st->getConnectionInfos();

		std::cout << std::endl;
		std::cout << "Connected to '" << ci->getHost() << "' (port " << ci->getPort() << ")" << std::endl;
		std::cout << "Connection is " << (st->isSecuredConnection() ? "" : "NOT ") << "secured." << std::endl;

		// Open the default folder in this store
		vmime::shared_ptr <vmime::net::folder> f = st->getDefaultFolder();
//		vmime::shared_ptr <vmime::net::folder> f = st->getFolder(vmime::utility::path("a"));

		f->open(vmime::net::folder::MODE_READ_WRITE);

		int count = f->getMessageCount();

		std::cout << std::endl;
		std::cout << count << " message(s) in your inbox" << std::endl;

		for (bool cont = true ; cont ; )
		{
			typedef std::map <int, vmime::shared_ptr <vmime::net::message> > MessageList;
			MessageList msgList;

			try
			{
				std::vector <std::string> choices;

				choices.push_back("Show message flags");
				choices.push_back("Show message structure");
				choices.push_back("Show message header");
				choices.push_back("Show message envelope");
				choices.push_back("Extract whole message");
				choices.push_back("Extract attachments");
				choices.push_back("Status");
				choices.push_back("List folders");
				choices.push_back("Change folder");
				choices.push_back("Add message (to the current folder)");
				choices.push_back("Copy message (into the current folder)");
				choices.push_back("Display trace output");
				choices.push_back("Return to main menu");

				const int choice = printMenu(choices);

				// Request message number
				vmime::shared_ptr <vmime::net::message> msg;

				if (choice == 1 || choice == 2 || choice == 3 || choice == 4 ||
				    choice == 5 || choice == 6 || choice == 11)
				{
					std::cout << "Enter message number: ";
					std::cout.flush();

					std::string line;
					std::getline(std::cin, line);

					std::istringstream iss(line);

					int num = 0;
					iss >> num;

					if (num < 1 || num > f->getMessageCount())
					{
						std::cerr << "Invalid message number." << std::endl;
						continue;
					}

					MessageList::iterator it = msgList.find(num);

					if (it != msgList.end())
					{
						msg = (*it).second;
					}
					else
					{
						msg = f->getMessage(num);
						msgList.insert(MessageList::value_type(num, msg));
					}

					std::cout << std::endl;
				}

				switch (choice)
				{
				// Show message flags
				case 1:

					f->fetchMessage(msg, vmime::net::fetchAttributes::FLAGS);

					if (msg->getFlags() & vmime::net::message::FLAG_SEEN)
						std::cout << "FLAG_SEEN" << std::endl;
					if (msg->getFlags() & vmime::net::message::FLAG_RECENT)
						std::cout << "FLAG_RECENT" << std::endl;
					if (msg->getFlags() & vmime::net::message::FLAG_REPLIED)
						std::cout << "FLAG_REPLIED" << std::endl;
					if (msg->getFlags() & vmime::net::message::FLAG_DELETED)
						std::cout << "FLAG_DELETED" << std::endl;
					if (msg->getFlags() & vmime::net::message::FLAG_MARKED)
						std::cout << "FLAG_MARKED" << std::endl;
					if (msg->getFlags() & vmime::net::message::FLAG_PASSED)
						std::cout << "FLAG_PASSED" << std::endl;

					break;

				// Show message structure
				case 2:

					f->fetchMessage(msg, vmime::net::fetchAttributes::STRUCTURE);
					printStructure(msg->getStructure());
					break;

				// Show message header
				case 3:

					f->fetchMessage(msg, vmime::net::fetchAttributes::FULL_HEADER);
					std::cout << msg->getHeader()->generate() << std::endl;
					break;

				// Show message envelope
				case 4:

					f->fetchMessage(msg, vmime::net::fetchAttributes::ENVELOPE);

#define ENV_HELPER(x) \
	try { std::cout << msg->getHeader()->x()->generate() << std::endl; } \
	catch (vmime::exception) { /* In case the header field does not exist. */ }

					ENV_HELPER(From)
					ENV_HELPER(To)
					ENV_HELPER(Date)
					ENV_HELPER(Subject)

#undef ENV_HELPER

					break;

				// Extract whole message
				case 5:
				{
					vmime::utility::outputStreamAdapter out(std::cout);
					msg->extract(out);

					break;
				}
				// Extract attachments
				case 6:
				{
					vmime::shared_ptr <vmime::message> parsedMsg = msg->getParsedMessage();

					std::vector <vmime::shared_ptr <const vmime::attachment> > attchs = 
						vmime::attachmentHelper::findAttachmentsInMessage(parsedMsg);

					if (attchs.size() > 0)
					{
						std::cout << attchs.size() << " attachments found." << std::endl;

						for (std::vector <vmime::shared_ptr <const vmime::attachment> >::iterator
						     it = attchs.begin() ; it != attchs.end() ; ++it)
						{
							vmime::shared_ptr <const vmime::attachment> att = *it;

							// Get attachment size
							vmime::size_t size = 0;

							if (att->getData()->isEncoded())
								size = att->getData()->getEncoding().getEncoder()->getDecodedSize(att->getData()->getLength());
							else
								size = att->getData()->getLength();

							std::cout << "Found attachment '" << att->getName().getBuffer() << "'"
							          << ", size is " << size << " bytes:" << std::endl;

							// Get attachment data
							std::cout << std::endl;
							std::cout << "========== BEGIN CONTENT ==========" << std::endl;

							vmime::utility::outputStreamAdapter osa(std::cout);
							att->getData()->extract(osa);

							std::cout << std::endl;
							std::cout << "========== END CONTENT ==========" << std::endl;

							// Or write it to a file
							/*
							vmime::shared_ptr <vmime::utility::fileSystemFactory> fsf
								= vmime::platform::getHandler()->getFileSystemFactory();

							vmime::shared_ptr <vmime::utility::file> file
								= fsf->create(vmime::utility::path::fromString
									("/path/to/attachment-file", "/", vmime::charsets::UTF_8));
							// -or- ("C:\\Temp\\attachment-file", "\\", vmime::charsets::UTF_8));

							file->createFile();

							vmime::shared_ptr <vmime::utility::outputStream> output =
								file->getFileWriter()->getOutputStream();

							att->getData()->extract(*output.get());
							*/
						}
					}
					else
					{
						std::cout << "No attachments found." << std::endl;
					}

					break;
				}
				// Status
				case 7:
				{
					int count, unseen;
					f->status(count, unseen);

					std::cout << "Status: count=" << count << ", unseen=" << unseen << std::endl;
					break;
				}
				// List folders
				case 8:
				{
					vmime::shared_ptr <vmime::net::folder>
						root = st->getRootFolder();

					printFolders(root);
					break;
				}
				// Change folder
				case 9:
				{
					std::cout << "Enter folder path (eg. /root/subfolder):" << std::endl;
					std::cout.flush();

					std::string path;
					std::getline(std::cin, path);

					vmime::shared_ptr <vmime::net::folder> newFolder = st->getRootFolder();

					for (std::string::size_type s = 0, p = 0 ; ; s = p + 1)
					{
						p = path.find_first_of('/', s);

						const std::string x = (p == std::string::npos)
							? std::string(path.begin() + s, path.end())
							: std::string(path.begin() + s, path.begin() + p);

						if (!x.empty())
							newFolder = newFolder->getFolder(x);

						if (p == std::string::npos)
							break;
					}

					newFolder->open(vmime::net::folder::MODE_READ_WRITE);

					count = newFolder->getMessageCount();

					std::cout << std::endl;
					std::cout << count << " message(s) in this folder" << std::endl;

					f->close(true);  // 'true' to expunge deleted messages
					f = newFolder;

					break;
				}
				// Add message
				case 10:
				{
					vmime::messageBuilder mb;

					mb.setExpeditor(vmime::mailbox("me@somewhere.com"));

					vmime::addressList to;
					to.appendAddress(vmime::make_shared <vmime::mailbox>("you@elsewhere.com"));
					mb.setRecipients(to);

					mb.setSubject(vmime::text("Test message from VMime example6"));
					mb.getTextPart()->setText(vmime::make_shared <vmime::stringContentHandler>(
						"Body of test message from VMime example6."));

					vmime::shared_ptr <vmime::message> msg = mb.construct();

					vmime::net::messageSet set = f->addMessage(msg);

					if (set.isEmpty())
					{
						std::cout << "Message has successfully been added, "
						          << "but its UID/number is not known." << std::endl;
					}
					else
					{
						const vmime::net::messageRange& range = set.getRangeAt(0);

						if (set.isUIDSet())
						{
							const vmime::net::message::uid uid =
								dynamic_cast <const vmime::net::UIDMessageRange&>(range).getFirst();

							std::cout << "Message has successfully been added, "
							          << "its UID is '" << uid << "'." << std::endl;
						}
						else
						{
							const int number =
								dynamic_cast <const vmime::net::numberMessageRange&>(range).getFirst();

							std::cout << "Message has successfully been added, "
							          << "its number is '" << number << "'." << std::endl;
						}
					}

					break;
				}
				// Copy message
				case 11:
				{
					vmime::net::messageSet set = f->copyMessages(f->getFullPath(),
						vmime::net::messageSet::byNumber(msg->getNumber()));

					if (set.isEmpty())
					{
						std::cout << "Message has successfully been copied, "
						          << "but its UID/number is not known." << std::endl;
					}
					else
					{
						const vmime::net::messageRange& range = set.getRangeAt(0);

						if (set.isUIDSet())
						{
							const vmime::net::message::uid uid =
								dynamic_cast <const vmime::net::UIDMessageRange&>(range).getFirst();

							std::cout << "Message has successfully been copied, "
							          << "its UID is '" << uid << "'." << std::endl;
						}
						else
						{
							const int number =
								dynamic_cast <const vmime::net::numberMessageRange&>(range).getFirst();

							std::cout << "Message has successfully been copied, "
							          << "its number is '" << number << "'." << std::endl;
						}
					}

					break;
				}
				// Display trace output
				case 12:

					std::cout << std::endl;
					std::cout << "Connection Trace:" << std::endl;
					std::cout << "=================" << std::endl;
					std::cout << traceStream->str();
					break;

				// Main menu
				case 13:

					f->close(true);  // 'true' to expunge deleted messages
					cont = false;
					break;
				}

/*
		// Append message
		std::istringstream iss(
			"From: me@localhost\r\n"
			"To: you@localhost\r\n"
			"Subject: Message Text\r\n"
			"\r\n"
			"This is a test message...\r\n"
			"Bye bye!\r\n"
		);

		f->addMessage(iss, iss.str().size());

		// Folder renaming
		{
			vmime::shared_ptr <vmime::net::folder> f = st->getFolder(vmime::net::folder::path("c"));
			f->rename(vmime::net::folder::path("c2"));

			vmime::shared_ptr <vmime::net::folder> g = st->getFolder(vmime::net::folder::path("c2"));
			g->rename(vmime::net::folder::path("c"));
		}

		// Message copy: copy all messages from 'f' to 'g'
		{
			vmime::shared_ptr <vmime::net::folder> g = st->getFolder(vmime::net::folder::path("TEMP"));

			if (!g->exists())
				g->create(vmime::net::folder::TYPE_CONTAINS_MESSAGES);

			f->copyMessages(g->getFullPath());
		}
*/
			}
			catch (vmime::exception& e)
			{
				std::cerr << std::endl;
				std::cerr << e << std::endl;
			}
			catch (std::exception& e)
			{
				std::cerr << std::endl;
				std::cerr << "std::exception: " << e.what() << std::endl;
			}
		} // for(cont)

		st->disconnect();
	}
	catch (vmime::exception& e)
	{
		std::cerr << std::endl;
		std::cerr << e << std::endl;
		throw;
	}
	catch (std::exception& e)
	{
		std::cerr << std::endl;
		std::cerr << "std::exception: " << e.what() << std::endl;
		throw;
	}
}


/* Show the main menu.
 *
 * @return true to quit the program, false to continue
 */
static bool menu()
{
	std::vector <std::string> items;

	items.push_back("Connect to a message store");
	items.push_back("Send a message");
	items.push_back("Quit");

	switch (printMenu(items))
	{
	// Connect to store
	case 1:

		connectStore();
		return false;

	// Send a message
	case 2:

		sendMessage();
		return false;

	// Quit
	case 3:

		return true;

	// Other choice
	default:

		return false;
	}
}


int main()
{
	// Set the global C and C++ locale to the user-configured locale.
	// The locale should use UTF-8 encoding for these tests to run successfully.
	try
	{
		std::locale::global(std::locale(""));
	}
	catch (std::exception &)
	{
		std::setlocale(LC_ALL, "");
	}

	for (bool quit = false ; !quit ; )
	{
		// Loop on main menu
		quit = menu();
	}

	return 0;
}

