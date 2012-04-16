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

#include "vmime/disposition.hpp"

#include "vmime/parserHelpers.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


disposition::disposition()
{
}


disposition::disposition(const string& actionMode, const string& sendingMode,
	const string& type, const string& modifier)
	: m_actionMode(actionMode), m_sendingMode(sendingMode), m_type(type)
{
	m_modifiers.push_back(modifier);
}


ref <component> disposition::clone() const
{
	ref <disposition> disp = vmime::create <disposition>();

	disp->m_actionMode = m_actionMode;
	disp->m_sendingMode = m_sendingMode;
	disp->m_type = m_type;
	disp->m_modifiers.resize(m_modifiers.size());

	std::copy(m_modifiers.begin(), m_modifiers.end(), disp->m_modifiers.begin());

	return (disp);
}


void disposition::copyFrom(const component& other)
{
	const disposition& disp = dynamic_cast <const disposition&>(other);

	m_actionMode = disp.m_actionMode;
	m_sendingMode = disp.m_sendingMode;
	m_type = disp.m_type;
	m_modifiers.resize(disp.m_modifiers.size());

	std::copy(disp.m_modifiers.begin(), disp.m_modifiers.end(), m_modifiers.begin());
}


disposition& disposition::operator=(const disposition& other)
{
	copyFrom(other);
	return (*this);
}


const std::vector <ref <component> > disposition::getChildComponents()
{
	return std::vector <ref <component> >();
}


void disposition::setActionMode(const string& mode)
{
	m_actionMode = mode;
}


const string& disposition::getActionMode() const
{
	return (m_actionMode);
}


void disposition::setSendingMode(const string& mode)
{
	m_sendingMode = mode;
}


const string& disposition::getSendingMode() const
{
	return (m_sendingMode);
}


void disposition::setType(const string& type)
{
	m_type = type;
}


const string& disposition::getType() const
{
	return (m_type);
}


void disposition::addModifier(const string& modifier)
{
	if (!hasModifier(modifier))
		m_modifiers.push_back(utility::stringUtils::toLower(modifier));
}


void disposition::removeModifier(const string& modifier)
{
	const string modifierLC = utility::stringUtils::toLower(modifier);

	for (std::vector <string>::iterator it = m_modifiers.begin() ;
	     it != m_modifiers.end() ; ++it)
	{
		if (*it == modifierLC)
		{
			m_modifiers.erase(it);
			break;
		}
	}
}


void disposition::removeAllModifiers()
{
	m_modifiers.clear();
}


bool disposition::hasModifier(const string& modifier) const
{
	const string modifierLC = utility::stringUtils::toLower(modifier);

	for (std::vector <string>::const_iterator it = m_modifiers.begin() ;
	     it != m_modifiers.end() ; ++it)
	{
		if (*it == modifierLC)
			return (true);
	}

	return (false);
}


const std::vector <string> disposition::getModifierList() const
{
	return (m_modifiers);
}


void disposition::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	// disposition-mode ";" disposition-type
	//      [ "/" disposition-modifier *( "," disposition-modifier ) ]
	//
	// disposition-mode = action-mode "/" sending-mode

	string::size_type pos = position;

	while (pos < end && parserHelpers::isSpace(buffer[pos]))
		++pos;

	// -- disposition-mode
	const string::size_type modeStart = pos;
	string::size_type modeEnd = pos;

	while (pos < end && buffer[pos] != ';')
	{
		++modeEnd;
		++pos;
	}

	while (modeEnd > modeStart && parserHelpers::isSpace(buffer[modeEnd - 1]))
		--modeEnd;

	const string mode = string(buffer.begin() + modeStart, buffer.begin() + modeEnd);
	const string::size_type slash = mode.find('/');

	if (slash != string::npos)
	{
		m_actionMode = string(mode.begin(), mode.begin() + slash);
		m_sendingMode = string(mode.begin() + slash + 1, mode.end());
	}
	else
	{
		m_actionMode = mode;
		m_sendingMode.clear();
	}

	if (pos < end)
	{
		// Skip ';'
		++pos;
	}

	while (pos < end && parserHelpers::isSpace(buffer[pos]))
		++pos;

	// -- disposition-type
	const string::size_type typeStart = pos;
	string::size_type typeEnd = pos;

	while (pos < end && buffer[pos] != '/')
	{
		++typeEnd;
		++pos;
	}

	while (typeEnd > typeStart && parserHelpers::isSpace(buffer[typeEnd - 1]))
		--typeEnd;

	m_type = string(buffer.begin() + typeStart, buffer.begin() + typeEnd);

	m_modifiers.clear();

	if (pos < end) // modifiers follow
	{
		// Skip '/'
		++pos;

		while (pos < end)
		{
			while (pos < end && parserHelpers::isSpace(buffer[pos]))
				++pos;

			const string::size_type modifierStart = pos;
			string::size_type modifierEnd = pos;

			while (pos < end && buffer[pos] != ',')
			{
				++modifierEnd;
				++pos;
			}

			while (modifierEnd > modifierStart && parserHelpers::isSpace(buffer[modifierEnd - 1]))
				--modifierEnd;

			if (modifierEnd > modifierStart)
			{
				m_modifiers.push_back(string(buffer.begin() + modifierStart,
				                             buffer.begin() + modifierEnd));
			}

			// Skip ','
			if (pos < end)
				++pos;
		}
	}

	if (newPosition)
		*newPosition = pos;
}


void disposition::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	const string actionMode = (m_actionMode.empty() ? "automatic-action" : m_actionMode);
	const string sendingMode = (m_sendingMode.empty() ? "MDN-sent-automatically" : m_sendingMode);

	os << actionMode << "/" << sendingMode << ";";
	pos += actionMode.length() + 1 + sendingMode.length() + 1;

	if (pos > maxLineLength)
	{
		os << NEW_LINE_SEQUENCE;
		pos = NEW_LINE_SEQUENCE_LENGTH;
	}

	const string type = (m_type.empty() ? "displayed" : m_type);

	os << type;
	pos += type.length();

	if (m_modifiers.size() >= 1)
	{
		os << "/" << m_modifiers[0];
		pos += 1 + m_modifiers[0].length();

		for (std::vector <string>::size_type i = 1 ; i < m_modifiers.size() ; ++i)
		{
			os << "," << m_modifiers[i];
			pos += 1 + m_modifiers[i].length();
		}
	}

	if (newLinePos)
		*newLinePos = pos;
}


}
