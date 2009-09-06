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

#ifndef VMIME_OPTIONS_HPP_INCLUDED
#define VMIME_OPTIONS_HPP_INCLUDED


#include "vmime/base.hpp"


namespace vmime
{


/** A class to set global options for VMime.
  */

class options
{
protected:

	/** Message-related options.
	  */
	class messageOptions
	{
	protected:

		friend class options;

		messageOptions()
			: m_maxLineLength(lineLengthLimits::convenient)
		{
		}

		string::size_type m_maxLineLength;

	public:

		const string::size_type& maxLineLength() const { return (m_maxLineLength); }
		string::size_type& maxLineLength() { return (m_maxLineLength); }
	};

	/** Multipart-related options.
	  */
	class multipartOptions
	{
	private:

		friend class options;

		multipartOptions();

		string m_prologText;
		string m_epilogText;

	public:

		const string& getPrologText() const;
		void setPrologText(const string& prologText);

		const string& getEpilogText() const;
		void setEpilogText(const string& epilogText);
	};

public:

	static options* getInstance();

	multipartOptions multipart;
	messageOptions message;
};


} // vmime


#endif // VMIME_OPTIONS_HPP_INCLUDED
