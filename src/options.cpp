//
// VMime library (http://vmime.sourceforge.net)
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

#include "vmime/options.hpp"


namespace vmime
{


options* options::getInstance()
{
	static options instance;
	return (&instance);
}


options::multipartOptions::multipartOptions()
	: m_prologText("This is a multi-part message in MIME format. Your mail reader " \
	               "does not understand MIME message format."),
	  m_epilogText("")
{
}


const string& options::multipartOptions::getPrologText() const
{
	return (m_prologText);
}


void options::multipartOptions::setPrologText(const string& prologText)
{
	m_prologText = prologText;
}


const string& options::multipartOptions::getEpilogText() const
{
	return (m_epilogText);
}


void options::multipartOptions::setEpilogText(const string& epilogText)
{
	m_epilogText = epilogText;
}


} // vmime
