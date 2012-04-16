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

#include "vmime/relay.hpp"
#include "vmime/text.hpp"
#include "vmime/parserHelpers.hpp"

#include <sstream>


namespace vmime
{


relay::relay()
{
}


relay::relay(const relay& r)
	: headerFieldValue()
{
	copyFrom(r);
}


/*

 RFC #2822:

     received    =  "Received"    ":"            ; one per relay
                       ["from" domain]           ; sending host
                       ["by"   domain]           ; receiving host
                       ["via"  atom]             ; physical path
                      *("with" atom)             ; link/mail protocol
                       ["id"   msg-id]           ; receiver msg id
                       ["for"  addr-spec]        ; initial form
*/

void relay::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pend - 1;

	// Find the beginning of the date part
	while (p >= pstart && *p != ';')
		--p;

	if (p >= pstart)
	{
		// Parse the date/time part
		m_date.parse(buffer, position + (p - pstart) + 1, end);

		// Parse the components
		std::istringstream iss(string
			(buffer.begin() + position, buffer.begin() + position + (p - pstart)));

		string word;
		std::vector <string> previous;

		enum Parts
		{
			Part_None,
			Part_From,              // The "from" part
			Part_By,                // The "by" part
			Part_Via,               // The "via" part
			Part_With,              // One "with" part
			Part_Id,                // The "id" part
			Part_For,               // The "for" part
			Part_End
		};

		Parts part = Part_None;
		bool cont = true;
		bool inComment = false;

		while (cont)
		{
			Parts newPart = Part_None;

			if ((cont = ((iss >> word) != 0)))
			{
				// A little hack for handling comments
				if (inComment)
				{
					string::size_type par = word.find(')');

					if (par != string::npos)
					{
						previous.push_back(string(word.begin(), word.begin() + par + 1));
						word.erase(word.begin(), word.begin() + par + 1);
						inComment = false;
					}
				}

				bool keyword = false;

				if (!inComment)
				{
					if (utility::stringUtils::isStringEqualNoCase(word, "from", 4))
					{
						newPart = Part_From;
						keyword = true;
					}
					else if (utility::stringUtils::isStringEqualNoCase(word, "by", 2))
					{
						newPart = Part_By;
						keyword = true;
					}
					else if (utility::stringUtils::isStringEqualNoCase(word, "via", 2))
					{
						newPart = Part_Via;
						keyword = true;
					}
					else if (utility::stringUtils::isStringEqualNoCase(word, "with", 2))
					{
						newPart = Part_With;
						keyword = true;
					}
					else if (utility::stringUtils::isStringEqualNoCase(word, "id", 2))
					{
						newPart = Part_Id;
						keyword = true;
					}
					else if (utility::stringUtils::isStringEqualNoCase(word, "for", 2))
					{
						newPart = Part_For;
						keyword = true;
					}
				}

				if (!keyword)
				{
					if (word.find('(') != string::npos)
						inComment = true;

					previous.push_back(word);
				}
			}

			if (!cont || newPart != Part_None)
			{
				if (part != Part_None)
				{
					std::ostringstream value;

					for (std::vector <string>::const_iterator
						it = previous.begin() ; it != previous.end() ; ++it)
					{
						if (it != previous.begin()) value << " ";
						value << *it;
					}

					switch (part)
					{
					case Part_From: m_from = value.str(); break;
					case Part_By: m_by = value.str(); break;
					case Part_Via: m_via = value.str(); break;
					case Part_With: m_with.push_back(value.str()); break;
					case Part_Id: m_id = value.str(); break;
					case Part_For: m_for = value.str(); break;
					default: break; // Should never happen...
					}
				}

				previous.clear();
				part = newPart;
			}
		}
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void relay::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	std::ostringstream oss;
	int count = 0;

	if (m_from.length()) oss << (count++ > 0 ? " " : "") << "from " << m_from;
	if (m_by.length()) oss << (count++ > 0 ? " " : "") << "by " << m_by;
	if (m_via.length()) oss << (count++ > 0 ? " " : "") << "via " << m_via;

	for (std::vector <string>::const_iterator
	     it = m_with.begin() ; it != m_with.end() ; ++it)
	{
		oss << (count++ > 0 ? " " : "") << "with " << *it;
	}

	if (m_id.length()) oss << (count++ > 0 ? " " : "") << "id " << m_id;
	if (m_for.length()) oss << (count++ > 0 ? " " : "") << "for " << m_for;

	oss << "; " << m_date.generate();

	text(oss.str()).encodeAndFold(os, maxLineLength,
		curLinePos, newLinePos, text::FORCE_NO_ENCODING);
}


void relay::copyFrom(const component& other)
{
	const relay& r = dynamic_cast <const relay&>(other);

	m_from = r.m_from;
	m_via = r.m_via;
	m_by = r.m_by;
	m_id = r.m_id;
	m_for = r.m_for;

	m_with.resize(r.m_with.size());
	std::copy(r.m_with.begin(), r.m_with.end(), m_with.begin());

	m_date = r.m_date;
}


relay& relay::operator=(const relay& other)
{
	copyFrom(other);
	return (*this);
}


ref <component> relay::clone() const
{
	return vmime::create <relay>(*this);
}


const string& relay::getFrom() const
{
	return (m_from);
}


void relay::setFrom(const string& from)
{
	m_from = from;
}


const string& relay::getVia() const
{
	return (m_via);
}


void relay::setVia(const string& via)
{
	m_via = via;
}


const string& relay::getBy() const
{
	return (m_by);
}


void relay::setBy(const string& by)
{
	m_by = by;
}


const string& relay::getId() const
{
	return (m_id);
}


void relay::setId(const string& id)
{
	m_id = id;
}


const string& relay::getFor() const
{
	return (m_for);
}


void relay::setFor(const string& for_)
{
	m_for = for_;
}


const datetime& relay::getDate() const
{
	return (m_date);
}


void relay::setDate(const datetime& date)
{
	m_date = date;
}


const std::vector <string>& relay::getWithList() const
{
	return (m_with);
}


std::vector <string>& relay::getWithList()
{
	return (m_with);
}


const std::vector <ref <component> > relay::getChildComponents()
{
	// TODO: should fields inherit from 'component'? (using typeAdapter)
	return std::vector <ref <component> >();
}


} // vmime
