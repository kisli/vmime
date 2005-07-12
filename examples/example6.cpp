//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include <iostream>
#include <sstream>
#include <vector>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"


// Global session object
static vmime::ref <vmime::messaging::session> g_session
	= vmime::create <vmime::messaging::session>();


// Authentification handler
class interactiveAuthenticator : public vmime::messaging::authenticator
{
	const vmime::messaging::authenticationInfos requestAuthInfos() const
	{
		vmime::string username, password;

		std::cout << std::endl;
		std::cout << "Please authenticate yourself:" << std::endl;

		std::cout << "   Username: ";
		std::cout.flush();

		std::getline(std::cin, username);

		std::cout << "   Password: ";
		std::cout.flush();

		std::getline(std::cin, password);

		return (vmime::messaging::authenticationInfos(username, password));
	}
};


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

		os << "    path = " << vmime::platformDependant::getHandler()->
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
static void printStructure(const vmime::messaging::structure& s, const int level = 0)
{
	for (int i = 1 ; i <= s.getCount() ; ++i)
	{
		const vmime::messaging::part& part = s[i];

		for (int j = 0 ; j < level * 2 ; ++j)
			std::cout << " ";

		std::cout << part.getNumber() << ". "
		          << part.getType().generate()
				<< " [" << part.getSize() << " byte(s)]"
				<< std::endl;

		printStructure(part.getStructure(), level + 1);
	}
}


static const vmime::string getFolderPathString(vmime::ref <vmime::messaging::folder> f)
{
	const vmime::string n = f->getName().getBuffer();

	if (n.empty()) // root folder
	{
		return "/";
	}
	else
	{
		vmime::ref <vmime::messaging::folder> p = f->getParent();
		return getFolderPathString(p) + n + "/";
	}
}


/** Print folders and sub-folders on the standard output.
  *
  * @param folder current folder
  */
static void printFolders(vmime::ref <vmime::messaging::folder> folder, const int level = 0)
{
	for (int j = 0 ; j < level * 2 ; ++j)
		std::cout << " ";

	std::cout << getFolderPathString(folder) << std::endl;

	std::vector <vmime::ref <vmime::messaging::folder> > subFolders = folder->getFolders(false);

	for (unsigned int i = 0 ; i < subFolders.size() ; ++i)
		printFolders(subFolders[i], level + 1);
}


/** Print a menu on the standard output.
  *
  * @param choices menu choices
  */
static const unsigned int printMenu(const std::vector <std::string>& choices)
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
		std::cout << "(eg. smtp://myserver.com, sendmail://localhost)" << std::endl;
		std::cout << "> ";
		std::cout.flush();

		vmime::string urlString;
		std::getline(std::cin, urlString);

		vmime::utility::url url(urlString);

		vmime::ref <vmime::messaging::transport> tr =
			g_session->getTransport(url, vmime::create <interactiveAuthenticator>());

		// You can also set some properties (see example7 to know the properties
		// available for each service). For example, for SMTP:
//		tr->setProperty("options.need-authentication", true);

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
				to.appendMailbox(vmime::create <vmime::mailbox>(toString));
		}

		std::cout << "Enter message data (end with '.' on a single line):" << std::endl;

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
		std::cout << "(eg. pop3://user:pass@myserver.com, imap://myserver.com:123)" << std::endl;
		std::cout << "> ";
		std::cout.flush();

		vmime::string urlString;
		std::getline(std::cin, urlString);

		vmime::utility::url url(urlString);

		// If no authenticator is given in argument to getStore(), a default one
		// is used. Its behaviour is to get the user credentials from the
		// session properties "auth.username" and "auth.password".
		vmime::ref <vmime::messaging::store> st;

		if (url.getUsername().empty() || url.getPassword().empty())
			st = g_session->getStore(url, vmime::create <interactiveAuthenticator>());
		else
			st = g_session->getStore(url);

		// Connect to the mail store
		st->connect();

		// Open the default folder in this store
		vmime::ref <vmime::messaging::folder> f = st->getDefaultFolder();
//		vmime::ref <vmime::messaging::folder> f = st->getFolder(vmime::utility::path("a"));

		f->open(vmime::messaging::folder::MODE_READ_WRITE);

		int count = f->getMessageCount();

		std::cout << std::endl;
		std::cout << count << " message(s) in your inbox" << std::endl;

		for (bool cont = true ; cont ; )
		{
			typedef std::map <int, vmime::ref <vmime::messaging::message> > MessageList;
			MessageList msgList;

			try
			{
				std::vector <std::string> choices;

				choices.push_back("Show message flags");
				choices.push_back("Show message structure");
				choices.push_back("Show message header");
				choices.push_back("Show message envelope");
				choices.push_back("Extract whole message");
				choices.push_back("List folders");
				choices.push_back("Return to main menu");

				const int choice = printMenu(choices);

				// Request message number
				vmime::ref <vmime::messaging::message> msg;

				if (choice != 6 && choice != 7)
				{
					std::cout << "Enter message number: ";
					std::cout.flush();

					std::string line;
					std::getline(std::cin, line);

					std::istringstream iss(line);

					int num = 0;
					iss >> num;

					if (num < 1 || num > count)
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

					f->fetchMessage(msg, vmime::messaging::folder::FETCH_FLAGS);

					if (msg->getFlags() & vmime::messaging::message::FLAG_SEEN)
						std::cout << "FLAG_SEEN" << std::endl;
					if (msg->getFlags() & vmime::messaging::message::FLAG_RECENT)
						std::cout << "FLAG_RECENT" << std::endl;
					if (msg->getFlags() & vmime::messaging::message::FLAG_REPLIED)
						std::cout << "FLAG_REPLIED" << std::endl;
					if (msg->getFlags() & vmime::messaging::message::FLAG_DELETED)
						std::cout << "FLAG_DELETED" << std::endl;
					if (msg->getFlags() & vmime::messaging::message::FLAG_MARKED)
						std::cout << "FLAG_MARKED" << std::endl;
					if (msg->getFlags() & vmime::messaging::message::FLAG_PASSED)
						std::cout << "FLAG_PASSED" << std::endl;

					break;

				// Show message structure
				case 2:

					f->fetchMessage(msg, vmime::messaging::folder::FETCH_STRUCTURE);
					printStructure(msg->getStructure());
					break;

				// Show message header
				case 3:

					f->fetchMessage(msg, vmime::messaging::folder::FETCH_FULL_HEADER);
					std::cout << msg->getHeader().generate() << std::endl;
					break;

				// Show message envelope
				case 4:

					f->fetchMessage(msg, vmime::messaging::folder::FETCH_ENVELOPE);

#define ENV_HELPER(x) \
	try { std::cout << msg->getHeader().x()->generate() << std::endl; } \
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
				// List folders
				case 6:
				{
					vmime::ref <vmime::messaging::folder>
						root = st->getRootFolder();

					printFolders(root);
					break;
				}
				// Main menu
				case 7:

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
			vmime::ref <vmime::messaging::folder> f = st->getFolder(vmime::messaging::folder::path("c"));
			f->rename(vmime::messaging::folder::path("c2"));

			vmime::ref <vmime::messaging::folder> g = st->getFolder(vmime::messaging::folder::path("c2"));
			g->rename(vmime::messaging::folder::path("c"));
		}

		// Message copy: copy all messages from 'f' to 'g'
		{
			vmime::ref <vmime::messaging::folder> g = st->getFolder(vmime::messaging::folder::path("TEMP"));

			if (!g->exists())
				g->create(vmime::messaging::folder::TYPE_CONTAINS_MESSAGES);

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
		}
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
static const bool menu()
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
	// VMime initialization
	vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

	for (bool quit = false ; !quit ; )
	{
		// Loop on main menu
		quit = menu();
	}

	return 0;
}

