//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "../src/vmime"
#include "common.inc"


//
// Authentification handler
//

class my_auth : public vmime::messaging::authenticator
{
	const vmime::messaging::authenticationInfos requestAuthInfos() const
	{
		vmime::string username, password;

		std::cout << "Username: "; std::cout.flush();
		std::cin >> username;

		std::cout << "Password: "; std::cout.flush();
		std::cin >> password;

		return (vmime::messaging::authenticationInfos(username, password));
	}
};





void printStructure(const vmime::messaging::structure& s, int level = 0)
{
	for (int i = 1 ; i <= s.count() ; ++i)
	{
		const vmime::messaging::part& part = s[i];

		for (int j = 0 ; j < level * 2 ; ++j)
			std::cout << " ";

		std::cout << part.number() << ". "
		          << part.type().generate()
				<< " [" << part.size() << " byte(s)]"
				<< std::endl;

		printStructure(part.structure(), level + 1);
	}
}



int main()
{
	// VMime initialization
	vmime::platformDependant::setHandler <my_handler>();

	//
	// Test the new enumeration system for encoders
	//

#if 0
	vmime::encoderFactory* ef = vmime::encoderFactory::getInstance();

	std::cout << "Available encoders:" << std::endl;

	for (vmime::encoderFactory::iterator it = ef->begin() ;
	     it != ef->end() ; ++it)
	{
		std::cout << "  * " << (*it).name() << std::endl;

		vmime::encoder* e = (*it).create();

		std::vector <vmime::string> props = e->availableProperties();

		for (std::vector <vmime::string>::const_iterator it2 = props.begin() ; it2 != props.end() ; ++it2)
			std::cout << "      - " << *it2 << std::endl;

		delete (e);
	}
#endif

	// ======================================================================================

	//
	// Test the new enumeration system for messaging services
	//

#if 1
	vmime::messaging::serviceFactory* sf = vmime::messaging::serviceFactory::getInstance();

	std::cout << "Available messaging services:" << std::endl;

	for (vmime::messaging::serviceFactory::const_iterator it = sf->begin() ;
	     it != sf->end() ; ++it)
	{
		std::cout << "  * " << (*it).name() << " (" << (*it).infos().defaultPort() << ")" << std::endl;

		std::vector <vmime::string> props = (*it).infos().availableProperties();

		for (std::vector <vmime::string>::const_iterator it2 = props.begin() ; it2 != props.end() ; ++it2)
			std::cout << "      - " << (*it).infos().propertyPrefix() + *it2 << std::endl;
	}
#endif

	vmime::messaging::session sess;
	sess.properties()["store.protocol"] = "imap";
	sess.properties()["transport.protocol"] = "smtp";

	my_auth auth;

	try
	{
		//
		// Test the sending of a message
		//

#if 0
		// Transport protocol configuration
		vmime::messaging::transport* tr = sess.getTransport();

		//sess.properties()[tr->infos().propertyPrefix() + "auth.username"] = "username";
		//sess.properties()[tr->infos().propertyPrefix() + "auth.password"] = "password";

		sess.properties()[tr->infos().propertyPrefix() + "server.address"] = "smtp.mydomain.com";

		//sess.properties()[tr->infos().propertyPrefix() + "options.need-authentification"] = true;

		// Connection
		tr->connect();

		// Expeditor
		vmime::mailbox from("me@somewhere.com");

		// Recipients list
		vmime::mailboxList to;
		to.append(vmime::mailbox("you@somewhere.com"));
		to.append(vmime::mailbox("somebody.else@anywhere.com"));

		std::istringstream iss("[MESSAGE DATA: HEADER + BODY]");
		tr->send(from, to, iss);

		// Note: you could also write this:
		//     vmime::message msg;
		//     ...
		//     tr->send(&msg);

		tr->disconnect();
#endif

		//
		// Test the access to a mail store
		//

#if 1
		// If no authenticator is given in argument to getStore(), a default one
		// is used. Its behaviour is to get the user credentials from the
		// session properties "auth.username" and "auth.password".
		vmime::messaging::store* st = sess.getStore(&auth);

		// Store protocol configuration
		//sess.properties()[st->infos().propertyPrefix() + "auth.username"] = "username";
		//sess.properties()[st->infos().propertyPrefix() + "auth.password"] = "password";

		sess.properties()[st->infos().propertyPrefix() + "server.address"] = "imap.mydomain.com";
		//sess.properties()[st->infos().propertyPrefix() + "server.port"] = 110;
		//sess.properties()[st->infos().propertyPrefix() + "server.socket-factory"] = "default";

		//sess.properties()[st->infos().propertyPrefix() + "options.apop"] = false;
		//sess.properties()[st->infos().propertyPrefix() + "options.apop.fallback"] = true;

		// Connection
		st->connect();

		// Open the default folder in this store
		vmime::messaging::folder* f = st->getDefaultFolder();

		f->open(vmime::messaging::folder::MODE_READ_WRITE);

		std::cout << f->getMessageCount() << " message(s) in your inbox" << std::endl;

		// Get a pointer to the first message
		vmime::messaging::message* m = f->getMessage(1);

		// To fetch the header
		f->fetchMessage(m, vmime::messaging::folder::FETCH_ENVELOPE |
			vmime::messaging::folder::FETCH_CONTENT_INFO);

		// To retrieve the whole message
		std::ostringstream oss;
		vmime::outputStreamAdapter out(oss);

		m->extract(out);

		// To fetch the header
		f->fetchMessage(m, vmime::messaging::folder::FETCH_ENVELOPE |
			vmime::messaging::folder::FETCH_CONTENT_INFO |
			vmime::messaging::folder::FETCH_STRUCTURE |
			vmime::messaging::folder::FETCH_SIZE |
			//vmime::messaging::folder::FETCH_FULL_HEADER |
			vmime::messaging::folder::FETCH_SIZE |
			vmime::messaging::folder::FETCH_FLAGS |
			vmime::messaging::folder::FETCH_UID);

		// Print structure
		std::cout << "STRUCTURE:" << std::endl;
		std::cout << "==========" << std::endl;

		printStructure(m->structure());

		std::cout << std::endl;

		std::cout << "Size = " << m->size() << " byte(s)" << std::endl;
		std::cout << "UID = " << m->uniqueId() << std::endl;
		std::cout << std::endl;

		std::cout << "ENVELOPE:" << std::endl;
		std::cout << "=========" << std::endl;
		try { std::cout << m->header().fields.From().generate() << std::endl; } catch (...) { }
		try { std::cout << m->header().fields.To().generate() << std::endl; } catch (...) { }
		try { std::cout << m->header().fields.Date().generate() << std::endl; } catch (...) { }
		try { std::cout << m->header().fields.Subject().generate() << std::endl; } catch (...) { }

		std::cout << std::endl;

		std::cout << "FULL HEADER:" << std::endl;
		std::cout << "============" << std::endl;
		std::cout << m->header().generate() << std::endl;

		std::cout << std::endl;
		std::cout << "=========================================================" << std::endl;

		vmime::outputStreamAdapter out2(std::cout);
		m->extractPart(m->structure()[1][2][1], out2, NULL); //, 0, 10);

		std::cout << "=========================================================" << std::endl;

		std::cout << std::endl;
		std::cout << "=========================================================" << std::endl;

		m->fetchPartHeader(m->structure()[1][2][1]);

		std::cout << m->structure()[1][2][1].header().generate() << std::endl;

		std::cout << "=========================================================" << std::endl;

		// Flags manipulation
		std::cout << "Flags = " << m->flags() << std::endl;
		m->setFlags(vmime::messaging::message::FLAG_REPLIED, vmime::messaging::message::FLAG_MODE_ADD);
		std::cout << "Flags = " << m->flags() << std::endl;
		m->setFlags(vmime::messaging::message::FLAG_REPLIED, vmime::messaging::message::FLAG_MODE_REMOVE);
		std::cout << "Flags = " << m->flags() << std::endl;

		f->setMessageFlags(m->number(), m->number(), vmime::messaging::message::FLAG_REPLIED, vmime::messaging::message::FLAG_MODE_ADD);
		std::cout << "Flags = " << m->flags() << std::endl;
		f->setMessageFlags(m->number(), m->number(), vmime::messaging::message::FLAG_REPLIED, vmime::messaging::message::FLAG_MODE_REMOVE);
		std::cout << "Flags = " << m->flags() << std::endl;


		std::cout << "=========================================================" << std::endl;

		// Append message
/*
		std::istringstream iss(
			"From: me@localhost\r\n"
			"To: you@localhost\r\n"
			"Subject: Message Text\r\n"
			"\r\n"
			"This is a test message...\r\n"
			"Bye bye!\r\n"
		);

		f->addMessage(iss, iss.str().size());
*/


/*
		// Folder renaming
		{
			vmime::messaging::folder* f = st->getFolder(vmime::messaging::folder::path("c"));
			f->rename(vmime::messaging::folder::path("c2"));
			delete (f);

			vmime::messaging::folder* g = st->getFolder(vmime::messaging::folder::path("c2"));
			g->rename(vmime::messaging::folder::path("c"));
			delete (g);
		}
*/

/*
		// Message copy
		{
			vmime::messaging::folder* g = st->getFolder(vmime::messaging::folder::path("TEMP"));

			if (!g->exists())
				g->create(vmime::messaging::folder::TYPE_CONTAINS_MESSAGES);

			f->copyMessages(g->fullPath());

			delete (g);
		}
*/

		delete (m);

		f->close(true);
		delete (f);

		st->disconnect();
		delete (st);
#endif
	}
	catch (vmime::exceptions::authentication_error& e)
	{
		std::cout << "vmime::authentication_error: " << e.what() << std::endl
		          << "Response is: '" << e.response() << "'." << std::endl;
		throw;
	}
	catch (vmime::exceptions::command_error& e)
	{
		std::cout << "vmime::command_error: " << e.what() << std::endl
		          << "Response is: '" << e.response() << "'." << std::endl;
		throw;
	}
	catch (vmime::exception& e)
	{
		std::cout << "vmime::exception: " << e.what() << std::endl;
		throw;
	}
	catch (std::exception& e)
	{
		std::cout << "std::exception: " << e.what() << std::endl;
		throw;
	}
}
