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

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"


// Global session object
static vmime::ref <vmime::net::session> g_session
	= vmime::create <vmime::net::session>();


#if VMIME_HAVE_SASL_SUPPORT

// SASL authentication handler
class interactiveAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator
{
	const std::vector <vmime::ref <vmime::security::sasl::SASLMechanism> > getAcceptableMechanisms
		(const std::vector <vmime::ref <vmime::security::sasl::SASLMechanism> >& available,
		 vmime::ref <vmime::security::sasl::SASLMechanism> suggested) const
	{
		std::cout << std::endl << "Available SASL mechanisms:" << std::endl;

		for (unsigned int i = 0 ; i < available.size() ; ++i)
		{
			std::cout << "  " << available[i]->getName();

			if (suggested && available[i]->getName() == suggested->getName())
				std::cout << "(suggested)";
		}

		std::cout << std::endl << std::endl;

		return defaultSASLAuthenticator::getAcceptableMechanisms(available, suggested);
	}

	void setSASLMechanism(vmime::ref <vmime::security::sasl::SASLMechanism> mech)
	{
		std::cout << "Trying '" << mech->getName() << "' authentication mechanism" << std::endl;

		defaultSASLAuthenticator::setSASLMechanism(mech);
	}

	const vmime::string getUsername() const
	{
		if (m_username.empty())
			m_username = getUserInput("Username");

		return m_username;
	}

	const vmime::string getPassword() const
	{
		if (m_password.empty())
			m_password = getUserInput("Password");

		return m_password;
	}

	static const vmime::string getUserInput(const std::string& prompt)
	{
		std::cout << prompt << ": ";
		std::cout.flush();

		vmime::string res;
		std::getline(std::cin, res);

		return res;
	}

private:

	mutable vmime::string m_username;
	mutable vmime::string m_password;
};

#else // !VMIME_HAVE_SASL_SUPPORT

// Simple authentication handler
class interactiveAuthenticator : public vmime::security::defaultAuthenticator
{
	const vmime::string getUsername() const
	{
		if (m_username.empty())
			m_username = getUserInput("Username");

		return m_username;
	}

	const vmime::string getPassword() const
	{
		if (m_password.empty())
			m_password = getUserInput("Password");

		return m_password;
	}

	static const vmime::string getUserInput(const std::string& prompt)
	{
		std::cout << prompt << ": ";
		std::cout.flush();

		vmime::string res;
		std::getline(std::cin, res);

		return res;
	}

private:

	mutable vmime::string m_username;
	mutable vmime::string m_password;
};

#endif // VMIME_HAVE_SASL_SUPPORT


#if VMIME_HAVE_TLS_SUPPORT

// Certificate verifier (TLS/SSL)
class interactiveCertificateVerifier : public vmime::security::cert::defaultCertificateVerifier
{
public:

	void verify(vmime::ref <vmime::security::cert::certificateChain> chain)
	{
		try
		{
			setX509TrustedCerts(m_trustedCerts);

			defaultCertificateVerifier::verify(chain);
		}
		catch (vmime::exceptions::certificate_verification_exception&)
		{
			// Obtain subject's certificate
			vmime::ref <vmime::security::cert::certificate> cert = chain->getAt(0);

			std::cout << std::endl;
			std::cout << "Server sent a '" << cert->getType() << "'" << " certificate." << std::endl;
			std::cout << "Do you want to accept this certificate? (Y/n) ";
			std::cout.flush();

			std::string answer;
			std::getline(std::cin, answer);

			if (answer.length() != 0 &&
			    (answer[0] == 'Y' || answer[0] == 'y'))
			{
				// Accept it, and remember user's choice for later
				if (cert->getType() == "X.509")
				{
					m_trustedCerts.push_back(cert.dynamicCast
						<vmime::security::cert::X509Certificate>());
				}

				return;
			}

			throw vmime::exceptions::certificate_verification_exception
				("User did not accept the certificate.");
		}
	}

private:

	static std::vector <vmime::ref <vmime::security::cert::X509Certificate> > m_trustedCerts;
};


std::vector <vmime::ref <vmime::security::cert::X509Certificate> >
	interactiveCertificateVerifier::m_trustedCerts;

#endif // VMIME_HAVE_TLS_SUPPORT


/** Returns the messaging protocols supported by VMime.
  *
  * @param type service type (vmime::net::service::TYPE_STORE or
  * vmime::net::service::TYPE_TRANSPORT)
  */
static const std::string findAvailableProtocols(const vmime::net::service::Type type)
{
	vmime::net::serviceFactory* sf = vmime::net::serviceFactory::getInstance();

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
static void printStructure(vmime::ref <const vmime::net::structure> s, const int level = 0)
{
	for (int i = 0 ; i < s->getPartCount() ; ++i)
	{
		vmime::ref <const vmime::net::part> part = s->getPartAt(i);

		for (int j = 0 ; j < level * 2 ; ++j)
			std::cout << " ";

		std::cout << (part->getNumber() + 1) << ". "
				<< part->getType().generate()
				<< " [" << part->getSize() << " byte(s)]"
				<< std::endl;

		printStructure(part->getStructure(), level + 1);
	}
}


static const vmime::string getFolderPathString(vmime::ref <vmime::net::folder> f)
{
	const vmime::string n = f->getName().getBuffer();

	if (n.empty()) // root folder
	{
		return "/";
	}
	else
	{
		vmime::ref <vmime::net::folder> p = f->getParent();
		return getFolderPathString(p) + n + "/";
	}
}


/** Print folders and sub-folders on the standard output.
  *
  * @param folder current folder
  */
static void printFolders(vmime::ref <vmime::net::folder> folder, const int level = 0)
{
	for (int j = 0 ; j < level * 2 ; ++j)
		std::cout << " ";

	std::cout << getFolderPathString(folder) << std::endl;

	std::vector <vmime::ref <vmime::net::folder> > subFolders = folder->getFolders(false);

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

		vmime::ref <vmime::net::transport> tr =
			g_session->getTransport(url, vmime::create <interactiveAuthenticator>());

#if VMIME_HAVE_TLS_SUPPORT

		// Enable TLS support if available
		tr->setProperty("connection.tls", true);

		// Set the object responsible for verifying certificates, in the
		// case a secured connection is used (TLS/SSL)
		tr->setCertificateVerifier
			(vmime::create <interactiveCertificateVerifier>());

#endif // VMIME_HAVE_TLS_SUPPORT

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
		vmime::ref <vmime::net::store> st;

		if (url.getUsername().empty() || url.getPassword().empty())
			st = g_session->getStore(url, vmime::create <interactiveAuthenticator>());
		else
			st = g_session->getStore(url);

#if VMIME_HAVE_TLS_SUPPORT

		// Enable TLS support if available
		st->setProperty("connection.tls", true);

		// Set the object responsible for verifying certificates, in the
		// case a secured connection is used (TLS/SSL)
		st->setCertificateVerifier
			(vmime::create <interactiveCertificateVerifier>());

#endif // VMIME_HAVE_TLS_SUPPORT

		// Connect to the mail store
		st->connect();

		// Display some information about the connection
		vmime::ref <vmime::net::connectionInfos> ci = st->getConnectionInfos();

		std::cout << std::endl;
		std::cout << "Connected to '" << ci->getHost() << "' (port " << ci->getPort() << ")" << std::endl;
		std::cout << "Connection is " << (st->isSecuredConnection() ? "" : "NOT ") << "secured." << std::endl;

		// Open the default folder in this store
		vmime::ref <vmime::net::folder> f = st->getDefaultFolder();
//		vmime::ref <vmime::net::folder> f = st->getFolder(vmime::utility::path("a"));

		f->open(vmime::net::folder::MODE_READ_WRITE);

		int count = f->getMessageCount();

		std::cout << std::endl;
		std::cout << count << " message(s) in your inbox" << std::endl;

		for (bool cont = true ; cont ; )
		{
			typedef std::map <int, vmime::ref <vmime::net::message> > MessageList;
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
				choices.push_back("Change folder");
				choices.push_back("Return to main menu");

				const int choice = printMenu(choices);

				// Request message number
				vmime::ref <vmime::net::message> msg;

				if (choice != 6 && choice != 7 && choice != 8)
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

					f->fetchMessage(msg, vmime::net::folder::FETCH_FLAGS);

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

					f->fetchMessage(msg, vmime::net::folder::FETCH_STRUCTURE);
					printStructure(msg->getStructure());
					break;

				// Show message header
				case 3:

					f->fetchMessage(msg, vmime::net::folder::FETCH_FULL_HEADER);
					std::cout << msg->getHeader()->generate() << std::endl;
					break;

				// Show message envelope
				case 4:

					f->fetchMessage(msg, vmime::net::folder::FETCH_ENVELOPE);

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
				// List folders
				case 6:
				{
					vmime::ref <vmime::net::folder>
						root = st->getRootFolder();

					printFolders(root);
					break;
				}
				// Change folder
				case 7:
				{
					std::cout << "Enter folder path (eg. /root/subfolder):" << std::endl;
					std::cout.flush();

					std::string path;
					std::getline(std::cin, path);

					vmime::ref <vmime::net::folder> newFolder = st->getRootFolder();

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
				// Main menu
				case 8:

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
			vmime::ref <vmime::net::folder> f = st->getFolder(vmime::net::folder::path("c"));
			f->rename(vmime::net::folder::path("c2"));

			vmime::ref <vmime::net::folder> g = st->getFolder(vmime::net::folder::path("c2"));
			g->rename(vmime::net::folder::path("c"));
		}

		// Message copy: copy all messages from 'f' to 'g'
		{
			vmime::ref <vmime::net::folder> g = st->getFolder(vmime::net::folder::path("TEMP"));

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
	// VMime initialization
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();

	for (bool quit = false ; !quit ; )
	{
		// Loop on main menu
		quit = menu();
	}

	return 0;
}

